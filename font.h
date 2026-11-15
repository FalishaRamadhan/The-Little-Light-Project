// font.h - 5x7 bitmap font drawn with rectangles (used for the title and the final message)
#pragma once
#include "cp2_transform.h"
#include <string>

inline const char* glyphRows(char c) {
    switch (c) {
        case 'A': return "01110100011000111111100011000110001";
        case 'B': return "11110100011000111110100011000111110";
        case 'C': return "01110100011000010000100001000101110";
        case 'D': return "11110100011000110001100011000111110";
        case 'E': return "11111100001000011110100001000011111";
        case 'F': return "11111100001000011110100001000010000";
        case 'G': return "01110100011000010111100011000101111";
        case 'H': return "10001100011000111111100011000110001";
        case 'I': return "01110001000010000100001000010001110";
        case 'J': return "00111000100001000010000101001001100";
        case 'K': return "10001100101010011000101001001010001";
        case 'L': return "10000100001000010000100001000011111";
        case 'M': return "10001110111010110101100011000110001";
        case 'N': return "10001110011010110011100011000110001";
        case 'O': return "01110100011000110001100011000101110";
        case 'P': return "11110100011000111110100001000010000";
        case 'Q': return "01110100011000110001101011001001101";
        case 'R': return "11110100011000111110101001001010001";
        case 'S': return "01111100001000001110000010000111110";
        case 'T': return "11111001000010000100001000010000100";
        case 'U': return "10001100011000110001100011000101110";
        case 'V': return "10001100011000110001100010101000100";
        case 'W': return "10001100011000110101101011010101010";
        case 'X': return "10001100010101000100010101000110001";
        case 'Y': return "10001100010101000100001000010000100";
        case 'Z': return "11111000010001000100010001000011111";
        case ',': return "00000000000000000000001100010001000";
        case '.': return "00000000000000000000000000110001100";
        default:  return "00000000000000000000000000000000000000000";
    }
}
// Draws text centred on cx. px = size of one font pixel in screen pixels (integer for crisp edges).
inline void drawText(Canvas& C, const std::string& s, float cx, float y, int px, Color col) {
    int w = (int)s.size() * 6 * px - px;
    int x0 = (int)std::lround(cx) - w / 2, yy = (int)std::lround(y);
    for (size_t i = 0; i < s.size(); i++) {
        const char* g = glyphRows(s[i]);
        for (int r = 0; r < 7; r++) {
            int c = 0;
            while (c < 5) {
                if (g[r * 5 + c] == '1') {
                    int c2 = c; while (c2 < 5 && g[r * 5 + c2] == '1') c2++;
                    C.rect((float)(x0 + (int)i * 6 * px + c * px), (float)(yy + r * px), (float)((c2 - c) * px), (float)px, col);
                    c = c2;
                } else c++;
            }
        }
    }
}
