#include "stdafx.h"
#include <windows.h>
#include <string>
#include <map>
#include <time.h>
using namespace std;

// Window size
int cx = 700, cy = 600;

// Window tree depth
int depth = 30; // !WARNING! keep this guy VERY low, ** < 35 **

// Bookkeeping to color windows and highlight current mouse recipient
map<HWND, HBRUSH> windowmap;
HWND hwndMouseLast = NULL;
COLORREF rgbHighlight = RGB(229, 244, 66);

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
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
        // Set the current window under the cursor as the highlight window,
        // and repaint the new and old highlight window
        if (hwndMouseLast != hwnd) {
            HWND hwndPrev = hwndMouseLast;
            hwndMouseLast = hwnd;
            InvalidateRect(hwnd, NULL, TRUE);
            InvalidateRect(hwndPrev, NULL, TRUE);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HINSTANCE hInst;
LPCWSTR WndClassTLW = L"Window Frame",
        WndClassChild = L"Child",
        WndTitleTLW = L"Window Tree Depth Test - Window Frame",
        WndTitleChild = L"Window Tree Depth Test - Child";

RECT GetClientRect(HWND hwnd)
{
    RECT rcClient;
    ::GetClientRect(hwnd, &rcClient);
    return rcClient;
}

#define RCWIDTH(rc) (rc.right-rc.left)
#define RCHEIGHT(rc) (rc.bottom-rc.top)

bool CreateWindowTree(int depth, HWND hwndParent, int step)
{
    if (depth <= 0) {
        return true;
    }

    // Each child is the size of it's parent indented by step
    RECT rcClient = GetClientRect(hwndParent);
    int cx = RCWIDTH(rcClient) - (2 * step);
    int cy = RCHEIGHT(rcClient) - (2 * step);

    // Create the child window
    HWND hwndChild = CreateWindowEx(
        0, WndClassChild, WndTitleChild, WS_CHILD,
        step, step, cx , cy,
        hwndParent, nullptr, hInst, nullptr);

    if (!hwndChild) {
        printf("CreateWindow failed, last error: %i\n", GetLastError());
        return false;
    }

    ShowWindow(hwndChild, SW_SHOW);

    // Create next child
    return CreateWindowTree(depth - 1, hwndChild, step);
}

bool CreateWindowTree()
{
    // Create top level window
    HWND hwnd = CreateWindowEx(0,
        WndClassTLW,
        WndTitleTLW,
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, cx, cy,
        NULL, nullptr, hInst, nullptr);

    if (!hwnd) {
        printf("Creating TLW failed, last error: %i\n", GetLastError());
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);

    // Calculate step
    RECT rc = GetClientRect(hwnd);
    int step = max(1, (min(RCWIDTH(rc), RCHEIGHT(rc)) / depth) / 2 - 2);
    printf("Using step size of %i\n", step);

    // Create child windows
    if (!CreateWindowTree(depth, hwnd, step)) {
        return false;
    }

    printf("Created %i windows.\n", windowmap.size());
    return true;
}

bool RegisterWindows()
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

    // Register top level window class
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = WndClassTLW;
    if (!RegisterClassEx(&wcex)) {
        printf("Failed to register TLW window class!\n");
        return false;
    }

    // Register child window class
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = WndClassChild;
    if (!RegisterClassEx(&wcex)) {
        printf("Failed to register child window class!\n");
        return false;
    }

    return true;
}

void CheckArgs(int argc, char *argv[])
{
    if (argc > 1) {
        int maxDepth = 35;
        try {
            int depthOverride = stoi(string(argv[1]));

            if (depthOverride > 0) {
                if (depthOverride < maxDepth ||
                    (argc > 2 && strcmp(argv[2], "-f"))) {
                    printf("Using depth: %i\n", depthOverride);
                    depth = depthOverride;
                } else {
                    printf("ERROR include '-f' for values > %i\n", maxDepth);
                    printf("... using default depth %i\n", depth);
                }
            }
        } catch (...) {
            printf("ERROR only accepts integer argument (depth).\n");
            printf("note: >%i requires - f\n", maxDepth);
        }
    }
}

int main(int argc, char *argv[])
{
    srand((int)time(NULL));
    hInst = GetModuleHandle(NULL);
    CheckArgs(argc, argv);

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

