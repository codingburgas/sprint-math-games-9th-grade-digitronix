// Minesweeper game using Raylib by Digitronix
#include <stdlib.h>
#include <time.h>
#include <raylib.h>
#include <raymath.h>

// Grid dimensions (changeable)
#define COLS 10
#define ROWS 10

// Screen dimensions
const int screen_width = 600;
const int screen_height = 600;

// Cell dimensions
const int cell_width = screen_width / COLS;
const int cell_height = screen_height / ROWS;

// Win/Lose messages
const char* youLose = "You Lost!";
const char* youWon = "You WIN!";
const char* pressRToRestart = "Press 'r' to play again!";

// Single cell in the grid
typedef struct Cell {
	int i, j; // Position in grid
	bool containsMine;
	bool revealed;
	bool flagged;
	int nearbyMines;
} Cell;

// Game states
typedef enum GameState {
	Playing,
	Lost,
	Won
} GameState;

// Global game variables
Cell grid[COLS][ROWS];
GameState state;
int tilesRevealed;
int presentMines;
int flagsPlaced;
float timeGameStarted;
float timeGameEnded;

// Textures (icons0
Texture2D flagSprite;
Texture2D mineSprite;

// Function declarations
void CellDraw(Cell cell);
bool IndexIsValid(int i, int j);
void CellReveal(int i, int j);
void CellFlag(int i, int j);
int CellCountMines(int i, int j);
void GridInit(void);
void GridFloodClearFrom(int i, int j);
void GridRevealAllMines(void);
void GameInit(void);

int main()
{
	srand(time(0));

	// Initialize window and resources
	InitWindow(screen_width, screen_height, "Minesweeper by Digitronix");
	SetTargetFPS(60);

	flagSprite = LoadTexture("Resources/flag.png");
	mineSprite = LoadTexture("Resources/bomb.png");

	GameInit();

	// Main game loop
	while (!WindowShouldClose()) {
		// Reveal cell
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			Vector2 mPos = GetMousePosition();
			int indexI = mPos.x / cell_width;
			int indexJ = mPos.y / cell_height;

			if (state == Playing && IndexIsValid(indexI, indexJ)) {
				CellReveal(indexI, indexJ);
			}
		}

		// Flag cell
		else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
			Vector2 mPos = GetMousePosition();
			int indexI = mPos.x / cell_width;
			int indexJ = mPos.y / cell_height;

			if (state == Playing && IndexIsValid(indexI, indexJ)) {
				CellFlag(indexI, indexJ);
			}
		}

		if (IsKeyPressed(KEY_R)) {
			GameInit();
		}

		BeginDrawing();

		ClearBackground(RAYWHITE);

		// Draw grid cells
		for (int i = 0; i < COLS; i++) {
			for (int j = 0; j < ROWS; j++) {
				CellDraw(grid[i][j]);
			}
		}

		// Lost game(state) overlay
		if (state == Lost) {
			DrawRectangle(0, 0, screen_width, screen_height, Fade(WHITE, 0.5f));
			DrawText(youLose, screen_width / 2 - MeasureText(youLose, 40) / 2, screen_height / 2 - 10, 40, DARKGRAY);
			DrawText(pressRToRestart, screen_width / 2 - MeasureText(pressRToRestart, 20) / 2, screen_height * 0.75f - 10, 20, DARKGRAY);

			int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
			int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
			DrawText(TextFormat("Time played: %d minutes, %d seconds", minutes, seconds), 20, screen_height - 40, 20, DARKGRAY);
			DrawText(TextFormat("Flags placed: %d", flagsPlaced), 20, screen_height - 65, 20, DARKGRAY);
		}

		// Won game(state) overlay
		if (state == Won) {
			DrawRectangle(0, 0, screen_width, screen_height, Fade(WHITE, 0.5f));
			DrawText(youWon, screen_width / 2 - MeasureText(youWon, 40) / 2, screen_height / 2 - 10, 40, DARKGRAY);
			DrawText(pressRToRestart, screen_width / 2 - MeasureText(pressRToRestart, 20) / 2, screen_height * 0.75f - 10, 20, DARKGRAY);

			int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
			int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
			DrawText(TextFormat("Time played: %d minutes, %d seconds", minutes, seconds), 20, screen_height - 40, 20, DARKGRAY);
			DrawText(TextFormat("Flags placed: %d", flagsPlaced), 20, screen_height - 65, 20, DARKGRAY);
		}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

