#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const Catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
:db_(db), renderer_(renderer){
}

std::optional<Catalogue::BusRoutInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const{
    try{
        const auto& info = db_.GetRouteInfo(bus_name);
        return info;
    }catch(Catalogue::TransportCatalogueException){
        return std::nullopt;
    }
}

// Возвращает маршруты, проходящие через
const std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
    try{
        const auto& info = db_.GetStopInfo(stop_name);
        return info;
    }catch(Catalogue::TransportCatalogueException){
        return std::nullopt;
    }
}

// svg::Document RequestHandler::RenderMap() const{
//     return renderer_.
// }