#ifndef REVOLVE_UTILS
#define REVOLVE_UTILS

#include <cstdint>
#include <fstream>
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

#endif // REVOLVE_UTILS
