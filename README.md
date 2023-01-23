# Hammer-OS
Hammer OS is a toy operating system for TISC-V. It is a hobby project to learn about operating systems and the RISC-V architecture.

## Build

### Prerequisites

- [QEMU on RISC-V](https://www.qemu.org/)
- RISC-V GNU Toolchain (riscv64-linux-gnu-*)

### Build & Run

```bash
cd src/arch/risc-v
make image -j
make qemu
```

Or just change to the `src/arch/risc-v` directory and run `make qemu -j`.

## Known Issues

- The project structure has not been optimized.
- There is some bugs in multi-core support.
- Memory disk is used as the root file system.
- The kernel is not fully tested.

## License

Hammer OS is licensed under the [MIT License](LICENSE).

## Acknowledgements

- [rCore-Tutorial](https://github.com/rcore-os/rCore-Tutorial-v3)