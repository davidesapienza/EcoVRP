#include "struttura_dati.h"
#include<cstdio>
#include <stdio.h>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include <time.h>
#include <unistd.h>

using namespace std;

int aggiungi_cliente(int cliente, int veicolo, routes_t &routes,const cliente_t clienti[]);
double calcola_costo(double d,double q);
double costo_riga(int route[], int dim_route,const cliente_t clienti[],const graph_t grafo);
double two_opt(int route[], int dim_route,const cliente_t clienti[],const graph_t grafo, int sol[]);
double swap2(routes_t routes,const cliente_t clienti[],const graph_t grafo,routes_t &best,tabu_t *tabu);
bool nearest_neighbor(routes_t &route, graph_t &grafo, const cliente_t clienti[]);
void resize_tabu(bool flag, tabu_t *tabu, int nveicoli);
bool search_in_tabu(tabu_t *tabu,double costo, int **matrice, int nveicoli, int nclienti);
void dealloca_item_tabu(tabu_item_t *item, int nveicoli);
void add_new_tabu_item(tabu_t *tabu,double costo, int **matrice, int nveicoli, int nclienti);
bool algoritmo(tabu_t *tabu, const cliente_t clienti[], routes_t routes, graph_t grafo, int iteraz);
void reimposta_routes(routes_t routes, const cliente_t clienti[], int nclient, int **matrice, graph_t grafo);
double move(tabu_t *tabu, const cliente_t clienti[], routes_t routes, graph_t grafo, int **matrix_migliore);
void stampa_tabu(tabu_t *tabu, int n_veicoli, int n_clienti);
