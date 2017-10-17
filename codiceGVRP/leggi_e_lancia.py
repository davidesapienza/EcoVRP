# -*- coding: utf-8 -*-

import networkx as nx
import os, sys
from subprocess import call
import matplotlib.pyplot as plt
import time
import math

FileSol="./algoritmo/file/soluzione.txt"
FilePath="./convertitore/file/percorsi.txt"
nome_eseguibile="./algoritmo/codice/gvrp"

'''
leggiSol legge la soluzione prodotta dal modulo in C. legge gli indici dei nodi.
La soluzione è composta da tante righe quante sono i veicoli. Ogni riga contiene
gli indici dei nodi che serve (lo 0 è la sorgente).
Input:
name: il nome del file da leggere e che contiene la soluzione
n_veicoli: il numero dei veicoli a disposizione.
Ritorna gli indici, o meglio le routes dei veicoli contenente gli indici dei nodi
in ordine di visita.
'''
def leggiSol(name, n_veicoli):
    in_file = open(name,"r")
    indici={};
    # la prima riga è da scartare: "soluzione del GVRP:"
    text = in_file.readline()
    # per ogni veicolo leggo la riga e tramite split multipli separo i diversi
    # indici per quella route
    for i in range(0,n_veicoli):
        text = in_file.readline()
        text=text.split(' ')
        # nell'ultimo devo togliere '\n'
        aus=text[-1].split('\n')
        text[-1]=aus[0]
        text=[int(x) for x in text]
        indici[i]=text
    in_file.close()
    return indici

'''
leggi_percorsi legge i percorsi da file per graficare la soluzione del modulo C.
Input:
name: nome del file da aprire il lettura
indici: gli indici della soluzione, cioè la soluzione stessa del C
Ritrona:
l'insieme di punti in ordine (l'indice 0 corrisponderà al primo nodo letto, e così
via). I punti sono rappresentati da due coordinate.
E i percorsi di un nodo rispetto ad ogni altro nodo con le coordinate.
'''
def leggi_percorsi(name, indici):
    in_file = open(name,"r")
    punti=[]
    percorsi={}
    while(True):
        text = in_file.readline()
        if (text=='\n'):
            break;
        text=text.split('\n')
        text=text[0]
        text=text.split(' ')
        n1=text[0]
        n2=text[1]
        punti.append(tuple([str(n1),str(n2)]))
    # da qui leggo i percorsi: ci saranno n * n-1 righe
    # !! credo di non aver gestito il caso in cui non ci sia un percorso tra un nodo e l'altro.. ma può capitare??
    # eventualmente sostituisci con un read until EOF
    # leggo tutti i percorsi con i relativi incroci intermedi: informazione
    # necessaria per la rappresentazione in mappa GIS.
    # Da ogni riga, tramite una serie si split recupero le coordinate di ogni
    # incrocio intermedio e costruisco il mio dizionario dei percorsi.
    for i in range(0,len(punti)*(len(punti)-1)):
        text = in_file.readline()
        text=text.split(':')
        key=text[0]
        key=key.split(" ")
        a=key[0].split(',')
        b=key[1].split(',')
        a=tuple([str(a[0]), str(a[1])])
        b=tuple([str(b[0]), str(b[1])])
        argument=text[1]
        argument=argument.split('\n')
        argument=argument[0]
        argument=argument.split(' ')
        arg=[]
        for j in argument:
            # controllo che non sia il costo, che adesso non mi serve
            if j!=argument[-1]:
                incrocio=j.split(',')
                incrocio=tuple([str(incrocio[0]), str(incrocio[1])])
                arg.append(incrocio)

        percorsi[tuple([a, b])]=arg

    in_file.close()
    return punti, percorsi

'''
disegna_soluzione crea uno shapefile per ogni veicolo con il percorso che deve seguire.
Ogni scape file sarà memorizzato in vi con i=0...nveicoli per identificare i diversi
percorsi dei veicoli. Ad ogni arco viene associato un numero incrementale che
rappresenta il passo in cui viene attraversato.
Input:
percorsi: contiene i percorsi (incroci) tra due punti
punti: i punti dei clienti
indici: gli indici, cioè la soluzione del C
n_veicoli: il numero di veicoli a disposizione
Restituisce:
non restituisce niente ma crea gli Shapefile.
'''
def disegna_soluzione(percorsi, punti, indici, n_veicoli):
    # per ogni veicolo controllo che ci siano almeno tre elementi in indici altrimenti
    # vuol dire che il veicolo rimane sempre in stazione: ogni percorso inizia
    # con 0 id_clienti visitati 0 (parte e termina nella sorgente)
    # n_veicoli=1
    print n_veicoli
    for i in range(0,n_veicoli):
        if(len(indici[i])==2):
            continue
        filename="./QGIS/output/v"+str(i)
        Gsol=nx.DiGraph()
        prec=0;
        visita=0
        for j in range(1, len(indici[i])):
            # voglio creare il cammino da punti[precedente] a punti[indici[j]]
            # quindi:
            # - prec corrisponde all'indice del veicolo da cui sto partendo
            # (all'inizio la sorgente - 0)
            # - devo andare al prossimo nodo (che è un cliente) visitato dal mio
            #  veicolo corrente, all'inizio sarà il secondo elemento del mio array
            #  di indici (array di visita del veicolo) recupero il nome completo
            #  da punti
            # - creo la chiave
            key=tuple([punti[prec],punti[indici[i][j]]])
            # devo aggiungere un arco per ogni elenento della lista percorsi con chiave key
            prev_point=percorsi[key][0]
            for k in percorsi[key]:
                if(k==percorsi[key][0]):
                    continue
                Gsol.add_edge(prev_point,k,visita=visita)
                visita=visita+1
                prev_point=k
            # aggiorno precedente, il percorso continua
            prec=indici[i][j]
        nx.write_shp(Gsol,filename)


# parte di main
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),'modello_emissioni'))
from csvreader import model_reader

var=model_reader('./modello_emissioni/modello.csv') #model_reader ritorna una lista di valori ricavati dal modello delle emissioni
var=[str(v) for v in var]
chiamata=[nome_eseguibile]
var=chiamata+var
print var
# time.sleep(3)
tempo=call(var) #chiamata all'eseguibile, seguito dai parametri precedentemente letti
out_file = open("tempi.txt","a")
out_file.write(""+str(tempo)+"\n");
out_file.close()
# recupero il numero dei veicoli
n_veicoli=var[4]
n_veicoli=n_veicoli.split('.')
n_veicoli=n_veicoli[0]
n_veicoli=int(n_veicoli)

# leggo il file di output del C
indici=leggiSol(FileSol, n_veicoli)
print "indici\n",indici
# leggo il file dei percorsi
punti, percorsi=leggi_percorsi(FilePath, indici)
print "punti\n",punti
print "percorsi\n",percorsi
# creo tanti file separati: tanti shapefile quanti sono i veicoli
disegna_soluzione(percorsi, punti, indici, n_veicoli)
