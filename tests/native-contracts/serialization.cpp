#include "SKEESerialization.h"
#include <cstdlib>
#include <iostream>
#include <string>

static void check(bool condition, const char* message) {
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
    using namespace Serialization;
    SKSE::SerializationInterface stream;
    bool value = true, result = false;
    check(WriteData(&stream, &value), "bool writes");
    check(stream.bytes == std::vector<unsigned char>{1}, "legacy bool wire byte");
    check(ReadData(&stream, &result) && result, "bool round trip");
    check(!ReadData(&stream, &result), "truncated bool rejected");
    stream = {};
    value = false;
    check(WriteData(&stream, &value) && stream.bytes == std::vector<unsigned char>{0}, "false wire byte");
    stream.bytes = {2};
    stream.cursor = 0;
    result = false;
    check(!ReadData(&stream, &result) && !result, "noncanonical bool rejected without destination mutation");

    stream = {};
    RE::BSFixedString text("Race"), restored("previous");
    check(WriteData(&stream, &text), "fixed string writes");
    check(stream.bytes == std::vector<unsigned char>{4, 0, 'R', 'a', 'c', 'e'}, "legacy string wire bytes");
    check(ReadData(&stream, &restored) && std::string(restored.c_str()) == "Race", "fixed string round trip");

    stream = {};
    RE::BSFixedString empty("");
    check(WriteData(&stream, &empty) && stream.bytes == std::vector<unsigned char>{0, 0}, "empty wire bytes");
    check(ReadData(&stream, &restored) && std::string(restored.c_str()).empty(), "empty clears destination");

    stream = {};
    RE::BSFixedString limit(std::string(256, 'x').c_str());
    check(WriteData(&stream, &limit) && stream.bytes.size() == 258 && stream.bytes[0] == 0 && stream.bytes[1] == 1, "256-byte limit writes");
    check(ReadData(&stream, &restored) && std::string(restored.c_str()).size() == 256, "256-byte limit reads");
    stream = {};
    RE::BSFixedString tooLong(std::string(257, 'x').c_str());
    check(!WriteData(&stream, &tooLong) && stream.bytes.empty(), "oversize write rejected before output");

    restored = RE::BSFixedString("unchanged");
    stream = {};
    stream.bytes = {1};
    check(!ReadData(&stream, &restored), "short length rejected");
    stream = {};
    stream.bytes = {4, 0, 'a', 'b'};
    check(!ReadData(&stream, &restored), "short string rejected");
    stream = {};
    stream.bytes = {1, 1};
    check(!ReadData(&stream, &restored), "oversize encoded length rejected");
    stream = {};
    stream.bytes = {4, 0, 'R', 'a', 'c', 'e'};
    stream.maxRead = 1;
    check(!ReadData(&stream, &restored), "partial positive length read rejected");
    stream = {};
    stream.bytes = {4, 0, 'R', 'a', 'c', 'e'};
    stream.maxRead = 2;
    check(!ReadData(&stream, &restored), "partial positive payload read rejected");
    check(std::string(restored.c_str()) == "unchanged", "failed reads preserve destination");
    std::cout << "legacy bytes, round trips, boundaries and partial reads passed\n";
}
