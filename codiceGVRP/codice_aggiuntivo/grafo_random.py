import random
def grafo_random(n_nodi,costo_min,costo_max)
    out_file = open("test.txt","w")
    out_file.write(""+str(n_nodi)+"\n");
    for i in range(0, n_nodi):
        for j in range(0,n_nodi):
            if i!=j:
                out_file.write(str(i)+" "+str(j)+" "+str(random.uniform(costo_min,costo_max))+"\n")
    out_file.close()
