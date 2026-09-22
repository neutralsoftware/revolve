# Revolve Roadmap

A from-scratch Wii emulator focused on correctness, understanding, and gradual hardware emulation.

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

- [ ] Implement the GX Command Processor
- [ ] Decode GX vertices
- [ ] Add primitive assembly support
- [ ] Implement the GX Transform Unit
- [ ] Add GX rasterization
- [ ] Implement GX TEV
- [ ] Add texture memory
- [ ] Decode GX texture formats
- [ ] Implement mipmaps
- [ ] Add EFB support
- [ ] Add XFB support

## First Graphics Milestones

- [ ] Render a triangle from emulated GX commands
- [ ] Render a basic homebrew application
- [ ] Produce a recognizable game frame

## Input

- [ ] Add GameCube Controller support
- [ ] Add Wii Remote support
- [ ] Add Wii Remote accelerometer support
- [ ] Add Wii Remote IR support
- [ ] Add Nunchuk support

## Audio / DSP

- [ ] Research DSP architecture
- [ ] Implement basic audio DMA
- [ ] Output PCM audio
- [ ] Synchronize audio with emulation
- [ ] Implement enough DSP functionality for games

## NAND

- [ ] Implement NAND filesystem
- [ ] Implement SYSCONF
- [ ] Implement save data

## Wii System Services

- [ ] Add ES
- [ ] Add FS
- [ ] Add ISFS
- [ ] Add STM
- [ ] Add DI
- [ ] Add USB
- [ ] Add Bluetooth
- [ ] Add network services
- [ ] Add WC24 stubs
- [ ] Add System Menu support

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