// Draw a single cell
void CellDraw(Cell cell) {
	if (cell.revealed) {
		if (cell.containsMine) {
			// Draw mine
			DrawRectangle(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, RED);

			Rectangle source = { 0, 0, mineSprite.width, mineSprite.height };
			Rectangle dest = { cell.i * cell_width, cell.j * cell_height, cell_width, cell_height };
			Vector2 origin = { 0, 0 };

			DrawTexturePro(mineSprite, source, dest, origin, 0.0f, WHITE);
		}
		else {
			DrawRectangle(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, LIGHTGRAY);

			// Show number of nearby mines
			if (cell.nearbyMines > 0) {
				DrawText(TextFormat("%d", cell.nearbyMines), cell.i * cell_width + 12, cell.j * cell_height + 4, cell_height - 8, DARKGRAY);
			}
		}
	}
	else if (cell.flagged) {
		// Draw flag textiure
		Rectangle source = { 0, 0, flagSprite.width, flagSprite.height };
		Rectangle dest = { cell.i * cell_width, cell.j * cell_height, cell_width, cell_height };
		Vector2 origin = { 0, 0 };

		DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.9f));
	}

	// Cell border
	DrawRectangleLines(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, BLACK);
}

bool IndexIsValid(int i, int j) {
	return i >= 0 && i < COLS && j >= 0 && j < ROWS;
}

// Reveal a cell when clicked
void CellReveal(int i, int j) {
	// Can't reveal flagged cells
	if (grid[i][j].flagged) return;

	grid[i][j].revealed = true;

	if (grid[i][j].containsMine) {
		// Hit a mine - reveal all mines and game over
		GridRevealAllMines();
		state = Lost;
		timeGameEnded = GetTime();
	}
	else {
		// Safe ceil

		// If no nearby mines, reveal neighbors
		if (grid[i][j].nearbyMines == 0) {
			GridFloodClearFrom(i, j);
		}

		tilesRevealed++;

		// Check win conditioun
		if (tilesRevealed >= (ROWS * COLS) - presentMines) {
			state = Won;
			timeGameEnded = GetTime();
		}
	}
}

// Toggle flag on a cell
void CellFlag(int i, int j) {
	// Can't flag revealed cells
	if (grid[i][j].revealed) return;

	if (grid[i][j].flagged) {
		grid[i][j].flagged = false;
		flagsPlaced--;
	}
	else {
		grid[i][j].flagged = true;
		flagsPlaced++;
	}
}

// Count mines around a cell
int CellCountMines(int i, int j) {
	int count = 0;

	// Check all 8 neighbors
	for (int iOff = -1; iOff <= 1; iOff++) {
		for (int jOff = -1; jOff <= 1; jOff++) {
			// Skip center cell
			if (iOff == 0 && jOff == 0) continue;

			int ni = i + iOff;
			int nj = j + jOff;

			// Skip out of bounds
			if (!IndexIsValid(ni, nj)) continue;

			// Count mines
			if (grid[ni][nj].containsMine) count++;
		}
	}

	return count;
}

// Initialize the grid
void GridInit(void) {
	// Create empty cells
	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			grid[i][j] = { i, j, false, false };
		}
	}

	// Place mines (10% of grid)
	presentMines = (int)(COLS * ROWS * 0.1f);
	int minesToPlace = presentMines;

	while (minesToPlace > 0) {
		int i = rand() % COLS;
		int j = rand() % ROWS;

		// Only place if cell is empty
		if (!grid[i][j].containsMine) {
			grid[i][j].containsMine = true;
			minesToPlace--;
		}
	}

	// Calculate nearby mine counts
	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			if (!grid[i][j].containsMine) {
				grid[i][j].nearbyMines = CellCountMines(i, j);
			}
		}
	}
}

// Reveal neighbors recursively
void GridFloodClearFrom(int i, int j) {
	// Check all 8 neighbors
	for (int iOff = -1; iOff <= 1; iOff++) {
		for (int jOff = -1; jOff <= 1; jOff++) {
			// Skip center
			if (iOff == 0 && jOff == 0) continue;

			int ni = i + iOff;
			int nj = j + jOff;

			// Skip out of bounds
			if (!IndexIsValid(ni, nj)) continue;

			// Reveal unrevealed neighbors
			if (!grid[ni][nj].revealed) CellReveal(ni, nj);
		}
	}
}

// Reveal all mines
void GridRevealAllMines(void) {
	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			if (grid[i][j].containsMine) {
				grid[i][j].revealed = true;
			}
		}
	}
}

// Start a new game
void GameInit(void) {
	GridInit();
	state = Playing;
	tilesRevealed = 0;
	flagsPlaced = 0;
	timeGameStarted = GetTime();
}