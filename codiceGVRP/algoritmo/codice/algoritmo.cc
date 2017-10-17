#include "algoritmo.h"

extern double COEF_DIST;
extern double COSTO_VUOTO;
extern double COEF_CARICO;

/** funzione che permette l'inserimento di un cliente in una delle route, validandolo
* Riceve in input gli interi che identificano cliente e veicolo, la struttura
* routes_t su cui scrivere e l'array dei clienti che contiene la domanda di ciascuno.
* La funzione valida l'id del cliente, controlla che non sia già servito e che sul
* veicolo passato ci sia capacita sufficiente per soddisfare la domanda, dopodiche
* si aggiorna la struttura routes inserendo il cliente nella riga corrispondente.
*/
int aggiungi_cliente(int cliente, int veicolo, routes_t &routes,const cliente_t clienti[]){
  if(cliente<=0){
    printf("indice cliente %d non valido!!",cliente);
    return 3;
  }
  if(routes.n_veicolo_cliente[cliente]!=-1){
    printf("errore, il cliente e' gia' servito da %d\n",routes.n_veicolo_cliente[cliente]);
    return 2;
  }
  if(routes.carico_route[veicolo]+clienti[cliente].richiesta>routes.capacita){
    printf("capacita' insufficiente\n");
    return 1;
  }

  routes.visita[veicolo][routes.n_clienti_veicolo[veicolo]]=cliente;
  routes.n_clienti_veicolo[veicolo]++;
  routes.carico_route[veicolo]+=clienti[cliente].richiesta;
  routes.n_veicolo_cliente[cliente]=veicolo;
  return 0;
}

double calcola_costo(double d,double q){
  return COEF_DIST*(COSTO_VUOTO+q*COEF_CARICO)*d;
}

/** funzione che calcola il costo totale di una route
* Riceve in ingresso la route (una riga della tabella di visita), la sua dimensione,
* l'array dei clienti e la struttura del grafo (che insieme determinano i costi degli archi).
* Se la route ha dimensione non nulla calcolo:
* -costo dall'ultimo nodo al deposito;
* -costo del percorso tra i nodi della route;
* -costo dal deposito al primo nodo.
* Ad ogni passo incremento q(carico del veicolo) della richiesta del nodo precedente,
* fino ad arrivare al deposito, da cui il veicolo percorre il primo arco con il carico totale della route.
*/
double costo_riga(int route[], int dim_route,const cliente_t clienti[],const graph_t grafo){
  if(0==dim_route)
    return 0;
  int i,conto=2;
  double costo_riga=calcola_costo(grafo.edges[route[dim_route-1]][0],0); //costo dall'ultimo nodo servito al deposito (veicolo vuoto)
  double q=clienti[route[dim_route-1]].richiesta; //il valore di q(carico veicolo) parte dalla domanda dell'ultimo servito.

  for(i=dim_route-1;i>0;i--){
    conto++;
    costo_riga+=calcola_costo(grafo.edges[route[i-1]][route[i]],q); //il costo per arrivare al nodo i, portando q pari alla domanda da esso alla fine del tragitto
    q+=clienti[route[i-1]].richiesta;
  }
  costo_riga=costo_riga+calcola_costo(grafo.edges[0][route[0]],q); //costo dal deposito al primo cliente
  if (conto!=dim_route+1){
    cout<<"la route conta "<<conto<<" archi, controllare!"<<endl;
  }
  return costo_riga;
}

double two_opt(int route[], int dim_route,const cliente_t clienti[],const graph_t grafo, int sol[]){
  int i,j,k;
  double c=costo_riga(route,dim_route,clienti,grafo);
  double new_c;
  int new_route[dim_route];
  for(i=0;i<dim_route;i++){

    sol[i]=route[i];
  }
  for(i=0;i<dim_route-1;i++){
    for(j=i+1;j<dim_route;j++){
      for(k=0;k<i;k++)
        new_route[k]=sol[k];
      int dec=0;
      for(k=i;k<=j;k++){
        new_route[k]=sol[j-dec];
        dec++;
      }
      for(k=j+1;k<dim_route;k++)
        new_route[k]=sol[k];
      new_c=costo_riga(new_route,dim_route,clienti,grafo);
      if(new_c<c){
        // cout<<"ho migliorato, da "<<c<<" a "<<new_c<<endl;
        c=new_c;
        for(k=0;k<dim_route;k++)
          sol[k]=new_route[k];
        i=-1;
        break;
      }
    }
  }
  // cout<<"2opt ritorna:\n";
  // for(int i=0;i<grafo.nclient;i++)
  //   cout<<sol[i]<<"  ";
  // cout<<endl;
  return c;
}

