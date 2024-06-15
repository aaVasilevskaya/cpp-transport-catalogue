#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer)
:db_(db), renderer_(renderer){
}

std::optional<catalogue::BusRoutInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const{
    try{
        const auto& info = db_.GetRouteInfo(bus_name);
        return info;
    }catch(catalogue::TransportCatalogueException){
        return std::nullopt;
    }
}

// Возвращает маршруты, проходящие через
const std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
    try{
        const auto& info = db_.GetStopInfo(stop_name);
        return info;
    }catch(catalogue::TransportCatalogueException){
        return std::nullopt;
    }
}

void RequestHandler::SetRouter(const std::shared_ptr<routing::TransportRouter>& router){
    router_ = router;
}

std::optional<routing::RouteData> RequestHandler::GetRoute(std::string_view from_stop, std::string_view to_stop) const{
    if (!router_) {
        return std::nullopt;
    }
    return router_->BuildRoute(from_stop, to_stop);
}
