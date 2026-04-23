#!/bin/bash

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN_APP="$PROJECT_ROOT/bin/robot_arm"
VENV_PYTHON="$PROJECT_ROOT/PyTest.venv/bin/python3"
VALIDATOR_PY="$PROJECT_ROOT/scripts/test_protocol.py"
SETUP_SH="$PROJECT_ROOT/scripts/virtual_serial.sh"

if [ ! -f "$BIN_APP" ]; then
    echo "Error: Compile the project first."
    exit 1
fi

if command -v gnome-terminal >/dev/null; then
    TERM_CMD="gnome-terminal -- bash -c"
elif command -v xterm >/dev/null; then
    TERM_CMD="xterm -e"
else
    echo "Error: No terminal emulator (gnome-terminal, xterm) found."
    exit 1
fi

echo "Initializing multi-terminal environment..."

$TERM_CMD "$SETUP_SH; exec bash" &
sleep 1.5

if [ -f "$VENV_PYTHON" ]; then
    $TERM_CMD "$VENV_PYTHON $VALIDATOR_PY --port /tmp/vnode1; exec bash" &
else
    echo "Warning: Virtual environment not found, launching with system python3."
    $TERM_CMD "python3 $VALIDATOR_PY --port /tmp/vnode1; exec bash" &
fi
sleep 1.5

echo "Launching control interface..."
$TERM_CMD "$BIN_APP /tmp/vnode0" &

echo "Stack started successfully."