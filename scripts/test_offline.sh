#!/bin/bash

# Risoluzione dinamica dei percorsi (indipendente da dove viene lanciato lo script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Selezione dell'eseguibile (fallback su build/ se bin/ non esiste)
EXEC_PATH="$ROOT_DIR/bin/robot_arm"
if [ ! -f "$EXEC_PATH" ] && [ -f "$ROOT_DIR/build/robot_arm" ]; then
    EXEC_PATH="$ROOT_DIR/build/robot_arm"
fi

# Selezione dell'interprete Python (VENV o sistema)
PYTHON_BIN="python3"
if [ -f "$ROOT_DIR/PyTest.venv/bin/python" ]; then
    PYTHON_BIN="$ROOT_DIR/PyTest.venv/bin/python"
fi

# Validazione preliminare della build C
if [ ! -f "$EXEC_PATH" ]; then
    echo "[-] Errore Critico: Il binario 'robot_arm' non esiste." >&2
    echo "    Esegui prima la build con CMake." >&2
    exit 1
fi

# Definizione del file di log per l'analisi post-esecuzione
LOG_FILE="$ROOT_DIR/logs/commads_logs.txt"

# Gestione dei processi in background
PIDS=()
cleanup() {
    for pid in "${PIDS[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            kill -TERM "$pid" 2>/dev/null
        fi
    done
}
# Trap per interruzioni brutali dello script bash
trap cleanup INT TERM

# Assicura i permessi sul modulo seriale virtuale
chmod +x "$SCRIPT_DIR/virtual_serial.sh"

# 1. Avvio del bridge seriale virtuale (/tmp/ttyV0 <-> /tmp/ttyV1)
"$SCRIPT_DIR/virtual_serial.sh" > /dev/null 2>&1 &
PIDS+=($!)
sleep 1

# 2. Avvio dello script Python per il parsing del protocollo
# Gestisce internamente la scrittura asincrona su logs/commads_logs.txt
"$PYTHON_BIN" "$SCRIPT_DIR/test_protocol.py" &
PYTHON_PID=$!
PIDS+=($!)
sleep 0.5

# 3. Esecuzione dell'applicazione NCurses (prende il controllo del terminale)
"$EXEC_PATH"

# 4. Fase di smantellamento (NCurses è stato chiuso dall'utente)
# Inviamo un SIGINT (2) a Python per triggerare l'except KeyboardInterrupt ed uscire pulito
kill -2 "$PYTHON_PID" 2>/dev/null
cleanup

# 5. Analisi automatica del log generato dal test
echo "------------------------------------------------------------------------"
if [ -f "$LOG_FILE" ]; then
    # Cerca pattern di errore nel log usando espressioni regolari estese
    if grep -qE "\[ERR\]|\[FATAL\]" "$LOG_FILE"; then
        echo -e "\033[0;31m[-] TEST FALLITI: Rilevati errori nel flusso dati di comunicazione.\033[0m" >&2
        echo "Controlla il file log per i dettagli: $LOG_FILE" >&2
        echo "------------------------------------------------------------------------"
        grep -E "\[ERR\]|\[FATAL\]" "$LOG_FILE" >&2
        exit 1
    else
        echo -e "\033[0;32m[+] TEST PASSATI: Nessun errore rilevato nel protocollo di comunicazione.\033[0m"
        echo "Log completo registrato in: $LOG_FILE"
        exit 0
    fi
else
    echo "[-] Errore: Il file di log non è stato generato dallo script Python." >&2
    exit 1
fi