#pragma once

template <typename T>
T parse_uint (char* bytes, int size = sizeof(T))
{
    T result{};
    for (int i = 0; i < size; i++)
    {
        result << 8;
        result |= bytes[i];
    }
    return result;
}