#include <sstream>
#include <unordered_set>
#include "transport_catalogue.h"

using std::string_literals::operator""s;
using transport_catalogue::detail::StopInfo;
using transport_catalogue::detail::BusInfo;

using namespace transport_catalogue;

void TransportCatalogue::AddStop(std::string_view stop_name, const Coordinates coords){
    stops_.push_back({std::string(stop_name), coords});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stop_to_buses_[&stops_.back()];
}

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const{
    if(stopname_to_stop_.count(std::string(stop_name))){
        return stopname_to_stop_.at(std::string(stop_name));
    }
    return nullptr;
}

void TransportCatalogue::AddBus(std::string_view bus_name, std::vector<std::string_view> route){
    std::vector<const Stop*> stops_for_bus;
    for(const std::string_view stop_name : route){
        const Stop* stop = FindStop(stop_name);
        stops_for_bus.push_back(stop);
        stop_to_buses_.at(stop).insert(std::string(bus_name));
    }
    buses_.push_back({std::string(bus_name), stops_for_bus});
    busname_to_bus_[buses_.back().name] = &buses_.back();
}

const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const{
    if(busname_to_bus_.count(std::string(bus_name))){
        return busname_to_bus_.at(std::string(bus_name));
    }
    return nullptr;
}


BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const{
    const Bus* bus = FindBus(bus_name);
    BusInfo result;
    if(bus == nullptr){
        return result;
    }
    result.isFind = true;
    result.R = GetStopsOnRoute(bus_name);
    result.U = GetUniqueStops(bus_name);
    result.L = GetRouteLength(bus_name);
    return result;

}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    const Stop* stop = FindStop(stop_name);
    StopInfo result;
    if(stop == nullptr){
        return result;
    }
    result.isFind = true;
    result.buses = GetSetBuses(stop);
    return result;
}

int TransportCatalogue::GetStopsOnRoute(std::string_view bus_name) const{
    return busname_to_bus_.at(std::string(bus_name))->stops.size();
}

int TransportCatalogue::GetUniqueStops(std::string_view bus_name) const{
    std::vector<const Stop*> stops = busname_to_bus_.at(std::string(bus_name))->stops;
    std::unordered_set<std::string> names_stops;
    for(const Stop* stop : stops){
        names_stops.insert(stop->name);
    }
    return names_stops.size();
}

double TransportCatalogue::GetRouteLength(std::string_view bus_name) const{
    std::vector<const Stop*> stops = busname_to_bus_.at(std::string(bus_name))->stops;
    double result = 0;
    for(int i = 0; i < stops.size()-1; ++i){
        result += geo::ComputeDistance(stops[i]->coords, stops[i+1]->coords);
    }
    return result;
}

std::set<std::string> TransportCatalogue::GetSetBuses(const Stop* stop) const{
    return stop_to_buses_.at(stop);
}
