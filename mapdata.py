import overpass


class MapData():
    def __init__(self) -> None:
        self.api = overpass.API(debug=False)
        self._format = "json"
    
    def get_data(self, n: float, s: float, e: float, w: float) -> bool:
        MapQuery = overpass.MapQuery(s, w, n, e)
        
        # needed format to get both nodes and ways
        self.data = self.api.get(MapQuery, responseformat = self._format)
    
    def store_data(self, node_file_name: str, way_file_name: str) -> bool:
        nodes_to_remove = []
        with open(node_file_name, 'w') as node_file, open(way_file_name, 'w') as way_file:
            for element in self.data["elements"]:
                # Firstly iterate ways to remove not wanted nodes
                if element["type"] == "way":
                    # Must parse 'building' ways, since represent the boundaries of a building and don't connect
                    # to main road
                    if 'building' not in element['tags'].keys():
                        # If it's not a building, we can write the way down
                        line = "id=" + str(element["id"]) + ";" + \
                            ";".join([str(node) for node in element["nodes"]]) + "\n"
                        way_file.write(line)
                    else:
                        # If it's a building, we will add its nodes to "nowrite" list
                        nodes_to_remove.extend(element["nodes"])
                        
            for element in self.data["elements"]:
                # Parse building nodes
                if element["type"] == "node" and element["id"] not in nodes_to_remove:
                    line = str(element["id"]) + ";" + \
                           str(element["lat"]) + ";" + \
                           str(element["lon"]) + "\n"
                    node_file.write(line)
    #TODO: delete test method
    def test(self):
        for element in self.data["elements"]:
            if element['id'] == 411117242:
                if 'building' in element['tags'].keys():
                    print('building')
                    print(element['nodes'])
                else:
                    print("nobuilding")
                    print(element, '\n')
                #print(element['tags'].keys(), '\n')
            if element['id'] == 4128489669:
                print(element)

if __name__ == '__main__':
    map_handler = MapData()
    map_handler.get_data(33.98671822, 33.985338580000004, -118.27241058, -118.27644422)
    map_handler.test()
    #map_handler.store_data("nodes.csv", "camins.csv")