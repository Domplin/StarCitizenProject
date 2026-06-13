#include "regionSelector.h"
#include "screen_capture.h"
#include "ocr_engine.h"
#include "resource_matcher.h"
#include <cstdio>
#include <windows.h>

int main() {
    SetProcessDPIAware();
    printf("Click and drag over the mining UI, then release. ESC to cancel.\n\n");

    RegionSelector selector;
    Region r = selector.select();
    if (r.w < 10 || r.h < 10) {
        fprintf(stderr, "Selection too small or cancelled.\n");
        return 1;
    }
    printf("Selected: (%d,%d) %dx%d\n", r.x, r.y, r.w, r.h);

    ScreenCapture cap;
    if (!cap.capture(r)) return 1;
    cap.saveDebugBmp("debug.bmp");

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string tessDir(exePath);
    tessDir = tessDir.substr(0, tessDir.find_last_of("\\/"));

    OcrEngine ocr;
    if (!ocr.init(tessDir)) return 1;
    std::string text = ocr.run(cap.pixels(), cap.width(), cap.height());
    printf("\n--- Raw OCR output ---\n%s\n", text.c_str());

    ResourceMatcher matcher;
    auto matches = matcher.parse(text);
    printf("--- Identified resources ---\n");
    if (matches.empty()) {
        printf("  No RS values detected — check debug.bmp\n");
    } else {
        for (const auto& m : matches)
            printf("  RS %-6d  (n x %-2d)  ->  %s\n", m.rsValue, m.multiplier, m.name.c_str());
    }

    return 0;
}