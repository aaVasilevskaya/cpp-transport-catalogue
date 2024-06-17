
#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace routing {

using Weight = double;

struct Settings{
    double bus_wait_time = 0;
    int bus_velocity = 0;// km/h
    double GetVelosityMetersPerMinut(){
        return bus_velocity * 1000.0 / 60;
    }
};

struct RouteEdge {
    std::string type;
    Weight time;
    std::string name;
};
struct WaitEdge : public RouteEdge{
    WaitEdge() = default;
    WaitEdge(std::string_view t, Weight w, std::string_view n)
        : RouteEdge{std::string(t), w, std::string(n)} {}
};

struct BusEdge : public RouteEdge {
    BusEdge() = default;
    size_t span_count;
    BusEdge(std::string_view t, Weight w, std::string_view n, size_t span)
        : RouteEdge{std::string(t), w, std::string(n)}, span_count(span) {}
};

using RoutEdgeVariants = std::variant<WaitEdge, BusEdge>;

struct RouteData {
    Weight total_time = 0;
    std::vector<RoutEdgeVariants> parts;
};


class TransportRouter{
public:
    TransportRouter(const catalogue::TransportCatalogue& catalog, Settings settings);

    std::optional<RouteData> BuildRoute(std::string_view from_stop, std::string_view to_stop) const;

private:
    Settings settings_;
    graph::DirectedWeightedGraph<Weight> graph_;
    std::optional<graph::Router<Weight>> router_;

    std::unordered_map<std::string, std::vector<graph::EdgeId>> stop_to_vertex_;
    std::unordered_map<graph::EdgeId, RoutEdgeVariants> dist_between_stops_;


    graph::DirectedWeightedGraph<Weight> GenerateGraph(const catalogue::TransportCatalogue& catalog);

    void AddStopWaitEdge(graph::DirectedWeightedGraph<Weight>& graph, const std::string& stop, size_t& numb_vertex);
    void AddBusEdges(graph::DirectedWeightedGraph<Weight>& graph, 
                    const catalogue::TransportCatalogue& catalog, const catalogue::Bus& bus);

};

}