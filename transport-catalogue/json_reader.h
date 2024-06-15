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
#include "json_builder.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

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

    void ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const;
    json::Document ApplyRequest(const RequestHandler& handler);
    void ApplyRender(renderer::MapRenderer& renderer);
    void ApplyRouter(routing::Settings& settings);


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
    routing::Settings routing_settings_;

    /*--------------------- Answer on requests ----------------------------*/
    json::Dict GenerateBusInfo(const json::Node& id, const std::optional<catalogue::BusRoutInfo>& info);
    json::Dict GenerateStopInfo(const json::Node& id, const std::optional<std::set<std::string_view>>& info);
    json::Dict GenerateErrorMessege(const json::Node& id);
    json::Dict GenerateMapInfo(const json::Node& id, const svg::Document& info);
    json::Dict GenerateRouteInfo(const json::Node& id, std::optional<routing::RouteData> info);


    /*--------------------- Parser ----------------------------*/
    std::vector<CommandDescription> ParseCommands(const json::Array& data);
    std::vector<RequestDescription> ParseRequest(const json::Array& data);
    renderer::RenderSettings ParseRender(const json::Dict& data);
    routing::Settings ParseRouteSetting(const json::Dict& data);
    
    //For commands
    geo::Coordinates ParseCoordinates(const CommandDescription& data) const;
    std::vector<geo::Distance> ParseDistances(const CommandDescription& data) const;
    catalogue::Bus ParseRoute(const CommandDescription& data, const catalogue::TransportCatalogue& catalogue) const;
    
    //For renderer
    svg::Color ParseColor(json::Node data);
};