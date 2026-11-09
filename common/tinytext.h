#ifndef TINYTEXT_H
#define TINYTEXT_H

class GraphicViewPortClass;

enum { TINY_TEXT_HEIGHT = 5 };
enum TinyTextShadowType { TINY_TEXT_OUTLINE, TINY_TEXT_DROP_SHADOW };

// Fixed 3x5 digits and formation 'F', with an outline or lighter drop shadow. No game font
// assets or global font state; coordinates are relative to the clipped viewport.
int Tiny_Text_Width(char const* text);
void Draw_Tiny_Text(GraphicViewPortClass& view,
                    char const* text,
                    int x,
                    int y,
                    unsigned char color,
                    unsigned char shadow,
                    TinyTextShadowType style = TINY_TEXT_OUTLINE);

#endif
