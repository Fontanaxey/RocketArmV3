#!/bin/bash

# Hardcoding dei percorsi assoluti (le virgolette gestiscono lo spazio in 'VS Code')
ROOT_DIR="/home/fontanaxey/Documents/VS Code/RocketArmV3"
SCRIPT_DIR="$ROOT_DIR/scripts"
EXEC_PATH="$ROOT_DIR/bin/robot_arm"
PYTHON_BIN="$ROOT_DIR/PyTest.venv/bin/python"

# Verifica preventiva dell'esistenza del binario
if [ ! -f "$EXEC_PATH" ]; then
    echo "[-] Errore: Il binario 'robot_arm' non esiste in: $EXEC_PATH" >&2
    exit 1
fi

# Fallback se il virtual environment non è ancora stato creato
if [ ! -f "$PYTHON_BIN" ]; then
    echo "[!] PyTest.venv non trovato. Fallback sul python3 di sistema."
    PYTHON_BIN="python3"
fi

# Assicura i permessi di esecuzione sugli script
chmod +x "$SCRIPT_DIR/virtual_serial.sh"

# 1. Avvia virtual_serial.sh in una nuova finestra GNOME Terminal
echo "[+] Lancio virtual_serial.sh in finestra separata..."
gnome-terminal --title="Virtual Serial" -- bash -c "\"$SCRIPT_DIR/virtual_serial.sh\"; exec bash" &

# Attesa per il binding dei nodi tty virtuali
sleep 1

# 2. Avvia test_protocol.py nel venv in una seconda finestra GNOME Terminal
echo "[+] Lancio test_protocol.py in finestra separata..."
gnome-terminal --title="Test Protocol" -- bash -c "\"$PYTHON_BIN\" \"$SCRIPT_DIR/test_protocol.py\"; exec bash" &

# Attesa stabilizzazione dello script Python
sleep 0.5

# 3. Esegue robot_arm in foreground nel terminale corrente
echo "[+] Esecuzione di robot_arm in questa shell..."
echo "------------------------------------------------------------------------"
"$EXEC_PATH"