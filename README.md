# 8-Bit CPU Emulator

A fully custom 8-bit computer emulated in software — complete with its own CPU, ISA, assembler, memory-mapped I/O devices, bootloader, operating system shell, filesystem, and a playable Snake game. Everything from the hardware up was designed and implemented from scratch.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Repository Layout](#repository-layout)
3. [Building](#building)
4. [Running](#running)
5. [Architecture](#architecture)
   - [Memory Map](#memory-map)
   - [CPU & Registers](#cpu--registers)
   - [Instruction Set](#instruction-set)
   - [Status Flags](#status-flags)
6. [Devices & MMIO](#devices--mmio)
   - [Console (0x8000–0x8001)](#console-0x80000x8001)
   - [Timer (0x8010–0x8014)](#timer-0x80100x8014)
   - [Disk (0x8020–0x8026)](#disk-0x80200x8026)
7. [Assembler](#assembler)
8. [Boot Sequence](#boot-sequence)
9. [Kernel & Shell](#kernel--shell)
10. [Filesystem](#filesystem)
11. [Disk Image](#disk-image)
12. [Built-in Programs](#built-in-programs)
    - [HELLO.BIN](#hellobin)
    - [SNAKE.BIN](#snakebin)
13. [Writing Your Own Programs](#writing-your-own-programs)

---

## Project Overview

The emulator simulates an 8-bit CPU connected to a bus that carries RAM, ROM, a serial console, a timer, and a block-device disk. The CPU is implemented as a fetch-decode-execute loop in C++17. All software — bootloader, shell, filesystem driver, and user programs — is written in the custom assembly language and assembled at startup by the built-in assembler.

The design goal was a complete, self-consistent stack: no borrowed ISAs, no pre-existing operating systems, nothing that didn't originate in this project.

---

## Repository Layout

```
.
├── Bus.cpp / Bus.hpp               Bus: routes addresses to the right device
├── CPU.cpp / CPU.hpp               CPU: fetch-decode-execute, interrupts, stack
├── Instructions.cpp / .hpp         Opcode table and execute() dispatcher
├── Registers.hpp                   CPUState struct (R0–R7, PC, SP, flags)
├── Memory.cpp / Memory.hpp         Base memory interface
├── RamDevice.cpp / .hpp            64 KiB RAM (0x0000–0x7FFF)
├── RomDevice.cpp / .hpp            4 KiB ROM (0xF000–0xFFFF)
├── ConsoleDevice.cpp / .hpp        UART-style serial console (MMIO)
├── TimerDevice.cpp / .hpp          16-bit countdown timer with IRQ (MMIO)
├── DiskDevice.cpp / .hpp           Block-device disk controller (MMIO)
├── Assembler.cpp / .hpp            Two-pass assembler (assembled at runtime)
├── Loader.cpp / .hpp               Loads assembled bytes onto the bus
├── main.cpp                        Entry point: assemble, load, reset, run
├── assemble_tool.cpp               Standalone assembler command-line tool
├── CMakeLists.txt                  CMake build (C++17, -O2)
├── bootrom.txt                     Boot ROM source (origin 0xF000)
├── kernel.txt                      Kernel + shell source (origin 0x0200)
├── hello_prog.txt                  Source for HELLO.BIN
├── snake.txt                       Source for SNAKE.BIN
├── hello.bin                       Assembled HELLO.BIN binary
├── snake.bin                       Assembled SNAKE.BIN binary
├── mkdisk.py                       Builds disk.img from the binary files
└── disk.img                        32 KiB disk image (generated)
```

---

## Building

### Quick build (manual, no CMake)

```bash
g++ -std=c++17 -Wall -Wextra -O2 -I. \
    Bus.cpp ConsoleDevice.cpp CPU.cpp Instructions.cpp \
    Loader.cpp Memory.cpp RamDevice.cpp RomDevice.cpp \
    TimerDevice.cpp DiskDevice.cpp Assembler.cpp main.cpp \
    -o emu_disk
```

Build the standalone assembler tool:

```bash
g++ -std=c++17 -I. \
    Assembler.cpp Loader.cpp Memory.cpp RamDevice.cpp RomDevice.cpp \
    Bus.cpp ConsoleDevice.cpp TimerDevice.cpp DiskDevice.cpp Instructions.cpp CPU.cpp \
    assemble_tool.cpp -o assemble_tool
```

### CMake build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
# Binaries land in build/bin/
```

### Rebuild the disk image

```bash
# Assemble the user programs first if you've edited them:
./assemble_tool hello_prog.txt hello.bin
./assemble_tool snake.txt snake.bin

# Recreate disk.img:
python3 mkdisk.py disk.img hello.bin snake.bin
```

---

## Running

```bash
./emu_disk kernel.txt bootrom.txt
```

The emulator reads `disk.img` from the working directory (or next to the binary). On first run it creates an empty `disk.img` automatically if none is found.

Typical startup output:

```
[BOOTROM]
KERNEL OK
[KERNEL]
>
```

The `>` prompt is the shell. Type `help` for a command list.

---

## Architecture

### Memory Map

| Address range | Size | Device |
|---|---|---|
| `0x0000–0x7FFF` | 32 KiB | RAM (general purpose) |
| `0x8000–0x8001` | 2 bytes | Console MMIO |
| `0x8010–0x8014` | 5 bytes | Timer MMIO |
| `0x8020–0x8026` | 7 bytes | Disk MMIO |
| `0x8002–0x800F`, `0x8027–0x80FF` | — | Reserved MMIO (reads 0xFF) |
| `0xF000–0xFFFF` | 4 KiB | ROM (boot ROM) |

Notable regions within RAM:

| Address | Use |
|---|---|
| `0x0000–0x0001` | Interrupt vector table: timer IRQ handler address (hi, lo) |
| `0x0200–0x02FF` | Kernel magic bytes `BK` + kernel code start |
| `0x6000–0x62FF` | User program load area (up to 3 × 256-byte blocks) |
| `0x6300–0x63FF` | User program scratch RAM (not loaded from disk) |
| `0xEFFF` | Initial stack pointer (grows downward) |

### CPU & Registers

The CPU is an 8-bit accumulator-style processor with a 16-bit address space.

**General-purpose registers:** R0–R7 (each 8 bits). Registers can also be named A–H as aliases (R0=A, R1=B, … R7=H).

**Special registers:**

| Register | Width | Description |
|---|---|---|
| PC | 16-bit | Program counter |
| SP | 16-bit | Stack pointer (grows downward) |
| SR | 4-bit | Status register (Z, C, V, S flags) |

**Reset state:** PC = `0xF000`, SP = `0xEFFF`, interrupts enabled.

Memory addressing uses a register-pair convention: `LDR Rd, Ra` reads from the address formed by `R[Ra]:R[Ra+1]` (big-endian, mod 8 wrap). Most code uses `R3:R4` as the address pair (R3 = page/high byte, R4 = offset/low byte).

### Instruction Set

All instructions are 1–3 bytes. The encoding is:

- **1 byte:** opcode only (NOP, RET, HLT, EI, DI, IRET, PUSHF, POPF)
- **2 bytes:** opcode + register byte (most ALU and single-register ops)
- **3 bytes:** opcode + two operand bytes (LDI, all jumps and calls)

Register-pair encoding for two-register instructions: the byte after the opcode packs both registers as `(Rd << 4) | Rs`.

#### Data Transfer

| Mnemonic | Encoding | Description |
|---|---|---|
| `MOV Rd, Rs` | `0x10 (d<<4\|s)` | Rd = Rs |
| `LDI Rd, imm` | `0x11 d imm` | Rd = immediate byte |
| `LDR Rd, Ra` | `0x12 (d<<4\|a)` | Rd = mem[R[Ra]:R[Ra+1]] |
| `STR Rs, Ra` | `0x13 (s<<4\|a)` | mem[R[Ra]:R[Ra+1]] = Rs |

#### Arithmetic & Logic

| Mnemonic | Opcode | Description |
|---|---|---|
| `ADD Rd, Rs` | `0x20` | Rd += Rs, sets Z C V S |
| `SUB Rd, Rs` | `0x21` | Rd -= Rs, sets Z C V S |
| `AND Rd, Rs` | `0x22` | Rd &= Rs, sets Z S, clears C V |
| `OR Rd, Rs` | `0x23` | Rd \|= Rs, sets Z S, clears C V |
| `XOR Rd, Rs` | `0x24` | Rd ^= Rs, sets Z S, clears C V |
| `NOT Rd` | `0x25` | Rd = ~Rd, sets Z S, clears C V |
| `SHL Rd` | `0x26` | Rd <<= 1, old bit 7 → C |
| `SHR Rd` | `0x27` | Rd >>= 1, old bit 0 → C |
| `CMP Rd, Rs` | `0x28` | Sets flags for Rd − Rs, no store |

#### Control Flow

| Mnemonic | Opcode | Condition |
|---|---|---|
| `JMP addr` | `0x30` | Unconditional |
| `JZ addr` | `0x31` | Z == 1 (equal / zero) |
| `JNZ addr` | `0x32` | Z == 0 (not equal / non-zero) |
| `JC addr` | `0x33` | C == 1 (carry set / unsigned less-than) |
| `JN addr` | `0x34` | S == 1 (negative result) |
| `JV addr` | `0x35` | V == 1 (signed overflow) |
| `CALL addr` | `0x40` | Push PC, jump to addr |
| `RET` | `0x41` | Pop PC |

All jump targets and `CALL` addresses are absolute 16-bit values (big-endian, two bytes after the opcode).

#### Stack

| Mnemonic | Opcode | Description |
|---|---|---|
| `PUSH Rs` | `0x50` | Push Rs onto stack |
| `POP Rd` | `0x51` | Pop top of stack into Rd |
| `PUSHF` | `0x73` | Push flags (Z C V S → bits 0–3) |
| `POPF` | `0x74` | Pop flags |

#### Output

| Mnemonic | Opcode | Description |
|---|---|---|
| `OUT Rs` | `0x60` | Write Rs as ASCII character to console |
| `OUTN Rs` | `0x61` | Write Rs as decimal integer to console |

Input is memory-mapped via the Console device rather than an instruction (see [Console MMIO](#console-0x80000x8001)).

#### Interrupts & Misc

| Mnemonic | Opcode | Description |
|---|---|---|
| `EI` | `0x70` | Enable interrupts |
| `DI` | `0x71` | Disable interrupts |
| `IRET` | `0x72` | Return from interrupt handler (pop PC, re-enable interrupts) |
| `NOP` | `0x00` | No operation |
| `HLT` | `0xFF` | Halt the CPU |

### Status Flags

| Flag | Bit | Set when |
|---|---|---|
| Z | 0 | Result == 0 |
| C | 1 | Unsigned carry/borrow out of bit 7 |
| V | 2 | Signed overflow |
| S | 3 | Result bit 7 == 1 (negative) |

`CMP Rd, Rs` performs `Rd - Rs` and sets flags without writing the result. The conventional unsigned less-than test is therefore `CMP Rd, Rs` followed by `JC` (carry set means Rd < Rs unsigned).

---

## Devices & MMIO

All devices live in the MMIO window `0x8000–0x80FF`. Access them with `LDR`/`STR` using an address pair pointing into this range.

### Console (0x8000–0x8001)

| Address | Read | Write |
|---|---|---|
| `0x8000` | Read next byte from input queue (0 if empty) | Write byte to stdout |
| `0x8001` | Status: bit 0 = output ready (always 1), bit 1 = input available | Control (reserved) |

The console puts the terminal into raw mode on startup so keypresses are delivered one byte at a time without line buffering or echo. The terminal is restored on exit (including Ctrl+C).

Typical polling loop used in kernel and programs:

```asm
rcr_wait:
        LDI R6, 0x80
        LDI R7, 0x01
        LDR R0, R6              ; read status
        LDI R5, 0x02
        AND R0, R5              ; isolate bit 1 (input ready)
        LDI R5, 0
        CMP R0, R5
        JZ  rcr_wait            ; loop until a byte arrives
        LDI R7, 0x00
        LDR R0, R6              ; read the byte
```

### Timer (0x8010–0x8014)

A 16-bit free-running counter that can raise an IRQ when it reaches a compare value.

| Offset | Register | Description |
|---|---|---|
| `+0` | COUNTER_LO | Counter low byte (R/W) |
| `+1` | COUNTER_HI | Counter high byte (R/W) |
| `+2` | COMPARE_LO | Compare value low byte (R/W) |
| `+3` | COMPARE_HI | Compare value high byte (R/W) |
| `+4` | CTRL | bit 0 = enable, bit 1 = auto-reload, bit 7 = IRQ flag (write 1 to clear) |

When `counter == compare` the IRQ flag is set. If auto-reload is on the counter resets to zero; otherwise the timer stops. The IRQ dispatches to the vector at `0x0000–0x0001` in RAM.

The kernel uses the timer's low byte as an entropy source for SNAKE.BIN's RNG seed.

### Disk (0x8020–0x8026)

A block device: 128 blocks × 256 bytes = 32 KiB total. The controller buffers one block at a time.

| Offset | Register | Description |
|---|---|---|
| `+0` | BLOCK_HI | Current block number high byte |
| `+1` | BLOCK_LO | Current block number low byte |
| `+2` | OFFSET | Byte offset within the buffer (0–255, auto-wraps) |
| `+3` | DATA | Read/write one byte at current offset, then offset++ |
| `+4` | CMD | Write 1 = load block into buffer, 2 = store buffer to disk, 3 = clear buffer. Read = status (0 = OK, 1 = error) |
| `+5` | CAPACITY_HI | Total block count high byte (read-only) |
| `+6` | CAPACITY_LO | Total block count low byte (read-only) |

Typical read sequence:

```asm
; set block number
LDI R3, 0x80 ; MMIO page
LDI R4, 0x20 ; BLOCK_HI
LDI R0, 0
STR R0, R3          ; block hi = 0
LDI R4, 0x21
LDI R0, 5
STR R0, R3          ; block lo = 5

; load block into buffer
LDI R4, 0x24
LDI R0, 1
STR R0, R3          ; CMD = load

; seek to byte 0, then read
LDI R4, 0x22
LDI R0, 0
STR R0, R3          ; OFFSET = 0
LDI R4, 0x23
LDR R0, R3          ; R0 = first byte of block
```

---

## Assembler

The assembler is built into the emulator and runs at startup to assemble `kernel.txt` and `bootrom.txt` in memory. It can also be used as a standalone tool:

```bash
./assemble_tool source.txt output.bin
# prints: origin=0x6000 size=636
```

### Syntax

Comments start with `;` and run to end of line. Labels end with `:`. Mnemonics and directives are case-insensitive. String/character literals use single quotes.

**Directives:**

| Directive | Example | Effect |
|---|---|---|
| `.ORG addr` | `.ORG 0x6000` | Set the assembly origin address |
| `.DB val, ...` | `.DB 'H','i',10,0` | Emit literal bytes or character literals |
| `.RESB n` | `.RESB 1` | Reserve n bytes (emits zeros) |

**Addressing helpers:**

`HI(label)` and `LO(label)` expand to the high and low bytes of a label's address, useful for loading 16-bit addresses into register pairs:

```asm
LDI R3, HI(my_string)
LDI R4, LO(my_string)
; now R3:R4 points at my_string
```

**Two-pass assembly:** the first pass collects all label addresses; the second pass emits bytes with all forward references resolved.

---

## Boot Sequence

1. The emulator loads `bootrom.txt` (assembled) into ROM at `0xF000` and `kernel.txt` (assembled) into RAM starting at `0x0200`.
2. The CPU resets: PC = `0xF000`, SP = `0xEFFF`, interrupts enabled.
3. The boot ROM prints `[BOOTROM]`, checks for the magic bytes `BK` at `0x0200–0x0201`, prints `KERNEL OK`, and jumps to `0x0202`.
4. The kernel initialises its data structures, opens `disk.img`, prints `[KERNEL]`, and enters the shell prompt loop.

If the kernel magic check fails the boot ROM prints `NO KERNEL` and halts.

---

## Kernel & Shell

The kernel is a single-file assembly program (`kernel.txt`, ~2100 lines) that implements a line-input shell, all filesystem operations, and helper routines used by both the shell and user programs.

### Shell Commands

| Command | Description |
|---|---|
| `help` | List available commands |
| `ls` | List all files with permissions and size |
| `cat <name>` | Print a text file to the console |
| `run <name>` | Load and execute a `.BIN` program |
| `write <name>` | Create or overwrite a file; type lines and end with a line containing only `.` |
| `rm <name>` | Delete a file (not permitted on system files) |
| `mem` | Dump a section of RAM as hex |
| `tick` | Print the current timer tick count |

### Kernel internals

The kernel provides these routines (used internally and by user programs via `CALL`):

- `print_str` — print a NUL-terminated string from an address in R1:R2
- `read_line` — read a CR- or LF-terminated line into a buffer
- `read_char` — blocking read of one character
- `streq` / `strprefix` — string comparison helpers used by the command dispatcher
- Filesystem primitives: `fs_find`, `fs_load_dir`, `disk_set_block`, `disk_load`, `disk_store`, `disk_read_byte`, `disk_write_byte`, `disk_seek`, `disk_clear`

---

## Filesystem

The filesystem is a flat directory stored in block 0 of the disk. It supports up to 15 files, each with a fixed allocation of 3 blocks (768 bytes).

### Directory Block (block 0)

| Offset | Size | Field |
|---|---|---|
| 0–3 | 4 bytes | Magic `FS01` |
| 4 | 1 byte | File count (0–15) |
| 5 | 1 byte | Reserved |
| 6–7 | 2 bytes | Next free block number (big-endian) |
| 8–255 | 16 bytes × 15 | Directory entries |

### Directory Entry (16 bytes each)

| Offset | Size | Field |
|---|---|---|
| 0–10 | 11 bytes | Filename, ASCII, NUL-padded |
| 11 | 1 byte | Permission byte |
| 12–13 | 2 bytes | File size in bytes (big-endian) |
| 14–15 | 2 bytes | Start block number (big-endian) |

### Permission Byte

| Bit | Mask | Meaning |
|---|---|---|
| 0 | `0x01` | Read |
| 1 | `0x02` | Write |
| 2 | `0x04` | Execute |
| 3 | `0x08` | System (cannot be written or deleted) |

### Constraints

- Maximum 15 files on disk at once.
- Maximum filename length: 11 characters.
- Maximum file size: 768 bytes (3 × 256-byte blocks), fixed allocation regardless of actual content size.
- Files written by the shell always get permissions `0x03` (read + write). The baked-in system files have permission `0x0D` (read + execute + system).

---

## Disk Image

`disk.img` is a 32 KiB flat binary file (128 × 256 byte blocks). It is created by `mkdisk.py` which bakes in three system files:

| File | Blocks | Permissions | Description |
|---|---|---|---|
| `README.TXT` | 1–3 | `0x09` (R + system) | Welcome text, visible via `cat README.TXT` |
| `HELLO.BIN` | 4–6 | `0x0D` (R + X + system) | Simple "Hello from disk!" program |
| `SNAKE.BIN` | 7–9 | `0x0D` (R + X + system) | Classic Snake game |

User-created files (via `write`) are placed starting at block 10.

Rebuild the disk image at any time:

```bash
python3 mkdisk.py disk.img hello.bin snake.bin
```

---

## Built-in Programs

### HELLO.BIN

The simplest possible user program. Assembled from `hello_prog.txt`, loaded to `0x6000` by `run hello.bin`. Prints `Hello from disk!` and returns to the shell.

```
> run HELLO.BIN
Hello from disk!
>
```

### SNAKE.BIN

A classic Snake game on an 8×8 grid. Assembled from `snake.txt` (636 bytes), loaded at `0x6000`. The game is entirely self-contained — it accesses the console and timer directly via MMIO and cannot call kernel routines.

```
> run SNAKE.BIN

........
....*...
........
........
....#...
........
........
........
```

**Controls:** `w` up, `s` down, `a` left, `d` right. Any other key continues in the current direction. The game is turn-based — the snake moves exactly once per keypress.

**Rules:**

- The snake starts as a single segment at position (4, 4) moving right.
- Eating food (`*`) grows the snake by one segment and places new food.
- The tail follows the head each turn; old tail cells are cleared.
- The game ends when the head hits a wall or the snake's own body.
- The score (number of food eaten) is printed on game over.

**Implementation notes:**

- The snake body is stored as an array of up to 20 grid indices at `0x6360–0x6373` in scratch RAM (outside the loaded program, so it doesn't count against the 768-byte file size limit).
- Grid state is tracked in `0x6300–0x633F` (64 bytes, one per cell: 0 = empty, 1 = body, 2 = food).
- Collision is checked before the tail vacates, so moving into the cell your tail is about to leave counts as a collision.
- Food placement uses a simple LCG (`rng = rng*5 + 1 mod 256`, masked to 0–63). Food may be placed on an occupied cell; it will appear once the body moves away.
- The RNG is seeded from the timer's free-running low byte at game start.

**Scratch RAM layout (not part of the binary):**

```
0x6300–0x633F   grid[64]         (0=empty, 1=body, 2=food)
0x6340          length           current snake length (starts at 1)
0x6341          head_x
0x6342          head_y
0x6343          direction        0=up 1=right 2=down 3=left
0x6344          rng              LCG state
0x6345          score            food eaten this game
0x6346          eating_flag      scratch spill inside step_snake
0x6360–0x6373   body[20]         grid indices, body[0]=head
```

---

## Writing Your Own Programs

User programs are standalone assembly files that get loaded to `0x6000` and called via `CALL 0x6000`. They must end with `RET` to return to the shell.

### Constraints

- Maximum size: 768 bytes (3 blocks).
- Self-contained: cannot call kernel routines (the kernel's internal addresses are not stable).
- Use scratch RAM above `0x6300` for variables; the region `0x6000–0x62FF` is where your code lives.
- Do not touch `0x0000–0x01FF` (IVT + kernel data) or `0xEFFF` downward (stack).

### Skeleton

```asm
.ORG 0x6000

my_program:
        ; your code here

        ; output a character:
        LDI R0, 'H'
        OUT R0

        ; read a character (blocking):
        LDI R6, 0x80
        LDI R7, 0x01
wait:   LDR R0, R6
        LDI R5, 0x02
        AND R0, R5
        LDI R5, 0
        CMP R0, R5
        JZ  wait
        LDI R7, 0x00
        LDR R0, R6      ; R0 = key pressed

        RET             ; return to shell
```

### Assembling and installing

```bash
# assemble
./assemble_tool my_program.txt my_program.bin

# rebuild disk with your program added
# (edit mkdisk.py to include it, or use the shell's 'write' command
#  for text files -- binary programs must go through mkdisk.py)
python3 mkdisk.py disk.img hello.bin snake.bin

# run
./emu_disk kernel.txt bootrom.txt
# then at the prompt:
# > run MY_PROGRAM.BIN
```

For text files you can use the shell's built-in `write` command instead:

```
> write NOTES.TXT
This is my note.
Another line.
.
> cat NOTES.TXT
This is my note.
Another line.
```