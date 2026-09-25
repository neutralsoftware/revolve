
#include "core/utils.h"
#include "ios/ios.h"

BluetoothUSBDevice::BluetoothUSBDevice(IOS &ios, InputManager &inputManager)
    : ios(ios) {
    for (std::size_t i = 0; i < 4; ++i) {
        wiimotes[i] = &inputManager.wiimoteDevice(i);

        auto &connection = connections[i];
        connection.wiimote = wiimotes[i];
        connection.address.bytes = {
            static_cast<uint8_t>(i + 1), 0x00, 0x00, 0x1D, 0x19, 0x00,
        };
        connection.handle = static_cast<uint16_t>(0x000B + i);
    }
}

IOSResult BluetoothUSBDevice::ioctlv(const IOSIoctlvRequest &request,
                                     const std::vector<IOSVector> &vectors) {
    switch (static_cast<USBV0Request>(request.request)) {
    case USBV0Request::Control:
        return handleControl(request, vectors);

    case USBV0Request::Bulk:
        return handleBulk(request, vectors);

    case USBV0Request::Interrupt:
        return handleInterrupt(request, vectors);

    default:
        return IOS::error(IOSError::Invalid);
    }
}

static uint8_t readU8(uint32_t address) { return Bus::readPhysical8(address); }

static uint16_t readBE16(uint32_t address) {
    return (uint16_t(Bus::readPhysical8(address)) << 8) |
           uint16_t(Bus::readPhysical8(address + 1));
}

static uint16_t readLE16(uint32_t address) {
    return uint16_t(Bus::readPhysical8(address)) |
           (uint16_t(Bus::readPhysical8(address + 1)) << 8);
}

static void writeGuest(uint32_t address, std::span<const uint8_t> data) {
    for (std::size_t i = 0; i < data.size(); ++i)
        Bus::writePhysical8(address + i, data[i]);
}

static std::vector<uint8_t> readGuest(uint32_t address, uint32_t size) {
    std::vector<uint8_t> result(size);

    for (uint32_t i = 0; i < size; ++i)
        result[i] = Bus::readPhysical8(address + i);

    return result;
}

static constexpr uint8_t HCI_EVENT_ENDPOINT = 0x81;

IOSResult
BluetoothUSBDevice::handleInterrupt(const IOSIoctlvRequest &request,
                                    const std::vector<IOSVector> &vectors) {
    if (vectors.size() < 3)
        return IOS::error(IOSError::Invalid);

    const uint8_t endpoint = readU8(vectors[0].address);
    const uint16_t length = readBE16(vectors[1].address);

    if (endpoint != HCI_EVENT_ENDPOINT)
        return IOS::error(IOSError::Invalid);

    if (!hciEvents.empty()) {
        auto event = std::move(hciEvents.front());

        hciEvents.pop_front();

        const std::size_t amount = std::min<std::size_t>(
            event.size(), std::min<std::size_t>(length, vectors[2].size));

        writeGuest(vectors[2].address, std::span(event).first(amount));

        return static_cast<int32_t>(amount);
    }

    pendingHCIRead = PendingRead{
        .ipcAddress = request.ipcAddress,
        .bufferAddress = vectors[2].address,
        .capacity = std::min<uint32_t>(length, vectors[2].size),
    };

    return std::nullopt;
}

void BluetoothUSBDevice::queueHCIEvent(std::vector<uint8_t> event) {
    if (pendingHCIRead) {
        const std::size_t amount =
            std::min<std::size_t>(event.size(), pendingHCIRead->capacity);

        writeGuest(pendingHCIRead->bufferAddress,
                   std::span(event).first(amount));

        const uint32_t request = pendingHCIRead->ipcAddress;

        pendingHCIRead.reset();

        ios.completeRequest(request, static_cast<int32_t>(amount));

        return;
    }

    hciEvents.push_back(std::move(event));
}

static constexpr uint8_t ACL_OUT_ENDPOINT = 0x02;
static constexpr uint8_t ACL_IN_ENDPOINT = 0x82;

