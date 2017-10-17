#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include "struttura_dati.h"
#include "operazioni_file.h"
#include "algoritmo.h"
#include <time.h>
#include <sys/time.h>

using namespace std;
#define SECONDS 1000000000
unsigned long long gettime(void)
{
    struct timespec t;
    int r;

    r = clock_gettime(CLOCK_MONOTONIC, &t);
    if (r < 0) {
        printf("Error to get time! (%i)\n", r);
        return -1;
    }
    return (unsigned long long) t.tv_sec * SECONDS + (unsigned long long)t.tv_nsec;
}

const char FileGrafo[]="./convertitore/file/grafo_per_algo.txt";
const char FileSol[]="./algoritmo/file/soluzione.txt";
// const char FileGrafo[]="../../convertitore/file/grafo_per_algo.txt";
// const char FileSol[]="../file/soluzione.txt";

double COEF_DIST=7.70938694955e-07;
double COSTO_VUOTO=910.120351852;
double COEF_CARICO=0.329422222222;
int N_VEICOLI=6;
double CAPACITA=1000;
int MAX_ITER=8000;
int LUN_TABU_INIZIALE=100;
int MAX_TABU=1000;
int FREQUENZA_AGG=1000;

// dichiarazioni di funzione
void inizializza_routes(routes_t *routes, int nclient, int n_veicoli, double capacita);
void inizializza_cliente(cliente_t *clienti, int nclient);
void display_solution(routes_t routes, graph_t grafo, cliente_t clienti[]);
void inizializza_tabu(tabu_t *tabu);

/**main: funzione principale in cui c'è una prima fase di inizializzazione e lettura_file
* parametri e file di input. Successivamente viene creata la soluzione iniziale e
* lanciato l'algoritmo.
*/
int main(int argc,char *argv[]){
  // setta la chiave della funzione rand ad un numero sempre diverso ad ogni lancio.
  srand(time(NULL));
  graph_t grafo;
  routes_t routes;
  cliente_t *clienti;
  tabu_t *tabu=new tabu_t;
  // controlla che ci siano sei argomenti passati in input.
  if (argc!=10){
    printf("ERRORE, NUMERO PARAMETRI ERRATO!\n");
    // return 1;
  }
  else{
    COEF_DIST=atof(argv[1]);
    COSTO_VUOTO=atof(argv[2]);
    COEF_CARICO=atof(argv[3]);
    N_VEICOLI=atoi(argv[4]);
    CAPACITA=atof(argv[5]);
    MAX_ITER=atoi(argv[6]);
    LUN_TABU_INIZIALE=atoi(argv[7]);
    MAX_TABU=atoi(argv[8]);
    FREQUENZA_AGG=atoi(argv[9]);
  }
  // legge il grafo: numero clienti e pesi degli archi
  if(!lettura_file(FileGrafo,&grafo)){
    cout<<"errore lettura grafo\n";
    return -1;
  }
  // fase di inizializzazione
  inizializza_tabu(tabu);
  inizializza_routes(&routes, grafo.nclient,N_VEICOLI,CAPACITA);
  clienti=new cliente_t [grafo.nclient];
  inizializza_cliente(clienti, grafo.nclient);

  // lettura e assegnamento delle richieste nell'array di clienti
  if(!lettura_richieste("./algoritmo/file/richieste.txt",clienti,grafo.nclient)){
    cout<<"errore in lettura richieste";
    return -1;
  }

  // crea una soluzione iniziale e controlla che in tale soluzione siano soddisfatti
  // tutti i clienti, altrimenti è necessario aumentare il numero dei veicoli a
  // disposizione.
  if(!nearest_neighbor(routes, grafo, clienti)){
    cout<<"errore in nearest_neighbor!\n";
    return -1;
  }

  // creo la matrice con la soluzione da inserire nella tabù search e la routes aggiornata
  int **matrice_iniziale_ottimizzata=new int * [routes.n_veicoli];
  int costo_iniziale=0;
  for(int i=0;i<routes.n_veicoli;i++){
    matrice_iniziale_ottimizzata[i]=new int [grafo.nclient];
    for(int j=0;j<grafo.nclient;j++)
      matrice_iniziale_ottimizzata[i][j]=-1;
    two_opt(routes.visita[i],routes.n_clienti_veicolo[i],clienti,grafo, matrice_iniziale_ottimizzata[i]);
    costo_iniziale+=costo_riga(matrice_iniziale_ottimizzata[i],routes.n_clienti_veicolo[i],clienti,grafo);
  }
  int *a=new int[grafo.nclient];
  int *a_opt=new int[grafo.nclient];
  for(int i=0;i<grafo.nclient-1;i++){
    a[i]=i+1;
    a_opt[i]=-1;
  }
  a[grafo.nclient-1]=a_opt[grafo.nclient-1]=-1;
  double lb=two_opt(a,grafo.nclient-1,clienti,grafo,a_opt);
  cout<<"costo_iniziale="<<costo_iniziale<<"\nlower bound ="<<lb<<endl;
  add_new_tabu_item(tabu,costo_iniziale, matrice_iniziale_ottimizzata, routes.n_veicoli, grafo.nclient);
  // imposto la matrice ottimizzata nella struttura routes che poi passerò all'algoritmo
  reimposta_routes(routes, clienti, grafo.nclient, matrice_iniziale_ottimizzata, grafo);
  display_solution(routes,grafo,clienti);
  unsigned long long t1,t0=gettime();
  // unsigned int t0=time(0);
  cout<<"COEF_CARICO main="<<COEF_CARICO<<endl;
  if(algoritmo(tabu, clienti, routes, grafo, MAX_ITER)){
    t1=gettime();
    // unsigned int t1=time(0);
    cout<<"tempo impiegato="<<(t1-t0)/1000<<" microsecondi"<<endl;
    display_solution(routes, grafo,clienti);
  }
  else{
    cout<<"non ho trovato un'altra soluzione.. la soluzione che avevo era già quella migliore.\n";
  }
  scrittura_file(FileSol,routes);
  // cout<<"secondo lancio\n";
  // COEF_CARICO=0;
  // inizializza_tabu(tabu);
  // if (algoritmo(tabu,clienti,routes,grafo,MAX_ITER)){
  //   COEF_CARICO=atof(argv[3]);
  //   display_solution(routes, grafo,clienti);
  // }
  FILE *pfile;
  pfile=fopen("./algoritmo/file/tempi.txt","a");

  if(pfile==NULL){
    printf("errore apertura in scrittura file tempi.txt!!!");
    return -1;
    }
  fprintf(pfile,"%llu",t1-t0);
  if(fclose(pfile)!=0){
		printf("errore in chiusura file!!!");
		return -1;
	}
  return 0;
}


