
#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace routing {

using Weight = double;

struct Settings{
    double bus_wait_time = 0;
    int bus_velocity = 0;
};

struct RouteEdge {
    std::string type;
    Weight time;
    std::string name;
};
struct WaitEdge : public RouteEdge{
    WaitEdge(std::string_view t, Weight w, std::string_view n)
        : RouteEdge{std::string(t), w, std::string(n)} {}
};

struct BusEdge : public RouteEdge {
    size_t span_count;

    BusEdge(std::string_view t, Weight w, std::string_view n, size_t span)
        : RouteEdge{std::string(t), w, std::string(n)}, span_count(span) {}
};

struct RouteData {
    Weight total_time = 0;
    std::vector<RouteEdge> parts;
};


class TransportRouter{
public:
    TransportRouter(const catalogue::TransportCatalogue& catalog, Settings settings)
    :settings_(std::move(settings)){
        graph_ = std::move(GenerateGraph(catalog));
        router_.emplace(graph_);
    }

    std::optional<RouteData> BuildRoute(std::string_view from_stop, std::string_view to_stop) const{
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
                const auto& edge = graph_.GetEdge(edge_id);
                result.parts.push_back(dist_between_stops_.at({edge.from, edge.to}));
            }
            return result;
        }
        return  std::nullopt;
    }

private:
    Settings settings_;
    graph::DirectedWeightedGraph<Weight> graph_;
    std::optional<graph::Router<Weight>> router_;

    std::unordered_map<std::string, std::vector<graph::EdgeId>> stop_to_vertex_;

    struct EdgeHash {
        size_t operator()(const std::pair<graph::VertexId, graph::VertexId>& edge) const noexcept {
            size_t hash_value = 0;
            hash_value ^= std::hash<graph::VertexId>{}(edge.first);
            hash_value ^= std::hash<graph::VertexId>{}(edge.second);
            return hash_value;
            // size_t hash_value = std::hash<graph::VertexId>{}(edge.first);
            // hash_value ^= std::hash<graph::VertexId>{}(edge.second) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
            return hash_value;
        }
    };
    std::unordered_map<std::pair<graph::VertexId, graph::VertexId>, RouteEdge, EdgeHash> dist_between_stops_;


    graph::DirectedWeightedGraph<Weight> GenerateGraph(const catalogue::TransportCatalogue& catalog){
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

    void AddStopWaitEdge(graph::DirectedWeightedGraph<Weight>& graph, const std::string& stop, size_t& numb_vertex){
        graph::Edge<Weight> edge = {.from = numb_vertex,
                                    .to = ++numb_vertex,
                                    .weight = settings_.bus_wait_time};
            
        // std::vector<graph::EdgeId> edge_ids{0, 1};
        // stop_to_vertex_[stop] = std::move(edge_ids);
        stop_to_vertex_.emplace(stop, std::vector<graph::EdgeId>{edge.from, edge.to});
        dist_between_stops_[{numb_vertex-1, numb_vertex}] = WaitEdge("Wait", settings_.bus_wait_time, stop);
        
        graph.AddEdge(edge);

    }

    void AddBusEdges(graph::DirectedWeightedGraph<Weight>& graph, const catalogue::TransportCatalogue& catalog, const catalogue::Bus& bus){
        const std::vector<const catalogue::Stop*>& stops_on_bus = bus.stops;
        for(size_t i = 0; i < stops_on_bus.size(); ++i){
            const auto& from_stop = stops_on_bus.at(i);

            for(size_t j = 1; j < stops_on_bus.size(); ++j){
                const auto& to_stop = stops_on_bus.at(j);
                Weight time_on_dist = static_cast<Weight>(catalog.GetStopsDistance(from_stop, to_stop))/settings_.bus_velocity;

                graph::Edge<Weight> edge = {.from = stop_to_vertex_.at(from_stop->name)[1],
                                            .to = stop_to_vertex_.at(to_stop->name)[0],
                                            .weight = time_on_dist};
                graph.AddEdge(edge);
                dist_between_stops_[{edge.from, edge.to}] = BusEdge("Bus", time_on_dist, bus.name, j - i);
            }
        }
    }

};

}