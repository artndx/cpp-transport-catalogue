#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue{

namespace stat_reader{

struct Request{
    std::string command;
    std::string data;
};

Request ParseRequest(std::string_view request);
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

}; //namespace stat_reader

}; //namespace transport_catalogue
