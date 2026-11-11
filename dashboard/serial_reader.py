import serial
from protocol import parse_status

PORT = "COM9"
BAUD_RATE = 115200

with serial.Serial(PORT, BAUD_RATE, timeout=1) as stm32:
    print(f"Connected to STM32 on {PORT}")
    
    while True:
        data = stm32.readline()
        
        if data:
            message = data.decode("utf-8", errors= "replace").strip()
            
            if message == "STM32 READY" :
                print("STM32 initialized successfully")
                continue
            status = parse_status(message)
            
            if status is not None:
                print(status)
            else:
                print(f"Unkown message:{message}")