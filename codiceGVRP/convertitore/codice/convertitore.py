# -*- coding: utf-8 -*-

# libreria per leggere il grafo dallo shapefile
import networkx as nx
# libreria ausiliaria non utilizzata in questa fase.
# import matplotlib.pyplot as plt
# libreria utilizzata per calcolare la distanza (in m) date due coordinate
from geopy.distance import vincenty
import time
import math
import sys

if len(sys.argv)==3:
    shapefile_edges=sys.argv[1]
    shapefile_client=sys.argv[2]
else:
    # definisco qui i nomi dei file che leggerò
	shapefile_edges="../../QGIS/input/strade/doppiosenso.shp"
	shapefile_client="../../QGIS/input/clienti/punti_20.shp"

FileGraph="../file/grafo_per_algo.txt"
FilePath="../file/percorsi.txt"


'''
check_orientation stabilisce se il nodo cliente deve o non deve essere assegnato
all'arco passato oppure
Input:
-nodo i (il primo)
-nodo j (il secondo) (l'arco considerato è quello da i a j)
-nodo k è il nodo cliente da inserire eventualmente nell'arco. lo si valuta per
capire se cade a sx o dx dell'arco
-orien indica la label associata all'arco i-j. Se l'etichetta è F(from) o T(to),
la stada è a senso unico, allora il cliente deve essere sempre associato
(indipendentemente se cade a sx o dx). Se l'etichetta è B(both) allora devo
valutare dove cade. Prenderò i clienti che cadono sulla destra.
Ritorna:
True se il nodo deve essere associato all'arco
False altrimenti
'''
def check_orientation(i,j,k,orien):
    # se ho un senso unico devo per forza permettere di associare il cliente a
    # questo arco indipendentemente se cade a dx o sx
    if (orien=='F' or orien=='T'):
        return True
    # ricavo tutte le x e y dei punti dell'arco(1,2) e del cliente(3)
    x3=k[0]
    y3=k[1]
    x1=i[0]
    y1=i[1]
    x2=j[0]
    y2=j[1]
    # caso particolare: la funzione della retta passante per i-j è del tipo: X=--
    # in questo caso la trattazione è diversa
    if (x1==x2):
        # controllo la x3 (di k)
        # se minore di quella della retta, e, la y1>y2 allora ritorna true: uso
        # l'arco i-j - cade a dx dell'arco orientato
        if (x3<x1 and y1>=y2):
            return True
        elif (x3<x1 and y1<y2):
            return False
        elif (x3>=x1 and y1>=y2):
            return False
        elif (x3>=x1 and y1<y2):
            return True
    # in tutti gli altri casi invece:
    else:
        # valuto la f(x3) e la confronto con y3
        # la funzione differisce in caso in cui ho una f parallela all'asse x, o
        # il resto dei casi
        if(y1==y2):
            f_x3=y1
        else:
            # valuto la funzione della retta in x3
            f_x3=((y2-y1)/(x2-x1))*(x3-x1)+y1
        # qui controllo dove cade
        if (y3>=f_x3 and x1>=x2):
            return True
        elif (y3>=f_x3 and x1<x2):
            return False
        elif (y3<f_x3 and x1>=x2):
            return False
        elif (y3<f_x3 and x1<x2):
            return True
    print "Se arrivo qui ho sbagliato qualcosa: manca una condizione!"
    return

