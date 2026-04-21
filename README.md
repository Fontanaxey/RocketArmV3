# RocketArmV3

A professional-grade **Linux CLI application** written in C for high-precision control of a multi-joint robotic arm. It features a robust binary protocol, real-time Ncurses-based telemetry, and a complete hardware-in-the-loop simulation environment.

## Key Features

* **Interactive UI**: Terminal-based interface using `ncurses` with real-time status alerts.
* **Robust Protocol**: Custom 5-byte binary packet with **XOR Checksum** verification to ensure basic data integrity over serial links.
* **Modular Architecture**: Clean separation between UI logic, Serial communication (POSIX `termios`), and Protocol handling.
* **Enterprise Logging**: Structured logging system for access and errors with absolute path resolution based on `/proc/self/exe`.
* **Testing Suite**: Integrated Python-based packet validator and virtual serial port simulation using `socat`.

## Project Structure

```text
.
├── bin/                # Compiled binaries
├── include/            # C Header files (.h)
├── src/                # C Source files (.c)
├── scripts/            # Automation & Testing (udev rules, python, socat)
├── logs/               # Application & Error logs
├── Doxyfile
├── Makefile
└── LICENSE.md
```

## Prerequisites

Ensure your Linux environment is ready:

```bash
sudo apt update
sudo apt install -y libncurses5-dev libncursesw5-dev socat python3-pip python3-venv
```

## Documentation

Generate the technical manual and call graphs using Doxygen:

```bash
sudo apt install doxygen graphviz
make doc
# Open docs/html/index.html in your browser
```

## Installation & Build

1. **Clone and Enter**:

   ```bash
   git clone [https://github.com/Fontanaxey/RocketArmV3.git](https://github.com/Fontanaxey/RocketArmV3.git)
   cd RocketArmV3
   ```

2. **Build the Project**:

    ```bash
    make clean && make
    ```

3. Setup Environment:

    ```Bash
    chmod +x scripts/*.sh
    ./scripts/setup.sh
    ```

## Protocol Specification

The arm communicates via a **5-byte binary frame** at **9600 baud**:

| Byte | Field | Description |
| :--- | :--- | :--- |
| 0 | **START** | Fixed marker: `0xAA` |
| 1 | **ID** | Motor/Joint Identifier |
| 2 | **VALUE** | Position value (0-180) |
| 3 | **CKSUM** | XOR Checksum ($ID \oplus VALUE$) |
| 4 | **END** | Fixed marker: `0x55` |

## License

This project is licensed under the **MIT License**. See the `LICENSE.md` file for details.

### Author

**Marco Fontana** - [@Fontanaxey](https://github.com/Fontanaxey)
