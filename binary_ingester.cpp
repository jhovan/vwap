#include <filesystem>
#include "binary_ingester.h"

void BinaryIngester::updateDirectory (MessageWrapper& message)
{
    auto stock_locate = message.getStockLocate();
    auto stock = message.getStock();

    // Directory size doesn't change
    if (directory.size() > stock_locate)
    {
        directory[stock_locate] = stock;
        return;
    }

    // Needs to expand the directory
    if (directory.size() < stock_locate)
    {
        directory.resize(stock_locate);
    }
    directory.push_back(stock);
}

void BinaryIngester::addOrder (MessageWrapper& message)
{
    auto ref_no = message.getRefNo();
    auto shares = message.getShares();
    auto price = message.getPrice();
    order_book[ref_no] = Order{shares, price};
}

void BinaryIngester::cancelOrder (MessageWrapper& message)
{   
    auto ref_no = message.getRefNo();
    auto cancelled_shares = message.getShares();
    order_book[ref_no].shares -= cancelled_shares;
    if (order_book[ref_no].shares <= 0)
    {
        order_book.erase(ref_no);
    }
}

void BinaryIngester::deleteOrder (MessageWrapper& message)
{
    auto ref_no = message.getRefNo();
    order_book.erase(ref_no);
}

void BinaryIngester::updateGlobalTimeBounds (uint8_t hour)
{
    if (is_first_trade)
    {
        min_hour = hour;
        max_hour = hour;
        is_first_trade = false;
    }
    min_hour = min(hour, min_hour);
    max_hour = max(hour, max_hour);
}

void BinaryIngester::addTrade(MessageWrapper& message)
{
    auto type = message.getType();
    auto hour = message.getHour();
    updateGlobalTimeBounds(hour);
    auto match_no = message.getMatchNo();
    auto stock_locate = message.getStockLocate();
    auto price = type == MessageType::OrderExecuted?
        order_book[message.getRefNo()].price: message.getPrice();  
    if  (type == MessageType::CrossTrade)
    {
        cross_trade_book[match_no] = CrossTrade{hour, stock_locate, message.getLongShares(), price};
    }
    else
    {
        trade_book[match_no] = Trade{hour, stock_locate, message.getShares(), price}; 
    } 
}

void BinaryIngester::removeTrade(MessageWrapper& message)
{
    auto match_no = message.getMatchNo();
    trade_book.erase(match_no);
    cross_trade_book.erase(match_no);
}

void BinaryIngester::processMessage (MessageWrapper message)
{
    auto type = message.getType();
    switch (type)
    {
    case MessageType::StockDirectory:
        updateDirectory(message);
        break;
    case MessageType::AddOrder:
    case MessageType::AddOrderMPID:
        addOrder(message);
        break;
    case MessageType::OrderExecutedWithPrice:
        if(!message.isPrintable()) break;
    case MessageType::OrderExecuted:
        addTrade(message);
        //cancelOrder(message);
        break;
    case MessageType::OrderCancel:
        //cancelOrder(message);
        break;
    case MessageType::OrderDelete:
        //deleteOrder(message);
        break;
    case MessageType::OrderReplace:
        //deleteOrder(message);
        addOrder(message);
        break;
    case MessageType::Trade:
    case MessageType::CrossTrade:
        addTrade(message);
        break;
    case MessageType::BrokenTrade:
        removeTrade(message);
    default:
        break;
    }
}

void BinaryIngester::processBuffer ()
{
    static char incomplete_msg[MAX_MESSAGE_SIZE];
    static int incomplete_msg_offset = 0;

    int size;

    // process pending message from previous buffer
    if (incomplete_msg_offset) 
    {
        copy(buffer, buffer + MAX_MESSAGE_SIZE - incomplete_msg_offset, incomplete_msg + incomplete_msg_offset);
        // size is not part of the message length and it takes 2 bytes
        size = parse_uint<uint16_t>(incomplete_msg) + MSG_SIZE_SIZE;
        processMessage(MessageWrapper{incomplete_msg});
    }

    // process new messages
    int begin = incomplete_msg_offset? size - incomplete_msg_offset: 0;
    int next_begin;
    incomplete_msg_offset = 0;
    while (begin < BUFFER_SIZE - 1)
    {
        // size is not part of the message length and it takes 2 bytes
        size = parse_uint<uint16_t>(&buffer[begin]) + MSG_SIZE_SIZE;
        next_begin = begin + size;
        // the message is incomplete
        if(next_begin > BUFFER_SIZE)
        {
            copy(buffer + begin, buffer + BUFFER_SIZE, incomplete_msg);
            incomplete_msg_offset = BUFFER_SIZE - begin;
            break;
        } 
        processMessage(MessageWrapper{buffer+begin});
        begin = next_begin;
    }
    // the size was incomplete, edge case
    if (begin == BUFFER_SIZE - 1)
    {
        incomplete_msg_offset = 1;
        incomplete_msg[0] = buffer[begin];
    }
}

