Utilizzo software sistemato:

- copiare la cartella "codiceGVRP" in una cartella a scelta.

- è necessario compilare il codice in C++. Andare quindi in 
	./codiceGVRP/algoritmo/codice/ e da terminale digitare:
	"g++ struttura_dati.h operazioni_file.h operazioni_file.cc \
	algoritmo.h algoritmo.cc  main.cc -o gvrp"

- il codice verrà poi utilizzato da moduloGVRP.py che gestirà la chiamata
	e il passaggio dei parametri.

- tornare alla cartella "./codiceGVRP", da qui si può chiamare 
	direttamente da terminale:
	"python moduloGVRP.py".


MODULO_CONVERTITORE_MAPPE_GIS

- il software del convertitore è contenuto in "./codiceGVRP/convertitore/".
	
- INPUT: le mappe QGIS delle strade e dei clienti presenti in 
	"./codiceGVRP/QGIS/input"

- OUTPUT: 
  1)vengono generati due file che servono all'algoritmo: 
	- grafo_per_algo.txt: rappresentazione del grafo con i soli clienti e 
		sorgente
	- percorsi.txt: rappresentazione degli archi tra ogni cliente, che 
		corrispondono ai cammini minimi.
	questi file sono presenti in "./codiceGVRP/convertitore/file/"

  2) vengono generati anche degli shapefile intermedi suddivisi in 4 cartelle:
	"./codiceGVRP/convertitore/clienti_originari/"
	"./codiceGVRP/convertitore/solo_nuovi_punti/"
	"./codiceGVRP/convertitore/con_clienti/"
	"./codiceGVRP/convertitore/ridotto/"

- per invocare il convertitore bisogna scostarsi in 
	"./progettoGVRP/convertitore/codice/" e da terminale digitare:
	"python convertitore.py"

MODULO_ALGORITMO

- il software dell'algoritmo è contenuto in "./codiceGVRP/algoritmo/codice".

- INPUT: 
	1) riceve i file del convertitore ("./codiceGVRP/convertitore/file").
	2) riceve le richieste dei clienti 
		("./codiceGVRP/algoritmo/file/richieste.txt")
	3) riceve dei parametri in input al momento della chiamata

- OUTPUT: fornisce la soluzione ("./codiceGVRP/algoritmo/file/soluzione.txt")

MODULO_MODELLO_EMISSIONI

- i dati legati alle emissioni sono contenuti in 
	"./codiceGVRP/modello_emissioni/modello.csv". questo dile viene letto
	dalla funzione "model_reader()" contenuta in "./codiceGVRP/csvreader.py".

- il file contiene anche altri parametri di input: N_VEICOLI, CAPACITA, 
	MAX_ITER, LUN_TABU_INIZIALE, MAX_TABU, FREQUENZA_AGG. 

MODULO_GVRP

- modulo generale che racchiude gli altri. Necessita dei file prodotti dal
	convertitore (sarà quindi necessario prima invocare almeno una volta il 
	convertitore).

- Per prima cosa legge il modello di costo (invoca model_reader()). Così
	facendo ricava i parametri da passare all'algoritmo. Ottiene la soluzione 
	dell'algoritmo, e recuperando i file dei percorsi del convertitore, mostra
	la soluzione in formato GIS. 

- i parametri che passa all'algoritmo, una volta letti dal modello di emissione
	sono:
	- COEF_DIST
    - COSTO_VUOTO
    - COEF_CARICO
    - N_VEICOLI
    - CAPACITA
    - MAX_ITER
    - LUN_TABU_INIZIALE
    - MAX_TABU
    - FREQUENZA_AGG

- la soluzione (in formato GIS) è contenuta in "./codiceGVRP/QGIS/output/".
	Dentro la cartella si troveranno tante sottocartelle, quanti sono i veicoli
	utilizzati dalla soluzione: si ottiene cosi la route per ogni veicolo.

- in ultimo, vengono generati anche altri due file:
	"./codiceGVRP/algoritmo/file/tempi.txt"
	"./codiceGVRP/tempi.txt"
	che contengono i tempi dell'esecuzione.
