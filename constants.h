#pragma once

#include <unordered_map>
#include <utility>

// Sizes in Bytes

inline const uint16_t BUFFER_SIZE = 64000;

// It corresponds to Trade message type (including 2 extra bytes to store size itself)
inline const uint8_t MAX_MESSAGE_SIZE = 52;
inline const uint8_t MSG_SIZE_SIZE = 2;

// Nanoseconds in 1 hour
inline const u_int64_t NANOSECONDS_IN_HOUR = 3600000000000;

// Based on historical data from the binary (8714)
inline const uint16_t DIRECTORY_SIZE_RESERVATION = 10000;

// StockLocate values start with from 1
inline const uint8_t DIRECTORY_OFFSET =  1;
// TODO Update: locate_offset
// TODO Time_offset

// Relevant Message 
enum class MessageType:char 
{
    StockDirectory = 'R',
    AddOrder = 'A',
    AddOrderMPID = 'F',
    OrderExecuted = 'E',
    OrderExecutedWithPrice = 'C',
    OrderCancel = 'X',
    OrderDelete = 'D',
    OrderReplace = 'U',
    Trade = 'P',
    CrossTrade = 'Q',
    BrokenTrade = 'B'
};

// Relevant message attributes (similar names and data types grouped)
enum class MessageAttribute:char 
{
    Timestamp,
    RefNo,
    NewRefNo,
    MatchNo,
    Shares,
    LongShares,
    StockLocate,
    Stock,
    Price,
    Printable,
};

// Relevant message attributes offsets and sizes by type
inline const std::unordered_map<MessageType,std::unordered_map<MessageAttribute, std::pair<uint8_t, uint8_t>>> GLOBAL_ATTRIBUTE_MAP = {
    {
        // Stock Directory
        MessageType::StockDirectory, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::Stock, {11, 8}}
        },
    },
    {
        // Add Order – No MPID Attribution
        MessageType::AddOrder, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}}
        }
    },
    {
        // Add Order with MPID Attribution
        MessageType::AddOrderMPID, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}}
        }
    },
    {
        // Order ExecutedMessage
        MessageType::OrderExecuted, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {19, 4}},
            {MessageAttribute::MatchNo, {23, 8}}
        }
    },
    {
        // Order Executed With PriceMessage
        MessageType::OrderExecutedWithPrice, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
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
        MessageType::OrderCancel, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::Shares, {19, 4}}
        }
    },
    {
        // Order Delete Message
        MessageType::OrderDelete, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
        }
    },
    {
        // Order Replace Message
        MessageType::OrderReplace, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::RefNo, {11, 8}},
            {MessageAttribute::NewRefNo, {19, 8}},
            {MessageAttribute::Shares, {27, 4}},
            {MessageAttribute::Price, {31, 4}}
        }
    },
    {
        // Trade Message (Non-Cross)
        MessageType::Trade, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::Shares, {20, 4}},
            {MessageAttribute::Price, {32, 4}},
            {MessageAttribute::MatchNo, {36, 8}}
        }
    },
    {
        // Cross Trade Message 
        MessageType::CrossTrade, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::LongShares, {11, 8}},
            {MessageAttribute::Price, {27, 4}},
            {MessageAttribute::MatchNo, {31, 8}}
        }
    },
    {
        // Broken Trade / Order ExecutionMessage
        MessageType::BrokenTrade, 
        {
            {MessageAttribute::StockLocate, {1, 2}},
            {MessageAttribute::Timestamp, {5, 6}},
            {MessageAttribute::MatchNo, {11, 8}}
        }
    },
    
};