/** Funzione che trova il migliore scambio di due clienti appartenenti a routes diverse
 * La funzione cicla su tutti i possibili scambi, evitando ripetizioni, e calcola il costo
 * delle soluzioni modificate, ritornando la migliore ammissibile, che non sia gia' presente in tabu-list
 * Riceve in input la soluzione di partenza 'routes', clienti e grafo per il calcolo dei costi,
 * un puntatore a matrice di interi in cui inserire la soluzione trovata e un puntatore alla tabu-list.
 */

double swap2(routes_t routes,const cliente_t clienti[],const graph_t grafo, int **best, tabu_t *tabu ){
  int i,j,k,l,m,ammissibile=0,non_ammissibile=0,presente_tabu=0,n=grafo.nclient;
  int sol_i[n],sol_j[n];
  int **mat_sol;
  double costo_i,costo_j,costo_iniziale=0,costo_best=-1,costo_sol;
  for(i=0;i<routes.n_veicoli;i++) // inizializzo il costo iniziale pari a quello della soluzione di partenza
    costo_iniziale+=routes.costo[i];
  mat_sol=new int*[routes.n_veicoli]; // alloco una matrice di appoggio su cui andro a fare gli swap
  for(i=0;i<routes.n_veicoli;i++){    // e la inizializzo uguale a quella di partenza (routes.visita)
    mat_sol[i]=new int[n];
    for(j=0;j<n;j++)
      mat_sol[i][j]=routes.visita[i][j];
  }

  for(i=0;i<routes.n_veicoli-1;i++){   // per ogni veicolo provo tutti gli scambi
    for(j=i+1;j<routes.n_veicoli;j++){ // con i clienti dei veicoli successivi
      for(k=0;k<routes.n_clienti_veicolo[i];k++){
        for(l=0;l<routes.n_clienti_veicolo[j];l++){
          if(routes.carico_route[i]-clienti[routes.visita[i][k]].richiesta+clienti[routes.visita[j][l]].richiesta<=routes.capacita &&
          routes.carico_route[j]-clienti[routes.visita[j][l]].richiesta+clienti[routes.visita[i][k]].richiesta<=routes.capacita){
            ammissibile++; // se lo scambio non viola i vincoli di capacita lo trascrivo su mat_sol
            mat_sol[i][k]=routes.visita[j][l];
            mat_sol[j][l]=routes.visita[i][k];
            for(m=0;m<n;m++){ //preparo le righe per la soluzione del 2-opt
              sol_i[m]=routes.visita[i][m];
              sol_j[m]=routes.visita[j][m];
            }
            //dopo aver effettuato lo scambio, ottimizzo il costo all'interno della route
            costo_i=two_opt(mat_sol[i],routes.n_clienti_veicolo[i],clienti,grafo, sol_i);
            costo_j=two_opt(mat_sol[j],routes.n_clienti_veicolo[j],clienti,grafo, sol_j);

            costo_sol=costo_iniziale+costo_i+costo_j-routes.costo[i]-routes.costo[j];
            for(m=0;m<n;m++){ //mat_sol viene aggiornata con le route ottimizzate dopo lo scambio
              mat_sol[i][m]=sol_i[m];
              mat_sol[j][m]=sol_j[m];
            }
            if(!search_in_tabu(tabu,costo_sol,mat_sol,routes.n_veicoli,grafo.nclient)){ //se non e' nella tabu-list
              if ((costo_sol<costo_best)||(costo_best==-1)){ //e miglioro rispetto alle altre soluzioni generate (costo_best),
                for(m=0;m<routes.n_veicoli;m++){ //sovrascrivo la matrice che ritornero' alla fine.
                  for(int z=0;z<n;z++){
                    best[m][z]=mat_sol[m][z];
                    // cout<<best[m][z]<<'\t';
                  }
                  // cout<<endl;
                }
                costo_best=costo_sol;
              }
            }
            else
              presente_tabu++;
          }
          else
            non_ammissibile++;
          for(m=0;m<n;m++){ //prima di passare allo scambio successivo, riporto mat_sol alla condizione iniziale
            mat_sol[i][m]=routes.visita[i][m];
            mat_sol[j][m]=routes.visita[j][m];
          }
        }
      }
    }
  }
  // cout<<"soluzioni ammissibili:"<<ammissibile<<"/"<<ammissibile+non_ammissibile<<endl;
  // cout<<"soluzioni gia in tabu:"<<presente_tabu<<endl;
  // for(i=0;i<routes.n_veicoli;i++){
  //   for(j=0;j<n;j++)
  //     cout<<best[i][j]<<' ';
  //   cout<<endl;
  // }
  if (costo_best==-1)
    return 0;
  // cout<<"costo_best="<<costo_best<<endl;
  return costo_best;
}