'''
getGraphClient ricava il grafo della mappa stradale con tanti nuovi nodi quanti
sono i clienti da soddisfare. Ogni cliente viene proiettato sull'arco o incrocio
più vicino (di distanza minima).
Input:
-g_e: grafo degli archi (strade)
-g_n: grafo dei nodi corrispondente ai nuovi clienti (in realtà è la sola parte
riguardante i nodi - parte presa dallo shapefile dei punti)
-liste_ad: lista delle adiacenze di ogni nodo.
Ritorna:
tre liste: la lista delle distanze minime da ogni cliente al segmento/punto più
vicino, la lista dei nuovi punti inseriti nel grafo(proiezione del cliente), la
lista degli archi in cui si è effettuata la proiezione (se esiste).
'''
def getGraphClient(g_e, g_n, liste_ad):
    # trova la proiezione dei clienti sugli incroci più vicini
    d_incrocio=[]
    min_incrocio=[]
    count_nodo=0
    for i, nattr in g_n.nodes(data=True):
        d_incrocio.append(None)
        min_incrocio.append(None)
        for j, nattr_edge in g_e.nodes(data=True):
            # teorema di pitagora: distanza tra due punti
            d=math.sqrt( (i[0]-j[0])*(i[0]-j[0]) + (i[1]-j[1])*(i[1]-j[1]) )
            # se miglioro, cambio nodo
            if (d_incrocio[count_nodo]==None or d_incrocio[count_nodo]>d):
                d_incrocio[count_nodo]=d
                min_incrocio[count_nodo]=j
        count_nodo+=1

    # dopo calcolo la distanza del nodo cliente dal segmento (arco)
    # vado a considerare solo le distanze perpendicolari
    # quindi se la perpendicolare alla retta (corrispondente al segmento dell'arco)
    # va a finire fuori dal segmento, allora non lo considero
    # avrò una distanza minima per ogni nodo cliente (al più rimarrà None) e
    # memorizzerò anche la coppia di cordinate corrispondenti all'arco
    d_arco=[]
    min_arco=[]
    count_nodo=0
    new_node=[]
    for k,nattr in g_n.nodes(data=True):
        x3=k[0]
        y3=k[1]
        d_arco.append(None)
        min_arco.append(None)
        new_node.append(None)
        # non scorro più il grafo degli archi ma la lista delle adiacenze.
        for i in liste_ad.keys():
            x1=i[0]
            y1=i[1]
            for j in liste_ad[i]:
                x2=j[0]
                y2=j[1]

                # qui devo calcolare la distanza tra nodo k e arco i-j
                # calcolo il coefficiente angolare della retta dell'arco
                m=(j[1]-i[1])/(j[0]-i[0])
                # il coef angolare della retta perpendicolare è -1/m
                # m_p=-(1/m)

                # trovo la retta con coef angolare -1/m che passa per k(cliente)
                # trovo punto intersezione tra retta arco e retta perpendicolare
                # a sistema metto la retta perpendicolare con la retta dell'arco
                x=(x3/(m*m)+y3/m-y1/m+x1)/(1+1/(m*m))
                y=x3/m-x/m+y3

                # ALTRO METODO PER CALCOLARLO
                # (FORSE C'è DA STARE ATTENTI ALL'APPROSSIMAZIONE DEL PC)
                # print "calcolo senza m in mezzo:"
                # x2_x1=x2-x1
                # y2_y1=y2-y1
                #
                # x=( (x2_x1/y2_y1)*(x2_x1/y2_y1)*x3 + (x2_x1/y2_y1)*y3
                #    - (x2_x1/y2_y1)*y1 + x1 )/(1+ ( x2_x1/y2_y1 )*( x2_x1/y2_y1 ))
                # y=( (x2_x1/y2_y1)*x3 - (x2_x1/y2_y1)*x + y3 )

                # controllo che il punto rientri nel segmento
                if (not((x<x1 and x<x2) or (x>x1 and x>x2) or (y<y1 and y<y2) or (y>y1 and y>y2))):
                    # PROBLEMA: il grafo di networkx.
                    # So che l'arco esiste ed è da i a j.. ma nel grafo potrebbe
                    # esiste solo come j-i con campo oneway=T..
                    # idea: metto in un try.. se non va bene, allora prova in
                    # verso contrario..
                    # -in questo modo però devo girare nella chiamata anche i e j
                    # (primi due parametri) così che ottengo lo stesso risultato
                    try:
                        bool_orien=check_orientation(i,j,k,g_e.edge[i][j]['oneway'])
                    except KeyError:
                        try:
                            # nel grafo quindi esiste l'arco j-i. io so però che l'orientamento
                            # è i-j.. se adesso chiamo check_orientation con i e  j
                            # scambiati, allora non devo più prendere il cliente se cade a dx,
                            # ma lo devo prendere se cade a sx,. sempre che la strada sia a doppio
                            # senso di circolazione.. se senso unico lo prendo comunque.
                            bool_orien=check_orientation(j,i,k,g_e.edge[j][i]['oneway'])
                            # devo invertire il risultato solo in caso in cui la strada sia
                            # a doppio senso di circolazione.. cioè se esiste la lista
                            # delle adiacenze j-->i
                            try:
                                if i in liste_ad[j]:
                                    if bool_orien:
                                        bool_orien=False
                                    else:
                                        bool_orien=True
                            except KeyError:
                                # se non c'è allora non inverte il risultato: va bene
                                pass
                        except KeyError:
                            print "c'è qualcosa che manca nel ragionamento dsopra descritto.."

                    # se posso associare il nodo al segmento:
                    if(bool_orien):
                        # se il punto cade dentro al segmento allora calcola
                        # distanza/altezza tra cliente e segmento
                        d=math.sqrt( (x-x3)*(x-x3) + (y-y3)*(y-y3) )
                        # fai controllo ed eventualmente aggiorna d_arco e min_arco
                        if (d_arco[count_nodo]==None or d_arco[count_nodo]>d):
                            d_arco[count_nodo]=d
                            min_arco[count_nodo]=[i,j]
                            new_node[count_nodo]=[x,y]
        count_nodo+=1

    # trovo la soluzione migliore tra gli incroci e le strade.
    count_nodo=0
    d_sol=[]
    edges_sol=[]
    new_node_sol=[]
    for i in xrange(g_n.number_of_nodes()):
        edges_sol.append(None)
        if d_incrocio[count_nodo]<d_arco[count_nodo]:
            d_sol.append(d_incrocio[count_nodo])
            new_node_sol.append(min_incrocio[count_nodo])
            # non aggiorno l'arco perchè corrisponde all'incrocio
        else:
            d_sol.append(d_arco[count_nodo])
            new_node_sol.append(new_node[count_nodo])
            edges_sol[count_nodo]=min_arco[count_nodo]

        count_nodo+=1
    return d_sol, new_node_sol, edges_sol


