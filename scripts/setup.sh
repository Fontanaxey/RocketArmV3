#!/bin/bash

echo "[*] Dependencies installation (ncurses)..."
sudo apt update && sudo apt install -y libncurses5-dev libncursesw5-dev python3-serial socat

echo "[*] Configuring udev rules..."
sudo cp scripts/99-robot-arm.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "[*] Making other scripts executable..."
chmod +x scripts/virtual_serial.sh

echo "[OK] Setup completed. Restart the serial port or reconnect the Arduino."