#pragma once

#include <iomanip>
#include <iostream>
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

class StatReader  {
public:

    explicit StatReader(std::istream& input_stream, std::ostream& output_stream);

    void ReadAndProcessStats(const Catalogue::TransportCatalogue& catalogue);

    void ParseAndPrintStat(const Catalogue::TransportCatalogue& catalogue, std::string_view request,
                       std::ostream& output);

private:
    std::istream& input_stream_;
    std::ostream& output_stream_;

    void PrintBusInfo(const Catalogue::TransportCatalogue& catalogue, const std::string& bus_name,
                    std::ostream& output);
    void PrintStopInfo(const Catalogue::TransportCatalogue& catalogue, const std::string& stop_name,
                        std::ostream& output);
};

