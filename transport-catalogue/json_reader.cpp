#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

JsonReader::JsonReader(std::istream& input_stream)
    :input_stream_(input_stream){
}

void JsonReader::ReadAndParse(){
    json::Document document_ = json::Load(input_stream_);

    const json::Dict& doc_as_dict = document_.GetRoot().AsMap();
    if(const auto base = doc_as_dict.find("base_requests"); base != doc_as_dict.end()){
        commands_ = std::move(ParseCommands(base->second.AsArray()));
    }

    if(const auto base = doc_as_dict.find("routing_settings"); base != doc_as_dict.end()){
        routing_settings_ = std::move(ParseRouteSetting(base->second.AsMap()));
    }

    if(const auto base = doc_as_dict.find("render_settings"); base != doc_as_dict.end()){
        renderer_ = std::move(ParseRender(base->second.AsMap()));
    }

    if(const auto base = doc_as_dict.find("stat_requests"); base != doc_as_dict.end()){
        request_ = std::move(ParseRequest(base->second.AsArray()));
    }  
}

void JsonReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    for(auto& command : commands_){
        if(command.command == "Stop"){
            catalogue.AddStop(command.id.AsString(), ParseCoordinates(command));
        }
    }
    for(auto& command : commands_){
        if(command.command == "Stop"){
            auto distances = ParseDistances(command);
            for(auto& dist:distances){
                const auto& stop_from = catalogue.GetStopByName(command.id.AsString());
                const auto& stop_to = catalogue.GetStopByName(dist.name_location);
                if(stop_from && stop_to){
                    catalogue.AddStopsDistance(stop_from.value(), stop_to.value(), dist.dist);
                }
            }
        }
    }
    for(auto& command : commands_){
        if(command.command == "Bus"){
            catalogue.AddBus(ParseRoute(command, catalogue));
        }
    }
}

json::Document JsonReader::ApplyRequest(const RequestHandler& handler){
    json::Array rez_array;
    for(const auto& element:request_){
        const auto& type_str = element.type.AsString();
        if(type_str == "Bus"){
            std::optional<catalogue::BusRoutInfo> info = handler.GetBusStat(element.description.at("name").AsString());
            rez_array.emplace_back(GenerateBusInfo(element.id, info));
        }else if(type_str == "Stop"){
            const auto info = handler.GetBusesByStop(element.description.at("name").AsString());
            rez_array.emplace_back(GenerateStopInfo(element.id, info));
        }else if(type_str == "Map"){
            const auto info = handler.RenderMap();
            rez_array.emplace_back(GenerateMapInfo(element.id, info));
        }else if(type_str == "Route"){
            const auto info = handler.GetRoute(element.description.at("from").AsString(), element.description.at("to").AsString());
            rez_array.emplace_back(GenerateRouteInfo(element.id, info));
        }
    }
    return json::Document(rez_array);

}

void JsonReader::ApplyRender(renderer::MapRenderer& renderer){
    renderer.SetSettings(renderer_);
}

void JsonReader::ApplyRouter(routing::Settings& settings){
    settings = routing_settings_;
}

/*--------------------- Parser ----------------------------*/
std::vector<CommandDescription> JsonReader::ParseCommands(const json::Array& data) {
    std::vector<CommandDescription> parsed_commands;
    for(const auto& command:data){
        json::Node type;
        json::Node id;
        std::unordered_map<std::string, json::Node> description;
        
        for(const auto& element:command.AsMap()){
            if(element.first == "type"){
                type = element.second;
            }else if(element.first == "name"){
                id = element.second;
            }else{
                description[element.first] = element.second;
            }  
        }
        parsed_commands.emplace_back(CommandDescription{std::move(type),std::move(id),std::move(description)});
    }
    return parsed_commands;
}

std::vector<RequestDescription> JsonReader::ParseRequest(const json::Array& data) {
    std::vector<RequestDescription> parsed_requests;
    for(const auto& command:data){
        json::Node type;
        json::Node id;
        std::unordered_map<std::string, json::Node> description;
        
        for(const auto& [key, value]:command.AsMap()){
            if(key == "id"){
                id = value;
            }else if(key == "type"){
                type = value;
            }else{
                description[key] = value;
            }  
        }
        parsed_requests.emplace_back(RequestDescription{std::move(id), std::move(type),std::move(description)});
    }
    return parsed_requests;
}

