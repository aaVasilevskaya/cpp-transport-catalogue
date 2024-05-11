#pragma once
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include <string>
#include <string_view>
#include <vector>
#include <istream>
#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include "json.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include "request_handler.h"

#include <iomanip>
#include <iostream>

struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.AsString().empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        json::Node command;      // Название команды
        json::Node id;           // id маршрута или остановки
        std::unordered_map<std::string, json::Node> description;  // Параметры команды
    };

struct RequestDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !type.AsString().empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    json::Node id;              // Уникальный числовой идентификатор запроса
    json::Node type;           // Тип запроса
    std::unordered_map<std::string, json::Node> description;//Параметры команды
};

class JsonReader {
public:

    explicit JsonReader(std::istream& input_stream);

    void ReadAndParse();

    void ApplyCommands([[maybe_unused]] Catalogue::TransportCatalogue& catalogue) const;
    json::Document ApplyRequest(const RequestHandler& handler);
    void ApplyRender(renderer::MapRenderer& renderer);


    const std::vector<CommandDescription>& GetCommandsDescription(){
        return commands_;
    }
    const std::vector<RequestDescription>& GetRequestsDescription(){
        return request_;
    }
      
private:
    std::istream& input_stream_;

    std::vector<CommandDescription> commands_;
    std::vector<RequestDescription> request_;
    renderer::RenderSettings renderer_;

    /*--------------------- Answer on requests ----------------------------*/
    json::Dict GenerateBusInfo(const json::Node& id, const std::optional<Catalogue::BusRoutInfo>& info);
    json::Dict GenerateStopInfo(const json::Node& id, const std::optional<std::set<std::string_view>>& info);
    json::Dict GenerateMapInfo(const json::Node& id, const svg::Document& info);


    /*--------------------- Parser ----------------------------*/
    std::vector<CommandDescription> ParseCommands(const json::Array& data);
    std::vector<RequestDescription> ParseRequest(const json::Array& data);
    renderer::RenderSettings ParseRender(const json::Dict& data);
    
    //For commands
    geo::Coordinates ParseCoordinates(const CommandDescription& data) const;
    std::vector<geo::Distance> ParseDistances(const CommandDescription& data) const;
    std::pair<bool, std::vector<std::string_view>> ParseRoute(const CommandDescription& data) const;
    
    //For renderer
    svg::Color ParseColor(json::Node data);
};