/** nearest_neighbor è una funzione che produce la soluzione iniziale ammissibile
* da cui l'algoritmo parte. Genera una soluzione molto semplice:
* partendo dal primo veicolo, si prende il cliente più vicino alla stazione,
* dopodichè si prende il cliente più vicino all'ultimo cliente preso. Quando il
* cliente designato andrebbe con la sua richiesta ad eccedere la
* capacità del veicolo, allora non lo si prende, si torna invece alla stazione e
* si riparte con gli stessi passi ma con un altro veicolo. Quando tutti i veicoli
* sono pieni e/o i clienti sono serviti, allora si termina la procedura e si
* ritorna la soluzione.
* In input prende tre strutture, routes modella il problema, il grafo mantiene i
* costi degli archi, clienti modella la parte relativa ai clienti (richiesta).
* La soluzione viene ritornata per riferimento, sarà contenuta in routes.
* la funzione ritorna true se ha trovato una soluzione ammissibile che tenga conto
* di tutti i clienti, false se invece non è riuscito a assegnare tutti i clienti.
*/
bool nearest_neighbor(routes_t &routes, graph_t &grafo, const cliente_t clienti[]){
  bool aus;
  // il ritorno di aggiungi_cliente
  int i;
  //ultimo nodo visitato
  int last_node=0;
  // numero clienti servito: parto da uno perchè la stazione è sempre presente
  int n_serviti=1;
  // indice del veicolo che sto considerando
  int n_veicolo=0;
  // nearest ammissibile: nodo più vicino ammissibile
  int nearest;
  // la sorgente è al primo posto (nodo 0)
  // finchè non ho servito tutti e non ho esaurito i veicoli a disposizione
  while(n_serviti!=grafo.nclient && n_veicolo!=routes.n_veicoli){
    // inizializzo nearest=last_node: il più vicino è lo stesso..
    // ci sarà un controllo che non possa essere lo stesso nodo
    nearest=last_node;
    aus=true;
    // ricava il nodo più vicino all'ultimo nodo
    // controllo che non sia già visitato e che non sfori la mia capacita
    // parte da uno perchè la stazione è lo 0
    for(i=1; i<grafo.nclient; i++){
      // io con me stesso no (sarò sempre il più vicino)
      if(i == last_node)
        continue;
      // se è già servito
      if(routes.n_veicolo_cliente[i]!=-1)
        continue;
      // se la traccia per il veicolo non ha nodi allora acceta qualsiasi nearest
      // se trovo un nodo più vicino aggirona
      if(aus || nearest==last_node || grafo.edges[last_node][i]<grafo.edges[last_node][nearest]){
        aus=false;
        nearest=i;}
    }
    // solo con il più vicino trovato controllo se questo eccede la capacità. se sì
    // allora non aggiungerlo e cambia veicolo
    if(routes.carico_route[n_veicolo]+clienti[nearest].richiesta>routes.capacita){
      n_veicolo++;
      last_node=0;
    }
    // altrimenti aggiungi il cliente, e aggiorna last_node
    else{
      aggiungi_cliente(nearest, n_veicolo, routes, clienti);
      last_node=nearest;
      n_serviti++;
    }
  }
  if(n_serviti!=grafo.nclient){
    cout<<"ATTENZIONE! NON SONO RIUSCITO A SERVIRE TUTTI I CLIENTI\n";
    return false;
  }
  return true;
}

