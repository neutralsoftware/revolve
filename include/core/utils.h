#ifndef REVOLVE_UTILS
#define REVOLVE_UTILS

#include <concepts>
#include <cstdarg>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class BigEndianStream {
  public:
    explicit BigEndianStream(const std::string &path);

    void moveTo(uint32_t offset);
    void moveBy(int32_t offset);

    uint8_t readByte();
    uint16_t readShort();
    uint32_t readInt();

    void skipBytes(size_t n);

    std::vector<uint8_t> readNBytes(size_t n);

  private:
    std::ifstream file;
};

enum class LogLevel {
    Info,
    Warning,
    Error,
};

class Loggable {
  public:
    virtual std::string log() const = 0;
    virtual std::string getLogSystem() const = 0;

    virtual ~Loggable() = default;
};

class Logger {
  public:
    static void log(std::string system, LogLevel level,
                    const std::string &message);
    static void logObject(const Loggable &object, LogLevel level);
};

namespace utils {
template <std::integral T> inline std::string toHexString(T value) {
    using U = std::make_unsigned_t<T>;

    std::ostringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setfill('0')
       << std::setw(sizeof(T) * 2) << static_cast<U>(value);

    return ss.str();
}
} // namespace utils

#endif // REVOLVE_UTILS
