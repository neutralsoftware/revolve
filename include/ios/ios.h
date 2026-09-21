#ifndef REVOLVE_IOS
#define REVOLVE_IOS

#include <cstdint>

enum class IOSCommand : uint32_t {
    Open = 1,
    Close = 2,
    Read = 3,
    Write = 4,
    Seek = 5,
    Ioctl = 6,
    Ioctlv = 7,
    Reply = 8
};

struct IOSRequest {
    uint32_t address;

    IOSCommand command;
    int32_t result;
    int32_t fd;

    uint32_t args[5];
};

class IOS {
  public:
    IOSRequest parseRequest(uint32_t address);

    void submitRequest(uint32_t address);

  private:
    int32_t dispatch(const IOSRequest &request);
};

#endif