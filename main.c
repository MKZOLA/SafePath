#include "raylib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define COLS 20
#define ROWS 11
#define CELL 40
#define GRID_X 32
#define GRID_Y 112
#define SCREEN_W 1120
#define SCREEN_H 700
#define MAX_CELLS (COLS * ROWS)

typedef enum {
    EMPTY = 0,
    WALL,
    HAZARD,
    START,
    EXIT_CELL
} CellType;

typedef enum {
    TOOL_WALL = 0,
    TOOL_HAZARD,
    TOOL_START,
    TOOL_EXIT,
    TOOL_ERASE
} Tool;

static CellType map[ROWS][COLS];
static bool visited[ROWS][COLS];
static int parentCell[ROWS][COLS];
static int pathX[MAX_CELLS], pathY[MAX_CELLS];
static int pathLength = 0;
static int startX = 1, startY = 1;
static int exitX = COLS - 2, exitY = ROWS - 2;
static bool hasStart = true, hasExit = true;
static bool routeFound = false;
static int exploredCount = 0;
static Tool currentTool = TOOL_WALL;
static char statusText[160] = "Place walls or hazards, then press FIND ROUTE.";
static const int dx[4] = {1, -1, 0, 0};
static const int dy[4] = {0, 0, 1, -1};

static Color cellColor(CellType type) {
    switch (type) {
        case WALL: return (Color){76, 86, 106, 255};
        case HAZARD: return (Color){210, 65, 65, 255};
        case START: return (Color){55, 145, 235, 255};
        case EXIT_CELL: return (Color){45, 170, 105, 255};
        default: return (Color){34, 42, 56, 255};
    }
}

static void clearRoute(void) {
    memset(visited, 0, sizeof(visited));
    pathLength = 0;
    exploredCount = 0;
    routeFound = false;
}

static bool inside(int x, int y) {
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

static void setStatus(const char *message) {
    snprintf(statusText, sizeof(statusText), "%s", message);
}

static void initializeMap(void) {
    memset(map, EMPTY, sizeof(map));
    startX = 1; startY = 1;
    exitX = COLS - 2; exitY = ROWS - 2;
    hasStart = hasExit = true;
    map[startY][startX] = START;
    map[exitY][exitX] = EXIT_CELL;
    clearRoute();
}

static void placeCell(int x, int y) {
    if (!inside(x, y)) return;

    if (currentTool == TOOL_START) {
        if (hasStart) map[startY][startX] = EMPTY;
        startX = x; startY = y; hasStart = true;
        map[y][x] = START;
    } else if (currentTool == TOOL_EXIT) {
        if (hasExit) map[exitY][exitX] = EMPTY;
        exitX = x; exitY = y; hasExit = true;
        map[y][x] = EXIT_CELL;
    } else {
        if (map[y][x] == START || map[y][x] == EXIT_CELL) return;
        if (currentTool == TOOL_WALL) map[y][x] = WALL;
        if (currentTool == TOOL_HAZARD) map[y][x] = HAZARD;
        if (currentTool == TOOL_ERASE) map[y][x] = EMPTY;
    }
    clearRoute();
}

static void findRoute(void) {
    if (!hasStart || !hasExit) {
        setStatus("Set both a START point and an EXIT first.");
        return;
    }

    clearRoute();

    int qx[MAX_CELLS], qy[MAX_CELLS];
    int head = 0, tail = 0;

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            parentCell[y][x] = -1;

    qx[tail] = startX;
    qy[tail++] = startY;
    visited[startY][startX] = true;

    while (head < tail) {
        int x = qx[head];
        int y = qy[head++];
        exploredCount++;

        if (x == exitX && y == exitY) break;

        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];

            if (!inside(nx, ny) || visited[ny][nx]) continue;
            if (map[ny][nx] == WALL || map[ny][nx] == HAZARD) continue;

            visited[ny][nx] = true;
            parentCell[ny][nx] = y * COLS + x;
            qx[tail] = nx;
            qy[tail++] = ny;
        }
    }

    if (!visited[exitY][exitX]) {
        routeFound = false;
        setStatus("No safe route found. Remove an obstacle and try again.");
        return;
    }

    int cx = exitX, cy = exitY;
    while (!(cx == startX && cy == startY) && pathLength < MAX_CELLS) {
        pathX[pathLength] = cx;
        pathY[pathLength] = cy;
        pathLength++;

        int p = parentCell[cy][cx];
        if (p < 0) break;
        cx = p % COLS;
        cy = p / COLS;
    }

    routeFound = true;
    float distanceMeters = pathLength * 2.0f; /* Assumption: each grid cell is 2 metres. */
    float walkingSeconds = distanceMeters / 1.2f; /* Assumption: 1.2 m/s walking speed. */
    snprintf(statusText, sizeof(statusText),
             "Route: %d steps | %.0f m | approx %.0f sec | %d cells explored.",
             pathLength, distanceMeters, walkingSeconds, exploredCount);
}

