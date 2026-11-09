#include "tinytext.h"
#include "graphicsviewport.h"
#include "drawbuff.h"

static unsigned char const Glyphs[][TINY_TEXT_HEIGHT] = {
    {7, 5, 5, 5, 7}, // 0
    {2, 6, 2, 2, 7}, // 1
    {7, 1, 7, 4, 7}, // 2
    {7, 1, 7, 1, 7}, // 3
    {5, 5, 7, 1, 1}, // 4
    {7, 4, 7, 1, 7}, // 5
    {7, 4, 7, 5, 7}, // 6
    {7, 1, 2, 2, 2}, // 7
    {7, 5, 7, 5, 7}, // 8
    {7, 5, 7, 1, 7}, // 9
    {7, 4, 6, 4, 4}, // F
};

int Tiny_Text_Width(char const* text)
{
    int width = 0;
    if (text) {
        while (*text++) {
            width += 4;
        }
    }
    return width ? width - 1 : 0;
}

void Draw_Tiny_Text(GraphicViewPortClass& view,
                    char const* text,
                    int x,
                    int y,
                    unsigned char color,
                    unsigned char shadow,
                    TinyTextShadowType style)
{
    if (!text || !*text || !view.Lock()) {
        return;
    }

    // Paint all shadows first so neighbouring glyphs cannot erase each other.
    for (int pass = 0; pass < 2; pass++) {
        int xx = x;
        for (char const* p = text; *p; p++, xx += 4) {
            int glyph = (*p >= '0' && *p <= '9') ? *p - '0' : (*p == 'F' ? 10 : -1);
            if (glyph < 0) {
                continue;
            }
            int first = pass == 0 ? (style == TINY_TEXT_DROP_SHADOW ? 1 : -1) : 0;
            int last = pass == 0 ? 1 : 0;
            for (int row = 0; row < TINY_TEXT_HEIGHT; row++) {
                for (int col = 0; col < 3; col++) {
                    if (!(Glyphs[glyph][row] & (4 >> col))) {
                        continue;
                    }
                    for (int dy = first; dy <= last; dy++) {
                        for (int dx = first; dx <= last; dx++) {
                            int px = xx + col + dx;
                            int py = y + row + dy;
                            if (px >= 0 && py >= 0 && px < view.Get_Width() && py < view.Get_Height()) {
                                Buffer_Put_Pixel(&view, px, py, pass == 0 ? shadow : color);
                            }
                        }
                    }
                }
            }
        }
    }
    view.Unlock();
}
