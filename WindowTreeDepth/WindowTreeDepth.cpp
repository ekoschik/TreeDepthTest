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

// Bookkeeping to color windows and highlight current mouse recipient
map<HWND, HBRUSH> windowmap;
HWND hwndMouseLast = NULL;
COLORREF rgbHighlight = RGB(229, 244, 66);

VOID WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bTLW)
{
    switch (message)
    {
    case WM_CREATE:
        // Add the window to the window map with a brush of a random color
        windowmap[hwnd] = CreateSolidBrush(
            RGB(rand() % 255, rand() % 255, rand() % 255));
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        // Fill the client area with the window's color, or the highlight
        // color if this window is the last mouse window
        static HBRUSH hbrHover = CreateSolidBrush(rgbHighlight);
        HBRUSH hbr = (hwnd == hwndMouseLast) ? hbrHover : windowmap[hwnd];
        FillRect(hdc, &rcClient, hbr);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_MOUSEMOVE:
        // Set this window as the highlight window, and repaint the previous
        // highlight window
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

    // Each child is the size of it's parent indented by step
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    int cx = rcClient.right - rcClient.left - (2 * step);
    int cy = rcClient.bottom - rcClient.top - (2 * step);

    // Create the child window
    HWND hwndChild = CreateWindowEx(
        0, WndClassChild, WndTitleChild, WS_CHILD,
        step, step, cx , cy,
        hwndParent, nullptr, hInst, nullptr);

    if (hwndChild == NULL) {
        printf("CreateWindow failed, last error: %i\n", GetLastError());
        return FALSE;
    }

    ShowWindow(hwndChild, SW_SHOW);

    // Create more children
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

    // Create child windows
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

