#pragma once
#include "screencap.h"
#include "regionSelector.h"
#include <vector>

class ScreenCapture {
public:
    bool capture(const Region& r);
    void saveDebugBmp(const char* path) const;
    const std::vector<Pixel>& pixels() const { return m_pixels; }
    int width()  const { return m_width; }
    int height() const { return m_height; }

private:
    std::vector<Pixel> m_pixels;
    int m_width  = 0;
    int m_height = 0;
};