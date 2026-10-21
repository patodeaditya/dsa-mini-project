# SmartPark Web Application (C++ DSA Mini Project)

## Requirements
- Windows 10/11
- Visual Studio Code
- Git for Windows
- Emscripten SDK (emsdk)
- Google Chrome or another modern browser
- Python 3 (for a local HTTP server)

## Files
- `index.html`: web page structure
- `style.css`: responsive dashboard design
- `app.js`: browser event handling and WebAssembly connection
- `smartpark.cpp`: five data structures and parking logic
- `build.ps1`: compile C++ to WebAssembly

## Setup Emscripten
Open PowerShell and run:

```powershell
cd C:\
git clone https://github.com/emscripten-core/emsdk.git
cd C:\emsdk
.\emsdk install latest
.\emsdk activate latest
.\emsdk_env.ps1
```

If PowerShell blocks activation, follow the Windows activation instructions in:
https://emscripten.org/docs/getting_started/downloads.html

## Build
Open an Emscripten-enabled terminal, then run:

```powershell
cd C:\SmartParkWeb
.\build.ps1
```

If you created the folder elsewhere, change the path accordingly.

## Run
Open another terminal in the project folder:

```powershell
python -m http.server 8000
```

Open http://localhost:8000 in your browser. Keep the terminal/server running.

## Five data structures
1. Array (`std::array`): stores the 12 parking slots.
2. Queue (`std::queue`): stores waiting vehicles in FIFO order.
3. Stack (`std::stack`): stores exited vehicles in LIFO order.
4. Linked list (custom `HistoryNode`): stores parking activity history.
5. Hash table (`std::unordered_map`): maps active vehicle registration numbers to slot indexes.

## Notes
- The app is a local demo. Data is held in memory and resets when the page reloads.
- The current version calculates no parking fee and has no database/login.
- For a class submission, be ready to explain how each structure works and why it fits the task.
