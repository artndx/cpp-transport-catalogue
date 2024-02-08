#pragma once

#include <variant>
#include <sstream>
#include <map>
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_catalogue{

namespace request_handler{

namespace detail{

using Distances = std::unordered_map<std::string, int>;
using Stops = std::vector<std::string>;
using Buses = std::set<std::string>;
using BusLocation = std::vector<std::pair<double, double>>;

//Requests
struct RequestAddStop {
    RequestAddStop(std::string type, std::string name,
                double latitude, double longtitude, Distances road_distances)
                : type_(type), name_(name),
                latitude_(latitude), longtitude_(longtitude), road_distances_(road_distances){}
    std::string type_;
    std::string name_;
    double latitude_ = 0;
    double longtitude_ = 0;
    Distances road_distances_;
};

struct RequestAddBus{
    RequestAddBus(std::string type, std::string name,
                Stops stops, bool is_roundtrip)
                : type_(type), name_(name), stops_(stops), is_roundtrip_(is_roundtrip){}
    std::string type_;
    std::string name_;
    Stops stops_;
    bool is_roundtrip_ = false;
};

class MultiBaseRequest : private std::variant<RequestAddStop, RequestAddBus>{
public:
    using variant::variant;

    bool IsRequestAddStop(){
        return std::holds_alternative<RequestAddStop>(*this);
    }

    RequestAddStop AsRequestAddStop(){
        return std::get<RequestAddStop>(*this);
    }

    bool IsRequestAddBus(){
        return std::holds_alternative<RequestAddBus>(*this);
    }

    RequestAddBus AsRequestAddBus(){
        return std::get<RequestAddBus>(*this);
    }
};

struct RequestGetInfo{
    RequestGetInfo(std::string type, std::string name, int id)
    : type_(type), name_(name), id_(id){}
    std::string type_;
    std::string name_;
    int id_ = 0;
};

struct RequestGetMap{
    RequestGetMap(std::string type, int id)
    : type_(type), id_(id){}
    std::string type_;
    int id_ = 0;
};

class MultiStatRequest : private std::variant<RequestGetInfo, RequestGetMap>{
public:
    using variant::variant;

    bool IsRequestGetInfo(){
        return std::holds_alternative<RequestGetInfo>(*this);
    }

    RequestGetInfo AsRequestGetInfo(){
        return std::get<RequestGetInfo>(*this);
    }

    bool IsRequestGetMap(){
        return std::holds_alternative<RequestGetMap>(*this);
    }

    RequestGetMap AsRequestGetMap(){
        return std::get<RequestGetMap>(*this);
    }
};

//Responses
struct BaseResponse{
    BaseResponse(int request_id)
    : request_id_(request_id){}
    int request_id_ = 0;
};

struct ResponseBusInfo : public BaseResponse{
    ResponseBusInfo(int request_id, double curvature, double route_length, int stop_count, int unique_stop_count)
    :BaseResponse(request_id), curvature_(curvature), route_length_(route_length),stop_count_(stop_count),
    unique_stop_count_(unique_stop_count){}

    double curvature_ = 0;
    double route_length_ = 0;
    int stop_count_ = 0;
    int unique_stop_count_ = 0;
};

struct ResponseStopInfo : public BaseResponse{
    ResponseStopInfo(int request_id, Buses buses)
    :BaseResponse(request_id), buses_(buses){}

    Buses buses_;
};

struct ResponseError : public BaseResponse{
    ResponseError(int request_id)
    :BaseResponse(request_id){}
    
    std::string error_message = "not found";
};

struct ResponseMap : BaseResponse{
    ResponseMap(int request_id, std::string map)
    : BaseResponse(request_id), map_(std::move(map)){}

    std:: string map_;
};

class MultiResponse : private std::variant<ResponseBusInfo, ResponseStopInfo, ResponseError, ResponseMap>{
public:
    using variant::variant;

    bool IsResponseBusInfo(){
        return std::holds_alternative<ResponseBusInfo>(*this);
    }

    ResponseBusInfo AsResponseBusInfo(){
        return std::get<ResponseBusInfo>(*this);
    }

    bool IsResponseStopInfo(){
        return std::holds_alternative<ResponseStopInfo>(*this);
    }

    ResponseStopInfo AsResponseStopInfo(){
        return std::get<ResponseStopInfo>(*this);
    }

    bool IsResponseError(){
        return std::holds_alternative<ResponseError>(*this);
    }

    ResponseError AsResponseError(){
        return std::get<ResponseError>(*this);
    }

    bool IsResponseMap(){
        return std::holds_alternative<ResponseMap>(*this);
    }

    ResponseMap AsResponseMap(){
        return std::get<ResponseMap>(*this);
    }
};

} // namespace detail

class RequestHandler{
public:
    void AddBaseRequest(detail::MultiBaseRequest&& request);
    void AddStatRequest(detail::MultiStatRequest&& request);
    void AddRenderSettings(map_render::RenderSettings&& settings);

    void ApplyRequests(TransportCatalogue& catalogue);
    std::vector<detail::MultiResponse>& GetResponses();
private:
    std::map<std::string, std::vector<detail::MultiBaseRequest>> base_requests;
    std::vector<detail::MultiStatRequest> stat_requests_;
    std::vector<detail::MultiResponse> responses_;
    map_render::MapRender map_render_;
};

} // namespace request_handler

} // namespace transport_catalogue