#include <iostream>
#include <chrono>
#include "binary_ingester.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) 
{
    string file_name;
    // Assigns input name, by argument or default value
    if (argc > 1)
    {
        file_name = argv[1];
    }
    else
    {
        file_name = "input";
    }
    BinaryIngester bi{file_name};
    auto start = high_resolution_clock::now();
    bi.processBinary();
    // Assigns output name, by argument or default value
    if (argc > 2)
    {
        bi.save_vwap_to_cvs(argv[2]);
    }
    else
    {
        bi.save_vwap_to_cvs();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    cout << "Time: " << duration.count() << " seconds" << endl;
}