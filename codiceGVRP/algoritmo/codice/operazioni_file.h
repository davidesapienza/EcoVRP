#include "struttura_dati.h"

bool lettura_file(const char * file_in, graph_t *grafo);
bool scrittura_file(const char * file_in, routes_t routes);
bool lettura_richieste(const char * file_in, cliente_t clienti[],int nclient);
