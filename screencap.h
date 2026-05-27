// screencap.h — cross-platform screen region capture
#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>

struct Pixel { uint8_t r, g, b, a; };

// ─────────────────────────────────────────────
//  WINDOWS
// ─────────────────────────────────────────────
#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

inline std::vector<Pixel> captureScreenRegion(int x, int y, int width, int height) {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem    = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp  = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBmp);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

    BITMAPINFOHEADER bi{};
    bi.biSize        = sizeof(bi);
    bi.biWidth       = width;
    bi.biHeight      = -height; // top-down
    bi.biPlanes      = 1;
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;

    std::vector<Pixel> pixels(width * height);
    GetDIBits(hdcMem, hBmp, 0, height, pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // GDI returns BGRA — swap to RGBA
    for (auto& p : pixels) std::swap(p.r, p.b);

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    return pixels;
}

// ─────────────────────────────────────────────
//  MACOS
// ─────────────────────────────────────────────
#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>

inline std::vector<Pixel> captureScreenRegion(int x, int y, int width, int height) {
    CGRect region = CGRectMake(x, y, width, height);
    CGImageRef img = CGWindowListCreateImage(region,
        kCGWindowListOptionOnScreenOnly, kCGNullWindowID, kCGWindowImageDefault);

    if (!img) throw std::runtime_error("CGWindowListCreateImage failed — check Screen Recording permission");

    CGDataProviderRef provider = CGImageGetDataProvider(img);
    CFDataRef data = CGDataProviderCopyData(provider);
    const uint8_t* raw = CFDataGetBytePtr(data);
    size_t bytesPerRow  = CGImageGetBytesPerRow(img);

    std::vector<Pixel> pixels(width * height);
    for (int j = 0; j < height; ++j)
        for (int i = 0; i < width; ++i) {
            const uint8_t* p = raw + j * bytesPerRow + i * 4;
            pixels[j * width + i] = { p[0], p[1], p[2], p[3] };
        }

    CFRelease(data);
    CGImageRelease(img);
    return pixels;
}

// ─────────────────────────────────────────────
//  LINUX (X11)
// ─────────────────────────────────────────────
#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

inline std::vector<Pixel> captureScreenRegion(int x, int y, int width, int height) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) throw std::runtime_error("Cannot open X11 display");

    Window root = DefaultRootWindow(display);
    XImage* img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
    if (!img) {
        XCloseDisplay(display);
        throw std::runtime_error("XGetImage failed");
    }

    std::vector<Pixel> pixels(width * height);
    for (int j = 0; j < height; ++j)
        for (int i = 0; i < width; ++i) {
            unsigned long px = XGetPixel(img, i, j);
            pixels[j * width + i] = {
                (uint8_t)((px & img->red_mask)   >> 16),
                (uint8_t)((px & img->green_mask) >> 8),
                (uint8_t)((px & img->blue_mask)  >> 0),
                255
            };
        }

    XDestroyImage(img);
    XCloseDisplay(display);
    return pixels;
}

#else
#error "Unsupported platform"
#endif