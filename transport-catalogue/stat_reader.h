#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

struct RequestDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
};

void ParseAndPrintStat(const Catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);

RequestDescription ParseRequestDescription(std::string_view line);
void PrintBusInfo(const Catalogue::TransportCatalogue& tansport_catalogue, const std::string& bus_name,
                 std::ostream& output);
void PrintStopInfo(const Catalogue::TransportCatalogue& tansport_catalogue, const std::string& stop_name,
                     std::ostream& output);