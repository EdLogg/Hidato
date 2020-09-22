// Hidato2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#define MAX_LOADSTRING 100

HBITMAP		hBoard = NULL;
HBITMAP		hBoardEnd = NULL;
HBITMAP		hBoardOn = NULL;
HBITMAP		hBoardOff = NULL;
HFONT		hGameFont0 = NULL;
HFONT		hGameFont1 = NULL;
HFONT		hGameFont2 = NULL;


// Global Variables:
HINSTANCE hInst;									// current instance
TCHAR	szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR	szWindowClass[MAX_LOADSTRING];				// the main window class name

hSolver s;											// create solver
STATE	state;										// current state of the board
int		xOffset, yOffset;							// offsets for drawing the tiles


int SortEnds(const void * e1, const void * e2)
{
	int i1 = *(int *)e1;
	int i2 = *(int *)e2;
	if (i1 > i2)
		return 1;
	return -1;
}


// look for the smallest differences between digits
bool FindObviousStep(HWND hWnd)
{
	int diffs[MAX_HEIGHT * MAX_WIDTH + 1];
	int largest = 1, smallest = s.puzzleMax;
	diffs[0] = 0;
	for (int y = 0; y < s.puzzleHeight; y++)
	{
		for (int x = 0; x < s.puzzleWidth; x++)
		{
			s.puzzle[y][x].used = 0;
		}
	}
	for (int n1 = 1; n1 < s.puzzleMax; )
	{
		for (int n2 = n1 + 1; n2 <= s.puzzleMax; n2++)
		{
			if (s.pos[n2].x >= 0)					// this is shown
			{
				int diff = n2 - n1;					// how many are in between
				diffs[n1] = diff;					// this is the difference between the numbers
				if (diff > 1)						// look at these
				{
					if (smallest > diff)
						smallest = diff;
					if (largest < diff)
						largest = diff;
				}
				n1 = n2;
				break;
			}
			diffs[n2] = 0;							// ignore holes
		}
	}
	for (int diff = smallest; diff <= largest; diff++)
	{
		for (int n = 1; n < s.puzzleMax; n += diffs[n])
		{
			if (diffs[n] == diff)
			{
				// OK now look for obvious solutions
				if (s.FindSolutions(n, n + diff))	// if we found one stop
					return true;
			}
		}
	}
	// see if there is only one path that touched some location
	int updated = 0;
	for (int y = 0; y < s.puzzleHeight; y++)
	{
		for (int x = 0; x < s.puzzleWidth; x++)
		{
			if (s.puzzle[y][x].used > 0)			// only used for one value
			{
				int val = s.puzzle[y][x].used;
				s.puzzle[y][x].val = val;
				s.puzzle[y][x].added = true;
				s.pos[val].x = x;
				s.pos[val].y = y;
				updated++;
			}
		}
	}
	if (updated > 0)								// Rule 5
		return true;
	// try looking at all solutions and see if there is anything in common
	s.SolvePuzzle(true);
	if (s.solutionCount == 0)
	{
		MessageBox(hWnd, "There are no solutions!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	int count = 0;
	for (int y = 0; y < s.puzzleHeight; y++)
	{
		for (int x = 0; x < s.puzzleWidth; x++)
		{
			int val = s.solutionCommon[y][x];
			if (val == 0
				|| val == s.puzzle[y][x].val)
				continue;
			count++;
			s.pos[val].x = x;
			s.pos[val].y = y;
			s.puzzle[y][x].val = val;
			s.puzzle[y][x].added = true;
		}
	}
	if (count > 0)
	{
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		MessageBox(hWnd, "I had to brute force all solutions!", "Very Interesting Situation", MB_OK | MB_ICONEXCLAMATION);
		return true;
	}
	// OK nothing will work
	MessageBox(hWnd, "There are several solutions to this puzzle!", "Error", MB_OK | MB_ICONEXCLAMATION);
	return false;
	// OK nothing will work
	return false;
}


//***********************************************************************
//*** Center the second window in relation to the desktop ***************
//***********************************************************************
void CenterWindow(HWND CW_h2, int horzOffset = 0, int vertOffset = 0)
{
	RECT CW_rect1, CW_rect2;
	int CW_midx, CW_midy, CW_wx, CW_wy;
	HWND CW_h1;

	CW_h1 = GetDesktopWindow();
	GetWindowRect(CW_h1, &CW_rect1);
	GetWindowRect(CW_h2, &CW_rect2);
	CW_midx = (CW_rect1.right + CW_rect1.left) >> 1;
	CW_midy = (CW_rect1.bottom + CW_rect1.top) >> 1;
	CW_wx = CW_rect2.right - CW_rect2.left;
	CW_wy = CW_rect2.bottom - CW_rect2.top;
	MoveWindow(CW_h2, CW_midx - (CW_wx >> 1) + horzOffset, CW_midy - (CW_wy >> 1) + vertOffset, CW_wx, CW_wy, false);
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,	// fixed size window
		0, 0, SCREEN_WIDTH + 16, SCREEN_HEIGHT + 60, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	CenterWindow(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}


// message handler for start a new puzzle
int puzzleWidth;
int puzzleHeight;
INT_PTR CALLBACK NewPuzzle(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND msg;
	char string[32];

	switch (message)
	{
	case WM_INITDIALOG:
		CenterWindow(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			msg = GetDlgItem(hDlg, IDC_PUZZLEWIDTH);
			GetWindowText(msg, string, sizeof(string));
			puzzleWidth = atoi(string);
			msg = GetDlgItem(hDlg, IDC_PUZZLEHEIGHT);
			GetWindowText(msg, string, sizeof(string));
			puzzleHeight = atoi(string);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			puzzleWidth = puzzleHeight = -1;		
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


int OriginalValue(int x, int y)
{
	int n;
	for (n = 1; n <= s.puzzleMax; n++)
	{
		if (s.pos[n].x == s.cursorX
		&&  s.pos[n].y == s.cursorY)
		{
			return n;
		}
	}
	return 0;
}


bool ManualSetValue(bool clear)
{
	if (s.cursorX < 0)
		return false;									// nothing to do
	int val = s.puzzle[s.cursorY][s.cursorX].val;
	int old = OriginalValue(s.cursorX, s.cursorY);		// original value if any
	if (val == 0)										// area is now blank
	{
		if (old != 0)
			s.pos[old].x = s.pos[old].y = -1;			// remove old value from our list
		if (clear)
			s.cursorX = s.cursorY = -1;
		return false;
	}
	bool done = false;
	if (val > 0 && val <= s.puzzleMax					// within range
	&& s.pos[val].x < 0)								// value was not known before				
	{
		if (state == STATE_SOLVE)
		{
			s.pos[val].x = s.cursorX;
			s.pos[val].y = s.cursorY;
			s.nodes++;
		}
		done = s.PuzzleSolved();						// see if we have solved the puzzle
	}
	else if (val != 0)
	{
		s.puzzle[s.cursorY][s.cursorX].val = old;		// clear any illegal value
		MessageBeep(MB_ICONERROR);
	}
	if (clear)
		s.cursorX = s.cursorY = -1;
	return done;
}


void	ChangeCursor(HWND hWnd, int x, int y)
{
	int xx = (x - xOffset) / TILE_WIDTH;
	int yy = (y - yOffset) / TILE_HEIGHT;
	if (state == STATE_FORMAT)
	{
		if (xx < 0
		|| xx >= s.puzzleWidth
		|| yy < 0
		|| yy >= s.puzzleHeight)
			return;
		if (s.puzzle[yy][xx].val == 0)
			s.puzzle[yy][xx].val = -1;
		else
			s.puzzle[yy][xx].val = 0;
	}
	else if (state == STATE_ENTER)
	{
		if (xx < 0
		|| xx >= s.puzzleWidth
		|| yy < 0
		|| yy >= s.puzzleHeight
		|| s.puzzle[yy][xx].val < 0)
		{
			s.cursorX = s.cursorY = -1;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			return;
		}
		s.cursorX = xx;
		s.cursorY = yy;
	}
	else if (state == STATE_SOLVE)
	{
		if (ManualSetValue(true))							// so set the value if it is valid
		{
			HMENU hMenu = GetMenu(hWnd);
			EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
			state = STATE_DONE;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			return;
		}
		if (xx < 0
		|| xx >= s.puzzleWidth
		|| yy < 0
		|| yy >= s.puzzleHeight
		|| s.puzzle[yy][xx].val < 0)
		{
			s.cursorX = s.cursorY = -1;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			return;
		}
		s.cursorX = xx;
		s.cursorY = yy;
	}
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HFONT hFont;
	LOGFONT logfont;

	HMENU hMenu = GetMenu(hWnd);
	switch (message)
	{
	case WM_CREATE:
		hBoard = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BOARD));
		hBoardEnd = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BOARDEND));
		hBoardOn = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BOARDON));
		hBoardOff = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BOARDOFF));
		if (hBoard == NULL
		|| hBoardEnd == NULL
		|| hBoardOn == NULL
		|| hBoardOff == NULL)
			MessageBox(hWnd, "Could not load art!", "Error", MB_OK | MB_ICONEXCLAMATION);

		// create fonts to use in the game
		hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		GetObject(hFont, sizeof(LOGFONT), &logfont);
		logfont.lfHeight = -20;						// in pixels
		logfont.lfWeight = 400;						// 400=normal 700=bold 1000=max
		hGameFont0 = CreateFontIndirect(&logfont);
		logfont.lfHeight = -40;						// in pixels
		logfont.lfWeight = 1000;					// 400=normal 700=bold 1000=max
		hGameFont1 = CreateFontIndirect(&logfont);
		logfont.lfHeight = -20;						// in pixels
		logfont.lfWeight = 1000;					// 400=normal 700=bold 1000=max
		hGameFont2 = CreateFontIndirect(&logfont);
		state = STATE_INVALID;
		EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
		goto START;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		s.ClearAdded();
		ChangeCursor(hWnd, lParam & 0xffff, (int)(lParam >> 16));
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			if (state == STATE_ENTER)
			{
				s.MoveLeft();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (state == STATE_SOLVE
			&& s.cursorX >= 0)
			{
				if (ManualSetValue(false))						// so set the value if it is valid
				{
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
					state = STATE_DONE;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					break;
				}
				s.MoveLeft();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case VK_UP:
			if (state == STATE_ENTER)
			{
				s.MoveUp();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (state == STATE_SOLVE
			&& s.cursorX >= 0)
			{
				if (ManualSetValue(false))							// so set the value if it is valid
				{
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
					state = STATE_DONE;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					break;
				}
				s.MoveUp();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case VK_RIGHT:
			if (state == STATE_ENTER)
			{
				s.MoveRight();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (state == STATE_SOLVE
			&& s.cursorX >= 0)
			{
				if (ManualSetValue(false))						// so set the value if it is valid
				{
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
					state = STATE_DONE;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					break;
				}
				s.MoveRight();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case VK_DOWN:
			if (state == STATE_ENTER)
			{
				s.MoveDown();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (state == STATE_SOLVE
			&& s.cursorX >= 0)
			{
				if (ManualSetValue(false))						// so set the value if it is valid
				{
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
					state = STATE_DONE;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					break;
				}
				s.MoveDown();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case VK_DELETE:
		case VK_BACK:
			if (state == STATE_ENTER)
			{
				s.ClearCurrentPos();
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if (state == STATE_SOLVE
			&&  s.cursorX >= 0)
			{
				int old = OriginalValue(s.cursorX, s.cursorY);		// original value if any
				if (old != 0)
					s.pos[old].x = s.pos[old].y = -1;
				s.puzzle[s.cursorY][s.cursorX].val = 0;
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (state == STATE_ENTER)
			{
				s.puzzle[s.cursorY][s.cursorX].val = 10 * s.puzzle[s.cursorY][s.cursorX].val + (int)wParam - '0';
			}
			else if (state == STATE_SOLVE
			&&  s.cursorX >= 0)
			{
				s.puzzle[s.cursorY][s.cursorX].val = 10 * s.puzzle[s.cursorY][s.cursorX].val + (int)wParam - '0';
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		case VK_RETURN:
			if (state == STATE_FORMAT)
			{
				s.InitCursor();
				s.SetMax();
				state = STATE_ENTER;
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
			else if (state == STATE_ENTER)
			{
				int x, y;
				s.FindStart(x, y);
				if (x < 0)
				{
					MessageBox(hWnd, "Could not find the starting number 1!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				s.FindEnd(x, y);
				if (x < 0)
				{
					MessageBox(hWnd, "Could not find the ending number!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				if (s.SetPositions() == false)					// set positions
				{
					s.ClearPositions();							// clear any position entries
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
					MessageBox(hWnd, "Removed duplicate entries!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				if (s.SolvePuzzle(true))
				{
					if (s.solutionCount != 1)
					{
						s.ClearPositions();						// clear any position entries
						char string[40];
						sprintf(string, "%d solutions found for this puzzle!", s.solutionCount);
						MessageBox(hWnd, string, "Error", MB_OK | MB_ICONEXCLAMATION);
						break;
					}
				}
				else
				{
					s.ClearPositions();							// clear any position entries
					MessageBox(hWnd, "There are no solutions for this puzzle!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				state = STATE_SOLVE;
				s.cursorX = s.cursorY = -1;
				s.nodes = 0;
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_ENABLED);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
			else if (state == STATE_SOLVE)
			{
				s.ClearAdded();
				ChangeCursor(hWnd, -1, -1);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case VK_SPACE:
			if (state == STATE_SOLVE)
				goto STEP;
			break;
		}
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_NEW:
START:		DialogBox(hInst, MAKEINTRESOURCE(IDD_NEWBOX), hWnd, NewPuzzle);
			if (puzzleWidth != -1
			&& puzzleHeight != -1)
			{
				if (puzzleWidth > 1 && puzzleWidth <= MAX_WIDTH
				&&  puzzleHeight > 1 && puzzleHeight <= MAX_HEIGHT)
				{
					xOffset = (SCREEN_WIDTH - puzzleWidth * TILE_WIDTH) / 2;
					yOffset = (MAX_HEIGHT - puzzleHeight) / 2 * TILE_HEIGHT;
					yOffset += SCREEN_HEIGHT - MAX_HEIGHT * TILE_HEIGHT;
					s.puzzleHeight = puzzleHeight;
					s.puzzleWidth = puzzleWidth;
					s.ClearPuzzle();
					state = STATE_FORMAT;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				}
				else
				{
					MessageBox(hWnd, "Invalid puzzle size!", "Error", MB_OK | MB_ICONEXCLAMATION);
				}
			}
			break;
		case IDM_FILE_LOAD:
			if (DoFileLoad(hWnd) == false)				// we must have clobbered something in the current puzzle
			{
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				state = STATE_INVALID;
				s.ClearPuzzle();
			}
			else
			{
				if (state == STATE_ENTER
				||  state == STATE_SOLVE)
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
				else
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				if (state == STATE_SOLVE)
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_ENABLED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				}
				xOffset = (SCREEN_WIDTH - s.puzzleWidth * TILE_WIDTH) / 2;
				yOffset = (MAX_HEIGHT - s.puzzleHeight) / 2 * TILE_HEIGHT;
				yOffset += SCREEN_HEIGHT - MAX_HEIGHT * TILE_HEIGHT;
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			break;
		case IDM_CHECK:
		case IDM_SOLVE:
			if (ManualSetValue(true))						// so set the value if it is valid
			{
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				state = STATE_DONE;
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				break;
			}
			if (s.SolvePuzzle(wmId == IDM_CHECK))
			{
				if (wmId == IDM_CHECK)
				{
					if (s.solutionCount > 1)
					{
						char string[40];
						sprintf(string, "%d solutions found for this puzzle!", s.solutionCount);
						MessageBox(hWnd, string, "Error", MB_OK | MB_ICONEXCLAMATION);
						RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					break;
				}		
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				state = STATE_DONE;
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
			else
			{
				MessageBox(hWnd, "No solution for this puzzle!", "Error", MB_OK | MB_ICONEXCLAMATION);
			}
			break;
		case IDM_STEP:
STEP:		if (ManualSetValue(true))						// so set the value if it is valid
			{
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				state = STATE_DONE;
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				break;
			}
			if (FindObviousStep(hWnd))
			{
				if (s.PuzzleSolved())						// see if we have solved the puzzle
				{
					EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
					state = STATE_DONE;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				}
				else
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			break;
		case IDM_FILE_SAVE:
			if (ManualSetValue(true))						// in case we are manually solving
			{
				EnableMenuItem(hMenu, IDM_FILE_SAVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hMenu, IDM_STEP, MF_BYCOMMAND | MF_GRAYED);
				state = STATE_DONE;
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
			else
			{
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				DoFileSave(hWnd);
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawPuzzle(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DeleteObject(hBoard);
		DeleteObject(hBoardEnd);
		DeleteObject(hBoardOn);
		DeleteObject(hBoardOff);
		DeleteObject(hGameFont0);
		DeleteObject(hGameFont1);
		DeleteObject(hGameFont2);
		PostQuitMessage(0);
		_CrtDumpMemoryLeaks();								// check for memory leaks
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HIDATO2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_HIDATO2);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_HIDATO2));

	return RegisterClassEx(&wcex);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_HIDATO2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HIDATO2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
