#include "input/native_bluetooth.h"
#import <Foundation/Foundation.h>
#import <IOBluetooth/IOBluetooth.h>
#include <array>
#include <deque>

@interface RevolveRemote : NSObject <IOBluetoothL2CAPChannelDelegate> {
@public
    IOBluetoothDevice *device;
    IOBluetoothL2CAPChannel *control;
    IOBluetoothL2CAPChannel *interrupt;
    bool ready;
    std::deque<std::vector<uint8_t>> reports;
}
- (void)connect:(IOBluetoothDevice *)target;
- (void)disconnect;
@end

@implementation RevolveRemote
- (void)connect:(IOBluetoothDevice *)target {
    device = target;
    IOBluetoothL2CAPChannel *channel = nil;
    if ([device openL2CAPChannelAsync:&channel withPSM:0x11 delegate:self] != kIOReturnSuccess) {
        [self disconnect];
        return;
    }
    control = channel;
}
- (void)disconnect {
    ready = false;
    [control setDelegate:nil];
    [interrupt setDelegate:nil];
    [control closeChannel];
    [interrupt closeChannel];
    control = nil;
    interrupt = nil;
    device = nil;
    reports.clear();
}
- (void)l2capChannelOpenComplete:(IOBluetoothL2CAPChannel *)channel status:(IOReturn)status {
    if (status != kIOReturnSuccess) {
        [self disconnect];
        return;
    }
    if ([channel getPSM] == 0x11) {
        control = channel;
        IOBluetoothL2CAPChannel *input = nil;
        if ([device openL2CAPChannelAsync:&input withPSM:0x13 delegate:self] != kIOReturnSuccess) {
            [self disconnect];
            return;
        }
        interrupt = input;
    } else {
        interrupt = channel;
        ready = true;
    }
}
- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel *)channel {
    [self disconnect];
}
- (void)l2capChannelData:(IOBluetoothL2CAPChannel *)channel data:(void *)bytes length:(size_t)length {
    const auto *data = static_cast<const uint8_t *>(bytes);
    if (length < 2 || length > 23 || data[0] != 0xA1 || reports.size() >= 128)
        return;
    reports.emplace_back(data + 1, data + length);
}
@end

@interface RevolveBluetooth : NSObject <IOBluetoothDeviceInquiryDelegate> {
@public
    IOBluetoothDeviceInquiry *inquiry;
    NSMutableArray<RevolveRemote *> *remotes;
    bool scanning;
    CFAbsoluteTime nextScan;
}
- (void)poll;
- (void)shutdown;
@end

@implementation RevolveBluetooth
- (instancetype)init {
    self = [super init];
    if (self) {
        remotes = [NSMutableArray array];
        for (unsigned i = 0; i < 4; ++i)
            [remotes addObject:[[RevolveRemote alloc] init]];
        inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
        inquiry.inquiryLength = 4;
        inquiry.updateNewDeviceNames = YES;
    }
    return self;
}
- (void)poll {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    if (scanning || CFAbsoluteTimeGetCurrent() < nextScan)
        return;
    bool full = true;
    for (RevolveRemote *remote in remotes)
        full &= remote->device != nil;
    if (full)
        return;
    nextScan = CFAbsoluteTimeGetCurrent() + 10;
    scanning = [inquiry start] == kIOReturnSuccess;
}
- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender error:(IOReturn)error aborted:(BOOL)aborted {
    scanning = false;
    if (error != kIOReturnSuccess || aborted)
        return;
    for (IOBluetoothDevice *candidate in [sender foundDevices]) {
        NSString *name = [candidate name];
        if (![name isEqualToString:@"Nintendo RVL-CNT-01"] && ![name isEqualToString:@"Nintendo RVL-CNT-01-TR"])
            continue;
        bool assigned = false;
        for (RevolveRemote *remote in remotes)
            assigned |= remote->device && [[remote->device addressString] isEqualToString:[candidate addressString]];
        if (assigned)
            continue;
        for (RevolveRemote *remote in remotes) {
            if (!remote->device) {
                [remote connect:candidate];
                break;
            }
        }
    }
}
- (void)shutdown {
    inquiry.delegate = nil;
    [inquiry stop];
    for (RevolveRemote *remote in remotes)
        [remote disconnect];
}
@end

static RevolveBluetooth *hub;

void pollNativeBluetooth() {
    @autoreleasepool {
        if (!hub)
            hub = [[RevolveBluetooth alloc] init];
        [hub poll];
    }
}

void shutdownNativeBluetooth() {
    @autoreleasepool {
        [hub shutdown];
        hub = nil;
    }
}

bool nativeBluetoothConnected(std::size_t slot) {
    return hub && slot < hub->remotes.count && hub->remotes[slot]->ready;
}

bool sendNativeBluetooth(std::size_t slot, uint8_t report, std::span<const uint8_t> payload) {
    if (!nativeBluetoothConnected(slot) || payload.size() > 21)
        return false;
    @autoreleasepool {
        RevolveRemote *remote = hub->remotes[slot];
        std::array<uint8_t, 23> data{};
        data[0] = 0xA2;
        data[1] = report;
        std::copy(payload.begin(), payload.end(), data.begin() + 2);
        if ([remote->interrupt writeSync:data.data() length:static_cast<UInt16>(payload.size() + 2)] != kIOReturnSuccess) {
            [remote disconnect];
            return false;
        }
    }
    return true;
}

std::vector<uint8_t> receiveNativeBluetooth(std::size_t slot) {
    if (!nativeBluetoothConnected(slot))
        return {};
    auto &queue = hub->remotes[slot]->reports;
    if (queue.empty())
        return {};
    auto report = std::move(queue.front());
    queue.pop_front();
    return report;
}