renderer::RenderSettings JsonReader::ParseRender(const json::Dict& data){
    renderer::RenderSettings settings;
    settings.width = data.at("width").AsDouble();
    settings.height = data.at("height").AsDouble();
    settings.padding = data.at("padding").AsDouble();
    settings.line_width = data.at("line_width").AsDouble();
    settings.stop_radius = data.at("stop_radius").AsDouble();
    settings.bus_label_font_size = data.at("bus_label_font_size").AsInt();

    settings.bus_label_offset[0] = data.at("bus_label_offset").AsArray()[0].AsDouble();
    settings.bus_label_offset[1] = data.at("bus_label_offset").AsArray()[1].AsDouble();

    settings.stop_label_font_size = data.at("stop_label_font_size").AsInt();

    settings.stop_label_offset[0] = data.at("stop_label_offset").AsArray()[0].AsDouble();
    settings.stop_label_offset[1] = data.at("stop_label_offset").AsArray()[1].AsDouble();
    
    settings.underlayer_color = ParseColor(data.at("underlayer_color"));
    settings.underlayer_width = data.at("underlayer_width").AsDouble();

    json::Array colorArray = data.at("color_palette").AsArray();
    for(const auto& element: colorArray){
        settings.color_palette.emplace_back(ParseColor(element));
    }
    return settings;
}

routing::Settings JsonReader::ParseRouteSetting(const json::Dict& data){
    routing::Settings settings;
    settings.bus_velocity = data.at("bus_velocity").AsInt();
    settings.bus_wait_time = data.at("bus_wait_time").AsInt();
    return settings;
}

geo::Coordinates JsonReader::ParseCoordinates(const CommandDescription& data) const{
    const auto& lat = data.description.find("latitude");
    const auto& lng = data.description.find("longitude");
    if(lat !=  data.description.end() &&  lng !=  data.description.end()){
        return {lat->second.AsDouble(), lng->second.AsDouble()};
    }
    return {NAN, NAN};
}

std::vector<geo::Distance> JsonReader::ParseDistances(const CommandDescription& data) const{
    std::vector<geo::Distance> dists;  
    const auto& road_distances = data.description.find("road_distances");
    if(road_distances != data.description.end()){
        for(const auto& location:road_distances->second.AsMap()){
            dists.push_back({static_cast<unsigned int>(location.second.AsInt()), location.first});
        }
    }
    return dists;
}

catalogue::Bus JsonReader::ParseRoute(const CommandDescription& data, const catalogue::TransportCatalogue& catalogue) const{
    catalogue::Bus bus;
    bus.name = data.id.AsString();

    const auto& stops = data.description.find("stops");
    bool is_roundtrip = false;
    
    if(stops !=  data.description.end()){
        is_roundtrip = data.description.find("is_roundtrip")->second.AsBool();
        bus.is_roundtrip = is_roundtrip;
        
        const auto& stops_as_array = stops->second.AsArray();
        if(is_roundtrip == true){
            // Для кольцевого маршрута [A,B,C,A]
            bus.stops.reserve(stops_as_array.size() + 1);
            for (auto it = stops_as_array.begin(); it != stops_as_array.end(); ++it) {
                bus.stops.push_back((catalogue.GetStopByName(it->AsString())).value());
            }

        } else {
            // Для некольцевого маршрута [A,B,C,D,C,B,A]
            bus.stops.reserve(stops_as_array.size() * 2 - 1);
            for (auto it = stops_as_array.begin(); it != stops_as_array.end(); ++it) {
                bus.stops.push_back((catalogue.GetStopByName(it->AsString())).value());
            }
            for (auto it = stops_as_array.rbegin() + 1; it != stops_as_array.rend(); ++it) {
                bus.stops.push_back((catalogue.GetStopByName(it->AsString())).value());
            }
        }
    }
    return bus;
}

