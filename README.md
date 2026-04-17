# RocketArmV3 Controller

A high-performance C-based Terminal User Interface (TUI) for real-time n-DOF robot arm control. Designed for Linux systems using `ncurses` and a custom binary serial protocol.

## Key Features

- **Responsive TUI:** Built with `ncurses` for low-latency feedback and intuitive menu navigation.
- **Binary Protocol:** Optimized 5-byte packet framing with XOR Checksum for data integrity.
- **Hardware Abstraction:** Modular design separating UI logic from POSIX-compliant serial communication (`termios`).
- **Robust Build System:** Automated Makefile with dependency tracking and clean artifact separation.

## Quick Start

### Prerequisites

Install `ncurses` development headers:

```bash

sudo apt update && sudo apt install libncurses5-dev libncursesw5-dev
