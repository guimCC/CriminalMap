import json
import os
import matplotlib.pyplot as plt
import numpy as np
import sys
import mplleaflet

# fitxer d'entrada
in_file = sys.argv[1]

# nom del fitxer de sortida: el mateix que l'entrada, canviant extensio per html
mapfile = in_file.split('.')[-2]+".html"
# convertim les dades del fitxer en un array del numpy
latitud  = []
longitud = []

with open(in_file, 'r') as f:
    for line in f:
        if line.startswith("#"): # Ignorem les linies que comencen per #
            continue
        r_fields = line.split('|')
        fields = list()
        for field in r_fields:
            fields.append(field.strip())
        latitud.append(float(fields[1]))
        longitud.append(float(fields[2]))

xy = np.array([[longitud[i],latitud[i]] for i in range(len(latitud))])

# Dibuixem el cami amb punts vermells connectats per linies blaves
fig = plt.figure()
plt.plot(xy[:,0], xy[:,1], 'r.')
plt.plot(xy[:,0], xy[:,1], 'b')

#plt.show()
# Creem el mapa i el guardem amb el nom guardat a mapfile
mplleaflet.show(path=mapfile)
#mplleaflet.display(fig=fig)

#
#go to line and file from the error message (def get_grid_style)
#change axis._gridOnMajor to axis._major_tick_kw['gridOn']
#