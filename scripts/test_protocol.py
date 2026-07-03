import os
import sys
import serial

SERIAL_PORT = '/tmp/ttyV1'
BAUD_RATE = 9600

# Risoluzione dinamica del percorso dei log relativo alla posizione di questo script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_FILE_PATH = os.path.join(SCRIPT_DIR, '..', 'logs', 'commands_logs.txt')

# Assicura che la cartella logs esista
os.makedirs(os.path.dirname(LOG_FILE_PATH), exist_ok=True)

def validate_packet(data):
    if len(data) != 5:
        # CORRETTO: Aggiunta la 'f' mancante per l'interpolazione della stringa
        return False, f"Invalid packet length: Expected 5 bytes, got {len(data)}"
    
    start, dev_id, val, cksum, end = data
    
    if start != 0xAA or end != 0x55:
        return False, f"Invalid markers: S:{hex(start)} E:{hex(end)}"
    
    expected_cksum = dev_id ^ val
    if cksum != expected_cksum:
        return False, f"Checksum failed: Received {hex(cksum)}, Expected {hex(expected_cksum)}"
    
    return True, f"ID: {dev_id} | Value: {val}"

# Apertura del file di log con buffering a 0 (flush immediato)
try:
    with open(LOG_FILE_PATH, 'w', encoding='utf-8') as log_file:
        log_file.write(f"[*] Listening on {SERIAL_PORT}...\n")
        log_file.flush()
        
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            while True:
                byte = ser.read(1)
                if byte == b'\xaa':
                    rest = ser.read(4)
                    full_packet = b'\xaa' + rest
                    
                    success, msg = validate_packet(full_packet)
                    if success:
                        log_file.write(f"[OK] Valid packet: {msg}\n")
                    else:
                        log_file.write(f"[ERR] {msg}\n")
                    
                    log_file.flush() # Forza la scrittura su disco immediata

except KeyboardInterrupt:
    # Gestione della chiusura pulita quando lo script viene terminato dal modulo bash
    sys.exit(0)

except Exception as e:
    # Se fallisce l'apertura del file o la seriale, tenta di scrivere l'errore fatale
    try:
        with open(LOG_FILE_PATH, 'a', encoding='utf-8') as log_file:
            log_file.write(f"[FATAL] Error: {e}\n")
    except Exception:
        print(f"[FATAL] Errore critico non loggabile su file: {e}", file=sys.stderr)
    sys.exit(1)