#include <iostream>
#include <fstream>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <chrono>
#include "constants.h"
#include "parse_util.h"

using namespace std;
using namespace std::chrono;


/*
Read again about the different types of Order and Trade messages
Particularly 1.3, 1.5 and 1.5
To decide which messages are actually relevant

I will probably have to use a buffer, like the CTC problem
Because I can't define different size arrays without using high level
Data structures and passing them around.

I can also just read size and type,
and load the whole message if relevant.


Idea

2 parallel processes:

1. read file and enqueue transaction messages
2. process messages in the queue, what if queue runs out of memory? 
// reading takes more time than processing
// except if we are printing too much

// I think skiping messages makes sense, and just trying threads if too slow.


Will I have to remember transactions ids?
Storage concern in that case.

Dictionary by stock and vwap.

Lets print output, but maybe save to file if too long.

Lets count transactions
*/

class MessageWrapper
{

    const unordered_map<MessageAttribute, pair<uint8_t, uint8_t>>* attributes_map;
    MessageType type;
    char* bytes;

    MessageWrapper () = delete;

    pair<uint8_t, uint8_t> getAttributeOffsetAndSize(MessageAttribute attribute) const
    {
        if (!attributes_map || attributes_map->find(attribute) == attributes_map->end())
        {
            throw ("Invalid Attribute or Type");
        }
        return attributes_map->at(attribute);
    }

    template<typename T>
    T getUIntAttribute(MessageAttribute attribute) const
    {
        auto pair = getAttributeOffsetAndSize(attribute);
        return parse_uint<T>(bytes + pair.first, pair.second);
    }

public:

    // Gets the pointer to the the start of the message (size bytes included)
    MessageWrapper (char* bytes): bytes{bytes + MSG_SIZE_SIZE} 
    {

        // test array
        /*
        char message[MAX_MESSAGE_SIZE];
        for (int i=0; i < MAX_MESSAGE_SIZE; i++) {
            message[i] = this->bytes[i];
        }
        */

        type = MessageType{this->bytes[0]};
        if(GLOBAL_ATTRIBUTE_MAP.find(type) != GLOBAL_ATTRIBUTE_MAP.end())
        {
            attributes_map = &GLOBAL_ATTRIBUTE_MAP.at(type);
        }
        else
        {
            attributes_map = nullptr;
        }
        
    }

    MessageType getType () const
    {
        return type;
    }

    uint64_t getTimestamp () const
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::Timestamp);
    }

    uint8_t getHour () const 
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::Timestamp)/NANOSECONDS_IN_HOUR;
    }

    uint64_t getRefNo () const
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::RefNo);
    }

    uint64_t getNewRefNo () const
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::NewRefNo);
    }
    
    uint64_t getMatchNo () const
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::MatchNo);
    }

    uint64_t getShares () const
    {
        return getUIntAttribute<uint64_t>(MessageAttribute::Shares);
    }

    string getStock () const
    {
        
        auto pair = getAttributeOffsetAndSize(MessageAttribute::Stock);
        return string(bytes + pair.first, pair.second);
    }

    uint16_t getStockLocate () const
    {
        return getUIntAttribute<uint16_t>(MessageAttribute::StockLocate);
    }

    uint32_t getPrice () const
    {
        return getUIntAttribute<uint32_t>(MessageAttribute::Price);
    }

    bool isPrintable () const
    {   
        auto printable = getUIntAttribute<char>(MessageAttribute::Printable);
        return printable == 'Y';
    }

};

class BinaryIngester
{
    struct Trade 
    {
        uint8_t hour;
        uint16_t stock_locate;
        // it is bigger to cover Cross Trades too
        uint64_t shares;
        uint32_t price;
    }; 

    struct Order
    {
        uint32_t shares;
        uint32_t prices;
    };
    

    ifstream file;
    char buffer[BUFFER_SIZE];
    vector<string> directory;

    //unordered_map<char, long long> counter_by_type  = unordered_map<char, long long>{};

    void updateDirectory(MessageWrapper message)
    {
        auto sl = message.getStockLocate();
        auto stock = message.getStock();

        // Directory size doesn't change
        if (directory.size() > sl)
        {
            directory[sl] = stock;
            return;
        }

        // Needs to expand the directory
        if (directory.size() < sl)
        {
            directory.resize(sl);
        }
        directory.push_back(stock);
    }

    void addOrder(MessageWrapper message)
    {
        //auto time = message.getHour();
    }

    void addTrade(MessageWrapper message)
    {
        auto time = message.getHour();
    }

    void processMessage (MessageWrapper message)
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
        case MessageType::Trade:
            addTrade(message);
            break;
        default:
            break;
        }
    }

    void processBuffer ()
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
    
public:

    BinaryIngester (string file_name) 
    {
        file.open(file_name, ios::binary | ios::in);
        directory = {""};
        directory.reserve(DIRECTORY_SIZE_RESERVATION);
    }

    void startProcessing () 
    {
        while (!file.eof()) 
        {
            file.read(buffer, BUFFER_SIZE);
            processBuffer();
        }
    }
};

int main() 
{
    string file_name = "input";
    BinaryIngester bi{file_name};
    auto start = high_resolution_clock::now();
    bi.startProcessing();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    cout << "Time: " << duration.count() << " seconds" << endl;
}