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
        self.crime_getters = [SpotCrime(), CrimeMapping()]
        #self.spotcrime = SpotCrime()
        self.data_parser = CrimeParser(4)
        self.ITER = 0.02
    
    def get_map_data(self, n, s, e, o):
        self.map_getter.get_data(n, s, e, o)
        self.map_getter.store_data("nodes.csv", "roads.csv")
    
    def get_crime_data(self, lat, long):
        total_results = []
        for crime_getter in self.crime_getters:
            source = crime_getter.get_main_page(lat, long)
            entries = crime_getter.get_crime_entries(source)
            results = crime_getter.parse_crime_entries(entries)
            total_results.extend(results)
        
        return total_results
    
    def parse_crime_data(self, data):
        data = self.data_parser.filter_same(data)
        data = self.data_parser.filter_old(data)
        self.data_parser.store_data(data, 'tmp.csv')

        # Define max_length
        max_length = 999
        
        # Execute C routine
        subprocess.run(["routine_filter_and_close.exe", str(max_length)])
        #TODO: Add error handling
    
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
            itS = int((long2 - long1) / abs(long2 - long1))
            st0 = lat1
            stS = int((lat2 - lat1) / abs(lat2 - lat1))
            lim = abs(long2 - long1) / self.ITER
            m = m0
        else:
            it0 = lat1
            itS = (lat2 - lat1) / abs(lat2 - lat1)
            st0 = long1
            stS = (long2 - long1) / abs(long2 - long1)
            lim = abs(lat2 - lat1) / self.ITER
            # Must invert slope since approach from different angle
            m = 1/m0
        #print("it0:", it0, "itS:", itS, "st0:", st0, "stS:", stS, "m:", m)
        for i in range(int(lim)):
            iti = it0 + self.ITER * i * itS
            sti = st0 + self.ITER * i * abs(m) * stS
            
            if abs(m0) > 1:
                points.append((iti, sti))
            else:
                points.append((sti, iti))
            
        points.append((lat2, long2))
        return points
        
    def get_start_end_cords(self, slat, slong, elat, elong):
        # Returns closest node to both the start and end address
        sid = subprocess.run(["routine_get_closest.exe", str(slat), str(slong)],
                             capture_output=True,
                             text=True)
        eid = subprocess.run(["routine_get_closest.exe", str(elat), str(elong)],
                             capture_output=True,
                             text=True)
        return sid.stdout.rstrip(), eid.stdout.rstrip()
    
    def find_closest_route(self, start, end, penalty):
        path = subprocess.run(["routine_closest_route.exe", str(start), str(end), str(penalty)], capture_output=True)
        
    def show_route(self, res_file):
        #TODO: add module not as functionality
        subprocess.run(["python Map_plot.py", ])
    
        
    def main(self, address1, address2):
        
        # we get it's coordinates, don't compute more than one time
        slat, slong = GeoUtils.get_lat_long(address1)
        elat, elong = GeoUtils.get_lat_long(address2)
        
        # retrieve the map's boundaries
        nB, sB, eB, oB = self.get_boundaries(slat, slong, elat, elong)
        
        # retrieve and store the map
        self.get_map_data(nB, sB, eB, oB)
        
        # get points along a straight line to retrieve crime from
        points = self.trace_route_points(slat, slong, elat, elong)
        
        # get crime entries of all the points along the line
        #crime_entries = []
        #for point in points:
        #    crime_entries.extend(self.get_crime_data(point[0], point[1]))
        
        # filter data and relate to closest point
        #self.parse_crime_data(crime_entries)
        
        # get closest ids from start and end
        sid, eid = self.get_start_end_cords(slat, slong, elat, elong)

        self.find_closest_route(sid, eid, 100)
        
        
        

if __name__ == "__main__":
    Client = CrimeMapClient()
    address1 = '500 W L St, Wilmington, CA'
    address2 = '1000 N Fries Ave, Wilmington, CA 90744'
    address3 = '1253 W 213th St, Torrance, CA 90502'
    address4 = '161 W 59th Pl, Los Angeles, CA 90003'
    address5 = '128-196 E 59th Pl, Los Angeles, CA 90003'
    
    Client.main(address4, address5)
    
    
    
    #points = Client.trace_route_points(0, 0, 2, -1)
    
    #for point in points:
    #    print(point)
    
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
    