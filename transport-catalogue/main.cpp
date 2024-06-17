#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "request_handler.h"

using namespace std;

int main() {
    catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    RequestHandler handler(catalogue, renderer);

    {
        JsonReader reader(std::cin);
        reader.ReadAndParse();
        reader.ApplyCommands(catalogue);
        reader.ApplyRender(renderer);

        routing::Settings rout_settings;
        reader.ApplyRouter(rout_settings);

        routing::TransportRouter router(catalogue, rout_settings);
        handler.SetRouter(router);
        
        const auto answer = reader.ApplyRequest(handler);
        Print(answer, std::cout);
    }
}