'''
getDictAd calcola la lista delle adiacenze per ogni nodo.
Input:
-g:il grafo di cui si vuole ottenere la lista delle adiacenze
'''
def getDictAd(g):
    liste_ad={}
    set_visita=set()
    prima_non_in=0
    prima_in=0
    for i, js in g.adjacency_iter():
        for j,eattr in js.items():
            orien=eattr['oneway']
            d=math.sqrt( (i[0]-j[0])*(i[0]-j[0]) + (i[1]-j[1])*(i[1]-j[1]) )
            # differenzio il comportamento per il campo oneway: from, to, both
            if(orien == 'F'):
                if(i not in set_visita):
                    set_visita.add(i)
                    liste_ad[i]=[j]

                else:
                    liste_ad[i].append(j)

            elif(orien == 'T'):
                # aggiungo l'arco al grafo --> imposto F perchè memorizzo j-i
                # quindi from j to i new_g.add_edge(j,i, oneway='F')

                # se i non e' ancora stato visitato neanche una volta
                if(j not in set_visita):
                    set_visita.add(j)
                    liste_ad[j]=[i]
                else:
                    liste_ad[j].append(i)
            # in questo caso in entrambi i versi
            else:
                # genero entrambi gli archi
                if(i not in set_visita):
                    set_visita.add(i)
                    liste_ad[i]=[j]
                else:
                    liste_ad[i].append(j)
                if(j not in set_visita):
                    set_visita.add(j)
                    liste_ad[j]=[i]
                else:
                    liste_ad[j].append(i)

    return liste_ad, set_visita

'''
list2graph converte una lista di adiacenze in un grafo orientato inserendo
come attributo dell'arco il peso dell'arco.
Input:
-liste_ad: lista delle adiacenze
-dist: lista dei pesi.
'''
def list2graph(liste_ad, dist):
    Gsol=nx.DiGraph()
    for i in liste_ad:
        for j in liste_ad[i]:
            leng=None
            if (dist != None):
                indice=liste_ad[i].index(j)
                leng=dist[i][indice]
            Gsol.add_edge(i,j, length=leng)
    return Gsol

