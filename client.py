import os
import subprocess

import time

from crimdata import SpotCrime, CrimeMapping
from mapdata import MapData
from geodata import GeoUtils
from crimpars import CrimeParser



class CrimeMapClient():
    def __init__(self) -> None:
        self.map_getter = MapData()
        #self.crime_getters = [SpotCrime(), CrimeMapping()]
        #self.spotcrime = SpotCrime()
        self.data_parser = CrimeParser(4)
        self.ITER = 0.02
    
    def get_map_data(self, n, s, e, o):
        self.map_getter.get_data(n, s, e, o)
        self.map_getter.store_data("nodes.csv", "camins.csv")
    
    def get_crime_data(self, address):
        total_results = []
        for crime_getter in self.crime_getters:
            source = crime_getter.get_main_page(address)
            entries = crime_getter.get_crime_entries(source)
            results = crime_getter.parse_crime_entries(entries)
            total_results.extend(results)
        
        return total_results
    
    def parse_crime_data(self, data):
        data = self.data_parser.filter_old(data)
        data = self.data_parser.filter_same(data) # posar abans el filter same?
        self.data_parser.store_data(data, 'tmp.csv')

        # Define max_length
        max_length = 999
        
        # Execute C routine
        subprocess.run(["routine.exe", str(max_length)])
        #print("final")
        return data
    
    def get_boundaries(self, lat1, long1, lat2, long2):
        # returns -> n, s, e, o
        
        # Can re-define boudary box
        dl = max(abs(lat1 - lat2), abs(long1 - long2)) * 0.15
        n = max(lat1, lat2) + dl
        s = min(lat1, lat2) - dl
        e = max(long1, long2) + dl
        o = min(long1, long2) - dl
        
        return n, s, e, o
    
    def trace_route_points(self, lat1: float, long1: float, lat2: float, long2: float):
        # firstly must determine if the line between A and B has a slope greater
        # than 1. In order to determine in which direction shall we iterate
        
        m0 = (lat2 - lat1) / (long2 - long1)
        points = []
        
        if abs(m0) < 1:
            # If the line has a slope less than 1, we will iterate over longitude
            it0 = long1
            st0 = lat1
            lim = abs(long2 - long1) / self.ITER
            m = m0
        else:
            it0 = lat1
            st0 = long1
            lim = abs(lat2 - lat1) / self.ITER
            # Must invert slope since approach from different angle
            m = 1/m0
        for i in range(int(lim)):
            iti = it0 + self.ITER * i
            sti = st0 + self.ITER * i * m
            
            if abs(m0) > 1:
                points.append((iti, sti))
            else:
                points.append((sti, iti))
            
        points.append((lat2, long2))
        return points
        
    def find_closest_route(self, data):
        pass
        
    def main(self):
        address1 = '500 W L St, Wilmington, CA' # important afegir l'estat i la city
        address2 = '1306 E Anaheim St, Wilmington, CA'
        
        # we get it's coordinates, don't compute more than one time
        lat1, long1 = GeoUtils.get_lat_long(address1)
        lat2, long2 = GeoUtils.get_lat_long(address2)
        
        # retrieve the map's boundaries
        n, s, e, o = self.get_boundaries(lat1, long1, lat2, long2)
        
        # retrieve and store the map
        self.get_map_data(n, s, e, o)
        
        points = self.trace_route_points(lat1, long1, lat2, long2)
        
        

if __name__ == "__main__":
    Client = CrimeMapClient()
    address1 = '500 W L St, Wilmington, CA' # important afegir l'estat i la city
    address2 = '1306 E Anaheim St, Wilmington, CA'
    
    
    points = Client.trace_route_points(0, 0, 1, 2)
    
    for point in points:
        print(point)
    
    # calculem fora per a haver-ho de fer només un cop
    #lat1, long1 = GeoUtils.get_lat_long(address1)
    #lat2, long2 = GeoUtils.get_lat_long(address2)
    
    #n, s, e, o = Client.get_boundaries(lat1, long1, lat2, long2)
    #print(n, s, e, o)
    #lt = 33.789955575
    #lng = -118.243220375
    #print(GeoUtils.get_address(lt, lng))
    
    
    #Client.get_map_data(33.79404351372362, 33.782849842522985, -118.25513428109659, -118.282223947399)
    #results = Client.get_crime_data(address)
    #data = Client.parse_crime_data(results)

    #for entry in data:
    #    print(data)
    #source = Client.spotcrime.get_main_page(address1)
    #time.sleep(30)
    #entries = Client.spotcrime.get_crime_entries(source)
    #results = Client.spotcrime.parse_crime_entries(entries)
    #Client.spotcrime.store_crime_entries(results, 'caca')
    
    #CrimeHandler = SpotCrime()
    #source = CrimeHandler.get_main_page(address)
    #entries = CrimeHandler.get_crime_entries(source)
    #results = CrimeHandler.parse_crime_entries(entries)
    #CrimeHandler.store_crime_entries(results, 'caca')
    