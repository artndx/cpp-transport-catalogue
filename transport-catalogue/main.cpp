#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport_catalogue::json_reader;

int main() {
    TransportCatalogue catalogue;
    JsonReader reader(cin);
    reader.SendRequests(catalogue);
    reader.GetResponses(cout);
}