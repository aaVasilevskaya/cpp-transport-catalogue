#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalog, Settings settings)
    :settings_(std::move(settings)){
        graph_ = std::move(GenerateGraph(catalog));
        router_.emplace(graph_);
}

std::optional<RouteData> TransportRouter::BuildRoute(std::string_view from_stop, std::string_view to_stop) const{
    if(!router_ ){
        return std::nullopt;
    }
    std::optional<graph::Router<Weight>::RouteInfo> route = 
                    router_.value().BuildRoute(stop_to_vertex_.at(std::string(from_stop))[0],stop_to_vertex_.at(std::string(to_stop))[0]);
    if(route.has_value()){
        const auto& route_info = *route;
        
        RouteData result;
        result.parts.reserve(route_info.edges.size());
        result.total_time = route_info.weight;

        for(const auto& edge_id : route_info.edges){
            result.parts.push_back(dist_between_stops_.at(edge_id));
        }
        return result;
    }
    return  std::nullopt;
}


graph::DirectedWeightedGraph<Weight> TransportRouter::GenerateGraph(const catalogue::TransportCatalogue& catalog){
    graph::DirectedWeightedGraph<Weight> graph(catalog.GetAllStops().size() * 2);

    const auto stops = catalog.GetAllStops();
    const auto buses = catalog.GetAllBuses();
    
    size_t numb_vertex = 0;
    
    stop_to_vertex_.reserve(stops.size() * 2);
    for(const auto& [stop_name, stop] : stops){
        AddStopWaitEdge(graph, stop->name, numb_vertex);
        ++numb_vertex;
    }
    
    for(const auto& [bus_name, bus] : buses){
        AddBusEdges(graph, catalog, *bus);
    }
    return graph;
}

void TransportRouter::AddStopWaitEdge(graph::DirectedWeightedGraph<Weight>& graph, const std::string& stop, size_t& numb_vertex){
    graph::Edge<Weight> edge = {.from = numb_vertex,
                                .to = ++numb_vertex,
                                .weight = settings_.bus_wait_time};
        
    stop_to_vertex_.emplace(stop, std::vector<graph::EdgeId>{edge.from, edge.to});

    const auto added_edge = graph.AddEdge(edge);
    dist_between_stops_[added_edge] = WaitEdge("Wait", settings_.bus_wait_time, stop);

}

void TransportRouter::AddBusEdges(graph::DirectedWeightedGraph<Weight>& graph, const catalogue::TransportCatalogue& catalog, const catalogue::Bus& bus){
    const std::vector<const catalogue::Stop*>& stops_on_bus = bus.stops;
    for(size_t i = 0; i < stops_on_bus.size(); ++i){
        const auto& from_stop = stops_on_bus.at(i);
        unsigned int dist = 0;

        for(size_t j = i + 1; j < stops_on_bus.size(); ++j){
            const auto& to_stop = stops_on_bus.at(j);
            dist += catalog.GetStopsDistance(stops_on_bus.at(j-1), to_stop);
            Weight time_on_dist = static_cast<Weight>(dist)/settings_.GetVelosityMetersPerMinut();

            graph::Edge<Weight> edge = {.from = stop_to_vertex_.at(from_stop->name)[1],
                                        .to = stop_to_vertex_.at(to_stop->name)[0],
                                        .weight = time_on_dist};
            const auto added_edge = graph.AddEdge(edge);
            size_t span_count = j - i;
            dist_between_stops_[added_edge] = BusEdge("Bus", time_on_dist, bus.name, span_count);
        }
    }
}

}