#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include <fstream>

using namespace std;

int main() {
    Catalogue::TransportCatalogue catalogue;

    {
        InputReader reader(cin);
        reader.ReadAndApplyCommands(catalogue);
    }

    {
        StatReader reader(cin, cout);
        reader.ReadAndProcessStats(catalogue);  
    }
}