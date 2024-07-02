#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "constants.h"
#include "message_wrapper.h"

using namespace std;

class BinaryIngester
{
    struct Trade 
    {
        uint8_t hour;
        uint16_t stock_locate;
        uint32_t shares;
        uint32_t price;
    }; 

    struct CrossTrade
    {
        uint8_t hour;
        uint16_t stock_locate;
        uint64_t shares;
        uint32_t price;
    };

    struct Order
    {
        uint32_t shares;
        uint32_t price;
    };

    ifstream file;
    char buffer[BUFFER_SIZE];
    float progress_increment;
    vector<string> directory;
    unordered_map<uint64_t, Order> order_book;
    unordered_map<uint64_t, Trade> trade_book;
    unordered_map<uint64_t, CrossTrade> cross_trade_book;
    uint8_t min_hour, max_hour, is_first_trade;

    void updateDirectory (MessageWrapper& message);

    void addOrder (MessageWrapper& message);

    void cancelOrder (MessageWrapper& message);

    void deleteOrder (MessageWrapper& message);

    void updateGlobalTimeBounds (uint8_t hour);

    void addTrade(MessageWrapper& message);

    void removeTrade(MessageWrapper& message);

    void processMessage (MessageWrapper message);

    void processBuffer ();

public:

    BinaryIngester (string file_name);

    void processBinary ();

    void save_vwap_to_cvs (string output_file_name = "output.csv");
};