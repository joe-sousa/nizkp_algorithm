import asyncio
from bleak import BleakClient, BleakScanner
from Crypto.Cipher import AES
import base64

# Target Device Name
target_name = "MeuNovoNome"
characteristic_uuid = "0000ffe1-0000-1000-8000-00805f9b34fb"  # UUID of the BLE module characteristic

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
            await asyncio.sleep(6.0)

            # Process the received data
            received_data = delegate.notification_data
            if received_data:
                received_str = received_data
                cipher_time = received_str[:5].decode('utf-8', errors='ignore')
                print(f"Time to cipher: {cipher_time}")
                
                # Convert received data from bytes to string using utf-8 encoding
                received_str = received_data.decode('utf-8', errors='ignore')
                print(f"Notification stored: {received_str}")
                cipher_received = received_str[10:42]
                #cipher_received = '7A7AEE056D0AAD5A3239093032564099'
                msg_received = received_str[42:].split(';')[0].strip()
                
                # Verifica se há um ponto e vírgula na string
                if ';' in received_str:
                    # Divide a string com base no ponto e vírgula e remove os caracteres de retorno de carro e nova linha
                    pack_sent_time = received_str.split(';')[1].strip()
                else:
                    pack_sent_time = ""

                # Exibe o valor extraído
                print("pack_sent_time:", pack_sent_time)


                print(f"Cifra recebida foi: {cipher_received}")
                print(f"Mensagem recebida foi: {msg_received}")
                
                
                # Converta o valor hexadecimal em string
                decoded_message = bytes.fromhex(msg_received).decode('utf-8', errors='ignore')
                print(f"Mensagem recebida em string: {decoded_message}")
                
                # Example: Decrypting the received ciphertext
                key = bytes.fromhex('02000500AC26BF9605F82F63C9D0EEBCA36DD7971F7F1D56E8E1B7D4EA4F8DB9')
                iv = bytes.fromhex('00000000000000000000000000000000')  # IV should be 16 bytes
                cipher = AES.new(key, AES.MODE_CBC, iv)
                decrypted_message = cipher.decrypt(bytes.fromhex(cipher_received))
                clear_decrypted_message = decrypted_message.decode('utf-8', errors='ignore').rstrip()
                print(f"Decrypted message: {clear_decrypted_message}")
                
                
                if(decoded_message == clear_decrypted_message):
                    print("Dispositivos autenticados")
                else:
                    print("Dispositivos não autenticados")

            await client.stop_notify(characteristic_uuid)

    except Exception as e:
        print(f"Error connecting or communicating with the device: {e}")

if __name__ == "__main__":
    asyncio.run(main())
