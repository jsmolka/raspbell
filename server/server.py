import asyncio
import socket
import websockets


connected = set()


async def query():
    while True:
        await asyncio.sleep(0.1)


async def register(websocket, path):
    connected.add(websocket)
    try:
        print(f"connect {websocket.local_address}")
        while websocket.open:
            await asyncio.sleep(0.1)
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        print(f"disconnect {websocket.local_address}")
        connected.remove(websocket)


host = socket.gethostbyname(socket.gethostname())
port = 8844
server = websockets.serve(register, host, port)

asyncio.get_event_loop().run_until_complete(server)
asyncio.get_event_loop().run_until_complete(query())
asyncio.get_event_loop().run_forever()
