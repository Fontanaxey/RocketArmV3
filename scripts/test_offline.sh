#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

EXEC_PATH="$ROOT_DIR/build/bin/robot_arm"
if [ ! -f "$EXEC_PATH" ] && [ -f "$ROOT_DIR/build/robot_arm" ]; then
    EXEC_PATH="$ROOT_DIR/build/robot_arm"
fi

PYTHON_BIN="python3"
if [ -f "$ROOT_DIR/PyTest.venv/bin/python" ]; then
    PYTHON_BIN="$ROOT_DIR/PyTest.venv/bin/python"
fi

if [ ! -f "$EXEC_PATH" ]; then
    echo "[-] Critical error: The binary 'robot_arm' does not exist." >&2
    echo "    Please build it first with CMake." >&2
    exit 1
fi

LOG_FILE="$ROOT_DIR/logs/commands_logs.txt"

PIDS=()
cleanup() {
    for pid in "${PIDS[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            kill -TERM "$pid" 2>/dev/null
        fi
    done
}

trap cleanup INT TERM

chmod +x "$SCRIPT_DIR/virtual_serial.sh"

"$SCRIPT_DIR/virtual_serial.sh" > /dev/null 2>&1 &
PIDS+=($!)
sleep 1

"$PYTHON_BIN" "$SCRIPT_DIR/test_protocol.py" &
PYTHON_PID=$!
PIDS+=($!)
sleep 0.5

"$EXEC_PATH"

kill -2 "$PYTHON_PID" 2>/dev/null
cleanup

echo "------------------------------------------------------------------------"
if [ -f "$LOG_FILE" ]; then
    if grep -qE "\[ERR\]|\[FATAL\]" "$LOG_FILE"; then
        echo -e "\033[0;31m[-] TEST FAILED: error found in log.\033[0m" >&2
        echo "check $LOG_FILE, for details" >&2
        echo "------------------------------------------------------------------------"
        grep -E "\[ERR\]|\[FATAL\]" "$LOG_FILE" >&2
        exit 1
    else
        echo -e "\033[0;32m[+] TEST PASSED: no errors found in log.\033[0m"
        echo "Log registered in: $LOG_FILE"
        exit 0
    fi
else
    echo "[-] Error: log file not found" >&2
    exit 1
fi