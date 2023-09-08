import matplotlib.pyplot as plt
import numpy as np
import mplleaflet

class MapPlotter():
    def __init__(self):
        self.lat, self.long = [], []

    def get_data(self, filename):
        with open(filename, 'r') as f:
            for line in f:
                if line.startswith("#"):
                    # Skip lines that start with #
                    continue
                r_fields = line.split('|')
                fields = list()
                for field in r_fields:
                    fields.append(field.strip())
                self.lat.append(float(fields[1]))
                self.long.append(float(fields[2]))
    
    def trace_path(self):
        xy = np.array([[self.long[i],self.lat[i]] for i in range(len(self.lat))])
        # Draw path nodes and lines between nodes
        fig = plt.figure()
        plt.plot(xy[:,0], xy[:,1], 'r.')
        plt.plot(xy[:,0], xy[:,1], 'b')

    def show_map(self, filename):
        mapfile = filename.split('.')[-2]+".html"
        mplleaflet.show(path=mapfile)
        
        
if __name__ == '__main__':
    MapPlt = MapPlotter()
    MapPlt.get_data('result.txt')
    MapPlt.trace_path()
    MapPlt.show_map('result.txt')