'''
insertiNewPoint inserisce i nuovi punti (clienti proiettati) nel grafo e nella
lista delle adiacenze, con tutto quello che ne consegue (aggiornamento delle
liste delle adiacenze dei nodi dell'arco in cui li inserisco, creazione nuovo punto
e sua lista di adiacenze).
Input:
-newpoint: i nuovi punti da inserire
-edges: gli archi in cui inserire i nodi
-liste_ad: lista delle adiacenze da aggiornare
Restituisce:
-lista delle adiacenze aggiornata
'''
def insertiNewPoint(newpoint, edges, liste_ad):
    # costruisco nuovo grafo inserendo questi nuovi punti
    Gsol=nx.Graph()

    for i in xrange(len(newpoint)):
        Gsol.add_node(newpoint[i])
        if edges[i]!=None:
            # aggiungo i
            Gsol.add_node(edges[i][0])
            # aggiungo j
            Gsol.add_node(edges[i][1])
            # aggiungo arco tra i e new point
            Gsol.add_edge(edges[i][0],newpoint[i])
            # aggiungo arco tra new point e j
            Gsol.add_edge(newpoint[i],edges[i][1])

    nx.write_shp(Gsol,'../soli_nuovi_punti')
    # qui inserisco un controllo negli archi per verificarte se due nodi cadono
    # sullo stesso segmento. gli archi che ho sono unici, orientati:
    # sia che sia senso unico, sia che non lo sia, ogni punto può essere associato
    # ad un singolo arco.. se mi ritrovo più punti dello stesso arco, mi serve
    # sapere quanti appartengono allo stesso arco, chi cade prima di chi, e a che distanza..
    # in base a questo, aggiorno le liste di adiacenza..
    # creo lista di nodi che cadono sullo stesso segmento
    l_id_stess=[]
    l_visita=[];
    for i in xrange(len(newpoint)):
        l_visita.append(False)

    for i in xrange(len(newpoint)):

        # inserisco nuovo punto, anche se già presente fa lo stesso.
        # il set toglie duplicati
        set_visita.add(newpoint[i])

        # se il cliente cade sull'incrocio, allora controllo se ci sono più clienti
        # per lo stesso incrocio. se ci sono devo creare la lista delle adiacenze
        # in entrambi i versi. So che il primo nodo corrisponde all'incrocio, gli
        # altri differenzieranno (eventualmente se esistono) di poco
        if edges[i]==None:
            # scorro tutti i nodi per recuperare gli indici di quelli molto simili
            # uno con l'altro differenziano di 0.00000000000001
            # metto una tolleranza di 100 : 100 nodi sullo stesso punto della mappa
            # stima forse sufficiente
            for j in xrange(len(newpoint)):
                # se j diverso da i
                # se anche lui non ha arco
                # se la prima coordinata è uguale
                # se la seconda differenzia di poco o in positivo o negativo
                if j!=i and edges[j]==None and newpoint[i][0]==newpoint[j][0] and ((float(newpoint[i][1])+0.000000000001)>float(newpoint[j][1]) or (float(newpoint[i][1])-0.000000000001)<float(newpoint[j][1])):
                    # allora crea lista adiacenze:
                    # creo la lista delle sole adiacenze di i. In realtà essendo
                    # punti molto vicini dovrei creare anche la lista al contrario.
                    # in realtà però il punto j lo visiterò successivamente e lì
                    # creerò la lista di j che punta ad i.
                    liste_ad[newpoint[i]].append(newpoint[j])
        # se invece l'arco è presente..
        else:
            # devo trovare tutti i punti con uno stesso arco
            # se già visitato allora lo salto
            if l_visita[i]==True:
                continue
            # trovo tutti gli id dei nuovi punti che hanno stesso arco.
            l_id_stess.append(i)
            l_visita[i]=True;
            # cerco tutti gli indici di quelli che cadono nello stesso segmento
            for j in xrange(len(newpoint)):
                if i!=j and edges[i]==edges[j]:
                    l_id_stess.append(j)
                    l_visita[j]=True
            # adesso li devo ordinare per ordine di visita.
            lista_ordinata=[]
            prima_coord=edges[i][0]
            for j in l_id_stess:
                lista_ordinata.append(newpoint[j])
            # ordina prima per la prima cordinata, e dopo per la seconda.
            # ordina però per grandezza del valore
            sorted(lista_ordinata)
            # controllo anche se l'arco è ordinato
            # se non lo è allora la lista ordinata di punti la inverto, così
            # dovrei già avere l'ordine di visita dell'altra parte

            if(edges[i]!=sorted(edges[j])):
                lista_ordinata=lista_ordinata[::-1]

            # trovo l'indice del secondo estremo dell'arco nella lista di adiacenze
            # del primo estremo e lo devo togliere perchè ora deve puntare al nuovo
            # punto
            index=liste_ad[edges[i][0]].index(edges[i][1])
            lista=liste_ad[edges[i][0]]
            del(lista[index])
            # tolgo completamente la lista delle adiacenze di i e rinserisco la lista
            # aggiornata
            del(liste_ad[edges[i][0]])
            # aggiungo il primo cliente
            liste_ad[edges[i][0]]=lista
            # devo scorrere per ogni nodo con stesso segmento.
            # parto dal primo estremo dell'arco
            primo=edges[i][0]
            for j in xrange(len(l_id_stess)):
                # se non è l'estremo del segmento (primo punto),
                # allora inizializza lista adiacenze a nulla
                if j!=0:
                    liste_ad[primo]=[]
                # inserisco il nuovo punto nella lista di adiacenze del precedente
                liste_ad[primo].append(lista_ordinata[j])
                primo=lista_ordinata[j]
            # una volta in fondo mi rimane solo l'altro capo dell'arco
            liste_ad[primo]=[edges[i][1]]
            # dopodichè guardo al contrario per verificare se ci sono nodi vicini e quindi
            # creare anche l'arco inverso per quei nodi che distano < 20m dal successivo
            lista_ordinata=lista_ordinata[::-1]
            primo=edges[i][1]
            for j in xrange(len(l_id_stess)):
                # se la distanza tra uno e l'altro è inferiore a 20 m
                # allora costruisco un arco a doppio senso
                d=vincenty(primo, lista_ordinata[j]).meters
                # se sono vicini allora anche dal mio nodo posso andare al precedente
                if d<=20:
                    liste_ad[lista_ordinata[j]].append(primo)
                # aggiorno primo al mio attuale nodo
                primo=lista_ordinata[j]

        l_id_stess=[]

    # ricostruisco il nuovo grafo dopo l'inserimento delle proiezioni dei clienti
    Gsol=nx.Graph()
    Gsol=list2graph(liste_ad, None)
    nx.write_shp(Gsol,'../con_clienti')
    return liste_ad


