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

map<HWND, HBRUSH> windowmap;
HWND hwndMouseLast = NULL;

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

HINSTANCE hInst;
LPCWSTR WndClassTLW = L"Window Frame",
        WndClassChild = L"Child",
        WndTitleTLW = L"Window Tree Depth Test - Window Frame",
        WndTitleChild = L"Window Tree Depth Test - Child";

BOOL CreateWindowTree(int depth, HWND hwndParent)
{
    if (depth <= 0) {
        return TRUE;
    }

    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int cx = rcClient.right - rcClient.left;
    int cy = rcClient.bottom - rcClient.top;

    HWND hwndChild = CreateWindowEx(
        0, WndClassChild, WndTitleChild, WS_CHILD,
        step, step, cx - (2 * step), cy - (2 * step),
        hwndParent, nullptr, hInst, nullptr);

    if (hwndChild == NULL) {
        printf("CreateWindow failed, last error: %i\n", GetLastError());
        return FALSE;
    }

    ShowWindow(hwndChild, SW_SHOW);
    return CreateWindowTree(depth - 1, hwndChild);
}

BOOL CreateWindowTree()
{
    // Create top level window
    HWND hwnd = CreateWindowEx(0,
        WndClassTLW,
        WndTitleTLW,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, cx, cy,
        NULL, nullptr, hInst, nullptr);

    if (!hwnd) {
        printf("Creating TLW failed, last error: %i\n", GetLastError());
        return NULL;
    }

    ShowWindow(hwnd, SW_SHOW);

    if (!CreateWindowTree(depth, hwnd)) {
        return FALSE;
    }

    printf("Created %i windows.\n", windowmap.size());
    return TRUE;
}

BOOL RegisterWindows()
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

    // Register top level window class
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProcTLW;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = WndClassTLW;
    if (!RegisterClassEx(&wcex)) {
        printf("Failed to register TLW window class!\n");
        return FALSE;
    }

    // Register child window class
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

    // Initialize windows
    if (!RegisterWindows() || !CreateWindowTree()) {
        return 1;
    }
    
    // Message pump
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

