
#include "screencap.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include <windows.h>


// ── Region selector ──────────────────────────────────────────────────────────
struct Region { int x, y, w, h; };

POINT  g_start, g_end;
bool   g_selecting = false;
bool   g_done      = false;
HWND   g_overlay   = NULL;

LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_LBUTTONDOWN:
        g_start = { LOWORD(lp), HIWORD(lp) };
        g_end   = g_start;
        g_selecting = true;
        SetCapture(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        if (g_selecting) {
            g_end = { LOWORD(lp), HIWORD(lp) };
            InvalidateRect(hwnd, NULL, FALSE); // trigger repaint
        }
        return 0;

    case WM_LBUTTONUP:
        if (g_selecting) {
            g_end = { LOWORD(lp), HIWORD(lp) };
            g_selecting = false;
            g_done      = true;
            ReleaseCapture();
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Semi-transparent dark overlay
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH dimBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, dimBrush);
        DeleteObject(dimBrush);

        // Draw selection rectangle in bright green
        if (g_selecting || g_done) {
            RECT sel = {
                std::min(g_start.x, g_end.x), std::min(g_start.y, g_end.y),
                std::max(g_start.x, g_end.x), std::max(g_start.y, g_end.y)

            };
            // Clear the selected area so you can see through it
            HBRUSH clearBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &sel, clearBrush);
            DeleteObject(clearBrush);

            // Green border
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
            HPEN old = (HPEN)SelectObject(hdc, pen);
            Rectangle(hdc, sel.left, sel.top, sel.right, sel.bottom);
            SelectObject(hdc, old);
            DeleteObject(pen);

            // Show dimensions
            char buf[64];
            sprintf(buf, "%dx%d", abs(g_end.x - g_start.x), abs(g_end.y - g_start.y));
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, sel.left + 4, sel.top + 4, buf, strlen(buf));
        }

        // Instructions
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 20, 20,
            "Click and drag to select the mining UI region. Release to capture.", 67);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) DestroyWindow(hwnd); // ESC to cancel
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

Region selectRegion() {
    // Register overlay window class
    WNDCLASSA wc    = {};
    wc.lpfnWndProc  = OverlayProc;
    wc.hInstance    = GetModuleHandleA(NULL);
    wc.lpszClassName = "OverlayClass";
    wc.hCursor      = LoadCursor(NULL, IDC_CROSS); // crosshair cursor
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    RegisterClassA(&wc);

    // Full screen overlay
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    g_overlay = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        "OverlayClass", "Select Region",
        WS_POPUP | WS_VISIBLE,
        0, 0, sw, sh,
        NULL, NULL, GetModuleHandleA(NULL), NULL
    );

    // 120/255 opacity — dark but still see-through enough
    SetLayeredWindowAttributes(g_overlay, 0, 120, LWA_ALPHA);
    ShowWindow(g_overlay, SW_SHOW);
    SetForegroundWindow(g_overlay);

    // Message loop
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    Region r = {
        std::min(g_start.x, g_end.x),
        std::min(g_start.y, g_end.y),
        abs(g_end.x - g_start.x),
        abs(g_end.y - g_start.y)
    };
    return r;
}

// ── Known Star Citizen mining resources ─────────────────────────────────────
static const std::vector<std::string> KNOWN_RESOURCES = {
    "Quantanium", "Bexalite", "Taranite", "Borase", "Stileron",
    "Aphorite", "Hadanite", "Janalite", "Dolivine", "Felsic",
    "Laranite", "Agricium", "Gold", "Titanium", "Tungsten",
    "Silicon", "Quartz", "Diamond", "Copper", "Iron",
    "Corundum", "Beryl", "Ice", "Inertite", "Orthoclase"
};

int editDistance(const std::string& a, const std::string& b) {
    int m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            dp[i][j] = (tolower(a[i-1]) == tolower(b[j-1]))
                ? dp[i-1][j-1]
                : 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
    return dp[m][n];
}

std::string matchResource(const std::string& word) {
    std::string best; int bestDist = 999;
    for (const auto& res : KNOWN_RESOURCES) {
        int d = editDistance(word, res);
        if (d < bestDist) { bestDist = d; best = res; }
    }
    return (bestDist <= 4) ? best : "";
}