'''
getDistances calcola tutte le distanze tra coppie di coordinate.
Input:
-la lista delle adiacenze di ogni nodo
Ritorna:
-un dizionario con ogni distanza (chiave il nodo), valore:la lista delle distanze
corrispondente ad ogni nodo (in ordine) rispetto alla lista delle adiacenze
'''
def getDistances(liste_ad):
    # ricavo le distanze per ogni segmento, così quando andrò a trovare il grafo
    # ridotto, l'arco corrispondente a più segmenti avrà la somma dei costi di tutti.
    distances={}
    set_visita=set()
    for key in liste_ad:
        for elem in liste_ad[key]:
            # invoco la funzione di libreria per ottenere la distanza in metri
            d=vincenty(key, elem).meters
            if key not in set_visita:
                set_visita.add(key)
                distances[key]=[d]
            else:
                distances[key].append(d)

    return distances

'''
getGradi ricava i gradi di ogni nodo: crea un dizionario in cui ho come chiave
il nodo, come lista uno dei nodi che lo punta, e il grado complessivo
Input:
-liste_ad: liste di adiacenza di ogni nodo
Ritorna:
il dizionario creato
'''
def getGradi(liste_ad):
    # gradi è un dizionario: chiave il nodo, attributi un nodo che lo punta e il grado
    gradi={}
    gradi= {elem:[elem, len(liste_ad[elem])] for elem in liste_ad}
    for key in liste_ad:
        for elem in liste_ad[key]:
            try:
                gradi[elem]=[key, gradi[elem][1]+1]

            except KeyError:
                # questi sono nodi finali: hanno solo nodi entranti e non hanno nodi uscenti
                pass
    return gradi

