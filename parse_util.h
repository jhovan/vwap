#pragma once

template <typename T>
T parse_uint (char* bytes, int size = sizeof(T))
{
    T result{};
    auto* ubytes = reinterpret_cast<unsigned char*>(bytes);
    for (int i = 0; i < size; i++)
    {
        result = (result << 8) | ubytes[i];
    }
    return result;
}

