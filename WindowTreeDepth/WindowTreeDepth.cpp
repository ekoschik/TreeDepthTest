#include "stdafx.h"
#include <windows.h>
#include <map>
#include <time.h>
using namespace std;

// Window size
int cx = 700, cy = 600;

// WARNING:
// A tree depth of 5 creates 1365 window,
// but 100 will take down the system (bluescreen).
// ... Do NOT tweak this on a critical system.

// Window tree depth and spacing
int depth = 4; // !WANRING! keep this guy VERY low, < 5 unless on a test machine
int step = 2; // spacing between a parent and it's children

HINSTANCE hInst;
LPCWSTR WndClassTLW = L"Window Frame",
        WndClassChild = L"Child",
        WndTitleTLW = L"Window Tree Depth Test - Window Frame",
        WndTitleChild = L"Window Tree Depth Test - Child";
HWND CreateWindowWrap(HWND hWndParent, int x, int y, int cx, int cy);
map<HWND, HBRUSH> windowmap;
HWND hwndMouseLast = NULL;


BOOL CreateWindowTree(int depth, int step, HWND hwnd)
{
    // When depth hits zero we've reached the depth limit
    if (depth <= 0) {
        return TRUE;
    }

    // For each window, first create four children filling up each corner,
    // with 'step' space between the parent and children.
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    int cx = rcClient.right - rcClient.left - (2 * step);
    int cy = rcClient.bottom - rcClient.top - (2 * step);

    HWND hwnd1 = CreateWindowWrap(hwnd, step, step, cx / 2, cy / 2);
    HWND hwnd2 = CreateWindowWrap(hwnd, step, cy / 2, cx / 2, cy / 2);
    HWND hwnd3 = CreateWindowWrap(hwnd, cx / 2, step, cx / 2, cy / 2);
    HWND hwnd4 = CreateWindowWrap(hwnd, cx / 2, cy / 2, cx / 2, cy / 2);
    if (!hwnd1 || !hwnd2 || !hwnd3 || !hwnd4) {
        printf("Creating child window failed, aborting tree creation.\n");
        return FALSE;
    }

    // Create a window tree for each child, decrementing depth by 1
    depth--;
    return CreateWindowTree(depth, step, hwnd1) &&
           CreateWindowTree(depth, step, hwnd2) &&
           CreateWindowTree(depth, step, hwnd3) &&
           CreateWindowTree(depth, step, hwnd4);
}

BOOL CreateWindowTree()
{
    HWND hwnd = CreateWindowWrap(NULL, CW_USEDEFAULT, CW_USEDEFAULT, cx, cy);
    if (!CreateWindowTree(depth, step, hwnd)) {
        return FALSE;
    }

    printf("Created %i windows.\n", windowmap.size());
    return TRUE;
}

VOID Draw(HDC hdc, HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    
    static HBRUSH hbrHover = CreateSolidBrush(RGB(229, 244, 66));
    FillRect(hdc, &rcClient,
        (hwnd == hwndMouseLast) ? hbrHover : windowmap[hwnd]);
}

VOID WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bTLW)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_CREATE:
        // Pick a random color, create a brush of that color, and add
        // brush to the window map
        windowmap[hwnd] = CreateSolidBrush(
            RGB(rand() % 255, rand() % 255, rand() % 255));
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;

    case WM_MOUSEMOVE:
        if (hwndMouseLast != hwnd) {
            HWND hwndPrev = hwndMouseLast;
            hwndMouseLast = hwnd;
            InvalidateRect(hwnd, NULL, TRUE);
            InvalidateRect(hwndPrev, NULL, TRUE);
        }
        break;

    }
}

LRESULT CALLBACK WndProcChild(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WndProc(hwnd, message, wParam, lParam, FALSE);
    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProcTLW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WndProc(hwnd, message, wParam, lParam, TRUE);

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND CreateWindowWrap(HWND hWndParent, int x, int y, int cx, int cy)
{
    BOOL bTLW = hWndParent == NULL;

    HWND hwnd = CreateWindowEx(
        0,
        bTLW ? WndClassTLW : WndClassChild,
        bTLW ? WndTitleTLW : WndTitleChild,
        bTLW ? WS_OVERLAPPEDWINDOW : WS_CHILD,
        x, y, cx, cy,
        hWndParent, nullptr, hInst, nullptr);

    if (!hwnd) {
        return NULL;
    }

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

BOOL RegisterWindows()
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProcTLW;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = WndClassTLW;
    if (!RegisterClassEx(&wcex)) {
        printf("Failed to register TLW window class!\n");
        return FALSE;
    }

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProcChild;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = WndClassChild;
    if (!RegisterClassEx(&wcex)) {
        printf("Failed to register child window class!\n");
        return FALSE;
    }

    return TRUE;
}

int main()
{
    srand(time(NULL));
    hInst = GetModuleHandle(NULL);

    if (!RegisterWindows()) {
        return 1;
    }
    
    if (!CreateWindowTree()) {
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

