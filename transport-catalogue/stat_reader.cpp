#include "stat_reader.h"

#include <iostream>
#include <iomanip>


void ParseAndPrintStat(const Catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    
    auto command_description = ParseRequestDescription(request);
    
    output<< command_description.command << " " << command_description.id << ": ";

    if(command_description.command == "Bus"){
        PrintBusInfo(tansport_catalogue, command_description.id, output);        
    }else if(command_description.command == "Stop"){
        PrintStopInfo(tansport_catalogue, command_description.id, output);
    }
}

RequestDescription ParseRequestDescription(std::string_view line) {
    auto colon_pos = line.find_last_not_of(' ') + 1;

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space))};
}

void PrintBusInfo(const Catalogue::TransportCatalogue& tansport_catalogue, const std::string& bus_name,
                     std::ostream& output){
    try{
        const Catalogue::BusRoutInfo info = tansport_catalogue.GetRouteInfo(bus_name);

        output << info.count_stops << " stops on route, " <<
                info.count_uniq_stops << " unique stops, " <<
                std::setprecision(6) << info.length <<" route length" << std::endl;
    }catch(Catalogue::TransportCatalogueException){
        output<< "not found" << std::endl;
    }
}

void PrintStopInfo(const Catalogue::TransportCatalogue& tansport_catalogue, const std::string& stop_name,
                     std::ostream& output){
    try{
        const auto& info = tansport_catalogue.GetStopInfo(stop_name);
        if(info.empty()){
            output << "no buses" << std::endl;
            return;
        }else{
            output << "buses";
            for(auto bus : info){
                output << " "<< bus;
            }
            output << std::endl;
        }
    }catch(Catalogue::TransportCatalogueException){
        output<< "not found" << std::endl;
    }
}
