from typing import List, Tuple
from datetime import date, datetime
import os

class CrimeParser():
    def __init__(self, limit) -> None:
        self.DATE_LIMIT = limit
        #const?
        self.date_format = "%d/%m/%Y"
    
    def store_data(self, results, filename):
        if os.path.exists(filename):
            os.remove(filename)
        
        with open(filename, 'a') as crime_file:
            for result in results:
                line = str(result[0]) + ";" + str(result[1][0]) \
                       + ";" + str(result[1][1]) \
                       + ";" + result[2] + "\n"
                crime_file.write(line)
    
    #: List[List[int, Tuple[float, float], str]]
    def filter_old(self, results):
        return filter(self.filter_old_method, results)

    # WARNING: no funciona doncs no podem fer set de una llista
    def filter_same(self, results):
        #return list(set(results))
        return results
    
    def filter_old_method(self, elemet):
        today = date.today().strftime("%d/%m/%Y")
        if (datetime.strptime(elemet[2], self.date_format) - datetime.strptime(today, self.date_format)).days <= self.DATE_LIMIT:
            return True
        else:
            return False
    