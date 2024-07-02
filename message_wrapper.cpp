#include "message_wrapper.h"

using namespace std;

pair<uint8_t, uint8_t> MessageWrapper::getAttributeOffsetAndSize(MessageAttribute attribute) const
{
    if (!attributes_map || attributes_map->find(attribute) == attributes_map->end())
    {
        throw ("Invalid Attribute or Type");
    }
    return attributes_map->at(attribute);
}

MessageWrapper::MessageWrapper (char* bytes): bytes{bytes + MSG_SIZE_SIZE} 
{
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

MessageType MessageWrapper::getType () const
{
    return type;
}

uint64_t MessageWrapper::getTimestamp () const
{
    return getUIntAttribute<uint64_t>(MessageAttribute::Timestamp);
}

uint8_t MessageWrapper::getHour () const 
{
    return getUIntAttribute<uint64_t>(MessageAttribute::Timestamp)/NANOSECONDS_IN_HOUR;
}

uint64_t MessageWrapper::getRefNo () const
{
    return getUIntAttribute<uint64_t>(MessageAttribute::RefNo);
}

uint64_t MessageWrapper::getNewRefNo () const
{
    return getUIntAttribute<uint64_t>(MessageAttribute::NewRefNo);
}

uint64_t MessageWrapper::getMatchNo () const
{
    return getUIntAttribute<uint64_t>(MessageAttribute::MatchNo);
}

uint32_t MessageWrapper::getShares () const
{
    return getUIntAttribute<uint32_t>(MessageAttribute::Shares);
}

uint64_t MessageWrapper::getLongShares () const
{
    return getUIntAttribute<uint64_t>(MessageAttribute::LongShares);
}

string MessageWrapper::getStock () const
{
    auto pair = getAttributeOffsetAndSize(MessageAttribute::Stock);
    return string(bytes + pair.first, pair.second);
}

uint16_t MessageWrapper::getStockLocate () const
{
    return getUIntAttribute<uint16_t>(MessageAttribute::StockLocate);
}

uint32_t MessageWrapper::getPrice () const
{
    return getUIntAttribute<uint32_t>(MessageAttribute::Price);
}

bool MessageWrapper::isPrintable () const
{   
    auto printable = getUIntAttribute<char>(MessageAttribute::Printable);
    return printable == 'Y';
}
