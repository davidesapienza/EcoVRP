from subprocess import call
import csv
import math
'''
funzione che legge da csv e inserisce in un dizionario i parametri definiti dal modello
di emissioni e li aggrega in modo compatto, in modo da poterli passare in input alla routine in linguaggio c++.
I valori da ricavare con i calcoli sono 3:
il coefficiente che moltiplica la distanza,
il costo del trasporto a vuoto ed
il coefficiente che moltiplica il carico.
Gli altri valori invece vengono letti e passati direttamente.
La descrizione piu dettagliata del csv e' presente nel file 'modello-descrizione'
'''
def model_reader(path_modello):
    reader = csv.reader(open(path_modello, 'r'))
    d = {}
    d=dict(reader)
    for sub in d:
        d[sub] = float(d[sub])

    phi1=d['OMEGA']+d['g']*math.sin(d['THETA'])+d['g']*d['Cr']*math.cos(d['THETA'])
    phi2=0.5*d['Cd']*d['DELTA']*d['A']
    chi1=1/(1000.*d['ETA']*d['CHI'])
    kappa1=d['XI']/(d['KAPPA']*d['PSI'])

    print 'phi1=',phi1
    print 'phi2=',phi2
    print 'chi1=',chi1

    val0=kappa1/d['S']
    val1=d['h']*d['Z']*d['V']+d['W']*chi1*d['S']*phi1
    val2=d['PHI']*chi1*phi1*d['S']
    val3=phi2*chi1*d['S']*d['S']*d['S']

    return [val0,val1+val3,val2,d['N_VEICOLI'],d['CAPACITA'],d['MAX_ITER'],d['LUN_TABU_INIZIALE'],d['MAX_TABU'],d['FREQUENZA_AGG']];
