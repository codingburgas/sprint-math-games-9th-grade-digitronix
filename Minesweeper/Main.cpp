#include <stdlib.h>
#include <time.h>

#include <raylib.h>
#include <raymath.h>

#define COLS 10 // Changable
#define ROWS 10

const int screen_width = 600;
const int screen_height = 600;

const int cell_width = screen_width / COLS;
const int cell_height = screen_height / ROWS;

typedef struct Cell {
	int i, j;
	bool containsMine;
	bool revealed;
	bool flagged;
	int nearbyMines;
} Cell;

Cell grid[COLS][ROWS];
int flagsPlaced;

Texture2D flagSprite;

void CellDraw(Cell);
bool IndexIsValid(int, int);
void CellReveal(int, int);
void CellFlag(int i, int j);
int CellCountMines(int, int);
void GridInit(void);

int main()
{
	srand(time(0));

	InitWindow(screen_width, screen_height, "My Minesweeper Game!");
	SetTargetFPS(60);

	flagSprite = LoadTexture("Resources/flag.png");

	GridInit();

	while (!WindowShouldClose()) {
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			Vector2 mPos = GetMousePosition();
			int indexI = mPos.x / cell_width;
			int indexJ = mPos.y / cell_height;

			if (IndexIsValid(indexI, indexJ)) {
				CellReveal(indexI, indexJ);
			}
		}

		else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
			Vector2 mPos = GetMousePosition();
			int indexI = mPos.x / cell_width;
			int indexJ = mPos.y / cell_height;

			if (IndexIsValid(indexI, indexJ)) {
				CellFlag(indexI, indexJ);
			}
		}

		BeginDrawing();

		ClearBackground(RAYWHITE);

		for (int i = 0; i < COLS; i++) {
			for (int j = 0; j < ROWS; j++) {
				CellDraw(grid[i][j]);
			}
		}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

void CellDraw(Cell cell) {
	if (cell.revealed) {
		if (cell.containsMine) {
			DrawRectangle(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, RED);
		}
		else {
			DrawRectangle(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, LIGHTGRAY);

			if (cell.nearbyMines > 0) {
				DrawText(TextFormat("%d", cell.nearbyMines), cell.i * cell_width + 12, cell.j * cell_height + 4, cell_height - 8, DARKGRAY);
			}
		}
	}
	else if (cell.flagged) {

		Rectangle source = { 0, 0, flagSprite.width, flagSprite.height };
		Rectangle dest = { cell.i * cell_width, cell.j * cell_height, cell_width, cell_height };
		Vector2 origin = { 0, 0 };

		DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.9f));
	}

	DrawRectangleLines(cell.i * cell_width, cell.j * cell_height, cell_width, cell_height, BLACK);
}

bool IndexIsValid(int i, int j) {
	return i >= 0 && i < COLS && j >= 0 && j < ROWS;
}

void CellReveal(int i, int j) {
	if (grid[i][j].flagged) return;

	grid[i][j].revealed = true;

	if (grid[i][j].containsMine) {
		// lose the game
	}
	else {
		// play sound
	}
}

void CellFlag(int i, int j) {
	if (grid[i][j].revealed) return;

	if (grid[i][j].flagged) {
		flagsPlaced--;
	}
	else {
		flagsPlaced++;
	}

	grid[i][j].flagged = !grid[i][j].flagged;
}

int CellCountMines(int i, int j) {
	int count = 0;
	for (int iOff = -1; iOff <= 1; iOff++)
	{
		for (int jOff = -1; jOff <= 1; jOff++)
		{
			if (iOff == 0 && jOff == 0) continue;

			int ni = i + iOff;
			int nj = j + jOff;

			if (!IndexIsValid(ni, nj)) continue;

			if (grid[ni][nj].containsMine) count++;
		}
	}

	return count;
}

void GridInit(void) {
	for (int i = 0; i < COLS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			grid[i][j] = { i, j, false, false };
		}
	}

	int minesToPlace = (int)(ROWS * COLS * 0.1f);
	while (minesToPlace > 0) {
		int i = rand() % COLS;
		int j = rand() % ROWS;

		if (!grid[i][j].containsMine)
		{
			grid[i][j].containsMine = true;
			minesToPlace--;
		}
	}

	for (int i = 0; i < COLS; i++) {
		for (int j = 0; j < ROWS; j++) {
			if (!grid[i][j].containsMine) {
				grid[i][j].nearbyMines = CellCountMines(i, j);
			}
		}
	}
}