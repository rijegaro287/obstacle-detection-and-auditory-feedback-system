import asyncio
from bleak import BleakClient, BleakScanner

# Replace with your server's MAC address or UUID
DEVICE_ADDRESS = "XX:XX:XX:XX:XX:XX"
CHARACTERISTIC_UUID = "87654321-4321-4321-4321-abcdefabcdef"

async def connect_and_communicate():
    # Scan for devices
    devices = await BleakScanner.discover()
    for device in devices:
        print(f"{device.address} \t{device.name}")
        # async with BleakClient(device) as client:
        #     print(f"Connected: {client.is_connected}")
            
        #     # Read characteristic
        #     value = await client.read_gatt_char(CHARACTERISTIC_UUID)
        #     print(f"Read value: {value}")
            
        #     # Write to characteristic
        #     await client.write_gatt_char(CHARACTERISTIC_UUID, b"Hello BLE")
        #     print("Wrote 'Hello BLE' to characteristic")
            
        #     # Start receiving notifications
        #     def notification_handler(sender, data):
        #         print(f"Notification from {sender}: {data}")
            
        #     await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        #     await asyncio.sleep(30)  # Keep receiving notifications for 30 seconds
        #     await client.stop_notify(CHARACTERISTIC_UUID)
        #     print("Stopped notifications")


if __name__ == "__main__":
    asyncio.run(connect_and_communicate())