IOSResult
BluetoothUSBDevice::handleBulk(const IOSIoctlvRequest &request,
                               const std::vector<IOSVector> &vectors) {
    if (vectors.size() < 3)
        return IOS::error(IOSError::Invalid);

    const uint8_t endpoint = readU8(vectors[0].address);

    const uint16_t length = readBE16(vectors[1].address);

    const uint32_t amount = std::min<uint32_t>(length, vectors[2].size);

    if (endpoint == ACL_OUT_ENDPOINT) {
        auto packet = readGuest(vectors[2].address, amount);

        handleACLFromWii(packet);

        return static_cast<int32_t>(amount);
    }

    if (endpoint == ACL_IN_ENDPOINT) {
        if (!aclPackets.empty()) {
            auto packet = std::move(aclPackets.front());

            aclPackets.pop_front();

            const std::size_t writeSize =
                std::min<std::size_t>(packet.size(), amount);

            writeGuest(vectors[2].address, std::span(packet).first(writeSize));

            return static_cast<int32_t>(writeSize);
        }

        pendingACLRead = PendingRead{
            .ipcAddress = request.ipcAddress,
            .bufferAddress = vectors[2].address,
            .capacity = amount,
        };

        return std::nullopt;
    }

    return IOS::error(IOSError::Invalid);
}

void BluetoothUSBDevice::queueACL(std::vector<uint8_t> packet) {
    if (pendingACLRead) {
        const std::size_t amount =
            std::min<std::size_t>(packet.size(), pendingACLRead->capacity);

        writeGuest(pendingACLRead->bufferAddress,
                   std::span(packet).first(amount));

        const uint32_t request = pendingACLRead->ipcAddress;

        pendingACLRead.reset();

        ios.completeRequest(request, static_cast<int32_t>(amount));

        return;
    }

    aclPackets.push_back(std::move(packet));
}

IOSResult
BluetoothUSBDevice::handleControl(const IOSIoctlvRequest &request,
                                  const std::vector<IOSVector> &vectors) {
    if (vectors.size() < 7)
        return IOS::error(IOSError::Invalid);

    const uint16_t length = readLE16(vectors[4].address);
    const uint32_t amount = std::min<uint32_t>(length, vectors[6].size);

    auto command = readGuest(vectors[6].address, amount);

    handleHCICommand(command);

    return static_cast<int32_t>(amount);
}

static void appendLE16(std::vector<uint8_t> &out, uint16_t value) {
    out.push_back(value & 0xFF);
    out.push_back(value >> 8);
}

static void appendBE16(std::vector<uint8_t> &out, uint16_t value) {
    out.push_back(value >> 8);
    out.push_back(value & 0xFF);
}

static void appendBE32(std::vector<uint8_t> &out, uint32_t value) {
    out.push_back(value >> 24);
    out.push_back(value >> 16);
    out.push_back(value >> 8);
    out.push_back(value);
}

void BluetoothUSBDevice::sendCommandComplete(
    uint16_t opcode, std::span<const uint8_t> parameters) {
    std::vector<uint8_t> event;

    event.push_back(0x0E);

    event.push_back(static_cast<uint8_t>(3 + parameters.size()));

    event.push_back(1);

    appendLE16(event, opcode);

    event.insert(event.end(), parameters.begin(), parameters.end());

    queueHCIEvent(std::move(event));
}

void BluetoothUSBDevice::sendCommandStatus(uint16_t opcode, uint8_t status) {
    std::vector<uint8_t> event{0x0F, 0x04, status, 0x01};
    appendLE16(event, opcode);
    queueHCIEvent(std::move(event));
}

