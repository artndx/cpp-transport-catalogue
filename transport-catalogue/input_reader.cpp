#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>

using namespace transport_catalogue;

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

geo::Distance ParseDistance(const std::string& str){
    auto m_pos =  str.find('m');
    auto to_pos = str.find("to");
    if(m_pos == str.npos || to_pos == str.npos){
        throw std::invalid_argument("Incorrect distance addition");
    }
    std::string distance = std::string(Trim(str.substr(0,m_pos)));
    std::string stop_name = std::string(Trim(str.substr(to_pos + 3, str.npos)));
    return {stop_name, stoi(distance)};
}
//"7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"
std::vector<geo::Distance> ParseDistances(const std::string& str){
    std::vector<Distance> result;
    if(str.find(',') == str.npos){
        result.push_back(ParseDistance(str));
        return result;
    }
    size_t begin = 0;
    size_t comma = str.find(',');
    while(comma != str.npos){
        std::string raw_distance = std::string(str.substr(begin, comma - begin));
        result.push_back(ParseDistance(raw_distance));
        begin = comma + 2;
        comma = str.find(',', begin + 1);
    }
    std::string raw_distance = std::string(str.substr(begin, comma - begin));
    result.push_back(ParseDistance(raw_distance));
    return result;
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

input_reader::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    std::string command = std::string(line.substr(0, space_pos));
    std::string id = std::string(line.substr(not_space, colon_pos - not_space));
    std::string description = std::string(line.substr(colon_pos + 2));


    return {command, id, description};
}

void input_reader::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_[command_description.command].push_back(std::move(command_description));
    }
}

void input_reader::InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    if(commands_.empty()){
        return;
    }
    std::unordered_map<std::string, std::string> stop_name_to_distance_commands;
    for(const CommandDescription& com : commands_.at("Stop")){
        auto pos = com.description.find(',', com.description.find(',') + 1);
        std::string coords = com.description.substr(0, pos);
        std::string distances;
        if(pos != com.description.npos){
            distances = com.description.substr(pos + 2, com.description.npos);
        }
        if(!distances.empty()){
            stop_name_to_distance_commands[com.id] = distances;
        }
        catalogue.AddStop(com.id, ParseCoordinates(coords));
    }
    for(const auto& [stop_id, stop_distances] : stop_name_to_distance_commands){
        std::vector<geo::Distance> distances = ParseDistances(stop_distances);
        for(const geo::Distance& dist : distances){
            catalogue.AddStopDistance(stop_id, dist.stop_name, dist.distance);
        }
    }
    for(const CommandDescription& com : commands_.at("Bus")){
        catalogue.AddBus(com.id, ParseRoute(com.description));
    }
}
