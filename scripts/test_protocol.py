import serial

SERIAL_PORT = '/tmp/ttyV1'
BAUD_RATE = 9600

def validate_packet(data):
    if len(data) != 5:
        return False, "Invalid packet length: Expected 5 bytes, got {len(data)}"
    
    start, dev_id, val, cksum, end = data
    
    if start != 0xAA or end != 0x55:
        return False, f"Invalid markers: S:{hex(start)} E:{hex(end)}"
    
    expected_cksum = dev_id ^ val
    if cksum != expected_cksum:
        return False, f"Checksum failed: Received {hex(cksum)}, Expected {hex(expected_cksum)}"
    
    return True, f"ID: {dev_id} | Value: {val}"

try:
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"[*] Listening on {SERIAL_PORT}...")
        while True:
            byte = ser.read(1)
            if byte == b'\xaa':
                rest = ser.read(4)
                full_packet = b'\xaa' + rest
                
                success, msg = validate_packet(full_packet)
                if success:
                    print(f"[OK] Valid packet: {msg}")
                else:
                    print(f"[ERR] {msg}")

except Exception as e:
    print(f"[FATAL] Error: {e}")