#include <iostream>
#include <fstream>
#include <format>
#include <string>
#include <unordered_map>
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


struct Transaction 
{
    uint32_t shares;
    float price;
    uint64_t timestamp;
};

auto counter_by_type  = unordered_map<char, long long>{};

class MessageStream 
{

    ifstream file;
    char buffer[BUFFER_SIZE];

    void process_message (char* bytes, int size)
    {
        char type = bytes[2];
        if(counter_by_type.find(type) == counter_by_type.end())
        {
            counter_by_type[type] = 1;
        }
        else
        {
            counter_by_type[type]++;
        }
        uint64_t time = parse_uint<uint64_t>(&bytes[5], 6);
    }

    void proccess_buffer ()
    {
        static char incomplete_msg[MAX_MESSAGE_SIZE];
        static int incomplete_msg_offset = 0;

        int size;

        // process pending message from previous buffer
        if (incomplete_msg_offset) 
        {
            copy(buffer, buffer + MAX_MESSAGE_SIZE - incomplete_msg_offset, incomplete_msg + incomplete_msg_offset);
            // size is not part of the message length and it takes 2 bytes
            size = parse_uint<uint16_t>(incomplete_msg) + 2;
            process_message(incomplete_msg, size);
        }

        // process new messages
        int begin = incomplete_msg_offset? size - incomplete_msg_offset: 0;
        int next_begin;
        incomplete_msg_offset = 0;
        while (begin < BUFFER_SIZE - 1)
        {
            // size is not part of the message length and it takes 2 bytes
            size = parse_uint<uint16_t>(&buffer[begin]) + 2;
            next_begin = begin + size;
            // the message is incomplete
            if(next_begin > BUFFER_SIZE)
            {
                copy(buffer + begin, buffer + BUFFER_SIZE, incomplete_msg);
                incomplete_msg_offset = BUFFER_SIZE - begin;
                break;
            } 
            process_message(&buffer[begin], size);
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
    unordered_map<char, int> message_counter;
    long long int counter = 0;

    MessageStream (string file_name) 
    {
        file.open(file_name, ios::binary | ios::in);
    }

    void start_processing () 
    {
        while (!file.eof()) 
        {
            file.read(buffer, BUFFER_SIZE);
            proccess_buffer();
        }
    }
};

int main() 
{
    string file_name = "input";
    MessageStream ms{file_name};
    auto start = high_resolution_clock::now();
    ms.start_processing();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<minutes>(stop - start);
    cout << "Total: " << ms.counter << endl;
    cout << "Time: " << duration.count() << " minutes" << endl;
    for(const auto& elem : counter_by_type)
    {
        cout << elem.first << " " << elem.second << "\n";
    }
}