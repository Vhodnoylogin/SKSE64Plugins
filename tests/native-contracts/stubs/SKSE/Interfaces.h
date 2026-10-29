#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>
namespace SKSE {
namespace stl {}
namespace log {
template<class... Args> void error(const char*, Args&&...) {}
template<class... Args> void info(const char*, Args&&...) {}
}
class SerializationInterface {
public:
    mutable std::vector<unsigned char> bytes;
    mutable std::size_t cursor = 0;
    std::uint32_t maxRead = UINT32_MAX;
    bool WriteRecordData(const void* value, std::uint32_t size) const {
        const auto* p = static_cast<const unsigned char*>(value);
        bytes.insert(bytes.end(), p, p + size);
        return true;
    }
    std::uint32_t ReadRecordData(void* value, std::uint32_t size) const {
        const auto available = static_cast<std::uint32_t>(bytes.size() - cursor);
        const auto count = (std::min)({available, maxRead, size});
        std::memcpy(value, bytes.data() + cursor, count);
        cursor += count;
        return count;
    }
};
}
