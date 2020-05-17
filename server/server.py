import asyncio
import websockets


connected = set()


async def register(websocket, path):
    connected.add(websocket)
    try:
        while True:
            await asyncio.sleep(10)
            await websocket.send("test")
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        connected.remove(websocket)


server = websockets.serve(register, "localhost", 8765)

asyncio.get_event_loop().run_until_complete(server)
asyncio.get_event_loop().run_forever()