/**algoritmo è la funzione che implementa la logica dell'algoritmo per la ricerca
* di una soluzione migliore rispetto a quella di partenza.
* Partendo da una soluzione iniziale, si cerca una soluzione ammissibile
* appartenente al vicinato (il vicinato viene costruito con la move e la swap 2).
* Ad ogni passo quindi si prende la soluzione migliore ed ammissibile del vicinato,
* si controlla se migliora la soluzione migliore fin'ora incontrata, e si inserisce
* tale soluzione in una Tabu List. La TL permette di non tornare su soluzioni
* visitate "di recente". In questo modo si permette all'algorimto di allontanarsi
* da minimi locali. In input prende quindi la Tabu list, l'array dei clienti (con
* le loro rispettive richieste), routes che mantiene la soluzione; grafo serve
* per ricavare i costi degli archi e iteraz indica il numero massimo di iterazioni
* che si possono fare. In realtà è anche possibile fermarsi prima: questa eventualità
* e data solo nel caso in cui non è presente nessuna soluzione ammissibile nel
* vicinato. Alla fine, la soluzione sarà memorizzata in routes.
* La funzione ritorna true se l'algoritmo ha trovato almeno una soluzione, false
* se invece non ha trovato neanche una soluzione.
*/
bool algoritmo(tabu_t *tabu, const cliente_t clienti[], routes_t routes, graph_t grafo, int iteraz){
  // costo prec mi mantiene il costo della soluzione di fTL passi precedenti
  double costo_prec=0;
  cout<<"COEF_CARICO="<<COEF_CARICO<<endl;
  // fTL è il numero di iterazioni che rappresentano la frequenza della
  // valutazione del ridimensionamento della TL
  // questo numero dipende dalla dimenzione del problema.
  // una volta fissato non cambia. fTL sta per frequenza tabu list
  // matrix_migliore mantiene la soluzione migliore ammissibile del vicinato
  int **matrix_migliore= new int *[routes.n_veicoli];
  // matrix_move mantiene la soluzione migliore tra tutte le move
  int **matrix_move= new int *[routes.n_veicoli];
  // matrix_swap mantiene la soluzione migliore tra tutte le swap2
  int **matrix_swap= new int *[routes.n_veicoli];
  // opt mantiene la soluzione migliore fin'ora incontrata
  int **opt=new int *[routes.n_veicoli];
  // inizializzazione
  for (int i=0;i<routes.n_veicoli;i++){
    matrix_migliore[i]=new int [grafo.nclient];
    matrix_move[i]=new int [grafo.nclient];
    matrix_swap[i]=new int [grafo.nclient];
    opt[i]=new int [grafo.nclient];
    for(int j=0;j<grafo.nclient;j++){
      matrix_migliore[i][j]=-1;
      matrix_move[i][j]=-1;
      matrix_swap[i][j]=-1;
      opt[i][j]=-1;
    }
  }
  // check controlla che venga trovata almeno una soluzione..
  bool check=false;
  // i costi della sol di move swap e migliore tra M e S
  double costoM=0, costoS=0, costo=0;
  // costo della soluzione migliore fin'ora incontrata
  double costo_opt=0;
  // inizialmente l'ottimo è la matrice visita passata in input: generata dal
  // nearest_neighbor
  for(int i=0;i<routes.n_veicoli;i++){
    for(int j=0;j<grafo.nclient;j++)
      opt[i][j]=routes.visita[i][j];
    costo_opt+=costo_riga(opt[i],routes.n_clienti_veicolo[i],clienti,grafo);
    // costo_opt+=routes.costo[i]; //dovrebbe essere la stessa cosa..
  }
  // count mi indica il passo corrente --> arrivera ad iteraz
  int count=0;
  while(count!=iteraz){
    // stampa_tabu(tabu, routes.n_veicoli, grafo.nclient);
    // fai move
    costoM=move(tabu, clienti, routes, grafo, matrix_move);
    costoS=swap2(routes,clienti,grafo,matrix_swap,tabu);
    // se non esiste una soluzione del vicinato allora esci
    if (costoM==0 && costoS==0)
      break;

    // se anche solo una volta arrivo qui vuol dire che un vicino l'ho trovato
    check=true;
    // recupero tra i due modi per trovare il vicinato la migliore delle due.
    if(costoS<costoM && costoS!=0){
      costo=costoS;
      for(int i=0;i<routes.n_veicoli;i++){
        for(int j=0;j<grafo.nclient;j++)
          matrix_migliore[i][j]=matrix_swap[i][j];
      }
    }
    else if(costoM!=0){
      costo=costoM;
      for(int i=0;i<routes.n_veicoli;i++){
        for(int j=0;j<grafo.nclient;j++)
            matrix_migliore[i][j]=matrix_move[i][j];
      }
    }
    // qui ho il costo e la matrice di visita migliore e se miglioro la migliore
    // fin'ora incontrata allora aggiorno la migliore a quella attuale.
    if(costo<costo_opt){
      for(int i=0;i<routes.n_veicoli;i++){
        for(int j=0;j<grafo.nclient;j++)
            opt[i][j]=matrix_migliore[i][j];
      }
      costo_opt=costo;
    }
    // in ogni caso, inserisco la soluzione trovata (matrix_migliore) nella tabù liste_ad
    // so per certo che ci può entrare.. il controllo l'ho fatto sia nella move
    // che nella swap.
    add_new_tabu_item(tabu,costo, matrix_migliore, routes.n_veicoli, grafo.nclient);
    // una volta inserito però dovrò andare a modificare routes
    reimposta_routes(routes, clienti, grafo.nclient, matrix_migliore, grafo);

    // qui implemento il meccanismo di resize della tabu list: idea
    // ogni fTL passi controllo la soluzione migliore (costo_opt) con la soluzione
    // precedente (l'opt di fTL passi prcedenti)
    // se ho migliorato la mia soluzione ottima (la migliore fino ad adesso) allora
    // posso presumere di avere migliorato in questi fTL passi. Viceversa se non sono
    // uguali allora vuol dire che non ho migliorato: o ho ottenuto soluzioni con stesso
    // costo o ho peggiorato. in questo caso allora aumento la tabu list.

// in questo modo occhio.. al primo passo count==0 allora costo_prec=0 e quindi
// entra nell'else, la tabu si allunga subito.. va bene per ora..
// alternativa mettere un controllo su cost_prec se uguale a zero.
// però così lo farebbe ad ogni passo: più costoso
    // sono alla fTL-esima iterazione
    if(count%(tabu->fTL)==0){
      if (costo_opt!=costo_prec)
        cout<<"costo_opt="<<costo_opt<<"\ncosto_prec="<<costo_prec<<"\ndim tabu="<<tabu->dim<<endl;
      if(count%100==0)
        cout<<"iterazioni:"<<count<<endl;
      // se il costo_opt è migliore del costo_prec allora decremento la TL
      if(costo_opt<costo_prec)
        resize_tabu(false, tabu, routes.n_veicoli);
      // altrimenti l'allungo
      else
        resize_tabu(true, tabu, routes.n_veicoli);
      // aggiorno soluzione precedente alla soluzione ottima di adesso
      costo_prec=costo_opt;
    }
    count++;
  }
  // se non era presente una soluzione ammissibile nel vicinato
  if(count!=iteraz){
    /*per l'ultima soluzione vista non ci sono vicini ammissibili:
    o tuttu i clienti sono sulla stessa route,
    o spostando chiunque si eccede la capacità cammion
    o soluzioni già presenti nella tabulist
    probabilmente però avrò trovato cmq nelle iteraz precedenti delle sol, se sì
    allora ritorna comunque true, altrimenti false
    */
    if(!check){
      cout<<"esco prima, tutte le soluzioni sono gia' state esplorate, iterazioni:"<<count<<endl;
      return false;
    }
  }
  //alla fine la soluzione l'avrò in routes in cui metto opt
  reimposta_routes(routes, clienti, grafo.nclient, opt, grafo);
  cout<<"algoritmo terminato, iterazioni:"<<count<<endl;
  return true;
}

