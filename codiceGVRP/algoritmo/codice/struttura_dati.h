#ifndef STRUTTURA_DATI
#define STRUTTURA_DATI

/** Struttura dati per il grafo. Il grafo viene letto da un file di input.
* nclienti --> rappresenta il numero dei clienti da servire
* edges --> rappresenta la matrice degli archi. Mantiene i costi.
* il grafo lo si consideri completo e orientato, allora la matrice è una
* nclient*nclient
*/
struct graph_t {
  int nclient;
  double **edges;
};

/** Struttura dati per il problema.
* n_veicoli --> Rappresenta il numero di veicoli a disposizione
* capacita --> Rappresenta la capacità dei veicoli (si considera capacità fissa
               e uguale per ogni veicolo)
* carico_route --> Rappresenta il carico di richieste soddisfatte per il singolo
               veicolo. Informazione che deve essere <= alla capacita
* n_clienti_veicolo --> Rappresenta il numero di clienti soddisfatti per ogni
               veicolo. é un array di n_veicoli elementi.
* n_veicolo_cliente --> Rappresenta il veicolo servente il tale cliente. array
               di dimensione pari a Grafo.nclient. Ogni cliente è associato ad un
               solo veicolo.
* visita --> Mantiene la matrice di visita dell'intero problema. Per righe ha i
               veicoli, per colonne l'indice di visita. Gli elementi sono gli
               indici dei clienti.
* costo --> Array che Rappresenta i costi di ogni routes (tragitto ogni veicolo)
*/
struct routes_t {
  int n_veicoli;
  double capacita;
  double *carico_route;
  int *n_clienti_veicolo;
  int *n_veicolo_cliente;
  int **visita;
  double *costo;
};

/** Struttura dati che rappresenti il cliente.
* Ogni cliente ha:
* id --> identificativo intero univoco
* richiesta --> la richiesta desiderata
*/
struct cliente_t {
  int id;
  double richiesta;
};

/** Struttura dati che rappresenta il singolo elemento della TABU LIST.
* Questo elemento è una soluzione ammissibile del problema.
* cost --> Rappresenta il costo di questa soluzione
* visita --> Rappresenta la soluzione vera e propria
* next --> Punta al prossimo elemento della TL
* t --> Rappresenta il numero di passi trascorsi da quando la soluzione è stata
        generata
*/
struct tabu_item_t{
  double cost;
  int **visita;
  tabu_item_t *next;
  int t;
};

/** Struttura dati che rappresenta la TABU LIST (TL). Essa mantiene le soluzioni
* già incontrate. Tali soluzioni non sono più ammissibili fin tanto che sono
* presenti in TL.
* dim --> Rappresenta la dimensione della TL, quanti elementi contiene.
* head --> punta all'ultima soluzione incontrata.
*/
struct tabu_t{
  int dim;
  int fTL;
  int max_tabu;
  tabu_item_t *head;
};

#endif