'''
getRidotto produce un grafo ridotto, in cui si è effettuata la semplificazione
sui segmenti. Le semplificazioni sono di due tipi:
1) sui nodi con grado due e un solo arco uscente
2) sui nodi con grado 4 e due archi uscenti. Questi due nodi puntati però devono
avere nella loro lista di adiacenze il nodo in esame.
Input:
-grado: dizionario con il grado di ogni nodo
-liste_ad: liste delle adiacenze di ogni nodo
-newpoint: i nuovi punti da inserire
-distances: dizionario delle distanze: costi
Ritorna:
la lista delle adiacenze aggiornata e le distanze aggiornate (dopo la riduzione).
Ritorna anche il grafo corrispondente alla riduzione
'''
def getRidotto(gradi, liste_ad, newpoint, distances):
    liste_ad1=liste_ad
    for key in gradi:
        # devo eliminare i nodi dei segmenti:
        # di quegli archi che hanno grado due e di cui uno è uscente,
        # qi quelli che però non sono clienti,
        if gradi[key][1]==2 and len(liste_ad1[key])==1 and key not in newpoint:
            # controllo che non siano strade chiuse
            check=True
            try:
                if key in liste_ad1[liste_ad1[key][0]]:
                    check=False
            except KeyError:
                check=True
            if check==True:
                # creo lo schema con i punti A, B, C (miglior leggibilità)
                B=key
                A=gradi[B][0]
                C=liste_ad1[B][0]
                liste_ad1[A].append(C)
                # cancello B dalla lista delle adiacenze di quello che lo puntava (a cui ho appena
                # aggiunto la lista delle adiacenze di B)
                list_predec=liste_ad1[A]
                list_distan=distances[A]
                pos_b=0
                for i in list_predec:
                    if i!=B:
                        pos_b+=1
                    else:
                        break
                # ho trovato la posizione in cui era presente il nodo che sto cancellando.
                # devo aggiornare distances. dato che ho appeso C alla lista delle adiacenze di A,
                # devo appendere anche la sua distanza in distances. questa distanza è la somma tra
                # la distanza di A da B, e di B a C.
                # da B a C ce l'ho! distances[B][0] (0 perchè ha un solo arco uscente)
                # da A a B adesso ce l'ho! distances[A][pos_b]
                # dopodichè dato che elimino B, devo eliminare anche la sua distanza
                list_distan.append(distances[B][0]+list_distan[pos_b])
                del(list_distan[pos_b])
                # cancello il nodo
                del(list_predec[pos_b])
                distances[A]=list_distan
                liste_ad1[A]=list_predec
                # setto A come nodo che punta a C
                try:
                    gradi[C][0]=A

                except KeyError:
                    pass
                del(liste_ad1[B])
                del(distances[B])
        # devo eliminare anche i nodi:
        # di quelli che hanno grado 4 ma due uscenti verso i due entranti
        if gradi[key][1]==4 and len(liste_ad1[key])==2 and key not in newpoint:
            # adesso devo controllare che entrmbi i nodi puntati,
            # abbiano nella loro lista di adiacenze key.
            check=False
            try:
                if key in liste_ad1[liste_ad1[key][0]] and key in liste_ad1[liste_ad1[key][1]]:
                    check=True
            except KeyError:
                check=False
            if check==True:
                # vuol dire che key deve sparire da entrmabe le liste di adiacenza
                # e che in entrmabe le liste di adiacenza, deve finire l'altro punto
                B=key
                A=liste_ad[B][0]
                C=liste_ad[B][1]
                liste_ad1[A].append(C)
                liste_ad1[C].append(A)
                # cancello B dalla lista delle adiacenze di quello che lo puntava
                # -A- (a cui ho appena aggiunto la lista delle adiacenze di B)
                list_predec=liste_ad1[A]
                list_poster=liste_ad1[C]
                list_distanA=distances[A]
                list_distanC=distances[C]
                # prima faccio per A
                pos_b=0
                for i in list_predec:
                    if i!=B:
                        pos_b+=1
                    else:
                        break
                # ho trovato la posizione in cui era presente il nodo che sto cancellando
                # aggirono distances, come prima..
                list_distanA.append(distances[B][1]+list_distanA[pos_b])
                del(list_distanA[pos_b])
                # cancello il nodo
                del(list_predec[pos_b])
                distances[A]=list_distanA
                liste_ad1[A]=list_predec
                try:
                    gradi[C][0]=A

                except KeyError:
                    pass
                # poi faccio per C
                pos_b=0
                for i in list_poster:
                    if i!=B:
                        pos_b+=1
                    else:
                        break
                # ho trovato la posizione in cui era presente il nodo che sto cancellando
                list_distanC.append(distances[B][0]+list_distanC[pos_b])
                del(list_distanC[pos_b])
                # cancello il nodo
                del(list_poster[pos_b])
                distances[C]=list_distanC
                liste_ad1[C]=list_poster
                try:
                    gradi[A][0]=C

                except KeyError:
                    pass
                # cancello B solo alla fine
                del(liste_ad1[B])
                del(distances[B])
    Gsol=nx.DiGraph()
    Gsol=list2graph(liste_ad1, distances)
    nx.write_shp(Gsol,'../ridotto')
    return liste_ad1, distances, Gsol

