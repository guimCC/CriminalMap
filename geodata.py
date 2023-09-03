import geocoder
from typing import Tuple

class GeoUtils():
    #static attribute accessed by all GeoUtils
    _geocoder_api = "Au4EjnCabLbXUDSqUptBF8DpVbu9ICuJyUzFR_sWyC9-lLxVGW9uS4gOM79C84N4"
    
    def __init__(self) -> None:
        pass
    
    #static method can be used without instancing a class object
    # returns lat and long given an address
    @staticmethod
    def get_lat_long(address: str) -> Tuple[float, float]:
        response = geocoder.bing(address, key = GeoUtils._geocoder_api)
        response = response.json
        
        return response['lat'], response['lng']
    
    # returns address given certain lat and long
    @staticmethod
    def get_address(lat: float, long: float) -> str:
        response = geocoder.bing([lat, long], method='reverse', key=GeoUtils._geocoder_api)
        response = response.json
                
        return response['address']
    
    # DEPRICATED
    @staticmethod
    def get_state(lat:float, long: float) -> str:
        pass


if __name__ == '__main__':
    #GeoUtil = GeoUtils()
    print(GeoUtils.get_lat_long('200 BLOCK OF E CARSON ST, CA'))