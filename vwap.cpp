#include <iostream>
#include <chrono>
#include "binary_ingester.h"

using namespace std;
using namespace std::chrono;

int main() 
{
    string file_name = "input";
    BinaryIngester bi{file_name};
    auto start = high_resolution_clock::now();
    bi.processBinary();
    bi.save_vwap_to_cvs();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    cout << "Time: " << duration.count() << " seconds" << endl;
}