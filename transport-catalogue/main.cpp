#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::input_reader;
using namespace transport_catalogue::stat_reader;

int main() {
    TransportCatalogue catalogue;

    ifstream input("input.txt");


    int base_request_count;
    input >> base_request_count >> ws;

    {
        InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(input, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    input >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, cout);
    }
}