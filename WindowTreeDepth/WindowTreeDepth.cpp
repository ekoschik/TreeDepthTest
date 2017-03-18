#include "stdafx.h"
#include <windows.h>
#include <map>
#include <time.h>
using namespace std;

map<HWND, HBRUSH> colormap;

VOID AddWindowToColorMap(HWND hwnd)
{
    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;
    printf("Giving window %X RGB(%i, %i, %i)\n",
        hwnd, r, g, b);

    HBRUSH hbr = CreateSolidBrush(RGB(r, g, b));
    colormap[hwnd] = hbr;
}

VOID Draw(HDC hdc, HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HBRUSH hbr = colormap[hwnd];
    FillRect(hdc, &rcClient, hbr);
}

LRESULT CALLBACK WndProcChild(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        AddWindowToColorMap(hwnd);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_MOUSEMOVE:
        printf("Child seeing mouse messages...\n");
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProcTLW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        AddWindowToColorMap(hwnd);
        break;

    case WM_MOUSEMOVE:
        printf("Frame seeing mouse messages...\n");
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND CreateWindowWrap(HWND, int, int, int, int);

BOOL CreateWindowTree()
{
    int def = CW_USEDEFAULT;
    HWND hwndTLW = CreateWindowWrap(NULL, def, def, 500, 400);
    HWND hwndC1 = CreateWindowWrap(hwndTLW, 50, 50, 200, 200);

    return TRUE;
}

HINSTANCE hInst;
LPCWSTR WndClassTLW = L"Window Frame",
WndClassChild = L"Child";
LPCWSTR WndTitleTLW = L"Window Tree Depth Test - Window Frame",
WndTitleChild = L"Window Tree Depth Test - Child";

HWND CreateWindowWrap(
    HWND hWndParent,
    int x, int y, int cx, int cy)
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
        printf("Failed to create %s\n",
            bTLW ? "window  frame" : "child window");
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

    printf("Initialized window, entering message loop.\n");

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

