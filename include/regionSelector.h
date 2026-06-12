#pragma once
#include <windows.h>

struct Region {int x, y, w, h; };

class RegionSelector {
public:
    Region select();
private:
    static LRESULT CALLBACK overlayProc(HWND, UINT, WPARAM, LPARAM);
    static POINT s_start, s_end;
    static bool s_selecting, s_done;
};