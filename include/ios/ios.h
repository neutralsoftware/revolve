#ifndef REVOLVE_IOS
#define REVOLVE_IOS

#include "core/memory.h"
#include "disc.h"
#include "input/manager.h"
#include "input/wiimote.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using IOSResult = std::optional<int32_t>;

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
    uint32_t ipcAddress;

    uint32_t request;
    uint32_t inPtr;
    uint32_t inSize;
    uint32_t outPtr;
    uint32_t outSize;
};

struct IOSIoctlvRequest {
    uint32_t ipcAddress;

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
    virtual IOSResult ioctl(const IOSIoctlRequest &request) { return -1; };
    virtual IOSResult ioctlv(const IOSIoctlvRequest &request,
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

class BluetoothUSBDevice;
class STMEventHookDevice;

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

    void completeRequest(uint32_t address, int32_t result);
    void update();
    void releaseSTMEventHook();

  private:
    IOSResult dispatch(const IOSRequest &request);

    int32_t allocateFileDescriptor(const std::string &path,
                                   std::shared_ptr<IOSDevice> device);

    std::unordered_map<int32_t, IOSFileDescriptor> fileDescriptors;
    std::unordered_map<std::string, std::shared_ptr<IOSDevice>> devices;
    int32_t nextFileDescriptor = 0;

    std::shared_ptr<BluetoothUSBDevice> bluetoothDevice;
    std::shared_ptr<STMEventHookDevice> stmEventHook;
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
    IOSResult ioctl(const IOSIoctlRequest &request) override;
};

class STMEventHookDevice : public IOSDevice {
  public:
    IOSResult ioctl(const IOSIoctlRequest &request) override;

    void triggerReset();
    void triggerPower();
    void release();
    int32_t close(int32_t fd) override;

  private:
    void complete(uint32_t event);
    std::optional<uint32_t> pendingRequest;
    uint32_t eventOutput = 0;
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
    static std::filesystem::path rootPath();
    static void initializeNAND();
    int32_t open(const std::string &path, uint32_t mode) override;

    int32_t close(int32_t fd) override;

    int32_t read(uint32_t buffer, uint32_t size) override;

    int32_t write(uint32_t buffer, uint32_t size) override;

    int32_t seek(int32_t offset, uint32_t whence) override;

    IOSResult ioctl(const IOSIoctlRequest &request) override;

    IOSResult ioctlv(const IOSIoctlvRequest &request,
                     const std::vector<IOSVector> &vectors) override;

  private:
    std::fstream file;
    std::filesystem::path filePath;
    uint32_t fileMode = 0;
    uint32_t position = 0;
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
    IOSResult ioctl(const IOSIoctlRequest &request) override;

    IOSResult ioctlv(const IOSIoctlvRequest &request,
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
    void prepareDiscBoot(uint64_t partitionOffset);
    int32_t open(const std::string &path, uint32_t mode) override;

    int32_t close(int32_t fd) override;

    IOSResult ioctlv(const IOSIoctlvRequest &request,
                     const std::vector<IOSVector> &vectors) override;

  private:
    uint64_t currentTitle = 0x0000000100000002ULL;
};

struct BluetoothAddress {
    std::array<uint8_t, 6> bytes{};
};

struct BluetoothConnection {
    WiiRemoteDevice *wiimote = nullptr;

    BluetoothAddress address{};

    bool basebandConnected = false;
    bool incomingRequested = false;

    uint16_t handle = 0;

    uint16_t hidControlLocalCID = 0;
    uint16_t hidControlRemoteCID = 0;

    uint16_t hidInterruptLocalCID = 0;
    uint16_t hidInterruptRemoteCID = 0;

    uint16_t sdpLocalCID = 0;
    uint16_t sdpRemoteCID = 0;

    bool hidControlConfigured = false;
    bool hidInterruptConfigured = false;
    bool sdpConfigured = false;
};

enum class USBV0Request : uint32_t {
    Control = 0,
    Bulk = 1,
    Interrupt = 2,
};

namespace HCI {
constexpr uint16_t Inquiry = 0x0401;
constexpr uint16_t CreateConnection = 0x0405;
constexpr uint16_t Disconnect = 0x0406;
constexpr uint16_t RemoteNameRequest = 0x0419;

constexpr uint16_t Reset = 0x0C03;
constexpr uint16_t WriteScanEnable = 0x0C1A;

constexpr uint16_t ReadLocalVersion = 0x1001;
constexpr uint16_t ReadLocalSupportedFeatures = 0x1003;
constexpr uint16_t ReadBufferSize = 0x1005;
constexpr uint16_t ReadBDADDR = 0x1009;
} // namespace HCI

class BluetoothUSBDevice final : public IOSDevice {
  public:
    BluetoothUSBDevice(IOS &ios, InputManager &inputManager);

    IOSResult ioctlv(const IOSIoctlvRequest &request,
                     const std::vector<IOSVector> &vectors) override;

    void update();

  private:
    struct PendingRead {
        uint32_t ipcAddress = 0;
        uint32_t bufferAddress = 0;
        uint32_t capacity = 0;
    };

    IOS &ios;

    std::array<WiiRemoteDevice *, 4> wiimotes{};
    std::array<BluetoothConnection, 4> connections{};

    std::optional<PendingRead> pendingHCIRead;
    std::optional<PendingRead> pendingACLRead;

    std::deque<std::vector<uint8_t>> hciEvents;
    std::deque<std::vector<uint8_t>> aclPackets;

    IOSResult handleControl(const IOSIoctlvRequest &request,
                            const std::vector<IOSVector> &vectors);

    IOSResult handleBulk(const IOSIoctlvRequest &request,
                         const std::vector<IOSVector> &vectors);

    IOSResult handleInterrupt(const IOSIoctlvRequest &request,
                              const std::vector<IOSVector> &vectors);

    void handleHCICommand(std::span<const uint8_t> command);

    void handleACLFromWii(std::span<const uint8_t> packet);

    void handleL2CAP(BluetoothConnection &connection,
                     std::span<const uint8_t> packet);

    void handleL2CAPSignaling(BluetoothConnection &connection,
                              std::span<const uint8_t> data);

    void handleHID(BluetoothConnection &connection, uint16_t destinationCID,
                   std::span<const uint8_t> data);

    void handleSDP(BluetoothConnection &connection,
                   std::span<const uint8_t> data);

    void queueHCIEvent(std::vector<uint8_t> event);

    void queueACL(std::vector<uint8_t> packet);

    void sendHIDInput(BluetoothConnection &connection,
                      std::span<const uint8_t> report);

    BluetoothConnection *findConnectionByHandle(uint16_t handle);

    BluetoothConnection *
    findConnectionByAddress(const BluetoothAddress &address);

    void sendCommandComplete(uint16_t opcode,
                             std::span<const uint8_t> parameters);
    void sendCommandStatus(uint16_t opcode, uint8_t status = 0);
    void sendInquiryResults();

    void sendRemoteName(const BluetoothAddress &address);
    void sendConnectionComplete(BluetoothConnection &connection);

    void sendL2CAP(BluetoothConnection &connection, uint16_t remoteCID,
                   std::span<const uint8_t> data);

    void sendL2CAPSignal(BluetoothConnection &connection, uint8_t code,
                         uint8_t identifier, std::span<const uint8_t> data);

    uint8_t scanEnable = 0;
    uint16_t nextCID = 0x0040;
    uint8_t nextSignalIdentifier = 1;
};

#endif