PIX* pixelsToPix(const std::vector<Pixel>& pixels, int width, int height) {
    PIX* pix = pixCreate(width, height, 32);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            const Pixel& p = pixels[y * width + x];
            pixSetPixel(pix, x, y, (p.r<<24)|(p.g<<16)|(p.b<<8)|0xFF);
        }
    return pix;
}

int main() {
    SetProcessDPIAware();
    printf("A full-screen overlay will open.\n");
    printf("Click and drag over the mining UI, then release.\n");
    printf("Press ESC to cancel.\n\n");

    // ── 1. Interactive region select ─────────────────────────────────────────
    Region r = selectRegion();
    if (r.w < 10 || r.h < 10) {
        fprintf(stderr, "Selection too small or cancelled.\n");
        return 1;
    }
    printf("Selected: (%d,%d) %dx%d\n", r.x, r.y, r.w, r.h);

    // ── 2. Capture ───────────────────────────────────────────────────────────
    std::vector<Pixel> pixels;
    try {
        pixels = captureScreenRegion(r.x, r.y, r.w, r.h);
    } catch (const std::exception& e) {
        fprintf(stderr, "Capture error: %s\n", e.what());
        return 1;
    }

    // ── 3. Save debug BMP ────────────────────────────────────────────────────
    {
        int w = r.w, h = r.h;
        int rowSize = (w * 3 + 3) & ~3;
        std::vector<uint8_t> bmp(54 + rowSize * h, 0);
        bmp[0]='B'; bmp[1]='M';
        uint32_t fs = 54 + rowSize * h; memcpy(&bmp[2], &fs, 4);
        bmp[10]=54;
        uint32_t hs=40; memcpy(&bmp[14],&hs,4);
        memcpy(&bmp[18],&w,4); memcpy(&bmp[22],&h,4);
        bmp[26]=1; bmp[28]=24;
        for (int j=0;j<h;j++) {
            int br=h-1-j;
            for (int i=0;i<w;i++) {
                const Pixel& p=pixels[j*w+i];
                int off=54+br*rowSize+i*3;
                bmp[off]=p.b; bmp[off+1]=p.g; bmp[off+2]=p.r;
            }
        }
        FILE* f=fopen("debug.bmp","wb");
        fwrite(bmp.data(),1,bmp.size(),f);
        fclose(f);
        printf("Saved debug.bmp\n");
    }

    // ── 4. OCR ───────────────────────────────────────────────────────────────
    PIX* pix = pixelsToPix(pixels, r.w, r.h);
    PIX* pixScaled = pixScale(pix, 2.0f, 2.0f);
    pixDestroy(&pix);

    tesseract::TessBaseAPI ocr;
    if (ocr.Init(".", "eng", tesseract::OEM_LSTM_ONLY)) {
        fprintf(stderr, "Tesseract init failed — is tessdata/eng.traineddata present?\n");
        return 1;
    }
    ocr.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    ocr.SetImage(pixScaled);

    char* rawText = ocr.GetUTF8Text();
    std::string text(rawText);
    delete[] rawText;
    ocr.End();
    pixDestroy(&pixScaled);

    printf("\n── Raw OCR output ──\n%s\n", text.c_str());

    // ── 5. Match resources ───────────────────────────────────────────────────
    printf("── Identified resources ──\n");
    std::string word, lastResource;
    bool foundAny = false;
    text += " ";
    for (char c : text) {
        if (isalnum(c) || c=='.' || c=='%') {
            word += c;
        } else {
            if (!word.empty()) {
                bool isNum = isdigit(word[0]) || word[0]=='.';
                if (isNum && !lastResource.empty()) {
                    printf("  %-15s : %s\n", lastResource.c_str(), word.c_str());
                    lastResource.clear();
                    foundAny = true;
                } else if (!isNum && word.size()>=3) {
                    std::string match = matchResource(word);
                    if (!match.empty()) lastResource = match;
                }
                word.clear();
            }
        }
    }

    if (!foundAny)
        printf("  No resources detected — check debug.bmp\n");

    return 0;
}