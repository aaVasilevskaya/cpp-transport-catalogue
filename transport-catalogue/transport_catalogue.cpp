#include "transport_catalogue.h"

namespace Catalogue {

void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coord){
    stops_.emplace_back(Stop{std::string(name), std::move(coord)});

    const std::string* tmp_name = &stops_.back().name;
    stop_ptrs_[*tmp_name] = &stops_.back();
        
    if (buses_on_stop_.count(*tmp_name) == 0) {
        buses_on_stop_[*tmp_name] = {};
    }
}

void TransportCatalogue::AddStopsDistance(std::string_view from_name, std::string_view to_name, unsigned int dist){
    dist_between_stops_[{stop_ptrs_.at(from_name), stop_ptrs_.at(to_name)}] = dist;
}

void TransportCatalogue::AddBus(std::string_view bus_name, bool is_roundtrip, const std::vector<std::string_view>& stop_names){

    buses_.emplace_back(Bus{std::string(bus_name), is_roundtrip, std::vector<const Stop*>(stop_names.size())});

    for(size_t i = 0; i < stop_names.size(); i++){
        buses_.back().stops[i] = stop_ptrs_.at(stop_names[i]);
        buses_on_stop_[stop_names[i]].insert(buses_.back().name);
    }
    bus_ptrs_[buses_.back().name] = &buses_.back();
}

BusRoutInfo TransportCatalogue::GetRouteInfo(std::string_view name) const{
    if(bus_ptrs_.count(name) > 0){
        const Bus* bus = bus_ptrs_.at(name);
        auto road_length = ComputeRoadRouteLength(bus);
        return {static_cast<unsigned int>(bus->stops.size()),
                CountUniqueStops(bus),
                road_length,
                road_length / ComputeGeographicalRouteLength(bus) };	
    }
    throw TransportCatalogueException();
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    if(auto info = buses_on_stop_.find(stop_name); info != buses_on_stop_.end()){
        return buses_on_stop_.at(stop_name);
    }
    throw TransportCatalogueException();
}

unsigned int TransportCatalogue::CountUniqueStops(const Bus* bus) const {
    std::unordered_set<std::string_view> unique_stops;
    for (const Stop* stop : bus->stops) {
        unique_stops.insert(stop->name);
    }
    return static_cast<unsigned int>(unique_stops.size());
}

unsigned int TransportCatalogue::GetStopsDistance(const Stop* from_stop, const Stop* to_stop) const{
    if(auto dist = dist_between_stops_.find({from_stop, to_stop}); dist != dist_between_stops_.end()){
        return dist->second;
    }else if (dist = dist_between_stops_.find({to_stop, from_stop}); dist != dist_between_stops_.end()){
       return dist->second;
    }
    return 0;
}

double TransportCatalogue::ComputeGeographicalRouteLength(const Bus* bus) const {
    double rout_length = 0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        rout_length += ComputeDistance(bus->stops[i - 1]->coord, bus->stops[i]->coord);
    }
    rout_length += ComputeDistance(bus->stops.back()->coord, bus->stops.front()->coord);
    return rout_length;
} 

unsigned int TransportCatalogue::ComputeRoadRouteLength(const Bus* bus) const {
    unsigned int rout_length = 0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        rout_length += GetStopsDistance(bus->stops[i - 1], bus->stops[i]);
    }
    rout_length += GetStopsDistance(bus->stops.back(),bus->stops.front());

    return rout_length;
} 

}