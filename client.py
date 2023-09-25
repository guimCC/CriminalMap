import os
import subprocess

import time

from crimdata import SpotCrime, CrimeMapping
from mapdata import MapData
from geodata import GeoUtils
from crimpars import CrimeParser
from mapplot import MapPlotter



class CrimeMapClient():
    def __init__(self, ite, m_length, d_limit, penalty) -> None:
        self.ITER = ite
        self._max_length = m_length 
        self._data_limit = d_limit
        self._penalty = penalty
        self.map_getter = MapData()
        self.crime_getters = [SpotCrime(), CrimeMapping()]
        self.data_parser = CrimeParser(self._data_limit)
        self.map_plotter = MapPlotter()
         
    
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
        print(len(data))
        data = self.data_parser.filter_same(data)
        print(len(data))
        data = self.data_parser.filter_old(data)
        print(len(data))
        self.data_parser.store_data(data, 'tmp.csv')
        
        # Execute C routine
        subprocess.run(["routine_filter_and_close.exe", str(self._max_length)])
        #TODO: Add error handling
    
    def get_boundaries(self, lat1, long1, lat2, long2):
        # returns -> n, s, e, o
        
        # Can re-define boundary box
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
        
    def trace_route(self, res_file):
        map_lat, map_long = self.map_plotter.get_map_data(res_file)
        self.map_plotter.trace_path(map_lat, map_long)
    
    def trace_crimes(self, crim_file):
        crim_lat, crim_long = self.map_plotter.get_crime_data(crim_file)
        self.map_plotter.trace_crimes(crim_lat, crim_long)
    
    def show_map(self, map_file):
        self.map_plotter.show_map(map_file)
    
    def set_penalty(self, n_penalty):
        #TODO: ADD ASSERTIONS (positive...)
        self._penalty = n_penalty
        
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
        crime_entries = []
        for point in points:
            crime_entries.extend(self.get_crime_data(point[0], point[1]))
        
        # filter data and relate to closest point
        self.parse_crime_data(crime_entries)
        
        # get closest ids from start and end
        sid, eid = self.get_start_end_cords(slat, slong, elat, elong)

        self.find_closest_route(sid, eid, self._penalty)
        
        self.trace_route('result.txt')
        self.trace_crimes('crimes.csv')
        
        self.show_map('result.txt')
        

if __name__ == "__main__":
    Client = CrimeMapClient(0.02, 100, 20, 100)
    address1 = '500 W L St, Wilmington, CA'
    address2 = '1000 N Fries Ave, Wilmington, CA 90744'
    address3 = '1253 W 213th St, Torrance, CA 90502'
    address4 = '161 W 59th Pl, Los Angeles, CA 90003'
    address5 = '128-196 E 59th Pl, Los Angeles, CA 90003'
    address6 = '201-257 W Lomita Blvd, Carson, CA 90745'
    address7 = '1367 N Avalon Blvd, Wilmington, CA 90744'
    address8 = '606 W Sepulveda Blvd, Carson, CA 90745'
    address9 = '1325 Bay View Ave, Wilmington, CA 90744'
    address10 = '810 Bay View Ave, Wilmington, CA 90744'
    
    Client.main(address9, address10)

    