#include "stat_reader.h"

#include <iostream>
#include <iomanip>
using namespace transport_catalogue;
using std::string_literals::operator""s;
using transport_catalogue::detail::BusInfo;
using transport_catalogue::detail::StopInfo;

stat_reader::Request stat_reader::ParseRequest(std::string_view request){
    size_t space = request.find_first_of(' ');
    std::string command = std::string(request.substr(0, space));
    std::string data = std::string(request.substr(space+1, request.size()));
    return {command, data};
}

void Trim(std::string& string) {
    const auto start = string.find_first_not_of(' ');
    const auto end = string.find_last_not_of(' ') - start + 1;
    string = string.substr(start, end);
}

void PrintBusInfo(std::string_view data, const BusInfo& result, std::ostream& output){
    if(!result.is_find){
        output << "Bus "s << data << ": not found\n"s;
        return;
    }
    output << std::fixed << std::setprecision(5) 
    << "Bus "s << data << ": "s 
    << result.route_length << " stops on route, " 
    << result.unique_stops << " unique stops, " 
    << result.lenght_bus << " route length, "
    << result.curvature << " route curvature\n";
}

void PrintStopInfo(std::string_view data, const StopInfo& result, std::ostream& output){
    if(!result.is_find){
        output << "Stop "s << data << ": not found\n"s;
        return;
    }
    std::set<std::string> buses = result.buses;
    if(buses.empty()){
        output << "Stop "s << data << ": no buses\n"s;
        return;
    }
    output << "Stop "s << data << ": buses";
    for(const std::string& bus : buses){
        output << ' ' << bus;
    }
    output << "\n";
}

void stat_reader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view raw_request,
                       std::ostream& output) {
    auto [command, data] = stat_reader::ParseRequest(raw_request);
    Trim(data);
    if(command == "Bus"){
        BusInfo result = transport_catalogue.GetBusInfo(data);
        PrintBusInfo(data, result, output);
    }
    if(command == "Stop"){
        StopInfo result = transport_catalogue.GetStopInfo(data);
        PrintStopInfo(data, result, output);
    }
}