#pragma once
#include <deque>
#include <set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace transport_catalogue{

namespace detail{

struct Stop{
	std::string name;
	geo::Coordinates coords;
};


struct Bus{
	std::string name;
	std::vector<const Stop*> stops;
};

struct StopInfo{
	bool isFind = false;
	std::set<std::string> buses {};
};

struct BusInfo{
	bool isFind = false;
	int R = 0;
	int U = 0;
	double L = 0;
};

}; //namespace detail

using geo::Coordinates;
using detail::Stop;
using detail::Bus;
using detail::StopInfo;
using detail::BusInfo;


class TransportCatalogue {
public:
	void AddStop(std::string_view stop_name, Coordinates coords);
	const Stop* FindStop(std::string_view stop_name) const;
	void AddBus(std::string_view bus_name, std::vector<std::string_view> route);
	const Bus* FindBus(std::string_view bus_name) const;
	BusInfo GetBusInfo(std::string_view bus_name) const;
	StopInfo GetStopInfo(std::string_view stop_name) const;

private:

	int GetStopsOnRoute(std::string_view bus_name) const;
	int GetUniqueStops(std::string_view bus_name) const;
	double GetRouteLength(std::string_view bus_name) const;

	std::set<std::string> GetSetBuses(const Stop* stop) const;

	std::deque<Stop> stops_; 
	std::unordered_map<std::string, Stop*> stopname_to_stop_;

	std::deque<Bus> buses_;	
	std::unordered_map<std::string, Bus*> busname_to_bus_;

	std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses_;
};

}; //namespace transport_catalogue