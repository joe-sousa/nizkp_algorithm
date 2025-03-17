import asyncio
from bleak import BleakClient, BleakScanner
import hmac
import hashlib

# Target Device Name
target_name = "MeuNovoNome"
characteristic_uuid = "0000ffe1-0000-1000-8000-00805f9b34fb"  # UUID of the BLE module characteristic

# This function generates an HMAC using SHA256
def generate_hmac_sha256(secret_key, message):
   # Check if the key is a hexadecimal string and convert to bytes
    if isinstance(secret_key, str):
        secret_key = bytes.fromhex(secret_key)
        
    # Check if the message is a string and convert to bytes if necessary
    if isinstance(message, str):
        message = message.encode()
    
    # Create a new HMAC object using the secret key and the SHA256 hash function
    hmac_object = hmac.new(secret_key, message, hashlib.sha256)
    
    # Return the hexadecimal representation of the HMAC
    return hmac_object.hexdigest()


class MyDelegate:
    def __init__(self):
        self.notification_data = b""
        self.hmac = None 
        self.message = None
        
    # Function to handle BLE notifications
    def handle_notification(self, characteristic, data):
        self.notification_data += data
        print(f"Notification received: {data}")
        if data.decode('utf-8', errors='ignore'):
            print(f"Notification as string: {data.decode('utf-8', errors='ignore')}")
        else:
            print("Notification as string is empty")

# Asynchronous function to scan for available devices and connect to the desired device  
async def scan_for_device(name):
    devices = await BleakScanner.discover()
    for device in devices:
        if device.name and name in device.name:
            print(f"Device found: {device.name} ({device.address})")
            return device.address
    return None

async def main():
    device_address = await scan_for_device(target_name)
    if not device_address:
        print("Device not found")
        return

    try:
        async with BleakClient(device_address) as client:
            print(f"Conecting to the device {device_address}...")

            delegate = MyDelegate()

            async def notification_handler(characteristic, data):
                delegate.handle_notification(characteristic, data)

            await client.start_notify(characteristic_uuid, notification_handler)

            print(f"Notifications enabled for the characteristic: {characteristic_uuid}")

            # Wait until all notifications are received. Must receive both the hash and the message
            await asyncio.sleep(7.0)

            # Process the received data
            received_data = delegate.notification_data
            if received_data:
                received_str = received_data
                
                # Convert received data from bytes to string using utf-8 encoding
                received_str = received_data.decode('utf-8', errors='ignore')
                print(f"Notification stored: {received_str}")
                
                
                # Check for the delimiter ';'
                if ';' in received_str:
                    time_hmac = received_str[:4]
                    print(f"Time to generate hmac:\n{time_hmac} seconds")
                    received_str = received_str[4:]
                    print(received_str)
                    # Check if the received string starts with 'PA'
                    if received_str.startswith("PA"):
                        # The HMAC is the first 32 bytes after 'PA'
                        hmac_str = received_str[2:66]  # Skip the first 2 characters 'PA', then 32 bytes/256 bits (64 characters in HEX)
                        
                        # The message is the remainder up to the delimiter ';'
                        message_str = received_str[66:received_str.index(';')].rstrip(';\r\n')
                        
                        time_packet = received_str[received_str.index(';') + 1:]
                        # Limitar a string aos primeiros seis caracteres
                        time_packet_limited = time_packet[:8]
                        print(f"Time to build and send packet: {time_packet_limited} seconds")
                        
                        # Convert the message from hexadecimal to bytes
                        message_bytes = bytes.fromhex(message_str)

                        # Convert the bytes back to string
                        cleaned_message_str = message_bytes.decode('utf-8', errors='ignore').strip()

                        # Display the original message received from arduino
                        print(f"Notification stored: {cleaned_message_str}")
                        
                        # Encode the HMAC string to bytes using utf-8 encoding
                        hmac = hmac_str.encode('utf-8')
                        
                        # Print the received HMAC by decoding the bytes back to a string
                        print(f"HMAC stored: {hmac.decode('utf-8', errors='ignore')}")
                        
                        # Key to be used to generate the HMAC
                        secret_key = "000500AC26BF9605F82F63C9D0EEBCA36DD7971F7F1D56E8E1B7D4EA4F8DB9C3"
                        
                        # Generate HMAC-SHA256 signature using the secret key and the cleaned message string
                        hmac_signature_generated = generate_hmac_sha256(secret_key, cleaned_message_str)
                        
                        print(f"HMAC stored: {hmac.decode('utf-8', errors='ignore')}")
                        print(f"HMAC-SHA256 generated: {hmac_signature_generated.upper()}")
                        
                        if hmac_str == hmac_signature_generated.upper():
                            print("Matching HMACs, devices authenticated.")
                        else:
                            print("Non-matching HMACs, devices not authenticated.")
                    else:
                        print(f"Invalid data received (does not start with 'PA'): {received_str}")
                else:
                    print(f"Incomplete data received: {received_str}")

            await client.stop_notify(characteristic_uuid)

    except Exception as e:
        print(f"Error connecting or communicating with the device: {e}")

if __name__ == "__main__":
    asyncio.run(main())
