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

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stop_names){

    buses_.emplace_back(Bus{std::string(bus_name),std::vector<const Stop*>(stop_names.size())});

    for(size_t i = 0; i < stop_names.size(); i++){
        std::string stop_name(stop_names[i]);
        buses_.back().stops[i] = stop_ptrs_.at(stop_name);
        buses_on_stop_[stop_name].insert(buses_.back().name);
    }
    bus_ptrs_[buses_.back().name] = &buses_.back();
}

BusRoutInfo TransportCatalogue::GetRouteInfo(std::string_view name) const{
    std::string bus_name(name);
    if(bus_ptrs_.count(bus_name)){
        const Bus* bus = bus_ptrs_.at(bus_name);
        return {bus->stops.size(),
                CountUniqueStops(bus),
                ComputeRouteLength(bus)};	
    }
    else{
        throw TransportCatalogueException();
    };
}

std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    if(buses_on_stop_.count(std::string(stop_name))){
        return buses_on_stop_.at(std::string(stop_name));
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

double TransportCatalogue::ComputeRouteLength(const Bus* bus) const {
    double rout_length = 0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        rout_length += ComputeDistance(bus->stops[i - 1]->coord, bus->stops[i]->coord);
    }
    rout_length += ComputeDistance(bus->stops.back()->coord, bus->stops.front()->coord);
    return rout_length;
}  

}