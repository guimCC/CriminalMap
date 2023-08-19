import overpass


class MapData():
    def __init__(self) -> None:
        self.api = overpass.API(debug=True)
        self._format = "json"
    
    def get_data(self, n: float, s: float, e: float, w: float) -> bool:
        MapQuery = overpass.MapQuery(s, w, n, e)
        
        # needed format to get both nodes and ways
        self.data = self.api.get(MapQuery, responseformat = self._format)
    
    def store_data(self, node_file_name: str, way_file_name: str) -> bool:
        # check the last update
        # TODO: correct file name. Coords?
        # TODO: 
        with open(node_file_name, 'w') as node_file, open(way_file_name, 'w') as way_file:
            for element in self.data["elements"]:
                if element["type"] == "node":
                    line = str(element["id"]) + ";" + \
                           str(element["lat"]) + ";" + \
                           str(element["lon"]) + "\n"
                    node_file.write(line)
                elif element["type"] == "way":
                    line = "id=" + str(element["id"]) + ";" + \
                           ";".join([str(node) for node in element["nodes"]]) + "\n"
                    way_file.write(line)

if __name__ == '__main__':
    map_handler = MapData()
    map_handler.get_data(50.748,50.746,7.157,7.154)
    map_handler.store_data("nodes.csv", "camins.csv")