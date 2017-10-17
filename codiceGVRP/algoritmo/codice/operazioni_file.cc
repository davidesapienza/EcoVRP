#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "operazioni_file.h"

/** Funzione di lettura del grafo da file. Lege prima il numero di clienti presenti,
* poi i costi di ogni arco. Memorizza le informazioni in grafo.
* Prende in input il nome del file da leggere e il riferimento alla struttura in
* andrà a memorizzare ciò che legge. Restituisce un booleano: true se la lettura_file
* è andata a buon fine, false altrimenti.
*/
bool lettura_file(const char * file_in, graph_t *grafo){
  FILE *pfile;
  int i,row,col;
  pfile=fopen(file_in,"r");

	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;
		}
  fscanf(pfile,"%d",&grafo->nclient);
  grafo->edges=new double* [grafo->nclient];

  for(i=0;i<grafo->nclient;i++)
    grafo->edges[i]=new double [grafo->nclient];
  // ciclo per nclienti*(nclienti-1) volte (numero archi)
  // si ricorda che il grafo è completo e orientato.
  for(i=0;i<grafo->nclient*(grafo->nclient-1);i++){
    fscanf(pfile,"%d ",&row);
    fscanf(pfile,"%d ",&col);
    fscanf(pfile,"%lg ",&grafo->edges[row][col]);
  }

  if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);
		return false;
		}
  return true;
}

/**Funzione che scrive la soluzione ottenuta su file. Il file servirà poi ad un
* altro modulo per fornire una rappresentazione in formato GIS. Prende in ingresso
* il nome del file che deve scrivere e la routes (soluzione).
* Restituisce un booleano: true se la scrittura file è andata a buon fine,
* false altrimenti.
*/
bool scrittura_file(const char * file_in, routes_t routes){
  FILE *pfile;
  pfile=fopen(file_in,"w");

	if(pfile==NULL){
		printf("errore apertura in scrittura file!!! error is %d\n",errno);
		return false;
		}
  fprintf(pfile,"soluzione del GVRP:\n");
  for(int i=0; i<routes.n_veicoli; i++){
    fprintf(pfile,"0 ");
    for(int j=0; j<routes.n_clienti_veicolo[i];j++)
      fprintf(pfile,"%d ",routes.visita[i][j]);
    fprintf(pfile,"0\n");
  }
  if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);
		return false;
		}
  return true;
}

bool lettura_richieste(const char * file_in, cliente_t clienti[],int nclient){
  FILE *pfile;
  pfile=fopen(file_in,"r");
  int c,i;
	if(pfile==NULL){
		printf("errore apertura in lettura file!!! error is %d\n",errno);
		return false;
		}
  fscanf(pfile,"%d",&c);
  if (nclient!=c){
    printf("errore, nclient diversi!\n");
    return false;
  }
  for(i=0;i<c;i++)
    fscanf(pfile,"\n%lf",&clienti[i].richiesta);
  if(fclose(pfile)!=0){
		printf("errore in chiusura file!!! error is %d\n",errno);
		return false;
		}
  return true;
}
