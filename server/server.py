import asyncio
import RPi.GPIO as GPIO
import socket
import websockets

connected = set()


def alert(channel):
    for websocket in connected:
        asyncio.run(websocket.send("bell"))


async def register(websocket, path):
    connected.add(websocket)
    try:
        print(f"client connected {websocket.remote_address[0]}:{websocket.remote_address[1]}")
        while websocket.open:
            await asyncio.sleep(0.1)
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        print(f"client disconnected {websocket.remote_address[0]}:{websocket.remote_address[1]}")
        connected.remove(websocket)


def get_host():
    # https://stackoverflow.com/a/166589/7057528
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    host = s.getsockname()[0]
    s.close()

    return host


def main():
    host = get_host()
    port = 8844
    print(f"starting server {host}:{port}")
    server = websockets.serve(register, host, port)

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(4, GPIO.IN)
    GPIO.add_event_detect(4, GPIO.FALLING, callback=alert, bouncetime=5000)

    asyncio.get_event_loop().run_until_complete(server)
    asyncio.get_event_loop().run_forever()


if __name__ == "__main__":
    main()
