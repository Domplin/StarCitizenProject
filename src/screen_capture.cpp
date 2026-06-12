

#include "screen_capture.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

bool ScreenCapture::capture(const Region& r){
    try{
        m_pixels = captureScreenRegion(r.x, r.y, r.w, r.h);
        m_width = r.w;
        m_height = r.h;
        return true;
    } catch (const std::exception& e){
        fprintf(stderr, "Capture error: %s\n", e.what());
        return false;
    }
}



void ScreenCapture::saveDebugBmp(const char* path) const {
    int rowSize = (m_width * 3 + 3) & ~3;
    std::vector<uint8_t> bmp(54 + rowSize * m_height, 0);
    bmp[0] = 'B'; bmp[1]='M';
    uint32_t fs = 54 + rowSize * m_height;
    memcpy(&bmp[2], &fs, 4);
    bmp[10] = 54;
    uint32_t hs = 40; memcpy(&bmp[14], &hs, 4);
    memcpy(&bmp[18], &m_width,  4);
    memcpy(&bmp[22], &m_height, 4);
    bmp[26]=1; bmp[28]=24;
    for (int j = 0; j < m_height; j++) {
        int br = m_height - 1 - j;
        for (int i = 0; i < m_width; i++) {
            const Pixel& p = m_pixels[j * m_width + i];
            int off = 54 + br * rowSize + i * 3;
            bmp[off]=p.b; bmp[off+1]=p.g; bmp[off+2]=p.r;
        }
    }
    FILE* f = fopen(path, "wb");
    fwrite(bmp.data(), 1, bmp.size(), f);
    fclose(f);
    printf("Saved %s\n", path);
}
