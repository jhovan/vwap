#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include "constants.h"
#include "parse_util.h"

using namespace std;

class MessageWrapper
{

    const unordered_map<MessageAttribute, pair<uint8_t, uint8_t>>* attributes_map;
    MessageType type;
    char* bytes;

    MessageWrapper () = delete;

    pair<uint8_t, uint8_t> getAttributeOffsetAndSize(MessageAttribute attribute) const;

    template<typename T>
    T getUIntAttribute(MessageAttribute attribute) const
    {
        auto pair = getAttributeOffsetAndSize(attribute);
        return parse_uint<T>(bytes + pair.first, pair.second);
    }

public:

    // Gets the pointer to the the start of the message (size bytes included)
    MessageWrapper (char* bytes);

    MessageType getType () const;

    uint64_t getTimestamp () const;

    uint8_t getHour () const;

    uint64_t getRefNo () const;

    uint64_t getNewRefNo () const;
    
    uint64_t getMatchNo () const;

    uint32_t getShares () const;

    uint64_t getLongShares () const;

    string getStock () const;

    uint16_t getStockLocate () const;

    uint32_t getPrice () const;

    bool isPrintable () const;

};