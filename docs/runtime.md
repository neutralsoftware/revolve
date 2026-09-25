# Runtime setup and current limitations

This implementation is incomplete. It does not establish playable Wii Sports, correct commercial-game audio, or full MotionPlus compatibility. No build, automated test, hardware test, or performance benchmark was run for these changes.

## Launching

With an existing Revolve executable:

```sh
revolve exec /path/to/game.iso
revolve --real-wiimotes exec /path/to/game.iso
revolve -S --real-wiimotes exec /path/to/game.iso
```

The `exec` command also accepts WBFS, RVZ, DOL, and ELF inputs. The `disc` command prints disc information; it does not run the game. `-S` opens the debugger.

## Physical Wii Remotes on macOS

`--real-wiimotes` enables IOBluetooth discovery and connects HID control and interrupt L2CAP channels. Put the remote into discovery mode with its SYNC button while Revolve is running. macOS may request Bluetooth access using the executable's embedded usage description. Connection attempts and failures are printed to stderr. Discovery repeats while fewer than four remotes are assigned.

Without this flag, Revolve looks for Wii Remotes already exposed through SDL HID. The native Bluetooth and SDL HID paths are alternatives. Native mode reserves the four emulated remote slots for physical remotes.

Output reports and input reports pass through to the real device, including extension register transactions. This provides the transport needed for Nunchuk and MotionPlus; it does not emulate missing physical attachments or establish that the guest initializes them correctly. Physical IR pointing still needs an IR source such as a powered sensor bar. Pairing and link-key handling are not complete, and controller models and macOS versions have not been exercised.

## Persistent NAND

The default NAND root is `nand` beneath SDL's per-user preference directory for organization `neutralsoftware` and application `Revolve`. Set an explicit root with:

```sh
REVOLVE_NAND_PATH=/absolute/path/to/nand revolve exec /path/to/game.iso
```

A fresh root receives filesystem directories, SYSCONF, and EUR/PAL English settings. Existing configuration files are preserved. Guest files and saves persist between runs. File attributes are stored in a sibling directory whose name is the NAND directory name plus `-metadata`; keep that directory with the NAND when moving it.

NAND ownership and permission enforcement, quotas, and complete title lifecycle semantics remain unfinished. This is a directory-backed filesystem implementation, not a raw NAND image importer.

## Imported System Menu

```sh
REVOLVE_NAND_PATH=/absolute/path/to/imported/nand revolve menu
REVOLVE_NAND_PATH=/absolute/path/to/imported/nand revolve -S menu
```

These commands locate title `00000001/00000002`, parse its `content/title.tmd`, locate the boot content, verify its SHA-1 digest, and load its DOL. Shared content uses `shared1/content.map`. An empty generated NAND does not include Nintendo system software. Raw NAND dumps, encrypted content installation, and a complete IOS reload path are not implemented. Loading the executable does not establish a functioning System Menu.

## Audio

The audio interface and DSP audio-DMA registers feed big-endian stereo PCM from guest RAM to SDL playback at the selected 32 or 48 kHz rate. Guest cycles drive consumption. Output is batched, and excessive host queue growth is bounded.

The DSP still lacks game microcode execution and AX/AXWii voice mixing. PCM playback alone cannot produce the sound expected by commercial games. DSP mailbox/task handling, ADPCM voices, resampling, filters, effects, and accurate DSP scheduling remain required.

## IOS and networking

Implemented services include persistent FS operations, ES metadata/content queries, asynchronous STM event hooks, queued IPC replies, IPv4 TCP/UDP operations, DNS, network configuration, and an adjustable session network clock. Guest sockets use host networking; blocking guest calls are deferred while emulation continues.

This is partial IOS coverage. ES title installation/launch/authentication, complete USB support, remaining DI commands, socket options/address-resolution variants, SSL, ICMP, and online WC24 behavior remain unfinished. WC24 lifecycle stubs do not restore Nintendo online services.

## Performance

Host input polling is limited to periodic intervals, emulated Wii Remote reports use guest time, host PCM writes are batched, and execution is paced against elapsed wall time. These changes reduce specific sources of overhead and runaway queue growth. They have not been benchmarked, and no speedup or full-speed gameplay is claimed. CPU, GPU, DMA, interrupt, and DSP timing still need validation.
