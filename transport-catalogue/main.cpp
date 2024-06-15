#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "request_handler.h"
#include <fstream>

using namespace std;

int main() {
    catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    RequestHandler handler(catalogue, renderer);

///////////////////////////////
    std::ifstream input_file("input.json");
    if (!input_file.is_open()) {
        std::cerr << "Failed to open input.json" << std::endl;
        return 1;
    }

    std::ofstream out;
    out.open("out.txt"); 
///////////////////////////

    {
        JsonReader reader(input_file);
        // JsonReader reader(std::cin);
        reader.ReadAndParse();
        reader.ApplyCommands(catalogue);
        reader.ApplyRender(renderer);

        routing::Settings rout_settings;
        reader.ApplyRouter(rout_settings);

        auto router_ptr = std::make_shared<routing::TransportRouter>(catalogue, rout_settings);
        handler.SetRouter(router_ptr);
        
        const auto answer = reader.ApplyRequest(handler);
        // Print(answer, std::cout);
        Print(answer, out);

    }
}