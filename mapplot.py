import matplotlib.pyplot as plt
import numpy as np
import mplleaflet

class MapPlotter():
    def __init__(self):
        pass

    def get_map_data(self, filename):
        map_lat, map_long = [], []
        with open(filename, 'r') as f:
            for line in f:
                if line.startswith("#"):
                    # Skip lines that start with #
                    continue
                r_fields = line.split('|')
                fields = list()
                for field in r_fields:
                    fields.append(field.strip())
                map_lat.append(float(fields[1]))
                map_long.append(float(fields[2]))
        return map_lat, map_long
    
    def get_crime_data(self, filename):
        crim_lat, crim_long = [], []
        with open(filename, 'r') as f:
            for line in f:
                fields = line.split(';')
                crim_lat.append(float(fields[3]))
                crim_long.append(float(fields[4]))
        return crim_lat, crim_long
    
    def trace_path(self, map_lat, map_long):
        xy = np.array([[map_long[i],map_lat[i]] for i in range(len(map_lat))])
        # Draw path nodes and lines between nodes
        #fig = plt.figure()
        plt.plot(xy[:,0], xy[:,1], 'b.', markersize=12)
        plt.plot(xy[:,0], xy[:,1], 'c', linewidth=5)
    
    def trace_crimes(self, crim_lat, crim_long):
        xy = np.array([[crim_long[i],crim_lat[i]] for i in range(len(crim_lat))])
        
        #fig = plt.figure()
        plt.plot(xy[:,0], xy[:,1], 'r.', markersize=20)

    def show_map(self, filename):
        mapfile = filename.split('.')[-2]+".html"
        mplleaflet.show(path=mapfile)
        
        
if __name__ == '__main__':
    MapPlt = MapPlotter()
    map_lat, map_long = MapPlt.get_map_data('result.txt')
    MapPlt.trace_path(map_lat, map_long)
    
    crim_lat, crim_long = MapPlt.get_crime_data('crimes.csv')
    MapPlt.trace_crimes(crim_lat, crim_long)
    
    MapPlt.show_map('result.txt')