'''
getPercorsi recupera i percorsi minimi da ogni nodo verso ogni altro nodo.
Viene utilizzata la funzione shortest_path della libreria networkx.
Appende anche il costo del cammino come ultimo elemento della lista
Input:
newpoint:i nuovi punti inseriti
Gsol: il grafo ridotto
Restituisce:
i percorsi minimi con i costi
'''
def getPercorsi(newpoint, Gsol):
    percorsi={}
    for i in newpoint:
        for j in newpoint:
            if (i!=j):
                p=nx.shortest_path(Gsol, source=i, target=j, weight="length")
                p_l=nx.shortest_path_length(Gsol, source=i, target=j,weight="length")
                p.append(p_l)
                percorsi[tuple([i,j])]=p
    return percorsi

'''
getIndexNewpoint2Str, dato il nome(coordinate) di un nodo, mi dice il suo indice
corrispondente nella lista newpoint, questo indice lo converte in stringa.
Questo procedimento mi serve per avere un nome univoco di un nodo in un formato
più semplice. Lo scopo è scrivere su file un insieme di numeri: 0,1,.. che sia
utilizzabile più facilmente dal modulo in C.
Input:
nodo: la coppia di coordinate corrispondente al nodo
newpoint: la lista dei nuovi punti (quelli proiettati)
Restituisce una stringa corrispondente all'indice della lista newpoint che contiene
nodo.
'''
def getIndexNewpoint2Str(nodo, newpoint):
    return str(newpoint.index(nodo))

'''
scriviFile scrive il file di testo per il modulo in C. scrive la coppia del percorso
sorgente-destinazione con il costo ad esso associato
Input:
name: il nome del file da scrivere
percorsi: il percorso (da cui si ottiene sorgente e destinazione) e il costo del cammino
newpoint: i nuovi punti. i clienti che devo visitare
'''
def scriviFile(name, percorsi, newpoint):
    # scrivo un file di testo
    out_file = open(name,"w")
    # out_file.write("This Text is going to out file\nLook at it and see\n")
    out_file.write(str(len(newpoint))+"\n")
    for i in percorsi:
        out_file.write(getIndexNewpoint2Str(i[0], newpoint)+" ")
        out_file.write(getIndexNewpoint2Str(i[1], newpoint)+" ")
        out_file.write(str(percorsi[i][-1])+"\n")
    out_file.close()