void BluetoothUSBDevice::sendInquiryResults() {
    for (const auto &connection : connections) {
        if (!connection.wiimote->isAvailable())
            continue;

        std::vector<uint8_t> event;

        event.push_back(0x02); // Inquiry Result

        // Parameter length:
        // count + one inquiry result
        event.push_back(14);

        event.push_back(1);

        for (uint8_t b : connection.address.bytes)
            event.push_back(b);

        // page scan repetition mode
        event.push_back(0x01);

        // reserved
        event.push_back(0x00);

        // class of device: 00:04:48
        event.push_back(0x04);
        event.push_back(0x25);
        event.push_back(0x00);

        // clock offset
        appendLE16(event, 0);

        queueHCIEvent(std::move(event));
    }

    queueHCIEvent({
        0x01, // Inquiry Complete
        0x01, // parameter length
        0x00  // success
    });
}

void BluetoothUSBDevice::sendRemoteName(const BluetoothAddress &address) {
    std::vector<uint8_t> event;

    event.push_back(0x07); // Remote Name Request Complete
    event.push_back(255);

    event.push_back(0x00);

    event.insert(event.end(), address.bytes.begin(), address.bytes.end());

    constexpr char name[] = "Nintendo RVL-CNT-01";

    event.insert(event.end(), name, name + sizeof(name));

    while (event.size() < 257)
        event.push_back(0);

    queueHCIEvent(std::move(event));
}

BluetoothConnection *
BluetoothUSBDevice::findConnectionByAddress(const BluetoothAddress &address) {
    for (auto &connection : connections) {
        if (connection.address.bytes == address.bytes)
            return &connection;
    }

    return nullptr;
}

void BluetoothUSBDevice::sendConnectionComplete(
    BluetoothConnection &connection) {
    std::vector<uint8_t> event;

    event.push_back(0x03);
    event.push_back(11);

    event.push_back(0x00);

    appendLE16(event, connection.handle);

    event.insert(event.end(), connection.address.bytes.begin(),
                 connection.address.bytes.end());

    // ACL link
    event.push_back(0x01);

    // encryption disabled
    event.push_back(0x00);

    queueHCIEvent(std::move(event));
}

