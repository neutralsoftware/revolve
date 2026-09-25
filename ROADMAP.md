# Revolve Roadmap

A from-scratch Wii emulator focused on correctness, understanding, and gradual hardware emulation.

Checkboxes describe implementation progress, not runtime certification. The audio, NAND, network, and physical-controller additions have not been built or tested. Commercial-game milestones require observed gameplay; Wii Sports compatibility is not established. See [runtime setup and limitations](docs/runtime.md).

---

## Executable Loading

- [x] Load DOL executables
- [x] Load ELF executables

## Memory

- [x] Implement generic memory interface and research physical memory map
- [x] Implement RAM
- [x] Implement MEM1
- [x] Implement MEM2
- [x] Implement big-endian reads and writes
- [x] Implement floating-point memory accesses
- [x] Add address validation, memory traces and dumps
- [x] Implement MMIO dispatch

## PowerPC / Broadway

- [x] Implement CPU state for PowerPC
- [x] Instruction fetch and decode
- [x] Integer instructions
- [x] Logical instructions
- [x] Shift and rotate instructions
- [x] Comparison instructions
- [x] Load instructions
- [x] Store instructions
- [x] Branch instructions
- [x] Special register instructions
- [x] Floating-point instructions
- [x] Paired singles
- [x] Run simple PowerPC programs
- [x] Implement exceptions
- [x] Add MMU and memory protection

## Core Hardware

- [x] Add hardware registers
- [x] Add the Processor Interface (PI)
- [x] Implement global clock and event scheduler
- [x] Implement system timers
- [x] Implement interrupts

## Starlet / IOS

- [x] Implement IPC for Starlet
- [x] PPC -> Starlet handling
- [x] Starlet -> PPC handling and IPC request queues
- [x] Implement HLE IOS
  - [x] `/dev/es`
  - [x] `/dev/fs`
  - [x] `/dev/di`
  - [x] `/dev/stm`
  - [x] `/dev/video`
  - [x] ioctl handling
  - [x] File descriptors
  - [x] Error codes

## Disc

- [x] Implement disc image reading
- [x] Implement disc image encryption/decryption
- [x] Parse FST
- [x] Read files by path
- [x] Load executables from disc images

## GX / Hollywood Graphics

- [x] Implement the GX Command Processor
- [x] Decode GX vertices
- [x] Add primitive assembly support
- [x] Implement the GX Transform Unit
- [x] Add GX rasterization
- [x] Implement GX TEV
- [x] Add texture memory
- [x] Decode GX texture formats
- [x] Implement mipmaps
- [x] Add EFB support
- [x] Add XFB support

## First Graphics Milestones

- [x] Render a triangle from emulated GX commands
- [x] Render a basic homebrew application
- [x] Produce a recognizable game frame

## Input

- [x] Add GameCube Controller support
- [x] Add Wii Remote support
- [x] Add Wii Remote accelerometer support
- [x] Add Wii Remote IR support
- [x] Add Nunchuk support
- [ ] Add Bluetooth support for Wii Remotes
  - [x] macOS discovery and HID L2CAP transport
  - [x] Forward physical reports, including extension and MotionPlus traffic
  - [ ] Complete pairing, reconnect, and controller-model compatibility

## Audio / DSP

- [x] Research DSP architecture and AXWii command variants
- [x] Implement basic audio DMA
- [x] Output PCM audio
- [x] Schedule DMA consumption using guest cycles and the selected sample rate
- [ ] Complete audio synchronization and DSP scheduling
- [ ] Implement enough DSP functionality for games

## NAND

- [ ] Implement NAND filesystem
  - [x] Persistent files, independent descriptors, directory operations, and attributes
  - [ ] Enforce title ownership, permissions, quotas, and complete error semantics
- [x] Initialize persistent SYSCONF and regional settings
- [x] Persist title save directories and file contents
- [ ] Complete save lifecycle and title isolation

## Wii System Services

- [ ] Complete ES
  - [x] Title identity, TMDs, ticket views, and installed-content reads
  - [ ] Title launch/reload, installation, and authentication
- [ ] Complete FS
- [ ] Complete ISFS
- [ ] Complete STM
  - [x] Asynchronous event hooks and shutdown
  - [ ] Hot reset and remaining lifecycle behavior
- [ ] Complete DI command coverage
- [ ] Complete USB beyond the emulated Bluetooth device
- [ ] Complete Bluetooth HCI and connection lifecycle
- [ ] Complete network services
  - [x] IPv4 TCP/UDP, asynchronous socket operations, DNS, and network configuration
  - [ ] Remaining socket APIs, SSL, and ICMP
- [x] Add offline WC24 lifecycle stubs and network clock
- [ ] Add System Menu support
  - [x] Load and hash-check imported NAND System Menu boot content
  - [ ] Complete runtime services needed by the System Menu

## Timing & Accuracy

- [ ] Verify CPU timing
- [ ] Verify GPU timing
- [ ] Verify DSP timing
- [ ] Verify DMA timing
- [ ] Verify interrupt timing
- [ ] Verify global clock synchronization

## Game Compatibility

- [ ] Boot Wii homebrew
- [ ] Boot first commercial game
- [ ] Reach first commercial game's menu
- [ ] Render first commercial game correctly
- [ ] Receive input in a commercial game
- [ ] Produce audio in a commercial game
- [ ] Run first playable commercial game
- [ ] Run Wii Play
- [ ] Run Wii Sports
- [ ] Run Mario Kart Wii
- [ ] Run Super Mario Galaxy
