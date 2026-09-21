#ifndef REVOLVE_IOS
#define REVOLVE_IOS

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

struct IOSOpenRequest {
    uint32_t pathAddress;
    uint32_t mode;
};

struct IOSReadRequest {
    uint32_t bufferAddress;
    uint32_t size;
};

struct IOSWriteRequest {
    uint32_t bufferAddress;
    uint32_t size;
};

struct IOSSeekRequest {
    uint32_t offset;
    uint32_t whence;
};

struct IOSIoctlRequest {
    uint32_t request;
    uint32_t inPtr;
    uint32_t inSize;
    uint32_t outPtr;
    uint32_t outSize;
};

struct IOSIoctlvRequest {
    uint32_t request;
    uint32_t inCount;
    uint32_t outCount;
    uint32_t vectorsAddress;
};

struct IOSVector {
    uint32_t address;
    uint32_t size;
};

class IOSDevice {
  public:
    virtual ~IOSDevice() = default;

    virtual int32_t open(const std::string &path, uint32_t mode) { return 0; };
    virtual int32_t close(int32_t fd) { return 0; };
    virtual int32_t read(uint32_t buffer, uint32_t size) { return -1; };
    virtual int32_t write(uint32_t buffer, uint32_t size) { return -1; };
    virtual int32_t seek(uint32_t offset, uint32_t whence) { return -1; };
    virtual int32_t ioctl(const IOSIoctlRequest &request) { return -1; };
    virtual int32_t ioctlv(const IOSIoctlvRequest &request) { return -1; };
};

struct IOSFileDescriptor {
    std::string path;
};

class IOS {
  public:
    IOSRequest parseRequest(uint32_t address);

    void init();

    void submitRequest(uint32_t address);

    IOSOpenRequest parseOpenRequest(const IOSRequest &request);
    IOSReadRequest parseReadRequest(const IOSRequest &request);
    IOSWriteRequest parseWriteRequest(const IOSRequest &request);
    IOSSeekRequest parseSeekRequest(const IOSRequest &request);
    IOSIoctlRequest parseIoctlRequest(const IOSRequest &request);
    IOSIoctlvRequest parseIoctlvRequest(const IOSRequest &request);

    std::vector<IOSVector> parseVectors(uint32_t address, uint32_t count);

    std::string readGuestString(uint32_t addr, size_t maxLength = 256);

  private:
    int32_t dispatch(const IOSRequest &request);

    int32_t allocateFileDescriptor(const std::string &path);

    std::unordered_map<uint32_t, IOSFileDescriptor> fileDescriptors;
    std::unordered_map<std::string, std::shared_ptr<IOSDevice>> devices;
    int32_t nextFileDescriptor = 0;
};

#endif