static void drawButton(Rectangle r, const char *label, bool selected) {
    Color fill = selected ? (Color){46, 105, 160, 255} : (Color){48, 58, 74, 255};
    DrawRectangleRounded(r, 0.18f, 6, fill);
    DrawRectangleRoundedLines(r, 0.18f, 6, (Color){92, 108, 130, 255});
    int tw = MeasureText(label, 17);
    DrawText(label, (int)(r.x + (r.width - tw) / 2), (int)(r.y + 10), 17, RAYWHITE);
}

static void drawInterface(void) {
    ClearBackground((Color){17, 23, 34, 255});

    DrawText("SAFEPATH", 32, 24, 30, RAYWHITE);
    DrawText("EMERGENCY EVACUATION ROUTE SIMULATOR", 32, 61, 14,
             (Color){150, 170, 195, 255});
    DrawLine(32, 91, SCREEN_W - 32, 91, (Color){58, 70, 88, 255});

    const char *toolNames[] = {"WALL", "HAZARD", "START", "EXIT", "ERASE"};
    Rectangle buttons[] = {
        {32, 630, 100, 38}, {140, 630, 112, 38}, {260, 630, 100, 38},
        {368, 630, 100, 38}, {476, 630, 100, 38}
    };
    for (int i = 0; i < 5; i++)
        drawButton(buttons[i], toolNames[i], currentTool == (Tool)i);

    Rectangle findBtn = {700, 630, 150, 38};
    Rectangle resetBtn = {862, 630, 110, 38};
    drawButton(findBtn, "FIND ROUTE", false);
    drawButton(resetBtn, "RESET MAP", false);

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int px = GRID_X + x * CELL;
            int py = GRID_Y + y * CELL;
            Rectangle r = {(float)px, (float)py, CELL - 2, CELL - 2};

            DrawRectangleRec(r, cellColor(map[y][x]));

            if (visited[y][x] && map[y][x] == EMPTY)
                DrawRectangleRec(r, (Color){64, 103, 132, 170});
        }
    }

    if (routeFound) {
        for (int i = 0; i < pathLength; i++) {
            int px = GRID_X + pathX[i] * CELL;
            int py = GRID_Y + pathY[i] * CELL;
            DrawRectangle(px + 5, py + 5, CELL - 12, CELL - 12,
                          (Color){70, 210, 145, 255});
        }
    }

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int px = GRID_X + x * CELL;
            int py = GRID_Y + y * CELL;
            if (map[y][x] == START)
                DrawText("S", px + 12, py + 7, 22, RAYWHITE);
            else if (map[y][x] == EXIT_CELL)
                DrawText("E", px + 12, py + 7, 22, RAYWHITE);
        }
    }

    int panelX = 860;
    DrawText("LEGEND", panelX, 125, 18, RAYWHITE);
    DrawRectangle(panelX, 158, 18, 18, cellColor(START));
    DrawText("Start point", panelX + 28, 158, 16, LIGHTGRAY);
    DrawRectangle(panelX, 188, 18, 18, cellColor(EXIT_CELL));
    DrawText("Emergency exit", panelX + 28, 188, 16, LIGHTGRAY);
    DrawRectangle(panelX, 218, 18, 18, cellColor(WALL));
    DrawText("Wall / blocked", panelX + 28, 218, 16, LIGHTGRAY);
    DrawRectangle(panelX, 248, 18, 18, cellColor(HAZARD));
    DrawText("Danger zone", panelX + 28, 248, 16, LIGHTGRAY);
    DrawRectangle(panelX, 278, 18, 18, (Color){70, 210, 145, 255});
    DrawText("Safe route", panelX + 28, 278, 16, LIGHTGRAY);
    DrawRectangle(panelX, 308, 18, 18, (Color){64, 103, 132, 255});
    DrawText("Explored cell", panelX + 28, 308, 16, LIGHTGRAY);

    DrawText("HOW TO USE", panelX, 365, 18, RAYWHITE);
    DrawText("1. Select a tool below.", panelX, 395, 15, LIGHTGRAY);
    DrawText("2. Click cells to edit the map.", panelX, 421, 15, LIGHTGRAY);
    DrawText("3. Press FIND ROUTE.", panelX, 447, 15, LIGHTGRAY);
    DrawText("4. Green cells show the route.", panelX, 473, 15, LIGHTGRAY);
    DrawText("Hazards and walls cannot be crossed.", panelX, 510, 14,
             (Color){235, 170, 100, 255});

    DrawText(statusText, 32, 590, 16, (Color){190, 205, 220, 255});
    DrawText("Mouse: place tool | Right-click: erase | R: reset | ENTER: find route",
             32, 682, 13, (Color){125, 145, 170, 255});
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "SafePath - Emergency Evacuation Simulator");
    SetTargetFPS(60);
    initializeMap();

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        if (IsKeyPressed(KEY_ONE)) currentTool = TOOL_WALL;
        if (IsKeyPressed(KEY_TWO)) currentTool = TOOL_HAZARD;
        if (IsKeyPressed(KEY_THREE)) currentTool = TOOL_START;
        if (IsKeyPressed(KEY_FOUR)) currentTool = TOOL_EXIT;
        if (IsKeyPressed(KEY_FIVE)) currentTool = TOOL_ERASE;
        if (IsKeyPressed(KEY_R)) {
            initializeMap();
            setStatus("Map reset to its default layout.");
        }
        if (IsKeyPressed(KEY_ENTER)) findRoute();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < 5; i++) {
                float offsets[5] = {0, 108, 228, 336, 444};
                float widths[5] = {100, 112, 100, 100, 100};
                Rectangle b = {32 + offsets[i], 630, widths[i], 38};
                if (CheckCollisionPointRec(mouse, b)) currentTool = (Tool)i;
            }
            if (CheckCollisionPointRec(mouse, (Rectangle){700,630,150,38}))
                findRoute();
            if (CheckCollisionPointRec(mouse, (Rectangle){862,630,110,38})) {
                initializeMap();
                setStatus("Map reset to its default layout.");
            }

            int gx = (int)(mouse.x - GRID_X) / CELL;
            int gy = (int)(mouse.y - GRID_Y) / CELL;
            if (mouse.x >= GRID_X && mouse.y >= GRID_Y &&
                gx >= 0 && gx < COLS && gy >= 0 && gy < ROWS)
                placeCell(gx, gy);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            int gx = (int)(mouse.x - GRID_X) / CELL;
            int gy = (int)(mouse.y - GRID_Y) / CELL;
            if (mouse.x >= GRID_X && mouse.y >= GRID_Y &&
                inside(gx, gy)) {
                currentTool = TOOL_ERASE;
                placeCell(gx, gy);
            }
        }

        BeginDrawing();
        drawInterface();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
