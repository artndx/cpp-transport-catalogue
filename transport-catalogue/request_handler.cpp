#include "request_handler.h"

namespace transport_catalogue{

namespace request_handler{

using namespace detail;

void RequestHandler::AddBaseRequest(MultiBaseRequest&& request){
    if(request.IsRequestAddStop()){
        base_requests["Stop"].push_back(std::move(request));
    } else if(request.IsRequestAddBus()){
        base_requests["Bus"].push_back(std::move(request));
    }
}

void RequestHandler::AddStatRequest(MultiStatRequest&& request){
    stat_requests_.push_back(std::move(request));
}

void RequestHandler::AddRenderSettings(map_render::RenderSettings&& settings){
    map_render_.SetSettings(settings);
}


void RequestHandler::ApplyRequests(TransportCatalogue& catalogue){
    std::vector<MultiBaseRequest>& stop_requests = base_requests["Stop"];
    std::vector<MultiBaseRequest>& bus_requests = base_requests["Bus"];
    // Сначала добавляем остановки
    // параллельно записывая
    // расстояния до других остановок
    std::unordered_map<std::string, Distances> stops_to_distances;
    for(MultiBaseRequest& req : stop_requests){
        RequestAddStop request = req.AsRequestAddStop();
        stops_to_distances[request.name_] = request.road_distances_;
        catalogue.AddStop(request.name_, {request.latitude_, request.longtitude_});
    }
    // Добавляем расстояния
    for(const auto& [stop, stop_distances] : stops_to_distances){
        for(const auto& [other_stop, distance] : stop_distances){
            catalogue.AddStopDistance(stop,other_stop,distance);
        }
    }
    // Добавляем маршруты
    for(MultiBaseRequest& req : bus_requests){
        RequestAddBus request = req.AsRequestAddBus();
        std::vector<std::string_view> route;
        if(!request.stops_.empty()){
            for(const std::string& stop : request.stops_){
                route.push_back(stop);
            }
            if(!request.is_roundtrip_){
                route.insert(route.end(), std::next(request.stops_.rbegin()), request.stops_.rend());
            }
        }
        catalogue.AddBus(request.name_, route, request.is_roundtrip_);
    }

    // Отправляем запросы 
    // на получение данных из базы
    for(MultiStatRequest& req : stat_requests_){
        if(req.IsRequestGetInfo()){
            RequestGetInfo info_req = req.AsRequestGetInfo();
                if(info_req.type_ == "Stop"){
                StopInfo stop_info = catalogue.GetStopInfo(info_req.name_);
                if(stop_info.is_find){
                    ResponseStopInfo response(info_req.id_, stop_info.buses);
                    responses_.emplace_back(response);
                } else {
                    ResponseError response(info_req.id_);
                    responses_.emplace_back(response);
                }
            } else if(info_req.type_ == "Bus"){
                BusInfo bus_info = catalogue.GetBusInfo(info_req.name_);
                if(bus_info.is_find){
                    ResponseBusInfo response(info_req.id_,bus_info.curvature,bus_info.route_length,
                    bus_info.stop_count,bus_info.unique_stops);
                    responses_.emplace_back(response);
                } else {
                    ResponseError response(info_req.id_);
                    responses_.emplace_back(response);
                }
            }
        } else if(req.IsRequestGetMap()){
            RequestGetMap map_req = req.AsRequestGetMap();
            // Отправляем запрос
             // на получение маршрутов
            std::map<std::string_view, const Bus*> buses = std::move(catalogue.GetBuses());
            ResponseMap response(map_req.id_, map_render_.Render(buses));
            responses_.emplace_back(response);
        }
    }
}

std::vector<MultiResponse>& RequestHandler::GetResponses(){
    return responses_;
}


} // namespace request_handler

} // namespace transport_catalogue

