#include "ocr_engine.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <cstdio>

OcrEngine::~OcrEngine() {
    if (m_api) {
        static_cast<tesseract::TessBaseAPI*>(m_api)->End();
        delete static_cast<tesseract::TessBaseAPI*>(m_api);
    }
}

bool OcrEngine::init(const std::string& tessDataDir) {
    auto* api = new tesseract::TessBaseAPI();
    std::string tessdata = tessDataDir + "/tessdata";  // <-- add this
    if (api->Init(tessdata.c_str(), "eng", tesseract::OEM_LSTM_ONLY)) {  // <-- change tessDataDir.c_str() to tessdata.c_str()
        fprintf(stderr, "Tesseract init failed\n");
        delete api;
        return false;
    }
    api->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    m_api = api;
    return true;
}

PIX* OcrEngine::pixelsToPix(const std::vector<Pixel>& pixels, int w, int h) {
    PIX* pix = pixCreate(w, h, 32);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            const Pixel& p = pixels[y * w + x];
            pixSetPixel(pix, x, y, (p.r<<24)|(p.g<<16)|(p.b<<8)|0xFF);
        }
    return pix;
}

std::string OcrEngine::run(const std::vector<Pixel>& pixels, int w, int h) {
    PIX* pix       = pixelsToPix(pixels, w, h);
    PIX* pixScaled = pixScale(pix, 2.0f, 2.0f);
    pixDestroy(&pix);

    auto* api = static_cast<tesseract::TessBaseAPI*>(m_api);
    api->SetImage(pixScaled);
    char* raw = api->GetUTF8Text();
    std::string result(raw);
    delete[] raw;
    pixDestroy(&pixScaled);
    return result;
}