svg::Color JsonReader::ParseColor(json::Node data){
    if(data.IsString()){
        return data.AsString();
    }
    if(data.IsArray()){
        const auto& colorArray = data.AsArray();
        if(colorArray.size() == 3) {
            svg::Rgb rgb;
            rgb.red = static_cast<uint8_t>(colorArray[0].AsInt());
            rgb.green = static_cast<uint8_t>(colorArray[1].AsInt());
            rgb.blue = static_cast<uint8_t>(colorArray[2].AsInt());
            return rgb;
        } else if (colorArray.size() == 4) {
            svg::Rgba rgba;
            rgba.red = static_cast<uint8_t>(colorArray[0].AsInt());
            rgba.green = static_cast<uint8_t>(colorArray[1].AsInt());
            rgba.blue = static_cast<uint8_t>(colorArray[2].AsInt());
            rgba.opacity = colorArray[3].AsDouble();
            return rgba;
        }
    }
    return {};
}

/*--------------------- Part of json request ----------------------------*/
json::Dict JsonReader::GenerateBusInfo(const json::Node& id, const std::optional<catalogue::BusRoutInfo>& info){
    if(!info){
        return GenerateErrorMessege(id);
    }
     return json::Builder{}.StartDict()
                            .Key("request_id").Value(id.GetValue())
                            .Key("stop_count").Value(static_cast<int>(info->count_stops))
                            .Key("unique_stop_count").Value(static_cast<int>(info->count_uniq_stops))
                            .Key("curvature").Value(info->curvature)
                            .Key("route_length").Value(static_cast<int>(info->lenght))
                            .EndDict().Build().AsMap();
}

json::Dict JsonReader::GenerateStopInfo(const json::Node& id, const std::optional<std::set<std::string_view>>& info){

    if(!info){
        return GenerateErrorMessege(id);
    }
    json::Array buses;
    for(const auto& bus:info.value()){
        buses.emplace_back(std::string(bus));
    }
     return json::Builder{}.StartDict()
                            .Key("request_id").Value(id.GetValue())
                            .Key("buses").Value(std::move(buses))
                            .EndDict().Build().AsMap();
    
}

json::Dict JsonReader::GenerateErrorMessege(const json::Node& id){
    return json::Builder{}.StartDict()
                            .Key("request_id").Value(id.GetValue())
                            .Key("error_message").Value(std::string("not found"))
                            .EndDict().Build().AsMap();
}

json::Dict JsonReader::GenerateMapInfo(const json::Node& id, const svg::Document& info){

    std::ostringstream stream;
    info.Render(stream);

    return json::Builder{}.StartDict()
                        .Key("map").Value(stream.str())
                        .Key("request_id").Value(id.GetValue())
                        .EndDict().Build().AsMap();

}

json::Dict JsonReader::GenerateRouteInfo(const json::Node& id, std::optional<routing::RouteData> info){

    if(!info){
        return GenerateErrorMessege(id);
    }
 
    json::Array rout_items;
    for (const auto& part : info.value().parts) {
        json::Dict dict_tmp;
        
        if (std::holds_alternative<routing::WaitEdge>(part)) {
            const auto& wait_item = std::get<routing::WaitEdge>(part);
            dict_tmp["type"] = wait_item.type;
            dict_tmp["stop_name"] = std::string(wait_item.name);
            dict_tmp["time"] = wait_item.time;
        } else if (std::holds_alternative<routing::BusEdge>(part)) {
            const auto& bus_item = std::get<routing::BusEdge>(part);
            dict_tmp["type"] = bus_item.type;
            dict_tmp["bus"] = std::string(bus_item.name);
            dict_tmp["span_count"] = static_cast<int>(bus_item.span_count);
            dict_tmp["time"] = bus_item.time;
        } 
        rout_items.push_back(std::move(dict_tmp));
    }

     return json::Builder{}.StartDict()
                            .Key("request_id").Value(id.GetValue())
                            .Key("total_time").Value(info.value().total_time)
                            .Key("items").Value(std::move(rout_items))
                            .EndDict().Build().AsMap();

}
