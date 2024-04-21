#include "transport_catalogue.h"

namespace Catalogue {

void TransportCatalogue::AddStop(std::string_view name, Coordinates coord){
    stops_.emplace_back(Stop{std::string(name), std::move(coord)});

    const std::string* tmp_name = &stops_.back().name;
    stop_ptrs_[*tmp_name] = &stops_.back();
        
    if (buses_on_stop_.count(*tmp_name) == 0) {
        buses_on_stop_[*tmp_name] = {};
    }
}

void TransportCatalogue::AddStopDistances(std::string_view name, std::vector<Distance>& dists){
    for(auto& dist:dists){
        dist_between_stops_[std::make_pair(stop_ptrs_.at(name), stop_ptrs_.at(dist.name_location))] = dist.dist;
    }
}

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stop_names){

    buses_.emplace_back(Bus{std::string(bus_name),std::vector<const Stop*>(stop_names.size())});

    for(size_t i = 0; i < stop_names.size(); i++){
        buses_.back().stops[i] = stop_ptrs_.at(stop_names[i]);
        buses_on_stop_[stop_names[i]].insert(buses_.back().name);
    }
    bus_ptrs_[buses_.back().name] = &buses_.back();
}

BusRoutInfo TransportCatalogue::GetRouteInfo(std::string_view name) const{
    if(bus_ptrs_.count(name)){
        const Bus* bus = bus_ptrs_.at(name);
        auto road_length = ComputeRoadRouteLength(bus);
        return {bus->stops.size(),
                CountUniqueStops(bus),
                road_length,
                road_length / ComputeGeographicalRouteLength(bus) };	
    }
    else{
        throw TransportCatalogueException();
    };
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    if(buses_on_stop_.count(stop_name)){
        return buses_on_stop_.at(stop_name);
    }else{
        throw TransportCatalogueException();
    }
}

size_t TransportCatalogue::CountUniqueStops(const Bus* bus) const {
    std::unordered_set<std::string_view> unique_stops;
    for (const Stop* stop : bus->stops) {
        unique_stops.insert(stop->name);
    }
    return unique_stops.size();
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
        auto stops_pair = std::make_pair(bus->stops[i - 1], bus->stops[i]);
        if(dist_between_stops_.count(stops_pair)){
            rout_length += dist_between_stops_.at(stops_pair);
        }else{
            stops_pair = std::make_pair(bus->stops[i], bus->stops[i - 1]);
            if(dist_between_stops_.count(stops_pair)){
                rout_length += dist_between_stops_.at(stops_pair);
            }
        } 
    }

    auto stops_pair = std::make_pair(bus->stops.back(), bus->stops.front());
    if(dist_between_stops_.count(stops_pair)){
        rout_length += dist_between_stops_.at(stops_pair);
    }
    return rout_length;
} 

}