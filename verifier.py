import asyncio
import re
from bleak import BleakClient, BleakScanner
from ecdsa import SigningKey, SECP256k1, VerifyingKey, ellipticcurve, curves
from ecdsa.ellipticcurve import Point
import hashlib
import ecdsa
import os

# Target device name
target_name = "MeuNovoNome"

# UUID of the BLE module characteristic
characteristic_uuid = "0000ffe1-0000-1000-8000-00805f9b34fb"


# Parameters of the SECP256k1 curve
curve = curves.SECP256k1.curve  # Curve parameters
generator = curves.SECP256k1.generator  # Generator point of the curve

# Get the parameter p (order of the finite field) from the elliptic curve
p = curve.p()

class MyDelegate:
    def __init__(self):
        # Initialize notification data as an empty byte string
        self.notification_data = b""

    def handle_notification(self, characteristic, data):
        # Accumulate the received notification data
        self.notification_data += data
        
        print(f"Notification received: {data}")
        
        #If the device is not registered, the response is "RA"; if already registered, the response is "R1"
        print(f"Notification as string: {data.decode('utf-8', errors='ignore')}")

async def scan_for_device(name):
    devices = await BleakScanner.discover()
    for device in devices:
        # Ensure device.name is not None before checking
        if device.name and name in device.name:
            print(f"Device found: {device.name} ({device.address})")
            return device.address
    return None

# Function that waits for the complete reception of the public key 
# # It checks for notifications and looks for the key end delimiter (';')
async def wait_for_public_key(delegate, timeout=5.0):
    while True:
        await asyncio.sleep(timeout)  # Wait for up to timeout seconds for notifications
        if delegate.notification_data:
            # Check if the received notification contains the key end delimiter (e.g., ';')
            if b';' in delegate.notification_data:
                print("End of public key detected.")
                break

    # Remove any extra end-of-line characters if necessary
    public_key = delegate.notification_data.replace(b'\r', b'').replace(b'\n', b'').strip()
    return public_key

# This function filters out the message portion and returns the cleaned public key data
def filter_public_key_data(public_key):
    if b';' in public_key:
        filtered_data = public_key[:35].strip()
        print(f"Stored notification (Key generation time): {filtered_data.decode('utf-8', errors='ignore')}")
        public_key = public_key[37:public_key.rfind(b';')].strip()
    else:
        print("Error: Delimiter ';' not found in received data.")
        public_key = b""
    return public_key

# Function to process the public key and return X and Y coordinates as integers  
def process_public_key(public_key):
    # Check length to ensure the public key has 64 bytes or 128 characters
    if len(public_key) != 128:
        print("Error: Public key does not have the expected length of 64 bytes.")
        return None, None
    else:
        # Split the public key into X and Y coordinates (64 bytes each)
        Qd_x_hex = public_key[:64]
        Qd_y_hex = public_key[64:]

        global_state.x_public_key = Qd_x_hex.decode('utf-8')
        # Print the X coordinate in hexadecimal
        print(f"Hexadecimal representation of the client's public key X coordinate: {global_state.x_public_key}")

        # Convert the X coordinate from hexadecimal to bytes
        Qd_x_bytes = bytes.fromhex(Qd_x_hex.decode('utf-8'))  # Convert from hex string to bytes
        Qd_y_bytes = bytes.fromhex(Qd_y_hex.decode('utf-8'))  # Convert from hex string to bytes

        # Convert to integers to be used in Shared Point.
        global_state.Qd_x_int = int.from_bytes(Qd_x_bytes, byteorder='big')
        global_state.Qd_y_int = int.from_bytes(Qd_y_bytes, byteorder='big')
        print(f"X and Y coordinates as integers respectively: {global_state.Qd_x_int} {global_state.Qd_y_int}")

        return global_state.Qd_x_int, global_state.Qd_y_int

# Function responsible for returning the X coordinate of point G.
def get_point_G_x_coordinate(generator):
    global_state.x_g1_hex = hex(generator.x())[2:].upper()  # Remove '0x' and convert to uppercase
    print(f"X coordinate value of point G for testing (pure hex): {global_state.x_g1_hex}")
    return global_state.x_g1_hex

from ecdsa import SigningKey, SECP256k1

# Function to generate the server's keys and return only the public key in bytes
def generate_server_public_key():
    # Generate the server's private key using ECC
    private_key = SigningKey.generate(curve=SECP256k1)
    
    # Generate the server's public key
    server_public_key = private_key.get_verifying_key()
    
    # Save the server's public key as bytes
    pub_key_bytes = server_public_key.to_string()
        
    return pub_key_bytes

