# SafePath – Emergency Evacuation Route Simulator

A small graphical C project that demonstrates grid-based pathfinding for emergency evacuation.

## Features
- Interactive grid map
- Walls and danger zones
- Editable start and exit positions
- Breadth-First Search (BFS) shortest-path search
- Visual display of explored cells and the resulting route
- Route recalculation after map changes

## Requirements (Windows)
- MSYS2 UCRT64
- GCC
- raylib
- Visual Studio Code (recommended)

Install the compiler and raylib packages from the **MSYS2 UCRT64** terminal:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-pkgconf
```

Add `C:\msys64\ucrt64\bin` to your Windows user PATH, then restart VS Code.

## Build and run
Open this folder in VS Code and use **Terminal → New Terminal**:

```bash
gcc main.c -o safepath.exe -lraylib -lopengl32 -lgdi32 -lwinmm
.\safepath.exe
```

Alternatively, run `build.bat` from a terminal where GCC is available, then run `safepath.exe`.

## Controls
- `1` or WALL button: place a wall
- `2` or HAZARD button: place a danger zone
- `3` or START button: move the start point
- `4` or EXIT button: move the exit
- `5` or ERASE button: erase a cell
- Left-click: apply the selected tool
- Right-click: erase a cell
- `Enter` or FIND ROUTE: run BFS
- `R` or RESET MAP: reset the map

## How the algorithm works
The map is a 2D grid. Each cell is either open, a wall, a hazard, the start, or an exit.
BFS explores open neighboring cells in four directions using a queue. It records each
cell's parent so that the route can be reconstructed after the exit is reached.

Because every grid move has equal cost, BFS finds a route with the fewest grid steps.
Walls and hazards are treated as blocked cells. The displayed distance assumes each cell
represents 2 metres, and the walking-time estimate assumes a constant speed of 1.2 m/s.
These are demonstration assumptions, not real-world measurements.

## Scope and limitations
This is an educational simulation, not a certified emergency guidance system. It assumes
a flat grid, four-direction movement, equal movement cost, and static hazards. Real
evacuation planning requires validated building plans, accessibility considerations,
dynamic hazard modelling, crowd behaviour, and professional safety review.
