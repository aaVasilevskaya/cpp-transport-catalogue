#pragma once

#include <string>
#include <array>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

#include "svg.h"
#include "geo.h"

#include "transport_catalogue.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer{

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(const PointInputIt points_begin, const PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->lng < rhs->lng; });
        min_lon_ = (*left_it)->lng;
        const double max_lon = (*right_it)->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs->lat < rhs->lat; });
        const double min_lat = (*bottom_it)->lat;
        max_lat_ = (*top_it)->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


struct RenderSettings{
    double width;               // ширина изображения в пикселях (вещественное число от 0 до 100000)
    double height;              // высота изображения в пикселях (вещественное число от 0 до 100000)
    double padding;             // отступ краёв карты от границ SVG-документа (вещественное число не меньше 0 и меньше min(width, height)/2)
    double line_width;          // толщина линий маршрутов (вещественное число от 0 до 100000)
    double stop_radius;         // радиус окружностей для остановок (вещественное число от 0 до 100000)
    int bus_label_font_size;    // размер текста названий автобусных маршрутов (целое число от 0 до 100000)
    std::array<double, 2> bus_label_offset; // смещение надписи с названием маршрута относительно координат конечной остановки на карте
    int stop_label_font_size;   // размер текста названий остановок (целое число от 0 до 100000)
    std::array<double, 2> stop_label_offset; // смещение названия остановки относительно её координат на карте
    svg::Color underlayer_color; // цвет подложки под названиями остановок и маршрутов
    double underlayer_width;    // толщина подложки под названиями остановок и маршрутов (вещественное число от 0 до 100000)
    std::vector <svg::Color> color_palette; // цветовая палитра
};

struct MapProjectorForBusesStops{
    SphereProjector projector;
    std::set<std::string_view> buses;
}; 

class MapRenderer{
public:
    MapRenderer() = default;
    MapRenderer(RenderSettings settings)
    : setting_(settings){

    }
    svg::Document GenerateMap(const Catalogue::TransportCatalogue& catalogue);

    void SetSettings(const RenderSettings& settings){
        setting_ = settings;
    }

private:

    RenderSettings setting_;

    MapProjectorForBusesStops GenerateMapProjector(const std::unordered_map<std::string_view, const Catalogue::Bus*>& data);

    void AddBusesPolyline(svg::Document& doc, SphereProjector proj, const std::set<std::string_view> buses_names,
                                    const std::unordered_map<std::string_view, const Catalogue::Bus*>& data);

    void AddBusesNames(svg::Document& doc, const SphereProjector& proj, const std::set<std::string_view> buses_names,
                    const std::unordered_map<std::string_view,const Catalogue::Bus*>& data);
 
    std::set<std::string_view> AddStopsCircle(svg::Document& doc, const SphereProjector& proj,
                    const std::unordered_map<std::string_view, const Catalogue::Stop*>& stops,
                    const std::unordered_map<std::string_view, std::set<std::string_view>>& buses_on_stop);

    void AddStopsNames(svg::Document& doc, const SphereProjector& proj, const std::set<std::string_view> stops_names,
                    const std::unordered_map<std::string_view, const Catalogue::Stop*>& stops_info);

    std::array<svg::Text,2> MakeNameOfBus(std::string_view name,svg::Point& position, svg::Color& color);
    std::array<svg::Text,2> MakeNameOfStop(std::string_view name, const svg::Point& position);

};
}