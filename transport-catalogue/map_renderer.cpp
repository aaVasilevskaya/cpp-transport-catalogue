#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer{
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document MapRenderer::GenerateMap(const Catalogue::TransportCatalogue& catalogue){
    svg::Document doc;
    
    const auto buses = catalogue.GetAllBuses();

    const auto& stops = catalogue.GetAllStops();
    const auto& buses_on_stop = catalogue.GetAllBusesOnStops();

    auto projector_and_names = GenerateMapProjector(buses);

    const auto proj(std::move(projector_and_names.projector));
    const auto buses_names(std::move(projector_and_names.buses));
    
    AddBusesPolyline(doc, proj, buses_names, buses);
    AddBusesNames(doc, proj, buses_names, buses);

    const auto stops_names = AddStopsCircle(doc, proj, stops, buses_on_stop);
    AddStopsNames(doc, proj, stops_names, stops);
    
    return doc;
}
//SphereProjector
MapProjectorForBusesStops MapRenderer::GenerateMapProjector(const std::unordered_map<std::string_view, const Catalogue::Bus*>& data){
    std::vector<const geo::Coordinates*> all_coordinates;
    std::set<std::string_view> buses_names;
        
    //container from all coordinates
    for(const auto& [name,bus]:data){
        buses_names.insert(name);
        for(const auto& stop:bus->stops){
            all_coordinates.push_back(&stop->coord);
        }
    }

    //make a coordinate projection
    const SphereProjector proj{
        all_coordinates.begin(), all_coordinates.end(),
        setting_.width, setting_.height, setting_.padding
    };
    return {std::move(proj), std::move(buses_names)};
}

void MapRenderer::AddBusesPolyline(svg::Document& doc, SphereProjector proj, const std::set<std::string_view> buses_names,
                                 const std::unordered_map<std::string_view, const Catalogue::Bus*>& data){

        //Generation of polylines
        auto color_it = setting_.color_palette.begin();
        for(const auto& name:buses_names){
            svg::Polyline polyline;
            for(const auto& stop:data.at(name)->stops){
                const svg::Point point = proj(stop->coord);
                polyline.AddPoint(point);
            }

            polyline.SetStrokeColor(*color_it)
            .SetFillColor("none")
            .SetStrokeWidth(setting_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(polyline));

            if (++color_it == setting_.color_palette.end()) {
                color_it = setting_.color_palette.begin();
            }
        }
    }

void MapRenderer::AddBusesNames(svg::Document& doc, const SphereProjector& proj, const std::set<std::string_view> buses_names,
                                    const std::unordered_map<std::string_view, const Catalogue::Bus*>& data){
    auto color_it = setting_.color_palette.begin();
    
    for(const auto& bus_name:buses_names){    
        const auto& first_stop = *data.at(bus_name)->stops.begin();
        
        svg::Point position = proj(first_stop->coord);
        auto svg_rout_name = MakeNameOfBus(bus_name, position, *color_it);
        doc.Add(svg_rout_name[0]);
        doc.Add(svg_rout_name[1]);
        
        if(data.at(bus_name)->is_roundtrip == false){
            
            size_t numb = data.at(bus_name)->stops.size()/2;
            const auto& last_stop = data.at(bus_name)->stops.at(numb);
            if(last_stop->name == first_stop->name){
                if (++color_it == setting_.color_palette.end()) {
                    color_it = setting_.color_palette.begin();
                }
                continue;
            }

            position = proj(last_stop->coord);
            svg_rout_name = MakeNameOfBus(bus_name, position, *color_it);
            doc.Add(svg_rout_name[0]);
            doc.Add(svg_rout_name[1]);

        }

        if (++color_it == setting_.color_palette.end()) {
            color_it = setting_.color_palette.begin();
        }
    }
}

std::array<svg::Text,2> MapRenderer::MakeNameOfBus(std::string_view name,svg::Point& position, svg::Color& color){
    svg::Text svg_base;
    svg::Text svg_name_of_rout;
    
    svg::Point offset(setting_.bus_label_offset[0], setting_.bus_label_offset[1]);

    svg_name_of_rout.SetPosition(position)
    .SetOffset(offset)
    .SetFontSize(static_cast<uint32_t>(setting_.bus_label_font_size))
    .SetFontFamily("Verdana")
    .SetFontWeight("bold")
    .SetFillColor(color)
    .SetData(std::string(name));

    svg_base.SetPosition(position)
    .SetOffset(offset)
    .SetFontSize(static_cast<uint32_t>(setting_.bus_label_font_size))
    .SetFontFamily("Verdana")
    .SetFontWeight("bold")
    .SetData(std::string(name))
    .SetFillColor(setting_.underlayer_color)
    .SetStrokeColor(setting_.underlayer_color)
    .SetStrokeWidth(setting_.underlayer_width)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return {std::move(svg_base), std::move(svg_name_of_rout)};
}

std::set<std::string_view> MapRenderer::AddStopsCircle(svg::Document& doc, const SphereProjector& proj,
                const std::unordered_map<std::string_view, const Catalogue::Stop*>& stops,
                const std::unordered_map<std::string_view, std::set<std::string_view>>& buses_on_stop){
    
    std::set<std::string_view> stops_names;
    for(const auto& [stop,buses]:buses_on_stop){
        if(!buses.empty()){
            stops_names.insert(stop);
        }     
    }
    for(const auto& name:stops_names){
        svg::Circle circle;
        const auto& position = proj(stops.at(name)->coord);
        circle.SetCenter(position).SetRadius(setting_.stop_radius).SetFillColor("white");
        doc.Add(std::move(circle));
    }
    return stops_names;
}

void MapRenderer::AddStopsNames(svg::Document& doc, const SphereProjector& proj, const std::set<std::string_view> stops_names,
                const std::unordered_map<std::string_view, const Catalogue::Stop*>& stops_info){
    
    for(const auto& stop:stops_names){
        svg::Text svg_name_of_stop;
        const auto& position = proj(stops_info.at(stop)->coord);
        auto svg_stop_name = MakeNameOfStop(stop, position);
        doc.Add(svg_stop_name[0]);
        doc.Add(svg_stop_name[1]);
    }
}

std::array<svg::Text,2> MapRenderer::MakeNameOfStop(std::string_view name, const svg::Point& position){
    svg::Text svg_base;
    svg::Text svg_name_of_stop;
    
    svg::Point offset(setting_.stop_label_offset[0], setting_.stop_label_offset[1]);

    svg_name_of_stop.SetPosition(position)
    .SetOffset(offset)
    .SetFontSize(static_cast<uint32_t>(setting_.stop_label_font_size))
    .SetFontFamily("Verdana")
    .SetData(std::string(name))
    .SetFillColor("black");

    svg_base.SetPosition(position)
    .SetOffset(offset)
    .SetFontSize(static_cast<uint32_t>(setting_.stop_label_font_size))
    .SetFontFamily("Verdana")
    .SetData(std::string(name))
    .SetFillColor(setting_.underlayer_color)
    .SetStrokeColor(setting_.underlayer_color)
    .SetStrokeWidth(setting_.underlayer_width)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return {std::move(svg_base), std::move(svg_name_of_stop)};
}

}