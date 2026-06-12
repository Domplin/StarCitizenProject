#pragma once
#include "screencap.h"
#include <leptonica/allheaders.h>  
#include <string>
#include <vector>

class OcrEngine {
public:
    ~OcrEngine();
    bool init(const std::string& tessDataDir);
    std::string run(const std::vector<Pixel>& pixels, int w, int h);

private:
    PIX* pixelsToPix(const std::vector<Pixel>& pixels, int w, int h);
    void* m_api = nullptr;
};