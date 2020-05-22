import argparse
import asyncio
import RPi.GPIO as GPIO
import socket
import websockets
from datetime import datetime

connected = set()


def alert(channel):
    print(datetime.now(), "alert")
    for websocket in connected:
        asyncio.run(websocket.send("bell"))


async def register(websocket, path):
    connected.add(websocket)
    try:
        print(datetime.now(), f"client connected {websocket.remote_address[0]}:{websocket.remote_address[1]}")
        while websocket.open:
            await asyncio.sleep(0.1)
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        print(datetime.now(), f"client disconnected {websocket.remote_address[0]}:{websocket.remote_address[1]}")
        connected.remove(websocket)


def get_host():
    # https://stackoverflow.com/a/166589/7057528
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    host = s.getsockname()[0]
    s.close()

    return host


def main(args):
    host = get_host()
    port = args.port
    server = websockets.serve(register, host, port)
    print(datetime.now(), f"starting server {host}:{port}")

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(args.pin, GPIO.IN)
    GPIO.add_event_detect(args.pin, GPIO.FALLING, callback=alert, bouncetime=5000)

    asyncio.get_event_loop().run_until_complete(server)
    asyncio.get_event_loop().run_forever()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-port", dest="port", type=int, required=True)
    parser.add_argument("-pin", dest="pin", type=int, required=True)

    main(parser.parse_args())
