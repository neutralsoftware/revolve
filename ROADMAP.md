# Revolve Roadmap

A from-scratch Wii emulator focused on correctness, understanding, and gradual hardware emulation.

---

## Executable Loading

- [x] Load DOL executables
- [x] Load ELF executables

## Memory

- [ ] Implement generic memory interface and research physical memory map
- [ ] Implement RAM
- [ ] Implement MEM1
- [ ] Implement MEM2
- [ ] Implement big-endian reads and writes
- [ ] Implement floating-point memory accesses
- [ ] Add address validation, memory traces and dumps
- [ ] Implement MMIO dispatch

## PowerPC / Broadway

- [ ] Implement CPU state for PowerPC
- [ ] Instruction fetch and decode
- [ ] Integer instructions
- [ ] Logical instructions
- [ ] Shift and rotate instructions
- [ ] Comparison instructions
- [ ] Load instructions
- [ ] Store instructions
- [ ] Branch instructions
- [ ] Special register instructions
- [ ] Floating-point instructions
- [ ] Paired singles
- [ ] Run simple PowerPC programs
- [ ] Implement exceptions
- [ ] Add MMU and memory protection

## Core Hardware

- [ ] Add hardware registers
- [ ] Add the Processor Interface (PI)
- [ ] Implement global clock and event scheduler
- [ ] Implement system timers
- [ ] Implement interrupts

## Starlet / IOS

- [ ] Implement IPC for Starlet
- [ ] PPC -> Starlet handling
- [ ] Starlet -> PPC handling and IPC request queues
- [ ] Implement HLE IOS
  - [ ] `/dev/es`
  - [ ] `/dev/fs`
  - [ ] `/dev/di`
  - [ ] `/dev/stm`
  - [ ] `/dev/video`
  - [ ] ioctl handling
  - [ ] File descriptors
  - [ ] Error codes

## Disc

- [ ] Implement disc image reading
- [ ] Implement disc image encryption/decryption
- [ ] Parse FST
- [ ] Read files by path
- [ ] Load executables from disc images

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