'''
scriviPercorsi scrive il file completo dei percorsi. Questo file sarà letto da un
altro modulo in python per rappresentare la soluzione sulla mappa GIS.
Input:
name: il nome del file da scrivere
percorsi: i percorsi completi da ogni cliente ad ogni altro cliente con il relativo costo
newpoint: i punti clienti
'''
def scriviPercorsi(name, percorsi, newpoint):
    # scrivo un file di testo
    out_file = open(name,"w")
    # prima scrivo i punti in ordine di indice
    for i in newpoint:
        v1=float(i[0])
        v2=float(i[1])
        out_file.write(str(v1)+" "+str(v2)+"\n")
    out_file.write("\n")
    # li scrivo nello stesso ordine rispetto all'altro file.
    # quando leggerò questo file, saprò gli indici corrispondenti: sono la soluzione del C
    for i in percorsi:
        v1=float(i[0][0])
        v2=float(i[0][1])
        v3=float(i[1][0])
        v4=float(i[1][1])
        out_file.write(str(v1)+","+str(v2)+" "+str(v3)+","+str(v4)+":")
        for j in percorsi[i]:
            # se j è il costo finale
            if j==percorsi[i][-1]:
                out_file.write(str(j))
            else:
                p1=float(j[0])
                p2=float(j[1])
                out_file.write(str(p1)+","+str(p2)+" ")
        out_file.write("\n")
    out_file.close()



# leggo il grafo degli archi
Garchi=nx.read_shp(shapefile_edges, False)
# ottengo la lista delle adiacenze per ogni nodo
liste_ad={}
set_visita=set()
liste_ad, set_visita = getDictAd(Garchi)

# leggo il grafo dei clienti
Gpunti=nx.read_shp(shapefile_client)
Gsol=nx.Graph()
Gsol=Gpunti
nx.write_shp(Gsol,'../clienti_originari')

# ottengo le distanze, i nuovi punti e gli archi corrispondenti dell'unione tra il
# grafo delle strade e i nuovi punti proiettati
d_newnode,newpoint, edges_sol=getGraphClient(Garchi, Gpunti, liste_ad)

# potrei avere i nuovi punti di più clienti che combaciano..
# in questo caso, dato che accedo alla lista delle adiacenze con la coordinata
# del punto, non riuscirei ad esprimere il fatto che ho clienti diversi..
# allora aggiusto leggermente il cliente modificandogli una delle cifre
# meno significativa della sua coordinata..
# per fare un lavoro fatto bene dovrei spostare di poco il cliente ma rimandendo
# su una strada.. problema: quei clienti che cadono sull'incrocio, è difficile
# fare in modo che caschino su una strada
# per ora allora non lo considero.
for i in xrange(len(newpoint)):
    for j in range(i+1,len(newpoint)):
        if newpoint[i]==newpoint[j]:
            newpoint[j]=(newpoint[j][0],newpoint[j][1]+0.00000000000001)

# casto i nuovi punti e gli archi in un array di tuple
newpoint=[tuple(x) for x in newpoint]
# edges_sol=[tuple(x) for x in edges_sol]
edges_sol_tuple=[]
for i in edges_sol:
    if i==None:
        edges_sol_tuple.append(i)
    else:
        edges_sol_tuple.append(tuple(i))
edges_sol=edges_sol_tuple
# inserisco i nuovi punti
liste_ad=insertiNewPoint(newpoint,edges_sol, liste_ad)

# ricavo le distanze tra i nodi
distances={}
distances=getDistances(liste_ad)

# recupero il grado di ogni nodo (numero archi incidenti)
gradi={}
gradi=getGradi(liste_ad)

# riduce il grafo
liste_ad, distances, Gsol=getRidotto(gradi, liste_ad, newpoint, distances)

# recupero i percorsi minimi da ogni cliente ad ogni altro cliente
percorsi={}
percorsi=getPercorsi(newpoint, Gsol)

# scrivo il file di testo necessario al modulo in C per applicare l'algoritmo di
# rountin


scriviFile(FileGraph, percorsi, newpoint)

# scrivo il file con i percorsi interi
scriviPercorsi(FilePath, percorsi, newpoint)
