import random
'''
genera un file contenente una colonna di numeri, di cui il primo elemento indica
il numero di clienti (nodi), a seguire invece ci sono i valori delle richieste di
ciascun cliente (il primo rappresenta il deposito e quindi ha richiesta = 0).
'''
def genera_richieste(n_nodi, capacita):
    out_file = open("file_richieste.txt","w")
    out_file.write(""+str(n_nodi)+"\n0\n");
    for i in range(1, n_nodi):
        out_file.write(str(random.uniform(1,capacita))+"\n")
    out_file.close()