# Extracts the receipt time and cleans the prover's packet, returning it as a hexadecimal
def convert_packet_to_string(packet):
    # Convert the packet to a string and remove "PA" and ";"
    packet_str = packet.decode('utf-8', errors='ignore').strip()
    
    # Extract the assembly and split the time from the packet
    time_pack = packet_str[-5:]
    print(f"Time to receive the packet from the prover: {time_pack}")
    
    # Find the position of ';'
    semicolon_pos = packet_str.find(';')
    
    # If there's a ';' in the string
    if semicolon_pos != -1:
        # Keep everything up to and including ';'
        packet_str = packet_str[:semicolon_pos + 1]
    else:
        print("The packet does not contain ';'.")
    
    if packet_str.startswith("PA") and packet_str.endswith(";"):
        packet_str = packet_str[2:-1]  # Remove "PA" and ";"

    # Use a regular expression to remove non-hexadecimal characters
    packet_str = re.sub(r'[^0-9A-Fa-f]', '', packet_str)

    return packet_str

# Function that converts the packet from hexadecimal to bytes and checks if it has the correct size
def validate_and_convert_to_bytes(packet_str):
    # Check if the packet is in a valid hexadecimal format
    if not all(c in '0123456789ABCDEFabcdef' for c in packet_str):
        print("Error: The packet contains non-hexadecimal characters.")
        return None

    # Convert the hexadecimal string to bytes
    try:
        packet_bytes = bytes.fromhex(packet_str)
    except ValueError:
        print("Error: The packet is not in a valid hexadecimal format.")
        return None

    # Check if the packet size is correct
    if len(packet_bytes) != 113:
        print(f"Error: Incorrect packet size: {len(packet_bytes)} bytes")
        return None

    return packet_bytes

def extract_and_process_packet_components(packet_bytes):
    # Extract the parts of the packet
    shared_point = packet_bytes[0:64]
    challenge_response = packet_bytes[64:96]
    device_id = packet_bytes[96:97]
    plaintext_message = packet_bytes[97:113]
    
    # Divide into 32 bytes for x and y coordinates of the shared point
    x_bytes = shared_point[:32]  # First 32 bytes
    y_bytes = shared_point[32:]  # Last 32 bytes
    
    # Print the X coordinate of point G in hexadecimal - commit value
    print(f"Shared point X coordinate in hexadecimal for testing (Commit): {x_bytes.hex().upper()}")

    # Convert parts to integers for calculations
    shared_point_x_int = int(x_bytes.hex(), 16)
    shared_point_y_int = int(y_bytes.hex(), 16)
    challenge_response_int = int(challenge_response.hex(), 16)
    device_id_int = int(device_id.hex(), 16)
    plaintext_message_str = plaintext_message.decode('utf-8')
    
    print(f"Printing the hash value on the verifier side: ")
    hash_int, x_hex = H(str(global_state.x_g1_hex), str(global_state.x_public_key), str(x_bytes.hex().upper()))
    x_hex = x_hex.upper()
    print(hash_int, "   ", x_hex)
        
    # Client's public coordinates (Qd)
    Qd_x = global_state.Qd_x_int
    Qd_y = global_state.Qd_y_int
    Qd = ellipticcurve.Point(curve, Qd_x, Qd_y)
    
    # Call the function to calculate P
    calculate_P(challenge_response_int, hash_int, generator, Qd, curve, p, shared_point_x_int, shared_point_y_int)
    
    # Print the parts of the packet
    print("Shared point (hex):", shared_point.hex())
    print("Challenge response (hex):", challenge_response.hex())
    print("Device ID (hex):", device_id.hex())
    print("Plaintext message:", plaintext_message_str)

    # Return processed data or whatever is needed
    return {
        "shared_point_x": shared_point_x_int,
        "shared_point_y": shared_point_y_int,
        "challenge_response": challenge_response_int,
        "device_id": device_id_int,
        "plaintext_message": plaintext_message_str
    }
    
def parse_received_packet(packet):
    # Convert the packet to a string
    packet_str = convert_packet_to_string(packet)
    
    # Validate and convert to bytes
    packet_bytes = validate_and_convert_to_bytes(packet_str)
    
    if packet_bytes is None:
        return None
    
    # Extract and process the components
    return extract_and_process_packet_components(packet_bytes)


# This function will be responsible for calculating the hash on the verifier's side
def H(*args):
    concatenated = "".join(args)
    hash_hex = hashlib.sha256(concatenated.encode('utf-8')).hexdigest()
    return int(hash_hex, 16), hash_hex

