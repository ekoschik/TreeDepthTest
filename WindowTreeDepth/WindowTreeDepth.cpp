#include "stdafx.h"
#include <windows.h>
#include <map>
#include <time.h>
using namespace std;

// Window size
int cx = 700, cy = 600;

// Window tree depth and spacing
int depth = 30; // !WARNING! keep this guy VERY low, ** < 35 **
int step = 7; // spacing between a parent and it's child

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
     if (depth <= 0) {
        return TRUE;
    }

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HWND hwndChild = CreateWindowWrap(hwnd, step, step,
        rcClient.right - rcClient.left - (2 * step),
        rcClient.bottom - rcClient.top - (2 * step));

    if (hwndChild == NULL) {
        return FALSE;
    }

    return CreateWindowTree(depth - 1, step, hwndChild);
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
        printf("CreateWindow failed, last error: %i\n", GetLastError());
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
    srand((int)time(NULL));
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