/**reimposta_routes è una funzione che aggiorna la struttura routes con una matrice
* di visita (matrice) passata in input. Per farlo, gli serve anche le informazioni
* relative ai clienti e agli archi.
*/
void reimposta_routes(routes_t routes, const cliente_t clienti[], int nclient, int **matrice, graph_t grafo){
  // imposto il veicolo della stazione
  routes.n_veicolo_cliente[0]=0;
  for(int i=0;i<routes.n_veicoli;i++){
    routes.carico_route[i]=0;
    routes.n_clienti_veicolo[i]=0;
    for(int j=0;j<nclient;j++){
      routes.visita[i][j]=matrice[i][j];
      // devo calcolare il carico di ogni route
      // per soli quelle posizioni di matrice in cui non c'è il meno uno, allora sono clienti
      // per questi allora assegno il veicolo i
      if(matrice[i][j]!=-1){
        routes.carico_route[i]+=clienti[matrice[i][j]].richiesta;
        routes.n_clienti_veicolo[i]+=1;
        routes.n_veicolo_cliente[matrice[i][j]]=i;
      }
    }
  }
  for(int i=0;i<routes.n_veicoli;i++){
    // ricalcolo il costo della riga
    routes.costo[i]=costo_riga(matrice[i],routes.n_clienti_veicolo[i],clienti,grafo);
  }
}

