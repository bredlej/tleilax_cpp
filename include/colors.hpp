//
// Created by geoco on 01.05.2022.
//

#ifndef TLEILAX_COLORS_HPP
#define TLEILAX_COLORS_HPP

#include <raylib.h>
#include <array>

struct Colors {
    constexpr static Color col_0 = {0x2e, 0x2c, 0x3b, 0xff};
    constexpr static Color col_1 = {0x3e, 0x41, 0x5f, 0xff};
    constexpr static Color col_2 = {0x55, 0x60, 0x7d, 0xff};
    constexpr static Color col_3 = {0x74, 0x7d, 0x88, 0xff};
    constexpr static Color col_4 = {0x41, 0xde, 0x95, 0xff};
    constexpr static Color col_5 = {0x2a, 0xa4, 0xaa, 0xff};
    constexpr static Color col_6 = {0x3b, 0x77, 0xa6, 0xff};
    constexpr static Color col_7 = {0x24, 0x93, 0x37, 0xff};
    constexpr static Color col_8 = {0x56, 0xbe, 0x44, 0xff};
    constexpr static Color col_9 = {0xc6, 0xde, 0x78, 0xff};
    constexpr static Color col_10 = {0xf3, 0xc2, 0x20, 0xff};
    constexpr static Color col_11 = {0xc4, 0x65, 0x1c, 0xff};
    constexpr static Color col_12 = {0xb5, 0x41, 0x31, 0xff};
    constexpr static Color col_13 = {0x61, 0x40, 0x7a, 0xff};
    constexpr static Color col_14 = {0x8f, 0x3d, 0xa7, 0xff};
    constexpr static Color col_15 = {0xea, 0x61, 0x9d, 0xff};
    constexpr static Color col_16 = {0xc1, 0xe5, 0xea, 0xff};

    constexpr static std::array<Color, 17> all{col_0, col_1, col_2, col_3, col_4, col_5, col_6, col_7, col_8, col_9, col_10, col_11, col_12, col_13, col_14, col_15, col_16};
    //constexpr static std::array<Color, 8> star_colors{col_4, col_5, col_6, col_8, col_9, col_11, col_12, col_15};
    constexpr static std::array<Color, 8> star_colors{col_12, col_11, col_9, col_8, col_6, col_5, col_4, col_15};
};
#endif//TLEILAX_COLORS_HPP
