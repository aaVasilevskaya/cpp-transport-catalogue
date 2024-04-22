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
	unsigned int lenght;
	double curvature;
};

struct TransportCatalogueException {};

class TransportCatalogue {

public:

	void AddStop(std::string_view name, Coordinates coord);
	void AddStopsDistance(std::string_view from_name, std::string_view to_name, unsigned int dist);
	void AddBus(std::string_view bus_name,  const std::vector<std::string_view>& stop_names);

	BusRoutInfo GetRouteInfo(std::string_view name) const;
	std::set<std::string_view> GetStopInfo(std::string_view stop_name) const;
	
private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Stop*> stop_ptrs_;
	std::unordered_map<std::string_view, const Bus*> bus_ptrs_;
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stop_;

	struct StopPairHash {
		size_t operator()(const std::pair<const Stop*, const Stop*>& p) const noexcept {
			size_t hash_value = 0;
			std::hash<const void*> hash_ptr;
			hash_value ^= hash_ptr(p.first);
			hash_value ^= hash_ptr(p.second);
			return hash_value;
		}
	};

	std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, StopPairHash> dist_between_stops_;

	size_t CountUniqueStops(const Bus* bus) const;
	
	unsigned int GetStopsDistance(const Stop* from_stop, const Stop* to_stop) const;
	
	double ComputeGeographicalRouteLength(const Bus* bus) const;
	unsigned int ComputeRoadRouteLength(const Bus* bus) const;

};

}