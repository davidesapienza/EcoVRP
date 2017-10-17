import random
'''
genera un file contenente 3 colonne di numeri, di cui il primo elemento indica
il numero di clienti (nodi), a seguire invece sono indicati tutti gli archi possibili
tra i clienti (rappresentati da una coppia di interi), con il loro valore di costo
associato (la lunghezza del cammino minimo tra i nodi)
'''
def grafo_random(n_nodi,costo_min,costo_max):
    out_file = open("grafo_random.txt","w")
    out_file.write(""+str(n_nodi)+"\n");
    for i in range(0, n_nodi):
        for j in range(0,n_nodi):
            if i!=j:
                out_file.write(str(i)+" "+str(j)+" "+str(random.uniform(costo_min,costo_max))+"\n")
    out_file.close()
