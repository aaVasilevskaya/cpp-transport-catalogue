#pragma once

#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#include "geo.h"

namespace Catalogue {

struct Stop {
    std::string name;
    Coordinates coord;
};
struct Bus {
	std::string name;
	std::vector<const Stop*> stops;
};

struct BusRoutInfo{
	size_t count_stops;
	size_t count_uniq_stops;
	double length;
};

struct TransportCatalogueException {};

class TransportCatalogue {

public:

	void AddStop(std::string_view name, Coordinates coord);
	void AddBus(std::string_view bus_name,  const std::vector<std::string_view>& stop_names);

	BusRoutInfo GetRouteInfo(std::string_view name) const;
	std::set<std::string_view> GetStopInfo(std::string_view stop_name) const;
	
private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Stop*> stop_ptrs_;
	std::unordered_map<std::string_view, const Bus*> bus_ptrs_;
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stop_;

	size_t CountUniqueStops(const Bus* bus) const;

	double ComputeRouteLength(const Bus* bus) const;

};

}