import time
from datetime import datetime
from abc import ABC, abstractmethod
import urllib.parse
import os
from typing import Tuple

from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from bs4 import BeautifulSoup

from geodata import GeoUtils

TIMER = 0.5

class CrimeGetter(ABC):
    def __init__(self) -> None:
        options = webdriver.ChromeOptions()
        options.add_argument('--ignore-certificate-errors')
        options.add_argument('--incognito')
        options.add_argument('--headless')
        #fixed size so always same scope
        options.add_argument('--window-size=1000,1000')
        options.add_argument('--log-level=3')
        #path = os.getcwd() + "\chrome\chromedriver"
        #print(path)
        #options.add_argument('--executable_path='+path)
        #options.add_argument('--start-fullscreen')
        
        self.driver = webdriver.Chrome(service=Service(), options=options)

    def get_main_page(self, lat: float, long: float):
        """

        Args:
            address (str): Address, including STATE, example: 123 STREET, CA
        """
        self.state = GeoUtils.get_state(lat, long)
    
    @abstractmethod
    def get_crime_entries(self):
        pass
    
    def parse_crime_entries(self, elements):
        """ 
        Parameters: Elements [raw_type, address, time]
        
        Adds self.state
        
        Turns into [gravity, (lat, long), time]
        """
        #transformar time a comú
        for i, element in enumerate(elements):
            new_element = (self.set_gravity_level(element[0].lower()), GeoUtils.get_lat_long(element[1]), datetime.strptime(element[2], self.date_format).strftime('%d/%m/%Y'))
            elements[i] = new_element
        return elements
        
    #realment no ens caldrà pq passarem per python i no borrarem aquí
    def store_crime_entries(self, elements, filename):
        #borrem fitxer de dades si cal
        for element in elements:
            print(element)
        return
        #TODO: això només ho farem un cop abans de començar
        if os.path.exists(filename):
            os.remove(filename)
        
        with open(filename, 'a') as crime_file:
            for element in elements:
                crime_file.write(element)

    def set_gravity_level(self, element):
        if "vehicle" in element or "driving" in element:
            return 6
        elif "assault" in element or "domestic" in element:
            return 7
        elif "theft" in element or "shoplifting" in element:
            return 6
        elif "burglary" in element:
            return 4
        elif "robbery" in element or "carjacking" in element:
            return 4
        elif "arson" in element:
            return 7
        elif "vandalism" in element:
            return 5
        elif "arrest" in element:
            return 3
        elif "shooting" in element or "weapon" in element:
            return 10
        elif "homicide" in element:
            return 10
        elif "conduct" in element or "fight" in element or "drunk" in element or "trespassing" in element:
            return 6
        elif "rape" in element or "prostitution" in element or "ofenses" in element:
            return 9
        elif "drug" in element or "narcotic" in element:
            return 8
        else:
            return 5
        

class SpotCrime(CrimeGetter):
    def __init__(self) -> None:
        super().__init__()
        self.date_format = "%m/%d/%Y %H:%M %p"
    
    def get_main_page(self, lat: float, long: float) -> "webdriver.Chrome().page_source":
        super().get_main_page(lat, long)
        #open the main page
        self.driver.get("https://spotcrime.com/map?lat={}&lon={}&address=".format(lat, long))
        time.sleep(TIMER)
        # try to click on accept cookies
        try:
            self.driver.find_element(By.XPATH, "/html/body/div[2]/div[2]/div[1]/div[2]/div[2]/button[1]").click()
            time.sleep(TIMER)
        except:
            #fer un proper raise.
            print("Element not found")
            #raise html element not found
        """ # try to open crime list
        try: 
            driver.find_element(By.XPATH, "/html/body/main/div[3]/div/button[1]").click()
            time.sleep(3)
        except:
            print("Element not found")
            # raise html element not found """

        return self.driver.page_source

    def get_crime_entries(self, page_source: "webdriver.Chrome().page_source") -> Tuple[str, str, str]:
        
        soup = BeautifulSoup(page_source, 'lxml')

        elements = []

        element_selector = soup.find_all('a', class_='map-page__crime-list__card')

        for element in element_selector:
            element_title = element.find_all('span', class_='map-page__crime-list__crime-card-title')[0].get_text()
            element_address = element.find_all('span', class_='map-page__crime-list__crime-card-address')[0].get_text()
            element_date = element.find_all('span', class_='map-page__crime-list__crime-card-date')[0].get_text()
            
            elements.append((element_title, element_address + ", {}".format(self.state), element_date))
            
        return elements
    
class CrimeMapping(CrimeGetter):
    def __init__(self) -> None:
        super().__init__()
        self.date_format = "%m-%d-%Y %H:%M %p"
        
    def get_main_page(self, lat: float, long: float) -> "webdriver.Chrome().page_source":
        super().get_main_page(lat, long)
        address = GeoUtils.get_address(lat, long)
        
        #open the main page
        parsed_address = urllib.parse.quote(address)
        self.driver.get("https://www.crimemapping.com/map/location/{}".format(parsed_address))
        #print("https://www.crimemapping.com/map/location/{}".format(parsed_address))
        time.sleep(TIMER*4)
        #time.sleep(2)
        # we will perform a zoom out via Ctrl + minus

        act = ActionChains(self.driver)
        act.key_down(Keys.CONTROL).send_keys('-').key_up(Keys.CONTROL).perform()
        time.sleep(TIMER*4)
        #time.sleep(20)
        # try to click on accept cookies
        try:
            self.driver.find_element(By.XPATH, "/html/body/div[3]/div[10]/ul/li[6]/a").click()
            time.sleep(TIMER*4)
            #time.sleep(2)
        except:
            #fer un proper raise.
            print("Element not found")
            #raise html element not found

        #retornar una array de pages, si n'hi ha
        return self.driver.page_source

    def get_crime_entries(self, page_source: "webdriver.Chrome().page_source") -> Tuple[str, str, str]:
        
        soup = BeautifulSoup(page_source, 'lxml')

        elements = []

        element_selector = soup.find_all('tr', role='row')[1:]
        for element in element_selector:
            fields = element.find_all('td')
            element_title = fields[2].get_text()
            element_address = fields[4].get_text()
            element_date = fields[6].get_text()
            
            elements.append((element_title, element_address, element_date))
        return elements


if __name__ == '__main__':
    
    #addresses xungues
    address = '500 W L St, Wilmington, CA'
    
    #CrimeHandler = SpotCrime()
    #source = CrimeHandler.get_main_page(address)
    #entries = CrimeHandler.get_crime_entries(source)
    #results = CrimeHandler.parse_crime_entries(entries)
    #CrimeHandler.store_crime_entries(results, 'caca')
    
    CrimeHandler2 = CrimeMapping()
    source = CrimeHandler2.get_main_page(address)
    #time.sleep(30)
    entries = CrimeHandler2.get_crime_entries(source)
    results = CrimeHandler2.parse_crime_entries(entries)
    CrimeHandler2.store_crime_entries(results, 'caca')