void BluetoothUSBDevice::handleHCICommand(std::span<const uint8_t> command) {
    if (command.size() < 3)
        return;

    const uint16_t opcode = uint16_t(command[0]) | (uint16_t(command[1]) << 8);

    const uint8_t size = command[2];

    if (command.size() < 3 + size)
        return;

    auto payload = command.subspan(3, size);

    switch (opcode) {
    case HCI::Reset: {
        aclPackets.clear();
        scanEnable = 0;
        for (auto &connection : connections) {
            connection.basebandConnected = false;
            connection.incomingRequested = false;
            connection.hidControlLocalCID = 0;
            connection.hidControlRemoteCID = 0;
            connection.hidInterruptLocalCID = 0;
            connection.hidInterruptRemoteCID = 0;
            connection.sdpLocalCID = 0;
            connection.sdpRemoteCID = 0;
            connection.hidControlConfigured = false;
            connection.hidInterruptConfigured = false;
            connection.sdpConfigured = false;
        }
        nextCID = 0x0040;
        nextSignalIdentifier = 1;
        const std::array<uint8_t, 1> response{0x00};

        sendCommandComplete(opcode, response);

        break;
    }
    case HCI::ReadBDADDR: {
        const std::array<uint8_t, 7> response{
            0x00,

            // HCI BD_ADDR is LSB first
            0x01,
            0x02,
            0x03,
            0x04,
            0x05,
            0x06,
        };

        sendCommandComplete(opcode, response);

        break;
    }
    case HCI::ReadBufferSize: {
        std::vector<uint8_t> response;

        response.push_back(0x00);

        // ACL packet size
        appendLE16(response, 339);

        // SCO packet size
        response.push_back(0);

        // ACL packet count
        appendLE16(response, 10);

        // SCO packet count
        appendLE16(response, 0);

        sendCommandComplete(opcode, response);

        break;
    }
    case HCI::ReadLocalVersion: {
        const std::array<uint8_t, 9> response{0x00, 0x03, 0xA7, 0x40, 0x03,
                                              0x0F, 0x00, 0x0E, 0x43};

        sendCommandComplete(opcode, response);

        break;
    }
    case HCI::ReadLocalSupportedFeatures: {
        const std::array<uint8_t, 9> response{0x00, 0xFF, 0xFF, 0x8D, 0xFE,
                                              0x9B, 0xF9, 0x00, 0x80};

        sendCommandComplete(opcode, response);

        break;
    }
    case 0x1004: {
        const std::array<uint8_t, 11> response{
            0x00, 0x00, 0x00, 0xFF, 0xFF, 0x8F, 0xFE, 0x9B, 0xF9, 0x00, 0x80};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x1007: {
        const std::array<uint8_t, 2> response{0x00, 0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C14: {
        std::vector<uint8_t> response{0x00};
        constexpr char localName[] = "Nintendo RVL-CNT";
        response.insert(response.end(), localName,
                        localName + sizeof(localName));
        response.resize(249, 0);
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C19: {
        const std::array<uint8_t, 2> response{0x00, scanEnable};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C23: {
        const std::array<uint8_t, 4> response{0x00, 0x04, 0x25, 0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C25: {
        const std::array<uint8_t, 3> response{0x00, 0x60, 0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C01:
    case 0x0C05:
    case 0x0C0A:
    case 0x0C13:
    case 0x0C18:
    case 0x0C24:
    case 0x0C26:
    case 0x0C33:
    case 0x0C37: {
        const std::array<uint8_t, 1> response{0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    case HCI::Inquiry:
        sendCommandStatus(opcode);
        sendInquiryResults();
        break;
    case HCI::RemoteNameRequest: {
        if (payload.size() < 6)
            break;

        BluetoothAddress address{};

        std::copy_n(payload.begin(), 6, address.bytes.begin());

        auto *connection = findConnectionByAddress(address);

        if (!connection) {
            sendCommandStatus(opcode, 0x02);
            break;
        }

        sendCommandStatus(opcode);
        sendRemoteName(address);
        break;
    }
    case 0x0409:
    case HCI::CreateConnection: {
        if (payload.size() < 6)
            break;

        BluetoothAddress address{};

        std::copy_n(payload.begin(), 6, address.bytes.begin());

        auto *connection = findConnectionByAddress(address);

        if (!connection || !connection->wiimote->isAvailable()) {
            sendCommandStatus(opcode, 0x02);
            break;
        }

        connection->basebandConnected = true;
        connection->incomingRequested = false;

        sendCommandStatus(opcode);
        sendConnectionComplete(*connection);

        break;
    }
    case HCI::Disconnect: {
        if (payload.size() < 3)
            break;
        const uint16_t handle =
            uint16_t(payload[0]) | (uint16_t(payload[1]) << 8);
        auto *connection = findConnectionByHandle(handle & 0x0FFF);
        sendCommandStatus(opcode, connection ? 0x00 : 0x02);
        if (!connection)
            break;
        connection->basebandConnected = false;
        connection->hidControlConfigured = false;
        connection->hidInterruptConfigured = false;
        connection->sdpConfigured = false;
        std::vector<uint8_t> event{0x05, 0x04, 0x00};
        appendLE16(event, handle & 0x0FFF);
        event.push_back(payload[2]);
        queueHCIEvent(std::move(event));
        break;
    }
    case HCI::WriteScanEnable: {
        if (payload.empty() || payload[0] > 3) {
            const std::array<uint8_t, 1> response{0x12};
            sendCommandComplete(opcode, response);
            break;
        }
        scanEnable = payload[0];
        const std::array<uint8_t, 1> response{0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C0D: {
        queueHCIEvent({0x15, 0x01, 0x00});

        std::vector<uint8_t> response;

        response.push_back(0x00);
        appendLE16(response, 255);
        appendLE16(response, 0);

        sendCommandComplete(opcode, response);
        break;
    }
    case 0x0C43:
    case 0x0C45:
    case 0x0C47:
    case 0xFC4C:
    case 0xFC4F: {
        const std::array<uint8_t, 1> response{0x00};
        sendCommandComplete(opcode, response);
        break;
    }
    default:
        Logger::log("HCI", LogLevel::Error,
                    "UNIMPLEMENTED HCI COMMAND 0x " +
                        utils::toHexString(opcode));

        sendCommandStatus(opcode, 0x01);
        break;
    }
}

void BluetoothUSBDevice::handleACLFromWii(std::span<const uint8_t> packet) {
    if (packet.size() < 4)
        return;

    const uint16_t handleFlags =
        uint16_t(packet[0]) | (uint16_t(packet[1]) << 8);

    const uint16_t handle = handleFlags & 0x0FFF;

    const uint16_t length = uint16_t(packet[2]) | (uint16_t(packet[3]) << 8);

    if (packet.size() < 4 + length)
        return;

    auto *connection = findConnectionByHandle(handle);

    if (!connection)
        return;

    handleL2CAP(*connection, packet.subspan(4, length));
}

BluetoothConnection *
BluetoothUSBDevice::findConnectionByHandle(uint16_t handle) {
    for (auto &connection : connections) {
        if (connection.basebandConnected && connection.handle == handle)
            return &connection;
    }

    return nullptr;
}

void BluetoothUSBDevice::handleL2CAP(BluetoothConnection &connection,
                                     std::span<const uint8_t> packet) {
    if (packet.size() < 4)
        return;

    const uint16_t length = uint16_t(packet[0]) | (uint16_t(packet[1]) << 8);

    const uint16_t cid = uint16_t(packet[2]) | (uint16_t(packet[3]) << 8);

    if (packet.size() < 4 + length)
        return;

    auto data = packet.subspan(4, length);

    if (cid == 0x0001) {
        handleL2CAPSignaling(connection, data);

        return;
    }

    handleHID(connection, cid, data);
}

constexpr uint16_t PSM_SDP = 0x0001;
constexpr uint16_t PSM_HID_CONTROL = 0x0011;
constexpr uint16_t PSM_HID_INTERRUPT = 0x0013;

void BluetoothUSBDevice::sendL2CAPSignal(BluetoothConnection &connection,
                                         uint8_t code, uint8_t identifier,
                                         std::span<const uint8_t> data) {
    std::vector<uint8_t> packet{code, identifier};
    appendLE16(packet, static_cast<uint16_t>(data.size()));
    packet.insert(packet.end(), data.begin(), data.end());
    sendL2CAP(connection, 0x0001, packet);
}

void BluetoothUSBDevice::handleL2CAPSignaling(BluetoothConnection &connection,
                                              std::span<const uint8_t> data) {
    while (data.size() >= 4) {
        const uint8_t code = data[0];
        const uint8_t identifier = data[1];
        const uint16_t length = uint16_t(data[2]) | (uint16_t(data[3]) << 8);
        if (data.size() < 4 + length)
            return;
        auto payload = data.subspan(4, length);

        if (code == 0x02 && payload.size() >= 4) {
            const uint16_t psm =
                uint16_t(payload[0]) | (uint16_t(payload[1]) << 8);
            const uint16_t remoteCID =
                uint16_t(payload[2]) | (uint16_t(payload[3]) << 8);
            uint16_t localCID = nextCID++;
            bool supported = true;
            if (psm == PSM_SDP) {
                connection.sdpLocalCID = localCID;
                connection.sdpRemoteCID = remoteCID;
            } else if (psm == PSM_HID_CONTROL) {
                connection.hidControlLocalCID = localCID;
                connection.hidControlRemoteCID = remoteCID;
            } else if (psm == PSM_HID_INTERRUPT) {
                connection.hidInterruptLocalCID = localCID;
                connection.hidInterruptRemoteCID = remoteCID;
            } else {
                supported = false;
                localCID = 0;
            }
            std::vector<uint8_t> response;
            appendLE16(response, localCID);
            appendLE16(response, remoteCID);
            appendLE16(response, supported ? 0x0000 : 0x0002);
            appendLE16(response, 0x0000);
            sendL2CAPSignal(connection, 0x03, identifier, response);
            if (supported) {
                std::vector<uint8_t> configure;
                appendLE16(configure, remoteCID);
                appendLE16(configure, 0x0000);
                sendL2CAPSignal(connection, 0x04, nextSignalIdentifier++,
                                configure);
            }
        } else if (code == 0x04 && payload.size() >= 4) {
            const uint16_t localCID =
                uint16_t(payload[0]) | (uint16_t(payload[1]) << 8);
            std::vector<uint8_t> response;
            appendLE16(response, localCID);
            appendLE16(response, 0x0000);
            appendLE16(response, 0x0000);
            sendL2CAPSignal(connection, 0x05, identifier, response);
            if (localCID == connection.sdpLocalCID)
                connection.sdpConfigured = true;
            if (localCID == connection.hidControlLocalCID)
                connection.hidControlConfigured = true;
            if (localCID == connection.hidInterruptLocalCID)
                connection.hidInterruptConfigured = true;
        } else if (code == 0x05 && payload.size() >= 6) {
            const uint16_t remoteCID =
                uint16_t(payload[0]) | (uint16_t(payload[1]) << 8);
            const uint16_t result =
                uint16_t(payload[4]) | (uint16_t(payload[5]) << 8);
            if (result == 0) {
                if (remoteCID == connection.sdpRemoteCID)
                    connection.sdpConfigured = true;
                if (remoteCID == connection.hidControlRemoteCID)
                    connection.hidControlConfigured = true;
                if (remoteCID == connection.hidInterruptRemoteCID)
                    connection.hidInterruptConfigured = true;
            }
        } else if (code == 0x06 && payload.size() >= 4) {
            sendL2CAPSignal(connection, 0x07, identifier, payload.first(4));
        } else if (code == 0x08) {
            sendL2CAPSignal(connection, 0x09, identifier, payload);
        } else if (code == 0x0A && payload.size() >= 2) {
            const uint16_t type =
                uint16_t(payload[0]) | (uint16_t(payload[1]) << 8);
            std::vector<uint8_t> response;
            appendLE16(response, type);
            appendLE16(response, type == 0x0002 ? 0x0000 : 0x0001);
            if (type == 0x0002) {
                response.push_back(0x00);
                response.push_back(0x00);
                response.push_back(0x00);
                response.push_back(0x00);
            }
            sendL2CAPSignal(connection, 0x0B, identifier, response);
        } else {
            std::vector<uint8_t> response;
            appendLE16(response, 0x0000);
            sendL2CAPSignal(connection, 0x01, identifier, response);
        }
        data = data.subspan(4 + length);
    }
}

void BluetoothUSBDevice::handleSDP(BluetoothConnection &connection,
                                   std::span<const uint8_t> data) {
    if (data.size() < 5)
        return;
    const uint8_t pdu = data[0];
    const uint16_t transaction = (uint16_t(data[1]) << 8) | uint16_t(data[2]);
    const uint16_t parameterLength =
        (uint16_t(data[3]) << 8) | uint16_t(data[4]);
    if (data.size() < 5 + parameterLength)
        return;

    std::vector<uint8_t> parameters;
    uint8_t responsePDU = 0x01;

    if (pdu == 0x02) {
        responsePDU = 0x03;
        appendBE16(parameters, 1);
        appendBE16(parameters, 1);
        appendBE32(parameters, 0x00010000);
        parameters.push_back(0);
    } else if (pdu == 0x04 || pdu == 0x06) {
        std::vector<uint8_t> attributes;
        const auto attribute = [&](uint16_t id,
                                   std::span<const uint8_t> value) {
            attributes.push_back(0x09);
            appendBE16(attributes, id);
            attributes.insert(attributes.end(), value.begin(), value.end());
        };
        const std::array<uint8_t, 5> handle{0x0A, 0x00, 0x01, 0x00, 0x00};
        attribute(0x0000, handle);
        const std::array<uint8_t, 5> serviceClass{0x35, 0x03, 0x19, 0x11, 0x24};
        attribute(0x0001, serviceClass);
        const std::array<uint8_t, 15> protocols{0x35, 0x0D, 0x35, 0x06, 0x19,
                                                0x01, 0x00, 0x09, 0x00, 0x11,
                                                0x35, 0x03, 0x19, 0x00, 0x11};
        attribute(0x0004, protocols);
        const std::array<uint8_t, 17> additionalProtocols{
            0x35, 0x0F, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00,
            0x09, 0x00, 0x13, 0x35, 0x03, 0x19, 0x00, 0x11};
        attribute(0x000D, additionalProtocols);
        const std::array<uint8_t, 10> profile{0x35, 0x08, 0x35, 0x06, 0x19,
                                              0x11, 0x24, 0x09, 0x01, 0x00};
        attribute(0x0009, profile);
        constexpr char name[] = "Nintendo RVL-CNT-01";
        std::vector<uint8_t> serviceName{
            0x25, static_cast<uint8_t>(sizeof(name) - 1)};
        serviceName.insert(serviceName.end(), name, name + sizeof(name) - 1);
        attribute(0x0100, serviceName);
        const std::array<uint8_t, 3> releaseNumber{0x09, 0x01, 0x00};
        attribute(0x0200, releaseNumber);
        const std::array<uint8_t, 3> parserVersion{0x09, 0x01, 0x11};
        attribute(0x0201, parserVersion);
        const std::array<uint8_t, 2> subclass{0x08, 0x04};
        attribute(0x0202, subclass);
        const std::array<uint8_t, 2> country{0x08, 0x00};
        attribute(0x0203, country);
        const std::array<uint8_t, 2> enabled{0x28, 0x01};
        attribute(0x0204, enabled);
        attribute(0x0205, enabled);
        const std::array<uint8_t, 21> reportDescriptor{
            0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01, 0x15, 0x00, 0x26, 0xFF,
            0x00, 0x75, 0x08, 0x95, 0x16, 0x09, 0x01, 0x81, 0x02, 0xC0};
        std::vector<uint8_t> descriptorString{
            0x25, static_cast<uint8_t>(reportDescriptor.size())};
        descriptorString.insert(descriptorString.end(),
                                reportDescriptor.begin(),
                                reportDescriptor.end());
        std::vector<uint8_t> descriptorEntry{0x35};
        descriptorEntry.push_back(
            static_cast<uint8_t>(descriptorString.size() + 2));
        descriptorEntry.push_back(0x08);
        descriptorEntry.push_back(0x22);
        descriptorEntry.insert(descriptorEntry.end(), descriptorString.begin(),
                               descriptorString.end());
        std::vector<uint8_t> descriptors{0x35};
        descriptors.push_back(static_cast<uint8_t>(descriptorEntry.size()));
        descriptors.insert(descriptors.end(), descriptorEntry.begin(),
                           descriptorEntry.end());
        attribute(0x0206, descriptors);
        const std::array<uint8_t, 10> language{0x35, 0x08, 0x35, 0x06, 0x09,
                                               0x04, 0x09, 0x09, 0x01, 0x00};
        attribute(0x0207, language);
        const std::array<uint8_t, 2> disabled{0x28, 0x00};
        attribute(0x0208, disabled);
        attribute(0x0209, enabled);
        attribute(0x020D, enabled);

        std::vector<uint8_t> sequence{0x36};
        appendBE16(sequence, static_cast<uint16_t>(attributes.size()));
        sequence.insert(sequence.end(), attributes.begin(), attributes.end());
        if (pdu == 0x06) {
            std::vector<uint8_t> records{0x36};
            appendBE16(records, static_cast<uint16_t>(sequence.size()));
            records.insert(records.end(), sequence.begin(), sequence.end());
            sequence = std::move(records);
            responsePDU = 0x07;
        } else {
            responsePDU = 0x05;
        }
        appendBE16(parameters, static_cast<uint16_t>(sequence.size()));
        parameters.insert(parameters.end(), sequence.begin(), sequence.end());
        parameters.push_back(0);
    } else {
        appendBE16(parameters, 0x0003);
    }

    std::vector<uint8_t> response{responsePDU};
    appendBE16(response, transaction);
    appendBE16(response, static_cast<uint16_t>(parameters.size()));
    response.insert(response.end(), parameters.begin(), parameters.end());
    sendL2CAP(connection, connection.sdpRemoteCID, response);
}

void BluetoothUSBDevice::handleHID(BluetoothConnection &connection,
                                   uint16_t cid,
                                   std::span<const uint8_t> data) {
    if (data.empty())
        return;

    if (cid != connection.hidInterruptLocalCID &&
        cid != connection.hidControlLocalCID && cid != connection.sdpLocalCID)
        return;

    if (cid == connection.sdpLocalCID) {
        handleSDP(connection, data);
        return;
    }

    const uint8_t hidType = data[0];

    if (hidType == 0xA2 || hidType == 0x52) {
        if (data.size() < 2)
            return;

        const uint8_t reportId = data[1];

        connection.wiimote->handleOutputReport(reportId, data.subspan(2));

        if (hidType == 0x52) {
            const std::array<uint8_t, 1> handshake{0x00};
            sendL2CAP(connection, connection.hidControlRemoteCID, handshake);
        }

        return;
    }
}

void BluetoothUSBDevice::update() {
    for (auto &connection : connections) {
        if (!connection.wiimote->isAvailable()) {
            connection.incomingRequested = false;
            if (connection.basebandConnected) {
                connection.basebandConnected = false;
                connection.hidControlConfigured = false;
                connection.hidInterruptConfigured = false;
                connection.sdpConfigured = false;
                std::vector<uint8_t> event{0x05, 0x04, 0x00};
                appendLE16(event, connection.handle);
                event.push_back(0x08);
                queueHCIEvent(std::move(event));
            }
            continue;
        }
        if (!connection.basebandConnected && !connection.incomingRequested &&
            (scanEnable & 2)) {
            std::vector<uint8_t> event{0x04, 0x0A};
            event.insert(event.end(), connection.address.bytes.begin(),
                         connection.address.bytes.end());
            event.insert(event.end(), {0x04, 0x25, 0x00, 0x01});
            queueHCIEvent(std::move(event));
            connection.incomingRequested = true;
        }
        if (!connection.basebandConnected)
            continue;

        if (!connection.hidInterruptConfigured)
            continue;

        connection.wiimote->update();

        while (connection.wiimote->hasInputReport()) {
            auto report = connection.wiimote->popInputReport();

            if (report.empty())
                continue;

            sendHIDInput(connection, report);
        }
    }
}

void BluetoothUSBDevice::sendHIDInput(BluetoothConnection &connection,
                                      std::span<const uint8_t> report) {
    std::vector<uint8_t> hid;

    // DATA | INPUT
    hid.push_back(0xA1);

    hid.insert(hid.end(), report.begin(), report.end());

    sendL2CAP(connection, connection.hidInterruptRemoteCID, hid);
}

void BluetoothUSBDevice::sendL2CAP(BluetoothConnection &connection,
                                   uint16_t remoteCID,
                                   std::span<const uint8_t> data) {
    std::vector<uint8_t> l2cap;

    appendLE16(l2cap, static_cast<uint16_t>(data.size()));

    appendLE16(l2cap, remoteCID);

    l2cap.insert(l2cap.end(), data.begin(), data.end());

    std::vector<uint8_t> acl;

    const uint16_t handleFlags = connection.handle | (0x2 << 12);

    appendLE16(acl, handleFlags);

    appendLE16(acl, static_cast<uint16_t>(l2cap.size()));

    acl.insert(acl.end(), l2cap.begin(), l2cap.end());

    queueACL(std::move(acl));
}