/**move è una funzione che implementa il meccanismo di move per l'algoritmo: ogni
* cliente viene messo in una qualsiasi altra traccia. Move quindi ricava la
* migliore soluzione ammissibile (controlla quindi che il cliente spostato non
* eccedi la capacità del veicolo e che la soluzione non sia già presente in TL)
* tra tutte quelle soluzioni del vicinato caratterizzate dal singolo spostamento
* di un cliente. in matrix_migliore avremo la soluzione migliore ammissibile e
* move ritornerà il costo di tale soluzione. In ingresso prende la soluzione di
* partenza (routes) dalla quale partire per la ricerca del vicinato, e le
* informazioni sui clienti e sul grafo.
*/
double move(tabu_t *tabu, const cliente_t clienti[], routes_t routes, graph_t grafo, int **matrix_migliore){
  // vet_route è un array ausiliario che mi mantiene la sola riga che cambio
  // ad ogni move cambio due righe route sorgente e route destinazione.
  int vet_route[grafo.nclient];
  // matrix mantiene la soluzione del vicinato che sto analizzando
  int **matrix=new int *[routes.n_veicoli];
  // trasla mi mantiene l'indice del cliente che tolgo da una route per spostarlo
  // in un altra. Da dove lo sposto devo coprire il buco con i possibili clienti
  // che lo seguivano.
  int trasla=0;
  // inizializzo matrix
  for(int i=0;i<routes.n_veicoli;i++){
    matrix[i]=new int [grafo.nclient];
    for(int j=0;j<grafo.nclient;j++)
      matrix[i][j]=-1;
    }
  // indice del veicolo che conteneva il cliente
  int old_veicolo;
  // costo della soluzione corrente (soluzione vicina che sto analizzando)
  double cost_neig=0;
  // costo della migliore soluzione vicina per ora incontrata.
  double cost_migliore=0;
  // faccio tutte le possibili move ed escludo la sorgente.
  for(int i=1; i<grafo.nclient; i++){
    for(int j=0; j<routes.n_veicoli; j++){
      // il cliente i-esimo dovrà avere id=i e lo si da per scontato..
      // devo controllare che il nuovo veicolo j non sia già il suo altrimenti continua
      if(routes.n_veicolo_cliente[i]==j)
        continue;
      // devo controllare che la traccia in cui lo sto inserendo non ecceda la
      // capacità del veicolo.
      if(routes.carico_route[j]+clienti[i].richiesta>routes.capacita)
        continue;
      // il caso in cui un veicolo contenga già tutti i clienti è autogestito..
      // il veicolo di i sarebbe j.

      // ad ogni cambio di veicolo e/o cliente matrix deve essere rinizializzata a -1
      for(int k=0;k<routes.n_veicoli;k++){
        for(int h=0;h<grafo.nclient;h++)
          matrix[k][h]=-1;
        }

      // parto con la riga da cui tolgo il cliente
      old_veicolo=routes.n_veicolo_cliente[i];
      trasla=0;
      for(int k=0; k<grafo.nclient; k++){
        //copio anche tutti i -1! è indispensabile.. altrimenti può succedere che il passo prima_in
        //avessi 5 val e adesso 3 e quindi mi rimarrebbero ancora gli altri due
        if(i!=routes.visita[old_veicolo][k]){
          vet_route[trasla]=routes.visita[old_veicolo][k];
          trasla++;
        }
      }
      // rimane l'ultimo
      vet_route[trasla]=-1;
      // impongo il suo veicolo ad j.. vale per entrambe le opt che seguono
      routes.n_veicolo_cliente[i]=j;
      cost_neig=0;
      // ho ottenuto l'array della route senza l'iesimo cliente.
      // ottimizzo questa route così da avere una rappresentazione univoca per il
      // dato insieme di clienti serviti.
      cost_neig+=two_opt(vet_route, routes.n_clienti_veicolo[old_veicolo]-1,clienti,grafo, matrix[old_veicolo]);

      // continuo con la route in cui inserisco il cliente
      // lo devo inserire a n_clienti_veicolo
      for(int k=0; k<grafo.nclient; k++){
        //copio anche tutti i -1! è indispensabile.. altrimenti può succedere che il passo prima_in
        //avessi 5 val e adesso 3 e quindi mi rimarrebbero ancora gli altri due
        vet_route[k]=routes.visita[j][k];
      }
      // inseirsco il cliente nella route nell'ultima posizione possibile
      vet_route[routes.n_clienti_veicolo[j]]=i;

      cost_neig+=two_opt(vet_route, routes.n_clienti_veicolo[j]+1,clienti,grafo, matrix[j]);
      // sommo anche il costo delle altre routes
      for(int k=0;k<routes.n_veicoli;k++){
        if(k!=old_veicolo && k!=j){
            cost_neig+=costo_riga(routes.visita[k],routes.n_clienti_veicolo[k],clienti,grafo);
            for(int h=0;h<grafo.nclient; h++){
              // termino anche di riempire le righe restanti della matrice
              matrix[k][h]=routes.visita[k][h];
            }
        }
      }
      // qui dovrei avere la nuova matrice con il costo ad essa associato.
      // devo controllare che non ci sia in tabu list
      // se non c'è allora devo controllare che sia la soluzione migliore che ho incontrato
      if(!search_in_tabu(tabu,cost_neig, matrix, routes.n_veicoli, grafo.nclient)){
        // devo controllare la sol attuale con la migliore
        if(cost_migliore==0 || cost_migliore>=cost_neig){
          cost_migliore=cost_neig;
          for(int k=0;k<routes.n_veicoli;k++){
            for(int h=0;h<grafo.nclient; h++)
              matrix_migliore[k][h]=matrix[k][h];
          }
        }
      }
      // prima di provare a fare nuove move per altre soluzioni, reimposto il
      // vecchio veicolo per il cliente i-esimo.
      routes.n_veicolo_cliente[i]=old_veicolo;
    }
  }
  return cost_migliore;
}

