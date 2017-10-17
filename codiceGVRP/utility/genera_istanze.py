import sys
from genera_richieste import genera_richieste
from grafo_random import grafo_random
'''
Questo script serve per lanciare le funzioni di generazione di grafi e di richieste
casuali, passando parametri a scelta dell'utente.
'''
#gli argomenti passati sono argv[0]=chiamata ; argv[1]=n_clienti
if (len(sys.argv)!=3) and (len(sys.argv)!=4) and (len(sys.argv)!=5):
    print "errore numero di parametri genera istanze!";
    sys.exit(0)
if len(sys.argv)==3: #se ho 3 argomenti genero le richieste
    genera_richieste(int(sys.argv[1]),int(sys.argv[2])) #argv[2]=richiesta_max
if len(sys.argv)==4: #se ho 4 argomenti genero un grafo
    grafo_random(int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3])) #argv[2]=costo_min, argv[3]=costo_max
if len(sys.argv)==5: #se ho 5 argomenti genero entrambi
    grafo_random(int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]))
    genera_richieste(int(sys.argv[1]),int(sys.argv[4])) #argv[4]=richiesta_max
