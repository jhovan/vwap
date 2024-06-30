#pragma once

#include<unordered_map>

// Sizes in Bytes

inline const int BUFFER_SIZE = 64000;

// It corresponds to Trade message type (including 2 extra bytes to store size itself)
inline const int MAX_MESSAGE_SIZE = 52;

enum class MessageAttribute {
    Timestamp,
    RefNo,
    MatchNo,
    Shares,
    Stock,
    Price,
    Printable,
};