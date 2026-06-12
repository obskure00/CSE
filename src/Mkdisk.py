#!/usr/bin/env python3
"""
mkdisk.py -- builds disk.img for the simple 8-bit-CPU emulator filesystem.

Disk layout: 128 blocks x 256 bytes = 32 KiB.

Block 0 (directory):
  [0:4]   magic "FS01"
  [4]     file_count (0-15)
  [5]     reserved
  [6:8]   next_free_block (16-bit big-endian)
  [8 + i*16 : 8 + i*16 + 16] for i in 0..14: directory entry, 16 bytes:
      [0:11]  name, ASCII, NUL-padded (max 11 chars)
      [11]    permission byte:
                bit0 = Read
                bit1 = Write
                bit2 = Execute
                bit3 = System (protected: cannot be written/deleted)
      [12:14] size in bytes (16-bit big-endian)
      [14:16] start block (16-bit big-endian)

Every file (baked-in or user-created) is given a fixed allocation of
MAX_FILE_BLOCKS blocks (512 bytes) for simplicity -- see cmd_write in
kernel.txt.
"""

import sys

BLOCK_SIZE = 256
NUM_BLOCKS = 128
MAX_FILE_BLOCKS = 2  # 512 bytes per file, fixed allocation

README_TEXT = (
    "Welcome to the simple filesystem!\n"
    "Disk: 32KB, 128 blocks of 256 bytes.\n"
    "\n"
    "Commands:\n"
    "  ls            list files\n"
    "  cat <name>    show a text file\n"
    "  run <name>    execute a program\n"
    "  write <name>  create/overwrite a file (end with '.')\n"
    "  rm <name>     delete a file\n"
    "\n"
    "README.TXT and HELLO.BIN are system files (read-only,\n"
    "cannot be deleted or overwritten).\n"
)


def make_entry(name, perm, size, start_block):
    name_bytes = name.encode("ascii")
    if len(name_bytes) > 11:
        raise ValueError(f"name too long: {name}")
    name_field = name_bytes + b"\x00" * (11 - len(name_bytes))
    entry = bytearray(16)
    entry[0:11] = name_field
    entry[11] = perm
    entry[12] = (size >> 8) & 0xFF
    entry[13] = size & 0xFF
    entry[14] = (start_block >> 8) & 0xFF
    entry[15] = start_block & 0xFF
    return bytes(entry)


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "disk.img"
    hello_bin_path = sys.argv[2] if len(sys.argv) > 2 else "hello.bin"

    with open(hello_bin_path, "rb") as f:
        hello_bytes = f.read()
    if len(hello_bytes) > MAX_FILE_BLOCKS * BLOCK_SIZE:
        raise ValueError("hello.bin too large for MAX_FILE_BLOCKS")

    readme_bytes = README_TEXT.encode("ascii")
    if len(readme_bytes) > MAX_FILE_BLOCKS * BLOCK_SIZE:
        raise ValueError("README too large for MAX_FILE_BLOCKS")

    disk = bytearray(BLOCK_SIZE * NUM_BLOCKS)

    # --- block 0: directory ---
    block0 = bytearray(BLOCK_SIZE)
    block0[0:4] = b"FS01"

    files = [
        ("README.TXT", 0x09, len(readme_bytes), 1),  # R + SYSTEM
        ("HELLO.BIN",  0x0D, len(hello_bytes),  1 + MAX_FILE_BLOCKS),  # R+X+SYSTEM
    ]
    next_free = 1 + MAX_FILE_BLOCKS * len(files)

    block0[4] = len(files)
    block0[6] = (next_free >> 8) & 0xFF
    block0[7] = next_free & 0xFF

    for i, (name, perm, size, start_block) in enumerate(files):
        off = 8 + i * 16
        block0[off:off + 16] = make_entry(name, perm, size, start_block)

    disk[0:BLOCK_SIZE] = block0

    # --- file data ---
    def write_file(start_block, data):
        for i, b in enumerate(data):
            block = start_block + (i // BLOCK_SIZE)
            off_in_block = i % BLOCK_SIZE
            disk[block * BLOCK_SIZE + off_in_block] = b

    write_file(files[0][3], readme_bytes)
    write_file(files[1][3], hello_bytes)

    with open(out_path, "wb") as f:
        f.write(disk)

    print(f"Wrote {out_path}: {len(disk)} bytes, {len(files)} files, "
          f"next_free_block={next_free}")


if __name__ == "__main__":
    main()