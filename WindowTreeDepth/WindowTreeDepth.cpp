#include "stdafx.h"
#include <windows.h>
#include <map>
#include <time.h>
using namespace std;

HINSTANCE hInst;
LPCWSTR WndClassTLW = L"Window Frame",
WndClassChild = L"Child";
LPCWSTR WndTitleTLW = L"Window Tree Depth Test - Window Frame",
WndTitleChild = L"Window Tree Depth Test - Child";

struct WindowInfo
{
    int index;
    HBRUSH hbr;
};
map<HWND, WindowInfo> windowmap;
HWND hwndMouseLast = NULL;

VOID AddWindowToMap(HWND hwnd)
{
    WindowInfo wi = {};

    // Assign window an index
    static int iIndex = 0;
    wi.index = iIndex++;

    // Assign window a color
    int r = rand() % 255;
    int g = rand() % 255;
    int b = rand() % 255;
    printf("Giving window %i RGB(%i, %i, %i)\n",
        wi.index, r, g, b);

    wi.hbr = CreateSolidBrush(RGB(r, g, b));

    // Add window info to window map
    windowmap[hwnd] = wi;
}

VOID Draw(HDC hdc, HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    HBRUSH hbr = windowmap[hwnd].hbr;
    FillRect(hdc, &rcClient, hbr);
}


VOID CALLBACK WndProcCommon(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_CREATE:
        AddWindowToMap(hwnd);
        break;

    case WM_PAINT:
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }
}

LRESULT CALLBACK WndProcChild(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WndProcCommon(hwnd, message, wParam, lParam);

    switch (message)
    {
    case WM_MOUSEMOVE:
        if (hwndMouseLast != hwnd) {
            printf("Child %i seeing mouse messages...\n",
                windowmap[hwnd].index);
            hwndMouseLast = hwnd;
        }
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
    WndProcCommon(hwnd, message, wParam, lParam);

    switch (message)
    {
    case WM_MOUSEMOVE:
        if (hwndMouseLast != hwnd) {
            printf("Frame seeing mouse messages...\n");
            hwndMouseLast = hwnd;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

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

BOOL CreateWindowTree(int depth, HWND hwndParent)
{
    if (depth <= 0) {
        return TRUE;
    }

    int cx = 700, cy = 600; // starting window size
    static int step = 10;
    if (hwndParent == NULL) {
        HWND hwndTLW = CreateWindowWrap(NULL, CW_USEDEFAULT, CW_USEDEFAULT, cx, cy);

        RECT rcClient;
        GetClientRect(hwndParent, &rcClient);
        cx = rcClient.right - rcClient.left;
        cy = rcClient.bottom - rcClient.top;

        step = cx / depth;
        CreateWindowTree(depth, hwndTLW);
        return TRUE;
    }

    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    cx = rcClient.right - rcClient.left;
    cy = rcClient.bottom - rcClient.top;

    return CreateWindowTree(depth - 1, CreateWindowWrap(hwndParent, step,   step,   cx / 2, cy / 2)) &&
           CreateWindowTree(depth - 1, CreateWindowWrap(hwndParent, step,   cy / 2, cx / 2, cy / 2)) &&
           CreateWindowTree(depth - 1, CreateWindowWrap(hwndParent, cx / 2, step,   cx / 2, cy / 2)) &&
           CreateWindowTree(depth - 1, CreateWindowWrap(hwndParent, cx / 2, cy / 2, cx / 2, cy / 2));
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
    
    if (!CreateWindowTree(6, NULL)) {
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

