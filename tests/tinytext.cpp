#include "common/gbuffer.h"
#include "common/tinytext.h"
#include "common/wwkeyboard.h"
#include <cstdio>
#include <cstring>

// Globals/stubs needed by the software GraphicBufferClass test environment.
bool GameInFocus;
int ScreenWidth;
int WindowList[9][9];
char* _ShapeBuffer = 0;
WWKeyboardClass* Keyboard;
void Process_Network() {}
void Focus_Restore() {}
void Focus_Loss() {}
int Open_File(char const*, int) { return 0; }
void Close_File(int) {}
int Read_File(int, void*, unsigned int) { return 0; }

int main()
{
    int failures = 0;
    auto check = [&](bool condition, char const* message) {
        if (!condition) {
            std::fprintf(stderr, "%s\n", message);
            failures++;
        }
    };
    check(Tiny_Text_Width(0) == 0, "Null text must have zero width");
    check(Tiny_Text_Width("") == 0, "Empty text must have zero width");
    check(Tiny_Text_Width("1") == 3, "A digit must be three pixels wide");
    check(Tiny_Text_Width("1F") == 7, "A formation label must have one-pixel glyph spacing");
    check(TINY_TEXT_HEIGHT == 5, "Glyphs must be exactly five pixels tall");

    GraphicBufferClass buffer(16, 16);
    check(buffer.Lock(), "Test buffer must lock");
    std::memset(buffer.Get_Buffer(), 9, 16 * 16);
    Draw_Tiny_Text(buffer, "1", 4, 4, 7, 1);
    static unsigned char const one[] = {2, 6, 2, 2, 7};
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if (one[row] & (4 >> col)) {
                check(buffer.Get_Pixel(4 + col, 4 + row) == 7, "Foreground must match the 3x5 glyph");
            }
        }
    }
    check(buffer.Get_Pixel(4, 3) == 1, "Digit must have a contrasting one-pixel outline");
    check(buffer.Get_Pixel(2, 2) == 9, "Text must not touch pixels beyond the outline");

    for (char digit = '0'; digit <= '9'; digit++) {
        std::memset(buffer.Get_Buffer(), 9, 16 * 16);
        char text[] = {digit, 'F', '\0'};
        Draw_Tiny_Text(buffer, text, 4, 4, 7, 1);
        int foreground = 0;
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                if (buffer.Get_Pixel(x, y) == 7) {
                    foreground++;
                    check(x >= 4 && x <= 10 && y >= 4 && y <= 8, "Foreground must remain inside glyph bounds");
                }
            }
        }
        check(foreground > 0, "Every group digit and formation suffix must render");
    }

    std::memset(buffer.Get_Buffer(), 9, 16 * 16);
    Draw_Tiny_Text(buffer, "8", 4, 4, 7, 1, TINY_TEXT_DROP_SHADOW);
    check(buffer.Get_Pixel(4, 3) == 9, "Lighter group labels must have no top outline");
    check(buffer.Get_Pixel(3, 4) == 9, "Lighter group labels must have no left outline");
    check(buffer.Get_Pixel(7, 9) == 1, "Lighter group labels must keep a lower-right shadow");
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (x < 4 || x >= 8 || y < 4 || y >= 10) {
                check(buffer.Get_Pixel(x, y) == 9, "Group digit including shadow must fit in 4x6 pixels");
            }
        }
    }

    std::memset(buffer.Get_Buffer(), 9, 16 * 16);
    GraphicViewPortClass clipped(&buffer, 5, 5, 4, 4);
    Draw_Tiny_Text(clipped, "8F", -1, -1, 7, 1);
    Draw_Tiny_Text(clipped, "1", 3, 3, 7, 1);
    Draw_Tiny_Text(clipped, "8F", -1, -1, 7, 1, TINY_TEXT_DROP_SHADOW);
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (x < 5 || x >= 9 || y < 5 || y >= 9) {
                check(buffer.Get_Pixel(x, y) == 9, "Clipped text must not overwrite the sidebar or neighbouring cells");
            }
        }
    }
    std::memset(buffer.Get_Buffer(), 9, 16 * 16);
    Draw_Tiny_Text(clipped, "1", -100, -100, 7, 1);
    Draw_Tiny_Text(clipped, "1", 100, 100, 7, 1);
    Draw_Tiny_Text(clipped, 0, 0, 0, 7, 1);
    for (int i = 0; i < 16 * 16; i++) {
        check(((unsigned char*)buffer.Get_Buffer())[i] == 9, "Off-screen/null text must leave the buffer untouched");
    }
    buffer.Unlock();
    return failures != 0;
}
