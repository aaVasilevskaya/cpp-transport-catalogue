#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <fstream>

using namespace std;

int main() {
    Catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    RequestHandler handler(catalogue, renderer);

    {
        JsonReader reader(std::cin);
        reader.ReadAndParse();
        reader.ApplyCommands(catalogue);
        reader.ApplyRender(renderer);
        
        const auto answer = reader.ApplyRequest(handler);
        Print(answer, std::cout);

    }
}