# This function will be responsible for calculating the shared point on the verifier's side
def calculate_P(challenge_response_int, hash_int, generator, Qd, curve, p, shared_point_x_int, shared_point_y_int):
    
    # 1. Calculate πG
    piG = generator * challenge_response_int

    # 2. Calculate σQd
    sigmaQd = Qd * hash_int
    
    # 3. Invert σQd
    sigmaQd_neg = Point(curve, sigmaQd.x(), (-sigmaQd.y()) % int(p))

    # 4. Add πG and -σQd
    P = piG + sigmaQd_neg
    
    # Print the coordinates x and y of P
    Px = P.x()
    Py = P.y()
    
    # Compare the values
    if Px == shared_point_x_int and Py == shared_point_y_int:
        print("Device authenticated correctly.")
    else:
        print("Device not authenticated.")

    print(f"Coordinate X of P: {Px}")
    print(f"Coordinate Y of P: {Py}")

    return P

# Class to encapsulate variables that need to be accessed globally
# This approach helps in organizing and managing the state more effectively
class GlobalState:
    def __init__(self):
        self.x_g1_hex = None  # Holds the X coordinate value of point G
        self.Qd_x_int = None      # Holds the integer value of Qd X coordinate
        self.Qd_y_int = None      # Holds the integer value of Qd Y coordinate
        self.x_public_key = None  # Holds the public key's X coordinate as a string

global_state = GlobalState()

async def main(global_state):
    device_address = await scan_for_device(target_name)
    if device_address:
        try:
            async with BleakClient(device_address) as client:
                # Device Connection
                print("# Device Connection")
                print(f"Connecting to the device {device_address}...\n")

                # Create an instance of MyDelegate, which will be used to handle notifications
                delegate = MyDelegate()

                # Add the callback function for notification
                async def notification_handler(characteristic, data):
                    delegate.handle_notification(characteristic, data)

                # Register the characteristic for notifications
                await client.start_notify(characteristic_uuid, notification_handler)

                # Send the value 'R' to the characteristic
                await client.write_gatt_char(characteristic_uuid, b'R')
                print("Message sent: R\n")

                # Wait for notifications
                await asyncio.sleep(2.0)  # Wait for up to 2 seconds for notification
                received_data1 = delegate.notification_data
                if received_data1:
                    print(f"Stored notification: {received_data1.decode('utf-8', errors='ignore')}\n")
                                         
                # Reset the notification buffer before sending the next command
                delegate.notification_data = b""
                
                # Send the device ID along with the letter "I" to the BLE and Arduino device
                await client.write_gatt_char(characteristic_uuid, b'I10')
                print("Message sent: I + 10\n")

                # Variable that holds the fully received public key after waiting for all parts to be received.
                public_key = await wait_for_public_key(delegate)

                # Store the cleaned public key data after filtering
                public_key = filter_public_key_data(public_key)
                
                # Retrieve the X coordinate of point G and store it in the global state
                global_state.x_coordinate = get_point_G_x_coordinate(generator)


                # Check if the public key was initialized correctly
                if public_key:
                    # Process the public key and update the global state
                    global_state.Qd_x_int, global_state.Qd_y_int = process_public_key(public_key)
                else:
                    print("Failed to process the public key.\n")

                server_public_key_bytes = generate_server_public_key()

                # Adding the character 'K' to the beginning of the public key for sending
                data_to_send = b'K' + server_public_key_bytes
                
                # Redefinir o buffer de notificação antes de enviar a chave pública
                delegate.notification_data = b""
                
                # Enviar a chave pública para o dispositivo BLE
                await client.write_gatt_char(characteristic_uuid, data_to_send)
                
                print("K + Verifier's public key sent to the Prover.\n")
                
                # Aguardando notificação
                await asyncio.sleep(5.0)  # Espera por até 5 segundos por notificações
                received_data2 = delegate.notification_data
                if received_data2:
                    print(f"Notificação armazenada: {received_data2.decode('utf-8', errors='ignore')}\n")
                
                # Sends the value 'D' to the characteristic
                await client.write_gatt_char(characteristic_uuid, b'D')
                print("Message sent: D\n")
                
                # Reset the notification buffer before waiting for the last notification
                delegate.notification_data = b""
                      
                # Waiting for notifications
                await asyncio.sleep(11.0) 
                received_data3 = delegate.notification_data
                if received_data3:
                    print(f"Stored notification: {received_data3.decode('utf-8', errors='ignore')}\n")
                        
                    # Process the received packet
                    processed_data = parse_received_packet(received_data3)
                    if processed_data:
                        print(f"Final processed packet: {processed_data}\n")
                    else:
                        print("Error processing the packet.\n")
                     
                    
                # Break notifications
                await client.stop_notify(characteristic_uuid)
                
        except Exception as e:
            print(f"Erro ao conectar ou comunicar com o dispositivo: {e}")
    else:
        print("Dispositivo não encontrado.")
        
if __name__ == "__main__":
    asyncio.run(main(global_state))