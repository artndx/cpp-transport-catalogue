#include "stat_reader.h"

using namespace transport_catalogue;


stat_reader::Request stat_reader::ParseRequest(std::string_view request){
    size_t space = request.find_first_of(' ');
    std::string command = std::string(request.substr(0, space));
    std::string data = std::string(request.substr(space+1, request.size()));
    return {command, data};
}

void stat_reader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view raw_request,
                       std::ostream& output) {
    auto [command, data] = stat_reader::ParseRequest(raw_request);
    if(command == "Bus"){
        output << transport_catalogue.GetBusInfo(data);
    }
    if(command == "Stop"){
        output << transport_catalogue.GetStopInfo(data);
    }
}

