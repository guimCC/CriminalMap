import os

from crimdata import SpotCrime, CrimeMapping
from mapdata import MapData
from geodata import GeoUtils
from crimpars import CrimeParser



class CrimeMapClient():
    def __init__(self) -> None:
        self.map_getter = MapData()
        self.crime_getters = [SpotCrime(), CrimeMapping()]
        self.spotcrime = SpotCrime()
        self.data_parser = CrimeParser(4)    
    
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
        data = self.data_parser.filter_same(data)
        self.data_parser.store_data(data, 'tmp.csv')

        #TODO: run C program to earase llunyans
        #TODO: run C program to link to most
        #print("final")
        return data


if __name__ == "__main__":
    Client = CrimeMapClient()
    address = '500 W L St, Wilmington, CA'
    
    Client.get_map_data(33.79404351372362, 33.782849842522985, -118.25513428109659, -118.282223947399)
    #results = Client.get_crime_data(address)
    #data = Client.parse_crime_data(results)
    
    #for entry in data:
    #    print(data)
    #source = Client.spotcrime.get_main_page(address)
    #entries = Client.spotcrime.get_crime_entries(source)
    #results = Client.spotcrime.parse_crime_entries(entries)
    #Client.spotcrime.store_crime_entries(results, 'caca')
    
    #CrimeHandler = SpotCrime()
    #source = CrimeHandler.get_main_page(address)
    #entries = CrimeHandler.get_crime_entries(source)
    #results = CrimeHandler.parse_crime_entries(entries)
    #CrimeHandler.store_crime_entries(results, 'caca')
    