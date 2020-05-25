# raspbell
Send doorbell alerts using a Pi 4 and a sound sensor.

## Setup
This project uses a Pi 4 and [sound sensor](https://www.makershop.de/en/sensoren/sound/schall-sensor-modul/). Feel free to read about it in a more detailled [post](https://eggcpt.de/posts/raspbell/).

## Usage
Run the server on the Pi.

```
python server.py -port 8844 -pin 4
```

Run the client and connect to the host.
```
client 192.168.178.111 8844
```

## Building
Install the Python requirements.

```
pip install -r requirements.txt
```

### Windows
Install SDL2 and boost with [vcpkg](https://github.com/microsoft/vcpkg).

```
vcpkg install sdl2:x64-windows
vcpkg install boost:x64-windows
vcpkg integrate install
```

Build the Visual Studio solution.

### Linux
Install SDL2 and boost with the package manager.

```
sudo apt-get install libsdl2-dev
sudo apt-get install libboost-all-dev
```

Build with CMake.

```
mkdir client/build
cd client/build/
cmake ..
make
```

### macOS
Install SDL2 and boost with [homebrew](https://brew.sh/).

```
brew install sdl2
brew install boost
```

Build like on Linux.
