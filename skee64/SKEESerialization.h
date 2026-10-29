#pragma once

#include "StringTable.h"
#include <cstring>

// SKSE's latent-functor wire format (skse64/Serialization.cpp): bool bytes,
// or uint16 length followed by at most 256 string bytes, without a terminator.
// Never serialize the BSFixedString pointer/control block.
namespace Serialization
{
template <>
inline bool WriteData<bool>(const SKSE::SerializationInterface* a_intfc, const bool* a_value)
{
    return a_intfc->WriteRecordData(a_value, sizeof(*a_value));
}

template <>
inline bool ReadData<bool>(const SKSE::SerializationInterface* a_intfc, bool* a_value)
{
    static_assert(sizeof(bool) == sizeof(std::uint8_t));
    std::uint8_t encoded = 0;
    if (a_intfc->ReadRecordData(&encoded, sizeof(encoded)) != sizeof(encoded) || encoded > 1)
        return false;
    *a_value = encoded != 0;
    return true;
}

template <>
inline bool WriteData<RE::BSFixedString>(const SKSE::SerializationInterface* a_intfc, const RE::BSFixedString* a_value)
{
    const auto* text = a_value->c_str();
    const auto size = std::strlen(text);
    if (size > 256)
        return false;
    const auto length = static_cast<std::uint16_t>(size);
    return a_intfc->WriteRecordData(&length, sizeof(length)) &&
           (length == 0 || a_intfc->WriteRecordData(text, length));
}

template <>
inline bool ReadData<RE::BSFixedString>(const SKSE::SerializationInterface* a_intfc, RE::BSFixedString* a_value)
{
    std::uint16_t length = 0;
    if (a_intfc->ReadRecordData(&length, sizeof(length)) != sizeof(length) || length > 256)
        return false;
    char text[257] = {};
    if (length != 0 && a_intfc->ReadRecordData(text, length) != length)
        return false;
    *a_value = RE::BSFixedString(text);
    return true;
}
}
