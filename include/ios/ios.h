#ifndef REVOLVE_IOS
#define REVOLVE_IOS

#include "core/memory.h"
#include "disc.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
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
    int32_t offset;
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
    virtual int32_t seek(int32_t offset, uint32_t whence) { return -1; };
    virtual int32_t ioctl(const IOSIoctlRequest &request) { return -1; };
    virtual int32_t ioctlv(const IOSIoctlvRequest &request,
                           const std::vector<IOSVector> &vectors) {
        return -1;
    }
};

struct IOSFileDescriptor {
    std::string path;
    std::shared_ptr<IOSDevice> device;
};

enum class IOSError : int32_t {
    Success = 0,

    AccessDenied = -1,
    Exists = -2,
    Stall = -3,
    Invalid = -4,
    TooManyFDs = -5,
    NotFound = -6,
    QueueFull = -8,
    Unknown = -9,
    IO = -12,
    NoMemory = -22,

    FS_Invalid = -101,
    FS_AccessDenied = -102,
    FS_Corrupt = -103,
    FS_Exists = -105,
    FS_NotFound = -106,
    FS_TooManyFiles = -107,
    FS_FileTooBig = -108,
    FS_FDExhausted = -109,
    FS_NameTooLong = -110,
    FS_FDAlreadyOpen = -111,
    FS_IO = -114,
    FS_NotEmpty = -115,
    FS_DirDepth = -116,
    FS_Busy = -118,

    ES_ShortRead = -1009,
    ES_IO = -1010,
    ES_InvalidSignatureType = -1012,
    ES_FDExhausted = -1016,
    ES_Invalid = -1017,
    ES_DeviceIDMismatch = -1020,
    ES_HashMismatch = -1022,
    ES_NoMemory = -1024,
    ES_AccessDenied = -1026,
    ES_UnknownIssuer = -1027,
    ES_NoTicket = -1028,
    ES_InvalidTicket = -1029,
};

class IOS {
  public:
    IOSRequest parseRequest(uint32_t address);

    void init();
    void prepareDiscBoot(uint64_t partitionOffset);

    void submitRequest(uint32_t address);

    void registerDevice(const std::string &path,
                        std::shared_ptr<IOSDevice> device);

    IOSOpenRequest parseOpenRequest(const IOSRequest &request);
    IOSReadRequest parseReadRequest(const IOSRequest &request);
    IOSWriteRequest parseWriteRequest(const IOSRequest &request);
    IOSSeekRequest parseSeekRequest(const IOSRequest &request);
    IOSIoctlRequest parseIoctlRequest(const IOSRequest &request);
    IOSIoctlvRequest parseIoctlvRequest(const IOSRequest &request);

    std::vector<IOSVector> parseVectors(uint32_t address, uint32_t count);

    std::string readGuestString(uint32_t addr, size_t maxLength = 256);

    static inline int32_t error(IOSError error) {
        return static_cast<int32_t>(error);
    }

  private:
    int32_t dispatch(const IOSRequest &request);

    int32_t allocateFileDescriptor(const std::string &path,
                                   std::shared_ptr<IOSDevice> device);

    std::unordered_map<int32_t, IOSFileDescriptor> fileDescriptors;
    std::unordered_map<std::string, std::shared_ptr<IOSDevice>> devices;
    int32_t nextFileDescriptor = 0;
};

enum class STMIoctl : uint32_t {
    EventHook = 0x1000,

    HotReset = 0x2001,
    HotResetForPD = 0x2002,
    Shutdown = 0x2003,
    Idle = 0x2004,
    Wakeup = 0x2005,

    GetIdleMode = 0x3001,
    ReleaseEH = 0x3002,

    ReadDDRReg = 0x4001,
    ReadDDRReg2 = 0x4002,

    VideoDimming = 0x5001,

    LEDFlash = 0x6001,
    LEDMode = 0x6002,

    ReadVersion = 0x7001,

    WriteDMCU = 0x8001,
};

class STMImmediateDevice : public IOSDevice {
  public:
    int32_t ioctl(const IOSIoctlRequest &request) override;
};

class STMEventHookDevice : public IOSDevice {
  public:
    int32_t ioctl(const IOSIoctlRequest &request) override;

    void triggerReset();
    void triggerPower();

  private:
    std::optional<uint32_t> pendingRequest;
};

enum class FSIOCtl : uint32_t {
    Format = 1,
    GetStats = 2,
    CreateDirectory = 3,
    ReadDirectory = 4, // ioctlv
    SetAttribute = 5,
    GetAttribute = 6,
    Delete = 7,
    Rename = 8,
    CreateFile = 9,
    SetFileVersionCtrl = 10,
    GetFileStats = 11,
    GetUsage = 12, // ioctlv
    Shutdown = 13,
};

class FSDevice : public IOSDevice {
  public:
    int32_t open(const std::string &path, uint32_t mode) override;

    int32_t close(int32_t fd) override;

    int32_t read(uint32_t buffer, uint32_t size) override;

    int32_t write(uint32_t buffer, uint32_t size) override;

    int32_t seek(int32_t offset, uint32_t whence) override;

    int32_t ioctl(const IOSIoctlRequest &request) override;

    int32_t ioctlv(const IOSIoctlvRequest &request,
                   const std::vector<IOSVector> &vectors) override;
};

enum class DIIoctl : uint32_t {
    Inquiry = 0x12,

    ReadDiskID = 0x70,
    Read = 0x71,

    WaitForCoverClose = 0x79,
    GetCoverRegister = 0x7A,

    GetLength = 0x83,
    GetCoverStatus = 0x88,

    Reset = 0x8A,
    OpenPartition = 0x8B, // IOCTLV
    ClosePartition = 0x8C,
    UnencryptedRead = 0x8D,

    RequestError = 0xE0,
};

enum class DIResult : int32_t {
    Success = 0x01,
    DriveError = 0x02,
    CoverClosed = 0x04,
    ReadTimedOut = 0x10,
    SecurityError = 0x20,
    VerifyError = 0x40,
    BadArgument = 0x80,
};

class DIDevice : public IOSDevice {
  public:
    DIDevice(Memory &memory, std::shared_ptr<DiscImage> disc)
        : memory(memory), disc(std::move(disc)) {}
    void prepareBoot(uint64_t partitionOffset) {
        currentPartition = partitionOffset;
        discIDRead = true;
    }
    int32_t ioctl(const IOSIoctlRequest &request) override;

    int32_t ioctlv(const IOSIoctlvRequest &request,
                   const std::vector<IOSVector> &vectors) override;

  private:
    Memory &memory;
    std::shared_ptr<DiscImage> disc;

    std::optional<uint64_t> currentPartition;

    bool discIDRead = false;

    uint32_t lastLength = 0;

    uint32_t lastDriveError = 0;
};

class ESDevice : public IOSDevice {
  public:
    int32_t open(const std::string &path, uint32_t mode) override;

    int32_t close(int32_t fd) override;

    int32_t ioctlv(const IOSIoctlvRequest &request,
                   const std::vector<IOSVector> &vectors) override;
};

#endif