/**add_new_tabu_item è la funzione che aggiunge la nuova soluzione alla tabù list.
* Inserire un elemento implica togliere il più vecchio, quindi, in base a quanti
* item ho già in TL, e alla dimensione della TL, inserisco in testa l'elemento
* nuovo (soluzione nuova) e elimino l'elemento in coda.
* l'inserimento di una soluzione consiste nell'inserire la matrice di visita e il
* suo relativo costo.
*/
void add_new_tabu_item(tabu_t *tabu,double costo, int **matrice, int nveicoli, int nclienti){
  // nuovo elemento della TL
  tabu_item_t *new_element=new tabu_item_t;
  new_element->cost=costo;
  new_element->visita=new int *[nveicoli];
  for(int i=0; i<nveicoli;i++){
    new_element->visita[i]=new int [nclienti];
    for(int j=0; j<nclienti; j++)
      new_element->visita[i][j]=matrice[i][j];
  }
  // lo imposto uguale a zero, così poi lo incremento e risulterà uno.
  new_element->t=0;
  // inserisco in testa l'elemnto.
  new_element->next=tabu->head;
  tabu->head=new_element;
  // adesso dovrei cancellare l'ultimo.. lo dealloco
  // ne approfitto per incrementargli il loto tempo (che indica per quanto ancora devono rimanere in tabu list)
  tabu_item_t *attuale=tabu->head;
  tabu_item_t *prossimo;
  // per prima cosa devo scorrere per tabu->dim la lista e incrementare solo il t
  // dentro però faccio un controllo che mi fermo quanto sono a NULL (vuol dire che ho meno item di dim)
  for(int i=0; i<tabu->dim && attuale!=NULL; i++){
    attuale->t++;
    prossimo=attuale->next;
    // qui occhio: devo fare un controllo se sono a dim. in questo il next potrebbe essere !=NULL ma io voglio che sia NULL
    if(i==tabu->dim-1)
      attuale->next=NULL;
    // gli riassegno il next
    attuale=prossimo;
  }

  // qui o ho finito gli item, o tutti quelli dopo li devo cancellare: dovrebbe
  // essere uno solo.
  while(attuale!=NULL){
    prossimo=attuale->next;
    dealloca_item_tabu(attuale,nveicoli);
    attuale=prossimo;
  }
  // stampa_tabu(tabu, nveicoli, nclienti);
}

