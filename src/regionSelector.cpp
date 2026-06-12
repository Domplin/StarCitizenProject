#include "regionSelector.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

POINT RegionSelector::s_start     = {};
POINT RegionSelector::s_end       = {};
bool  RegionSelector::s_selecting = false;
bool  RegionSelector::s_done      = false;

LRESULT CALLBACK RegionSelector::overlayProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_LBUTTONDOWN:
        s_start     = { LOWORD(lp), HIWORD(lp) };
        s_end       = s_start;
        s_selecting = true;
        SetCapture(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        if (s_selecting) {
            s_end = { LOWORD(lp), HIWORD(lp) };
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_LBUTTONUP:
        if (s_selecting) {
            s_end       = { LOWORD(lp), HIWORD(lp) };
            s_selecting = false;
            s_done      = true;
            ReleaseCapture();
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);

        // Double buffer
        HDC memDC       = CreateCompatibleDC(hdc);
        HBITMAP memBmp  = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP oldBmp  = (HBITMAP)SelectObject(memDC, memBmp);

        // Fill entire buffer black
        HBRUSH dim = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(memDC, &rc, dim);
        DeleteObject(dim);

        if (s_selecting || s_done) {
            RECT sel = {
                std::min(s_start.x, s_end.x), std::min(s_start.y, s_end.y),
                std::max(s_start.x, s_end.x), std::max(s_start.y, s_end.y)
            };

            // Punch transparent hole using color key
            HBRUSH hole = CreateSolidBrush(RGB(0, 1, 0));
            FillRect(memDC, &sel, hole);
            DeleteObject(hole);

            // Green border
            HPEN pen        = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
            HPEN oldPen     = (HPEN)SelectObject(memDC, pen);
            HBRUSH noBrush  = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, noBrush);
            Rectangle(memDC, sel.left, sel.top, sel.right, sel.bottom);
            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBrush);
            DeleteObject(pen);

            // Size label
            char buf[64];
            sprintf(buf, "%dx%d", abs(s_end.x - s_start.x), abs(s_end.y - s_start.y));
            SetTextColor(memDC, RGB(0, 255, 0));
            SetBkMode(memDC, TRANSPARENT);
            TextOutA(memDC, sel.left + 4, sel.top + 4, buf, strlen(buf));
        }

        // Instructions
        SetTextColor(memDC, RGB(255, 255, 255));
        SetBkMode(memDC, TRANSPARENT);
        TextOutA(memDC, 20, 20,
            "Click and drag to select the mining UI region. Release to capture.", 67);

        // Blit to screen in one shot
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

Region RegionSelector::select() {
    s_start     = {};
    s_end       = {};
    s_selecting = false;
    s_done      = false;

    WNDCLASSA wc     = {};
    wc.lpfnWndProc   = overlayProc;
    wc.hInstance     = GetModuleHandleA(NULL);
    wc.lpszClassName = "OverlayClass";
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wc.style         = CS_HREDRAW | CS_VREDRAW;

    WNDCLASSA existing = {};
    if (!GetClassInfoA(GetModuleHandleA(NULL), "OverlayClass", &existing))
        RegisterClassA(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    HWND overlay = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        "OverlayClass", "Select Region",
        WS_POPUP | WS_VISIBLE,
        0, 0, sw, sh,
        NULL, NULL, GetModuleHandleA(NULL), NULL
    );
    SetLayeredWindowAttributes(overlay, RGB(0, 1, 0), 120, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(overlay, SW_SHOW);
    SetForegroundWindow(overlay);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return {
        std::min(s_start.x, s_end.x),
        std::min(s_start.y, s_end.y),
        abs(s_end.x - s_start.x),
        abs(s_end.y - s_start.y)
    };
}