/**inizializza_routes inizializza la struttura che mi manterrà la soluzione.
* alcune informazioni devono essere settate: la dimensione della matrice di
* visita, la dimensione degli altri vettori, il numero di veicoli e la loro capacità.
*/
void inizializza_routes(routes_t *routes, int nclient, int n_veicoli, double capacita){
  routes->n_veicoli=n_veicoli;
  routes->capacita=capacita;

  routes->visita=new int*[routes->n_veicoli];
  routes->n_clienti_veicolo=new int [routes->n_veicoli];
  routes->n_veicolo_cliente=new int [nclient];
  routes->carico_route=new double [routes->n_veicoli];
  routes->costo=new double [routes->n_veicoli];

  for(int i=0; i<routes->n_veicoli; i++){
    routes->visita[i]=new int [nclient];
    for(int j=0; j<nclient; j++)
      routes->visita[i][j]=-1;
    routes->n_clienti_veicolo[i]=0;
    routes->carico_route[i]=0;
    routes->costo[i]=0;
  }
  for(int i=0;i<nclient;i++)
    routes->n_veicolo_cliente[i]=-1;

}

/**inizializza_cliente inizializza l'array di clienti inserendo la richiesta di
* ogni cliente, e il suo id.
*/
void inizializza_cliente(cliente_t *clienti, int nclient){
  for(int i=0;i<nclient;i++){
    clienti[i].id=i;
    clienti[i].richiesta=1;
  }
}

/**display_solution è una funzione ausiliaria per il debug. Non ha un importante
ruolo nel progetto. Permette di stampare la routes sul terminale.
*/
void display_solution(routes_t routes, graph_t grafo, cliente_t clienti[]){
  double costo=0;
  for(int i=0;i<routes.n_veicoli;i++)
    costo+=costo_riga(routes.visita[i], routes.n_clienti_veicolo[i],clienti,grafo);
    // costo+=routes.costo[i];
  cout<<"la soluzione costa "<<costo<<" ed è:\n";
  for(int i=0;i<routes.n_veicoli;i++){
    // non stampo per nclient ma per n. clienti serviti
    for(int j=0;j<routes.n_clienti_veicolo[i];j++)
      cout<<routes.visita[i][j]<<" ";
    cout<<endl;
  }
}

/**inizializza_tabu inizializza la tabu list con una dimensione fissata iniziale
* con zero elementi presenti.
*/
void inizializza_tabu(tabu_t *tabu){
  tabu->fTL=FREQUENZA_AGG;
  tabu->max_tabu=MAX_TABU;
  tabu->dim=LUN_TABU_INIZIALE;
  tabu->head=NULL;
}
