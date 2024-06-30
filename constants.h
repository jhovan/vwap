#pragma once

#include <unordered_map>
#include <utility>

// Sizes in Bytes

inline const int BUFFER_SIZE = 64000;

// It corresponds to Trade message type (including 2 extra bytes to store size itself)
inline const int MAX_MESSAGE_SIZE = 52;

// Nanoseconds in 1 hour
inline const u_int64_t  NANOSECONDS_IN_HOUR = 3600000000000;

// Relevant message attributes (similar names and data types grouped)
enum class MessageAttribute:char {
    Timestamp,
    RefNo,
    NewRefNo,
    MatchNo,
    Shares,
    LongShares,
    StockLocale,
    Stock,
    Price,
    Printable,
};

// Relevant message attributes offsets and sizes by type
inline const std::unordered_map<char,std::unordered_map<MessageAttribute, std::pair<uint8_t, uint8_t>>> GLOBAL_ATTRIBUTE_MAP = {
    {
        // Stock Directory
        'R', 
        {
            {MessageAttribute::StockLocale, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::Stock, {11, 8}}
        },
    },
    {
        'A', 
        {
            {MessageAttribute::StockLocale, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}}
        }
    },
    {
        // Add Order with MPID Attribution
        'F', 
        {
            {MessageAttribute::StockLocale, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}}
        }
    },
    {
        // Order ExecutedMessage
        'E', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {19, 4}},
            {MessageAttribute::MatchNo, {23, 8}}
        }
    },
    {
        // Order Executed With PriceMessage
        'C', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {19, 4}},
            {MessageAttribute::MatchNo, {23, 8}},
            {MessageAttribute::Printable, {31, 1}},
            {MessageAttribute::Price, {32, 4}}
        }
    },
    {
        // Order Cancel Message
        'X', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {19, 4}}
        }
    },
    {
        // Order Delete Message
        'D', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
        }
    },
    {
        // Order Replace Message
        'U', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::NewRefNo, {19, 8}},
            {MessageAttribute::Shares, {27, 4}},
            {MessageAttribute::Price, {31, 4}}
        }
    },
    {
        // Trade Message (Non-Cross)
        'P', 
        {
            {MessageAttribute::StockLocale, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}},
            {MessageAttribute::MatchNo, {36, 8}}
        }
    },
    {
        // Cross Trade Message 
        'Q', 
        {
            {MessageAttribute::StockLocale, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::LongShares, {11, 8}},
            {MessageAttribute::Price, {27, 4}},
            {MessageAttribute::MatchNo, {31, 8}}
        }
    },
    {
        // Broken Trade / Order ExecutionMessage
        'B', 
        {
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::MatchNo, {11, 8}}
        }
    },
    
};