/**dealloca_item_tabu dealloca l'elemento passato in input
*/
void dealloca_item_tabu(tabu_item_t *item, int nveicoli){
  for(int i=0; i<nveicoli; i++)
    delete []item->visita[i];
  delete []item->visita;
  delete item;
}

/**search_in_tabu effettua la ricerca all'interno della TL. Controlla se una
* soluzione (matrice passata in input) è presente o no nella TL. Se è presente
* allora non è una soluzione ammissibile. La ricerca per motivi di efficienza
* avviene prima sul costo (singolo valore numerico); e solo a parità di costo allora
* viene confrontata la matrice di visita.
* la funzione ritorna true se la soluzine è già presente in tabù list, false altrimenti.
*/
bool search_in_tabu(tabu_t *tabu,double costo, int **matrice, int nveicoli, int nclienti){
  int i=0, j=0;
  bool flag;
  // si parte dalla testa della lista
  tabu_item_t *attuale=tabu->head;
  while(attuale!=NULL){
    // si controlla prima per costo della soluzione
    if(costo==attuale->cost){
      // allora confronto la matrice
      // flag true è se ho già la stessa soluzione
      flag=true;
      // scorro le due matrici e basta che ci sia un numero diverso allora sono diverse.
      for(i=0; i<nveicoli; i++){
        for(j=0; j<nclienti; j++){
          if(matrice[i][j]!=attuale->visita[i][j])
            flag=false;
            break;//se diverse mi fermo subito basta solo un numero
          }
        if(!flag)
          break;
      }
      // se alla fine flag è ancora true vuol dire che questa soluzione è già presente in tabu
      if(flag)
        return true;
    }
    attuale=attuale->next;
  }
  // se arrivo fino alla fine allora vuol dire che non è ancora presente in tabu
  return false;
}

/**resize_tabu ridimensiona la TL. La modifica della dimensione potrebbe aiutare
* velocizzare la ricerca di una soluzione migliore. L'idea è: se miglioro, allora
* accorcio la TL; in questo modo scorro più velocemente la TL stessa. Se invece
* peggioro, allora mi voglio allontanare dalle soluzioni correnti e quindi allungo
* la TL.
*/
void resize_tabu(bool flag, tabu_t *tabu, int nveicoli){
  tabu_item_t *attuale;
  tabu_item_t *prossimo;
  int conta=0;
  int random=rand()%100+1;
  // l'incremento può andare dal 1% al 100% rispetto alla dimensione attuale
  // quindi raddoppierei la dimensione
  if(flag){
    tabu->dim+=int(random*tabu->dim/100.0);
    if (tabu->dim>tabu->max_tabu)
      tabu->dim=tabu->max_tabu;
  }
  // per il decremento ci sono alcuni passaggi aggiuntivi
  else{
    tabu->dim-=int(random*tabu->dim/200.0);
    // per il decremento devo stare attento sdi non scendere sotto 1
    if(tabu->dim<1)
      tabu->dim=1;
    // scorro fino a dim e gli altri elementi li devo eliminare
    attuale=tabu->head;
    for(int i=0;i<tabu->dim-1 && attuale!=NULL;i++)
      attuale=attuale->next;
    // se avessi ancora degli elementi, allora devo deallocarli
    if(attuale != NULL){
      // una volta finito metto attuale->next a null perchè voglio che la mia lista
      // termini e non deve puntare a nessun altro
      prossimo=attuale->next;
      attuale->next=NULL;
      attuale=prossimo;
      // scorro i rimanenti item per deallocarli
      // provo a contarli, dovrebbero essere dim_old-dim
      while(attuale!=NULL){
        prossimo=attuale->next;
        dealloca_item_tabu(attuale,nveicoli);
        attuale=prossimo;
        conta++;
      }
    }
  }
}

/**stampa_tabu è una funzione ausiliaria per il debug in cui vengono stampati
* tutti gli elementi della TL.
*/
void stampa_tabu(tabu_t *tabu, int n_veicoli, int n_clienti){
  tabu_item_t *attuale;
  cout<<"v: "<<n_veicoli<<" e c: "<<n_clienti<<endl;
  attuale=tabu->head;
  while(attuale!=NULL){
    cout<<"------\n";
    cout<<attuale->cost<<endl;
    for(int i=0; i<n_veicoli;i++){
      for(int j=0; j<n_clienti; j++)
        cout<<attuale->visita[i][j]<<" ";
      cout<<endl;
    }
    attuale=attuale->next;
  }
  cout<<"fine\n\n";
}