BinaryIngester::BinaryIngester (string file_name) 
{
    file.open(file_name, ios::binary | ios::in);
    progress_increment = static_cast<float>(BUFFER_SIZE)/filesystem::file_size(file_name);
    is_first_trade = true;
    directory = {""};
    directory.reserve(DIRECTORY_RESERVATION_SIZE);
    order_book.reserve(ORDER_BOOK_RESERVATION_SIZE);
    trade_book.reserve(TRADE_BOOK_RESERVATION_SIZE);
    cross_trade_book.reserve(CROSS_TRADE_BOOK_RESERVATION_SIZE);
}

void BinaryIngester::processBinary () 
{
    int buffer_counter = 0;
    int prev_percentage = 0;
    int percentage;
    while (!file.eof()) 
    {   
        file.read(buffer, BUFFER_SIZE);
        processBuffer();
        buffer_counter++;
        percentage = static_cast<int>(100 * progress_increment * buffer_counter);
        if(percentage > prev_percentage)
        {
            cout << "Processing File: " << percentage << "%" << endl;
        }
        prev_percentage = percentage;
    }
    file.close();
}

void BinaryIngester::save_vwap_to_cvs (string output_file_name){

    int time_range = max_hour - min_hour + 1;
    double vwap[time_range][directory.size()];
    long long total_shares[time_range][directory.size()];
    // Initialize Arrays
    for (int stock = 0; stock < directory.size(); stock++)
    {
        for (int hour = 0; hour < time_range - 1; hour++)
        {
            vwap[hour][stock] = 0;
            total_shares[hour][stock] = 0;
        }
    }
    for (auto it: trade_book) {
        Trade& t = it.second;
        // the price has 4 implicit decimal positions
        vwap[t.hour - min_hour][t.stock_locate] += t.shares * (t.price/10000.0);
        total_shares[t.hour - min_hour][t.stock_locate] += t.shares;
    }
    for (auto it: cross_trade_book) {
        CrossTrade& t = it.second;
        // the price has 4 implicit decimal positions
        vwap[t.hour - min_hour][t.stock_locate] += t.shares * (t.price/10000.0);
        total_shares[t.hour - min_hour][t.stock_locate] += t.shares;
    }
    for (int stock = 1; stock < directory.size(); stock++)
    {
        vwap[1][stock] += vwap[0][stock];
        vwap[0][stock] = total_shares[0][stock]?vwap[0][stock]/total_shares[0][stock]: 0;
        for (int hour = 1; hour < time_range - 1; hour++)
        {
            vwap[hour + 1][stock] += vwap[hour][stock];
            total_shares[hour][stock] += total_shares[hour - 1][stock];
            vwap[hour][stock] = total_shares[hour][stock]?vwap[hour][stock]/total_shares[hour][stock]: 0;
        }
        int last_hour = time_range - 1;
        total_shares[last_hour][stock] += total_shares[last_hour - 1][stock];
        vwap[last_hour][stock] = total_shares[last_hour][stock]?vwap[last_hour][stock]/total_shares[last_hour][stock]: 0;
    }

    ofstream output_file;
    output_file.open(output_file_name, ios::out|ios::trunc);
    output_file << ",";
    for (int i = 0; i < time_range; i++) 
    {
        output_file << i + min_hour <<":00,";
    }
    output_file << '\n';
    for (int i = 1; i < directory.size(); i++) 
    {
        output_file << directory[i] <<",";
        for (int j = 0; j < time_range; j++)
            output_file << vwap[j][i] <<',';
        output_file << '\n';
    }
    output_file.close();
    cout << "VWAP exported to " << output_file_name << endl;
}
