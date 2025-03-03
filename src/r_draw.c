/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2021 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is a part of DOOM Retro.

  DOOM Retro is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM Retro is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries, and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "i_colors.h"
#include "m_config.h"
#include "m_random.h"
#include "r_local.h"
#include "st_stuff.h"
#include "v_video.h"

#define NOFUZZ  251

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about coordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

int         viewwidth;
int         viewheight;
int         viewwindowx;
int         viewwindowy;

int         fuzzrange[3];
int         fuzzpos;
int         fuzztable[MAXSCREENAREA];

static byte *ylookup0[MAXHEIGHT];
static byte *ylookup1[MAXHEIGHT];

static const byte redtoblue[] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

static const byte redtogreen[] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    118, 118, 119, 119, 120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

lighttable_t    *dc_colormap[2];
lighttable_t    *dc_nextcolormap[2];
int             dc_x;
int             dc_yl;
int             dc_yh;
int             dc_z;
fixed_t         dc_iscale;
fixed_t         dc_texturemid;
fixed_t         dc_texheight;
fixed_t         dc_texturefrac;
byte            dc_solidblood;
byte            *dc_blood;
byte            *dc_brightmap;
int             dc_floorclip;
int             dc_ceilingclip;
int             dc_numposts;
byte            dc_black;
byte            *dc_black33;
byte            *dc_black40;
byte            *dc_source;

#define         DITHERSIZE  8

static const byte dithermatrix[DITHERSIZE][DITHERSIZE] =
{
    {   0,   0, 224, 224,  48,  48, 208, 208 },
    {   0,   0, 224, 224,  48,  48, 208, 208 },
    { 176, 176,  80,  80, 128, 128,  96,  96 },
    { 176, 176,  80,  80, 128, 128,  96,  96 },
    { 192, 192,  32,  32, 240, 240,  16,  16 },
    { 192, 192,  32,  32, 240, 240,  16,  16 },
    { 112, 112, 144, 144,  64,  64, 160, 160 },
    { 112, 112, 144, 144,  64,  64, 160, 160 }
};

#define dither(x, y, intensity) \
    (dithermatrix[((y) << r_detail) & (DITHERSIZE - 1)][((x) << r_detail) & (DITHERSIZE - 1)] < (intensity))

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z-depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

void R_DrawColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = colormap[dc_source[frac >> FRACBITS]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dc_source[frac >> FRACBITS]];
}

void R_DrawDitherColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]];
}

void R_DrawCorrectedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = colormap[nearestcolors[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[nearestcolors[dc_source[frac >> FRACBITS]]];
}

void R_DrawCorrectedDitherColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][nearestcolors[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dither(dc_x, y, fracz)][nearestcolors[dc_source[frac >> FRACBITS]]];
}

void R_DrawColorColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  color = dc_colormap[0][NOTEXTURECOLOR];

    while (--count)
    {
        *dest = color;
        dest += SCREENWIDTH;
    }

    *dest = color;
}

void R_DrawColorDitherColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][NOTEXTURECOLOR];
        dest += SCREENWIDTH;
    }

    *dest = colormap[dither(dc_x, y, fracz)][NOTEXTURECOLOR];
}

void R_DrawShadowColumn(void)
{
    int     count = dc_yh - dc_yl;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    if (!count)
        *dest = *(*dest + dc_black33);
    else if (count == 1)
    {
        *dest = *(*dest + dc_black33);
        dest += SCREENWIDTH;
        *dest = *(*dest + dc_black33);
    }
    else
    {
        *dest = *(*dest + dc_black33);
        dest += SCREENWIDTH;

        while (--count)
        {
            *dest = *(*dest + dc_black40);
            dest += SCREENWIDTH;
        }

        *dest = *(*dest + (dc_yh == dc_floorclip ? dc_black40 : dc_black33));
    }
}

void R_DrawFuzzyShadowColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    if (((consoleactive || freeze) && !fuzztable[fuzzpos++])
        || (!consoleactive && !freeze && !(M_BigRandom() & 3)))
        *dest = *(*dest + dc_black33);

    dest += SCREENWIDTH;

    while (--count)
    {
        *dest = *(*dest + dc_black33);
        dest += SCREENWIDTH;
    }

    if (dc_yh < dc_floorclip
        && (((consoleactive || freeze) && !fuzztable[fuzzpos++])
            || (!consoleactive && !freeze && !(M_BigRandom() & 3))))
        *dest = *(*dest + dc_black33);
}

void R_DrawSolidShadowColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_black;
        dest += SCREENWIDTH;
    }

    *dest = dc_black;
}

void R_DrawSolidFuzzyShadowColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    if (((consoleactive || freeze) && !fuzztable[fuzzpos++])
        || (!consoleactive && !freeze && !(M_BigRandom() & 3)))
        *dest = dc_black;

    dest += SCREENWIDTH;

    while (--count)
    {
        *dest = dc_black;
        dest += SCREENWIDTH;
    }

    if (dc_yh < dc_floorclip
        && (((consoleactive || freeze) && !fuzztable[fuzzpos++])
            || (!consoleactive && !freeze && !(M_BigRandom() & 3))))
        *dest = dc_black;
}

void R_DrawBloodSplatColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = *(*dest + dc_blood);
        dest += SCREENWIDTH;
    }

    *dest = *(*dest + dc_blood);
}

void R_DrawSolidBloodSplatColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;

    while (--count)
    {
        *dest = dc_solidblood;
        dest += SCREENWIDTH;
    }

    *dest = dc_solidblood;
}

void R_DrawWallColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];
    fixed_t             heightmask = dc_texheight - 1;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            *dest = colormap[dc_source[frac >> FRACBITS]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = colormap[dc_source[frac >> FRACBITS]];
    }
    else
    {
        while (--count)
        {
            *dest = colormap[dc_source[(frac >> FRACBITS) & heightmask]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        *dest = colormap[dc_source[(frac >> FRACBITS) & heightmask]];
    }
}

void R_DrawDitherWallColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturemid + (y - centery) * dc_iscale;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    fixed_t             heightmask = dc_texheight - 1;
    const int           fracz = ((dc_z >> 5) & 255);

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            *dest = colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        *dest = colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]];
    }
    else
    {
        while (--count)
        {
            *dest = colormap[dither(dc_x, y++, fracz)][dc_source[(frac >> FRACBITS) & heightmask]];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        *dest = colormap[dither(dc_x, y, fracz)][dc_source[(frac >> FRACBITS) & heightmask]];
    }
}

void R_DrawBrightmapWallColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup0[dc_yl] + dc_x;
    fixed_t frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    fixed_t heightmask = dc_texheight - 1;
    byte    dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            dot = dc_source[frac >> FRACBITS];
            *dest = dc_colormap[dc_brightmap[dot]][dot];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = dc_source[frac >> FRACBITS];
        *dest = dc_colormap[dc_brightmap[dot]][dot];
    }
    else
    {
        while (--count)
        {
            dot = dc_source[(frac >> FRACBITS) & heightmask];
            *dest = dc_colormap[dc_brightmap[dot]][dot];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        dot = dc_source[(frac >> FRACBITS) & heightmask];
        *dest = dc_colormap[dc_brightmap[dot]][dot];
    }
}

void R_DrawBrightmapDitherWallColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturemid + (y - centery) * dc_iscale;
    const lighttable_t  *colormap[2][2] = { { dc_colormap[0], dc_nextcolormap[0] }, { fullcolormap, fullcolormap } };
    fixed_t             heightmask = dc_texheight - 1;
    const int           fracz = ((dc_z >> 5) & 255);
    byte                dot;

    if (dc_texheight & heightmask)
    {
        heightmask = (heightmask + 1) << FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        while (--count)
        {
            dot = dc_source[frac >> FRACBITS];
            *dest = colormap[dc_brightmap[dot]][dither(dc_x, y++, fracz)][dot];
            dest += SCREENWIDTH;

            if ((frac += dc_iscale) >= heightmask)
                frac -= heightmask;
        }

        dot = dc_source[frac >> FRACBITS];
        *dest = colormap[dc_brightmap[dot]][dither(dc_x, y, fracz)][dot];
    }
    else
    {
        while (--count)
        {
            dot = dc_source[(frac >> FRACBITS) & heightmask];
            *dest = colormap[dc_brightmap[dot]][dither(dc_x, y++, fracz)][dot];
            dest += SCREENWIDTH;
            frac += dc_iscale;
        }

        dot = dc_source[(frac >> FRACBITS) & heightmask];
        *dest = colormap[dc_brightmap[dot]][dither(dc_x, y, fracz)][dot];
    }
}

void R_DrawPlayerSpriteColumn(void)
{
    int     count = dc_yh - dc_yl + 1;
    byte    *dest = ylookup1[dc_yl] + dc_x;
    fixed_t frac = dc_texturefrac;

    while (--count)
    {
        *dest = dc_source[frac >> FRACBITS];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = dc_source[frac >> FRACBITS];
}

void R_DrawFlippedSkyColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturemid + (dc_yl - centery) * dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];
    fixed_t             i;

    while (--count)
    {
        *dest = colormap[dc_source[((i = frac >> FRACBITS) < 128 ? i : 126 - (i & 127))]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dc_source[((i = frac >> FRACBITS) < 128 ? i : 126 - (i & 127))]];
}

void R_DrawSkyColorColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  color = dc_colormap[0][r_skycolor];

    while (--count)
    {
        *dest = color;
        dest += SCREENWIDTH;
    }

    *dest = color;
}

void R_DrawRedToBlueColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = colormap[redtoblue[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[redtoblue[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherRedToBlueColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][redtoblue[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dither(dc_x, y, fracz)][redtoblue[dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedToBlue33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[redtoblue[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[redtoblue[dc_source[frac >> FRACBITS]]]];
}

void R_DrawDitherTranslucentRedToBlue33Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][redtoblue[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y, fracz)][redtoblue[dc_source[frac >> FRACBITS]]]];
}

void R_DrawRedToGreenColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = colormap[redtogreen[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[redtogreen[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherRedToGreenColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][redtogreen[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dither(dc_x, y, fracz)][redtogreen[dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedToGreen33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[redtogreen[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[redtogreen[dc_source[frac >> FRACBITS]]]];
}

void R_DrawDitherTranslucentRedToGreen33Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][redtogreen[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y, fracz)][redtogreen[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucentColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabadditive[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabadditive[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabadditive[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabadditive[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac + SPARKLEFIX;
    const fixed_t       fracstep = dc_iscale - SPARKLEFIX;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }

    *dest = tranmap[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucent50Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac + SPARKLEFIX;
    const fixed_t       fracstep = dc_iscale - SPARKLEFIX;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }

    *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawCorrectedTranslucent50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const fixed_t       fracstep = dc_iscale;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + colormap[nearestcolors[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }

    *dest = tranmap[(*dest << 8) + colormap[nearestcolors[dc_source[frac >> FRACBITS]]]];
}

void R_DrawCorrectedDitherTranslucent50Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac + SPARKLEFIX;
    const fixed_t       fracstep = dc_iscale - SPARKLEFIX;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][nearestcolors[dc_source[frac >> FRACBITS]]]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }

    *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y, fracz)][nearestcolors[dc_source[frac >> FRACBITS]]]];
}

void R_DrawTranslucent50ColorColumn(void)
{
    int         count = dc_yh - dc_yl + 1;
    byte        *dest = ylookup0[dc_yl] + dc_x;
    const byte  color = dc_colormap[0][NOTEXTURECOLOR];

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + color];
        dest += SCREENWIDTH;
    }

    *dest = tranmap[(*dest << 8) + color];
}

void R_DrawDitherTranslucent50ColorColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][NOTEXTURECOLOR]];
        dest += SCREENWIDTH;
    }

    *dest = tranmap[(*dest << 8) + colormap[dither(dc_x, y, fracz)][NOTEXTURECOLOR]];
}

void R_DrawTranslucent33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucent33Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttab33[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabred[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabred[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentRedColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabred[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabred[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedWhiteColumn1(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabredwhite1[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite1[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentRedWhiteColumn1(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabredwhite1[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite1[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedWhiteColumn2(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabredwhite2[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite2[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentRedWhiteColumn2(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabredwhite2[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite2[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRedWhite50Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabredwhite50[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite50[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentRedWhite50Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabredwhite50[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabredwhite50[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentGreenColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabgreen[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabgreen[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentGreenColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabgreen[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabgreen[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentBlueColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabblue[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabblue[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentBlueColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabblue[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabblue[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentRed33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabred33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabred33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentRed33Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabred33[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabred33[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentGreen33Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabgreen33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabgreen33[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentGreen33Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabgreen33[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabgreen33[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

void R_DrawTranslucentBlue25Column(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = tinttabblue25[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabblue25[(*dest << 8) + colormap[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslucentBlue25Column(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = tinttabblue25[(*dest << 8) + colormap[dither(dc_x, y++, fracz)][dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = tinttabblue25[(*dest << 8) + colormap[dither(dc_x, y, fracz)][dc_source[frac >> FRACBITS]]];
}

//
// Spectre/Invisibility.
//
void R_DrawFuzzColumn(void)
{
    byte    *dest;
    int     count = dc_yh - dc_yl;

    if (!count)
        return;

    dest = ylookup0[dc_yl] + dc_x;

    // top
    if (!dc_yl)
        *dest = fullcolormap[6 * 256 + dest[(fuzztable[fuzzpos++] = FUZZ(0, 1))]];
    else if (!(M_BigRandom() & 3))
        *dest = fullcolormap[12 * 256 + dest[(fuzztable[fuzzpos++] = FUZZ(-1, 1))]];

    dest += SCREENWIDTH;

    while (--count)
    {
        // middle
        *dest = fullcolormap[6 * 256 + dest[(fuzztable[fuzzpos++] = FUZZ(-1, 1))]];
        dest += SCREENWIDTH;
    }

    // bottom
    *dest = fullcolormap[5 * 256 + dest[(fuzztable[fuzzpos++] = FUZZ(-1, 0))]];

    if (dc_yh < dc_floorclip && !(M_BigRandom() & 3))
    {
        dest += SCREENWIDTH;
        *dest = fullcolormap[14 * 256 + dest[(fuzztable[fuzzpos++] = FUZZ(-1, 0))]];
    }
}

void R_DrawPausedFuzzColumn(void)
{
    byte    *dest;
    int     count = dc_yh - dc_yl;

    if (!count)
        return;

    dest = ylookup0[dc_yl] + dc_x;

    // top
    if (!dc_yl)
        *dest = fullcolormap[6 * 256 + dest[MAX(0, fuzztable[fuzzpos++])]];
    else if (!fuzztable[fuzzpos++])
        *dest = fullcolormap[12 * 256 + dest[fuzztable[fuzzpos++]]];

    dest += SCREENWIDTH;

    while (--count)
    {
        // middle
        *dest = fullcolormap[6 * 256 + dest[fuzztable[fuzzpos++]]];
        dest += SCREENWIDTH;
    }

    // bottom
    *dest = fullcolormap[5 * 256 + dest[MIN(fuzztable[fuzzpos++], 0)]];

    if (dc_yh < dc_floorclip && !fuzztable[fuzzpos++])
    {
        dest += SCREENWIDTH;
        *dest = fullcolormap[12 * 256 + dest[fuzztable[fuzzpos++]]];
    }
}

void R_DrawFuzzColumns(void)
{
    const int   w = viewwindowx + viewwidth;
    const int   h = (viewwindowy + viewheight) * SCREENWIDTH;

    for (int x = viewwindowx; x < w; x++)
        for (int y = viewwindowy * SCREENWIDTH; y < h; y += SCREENWIDTH)
        {
            const int   i = x + y;
            byte        *src = screens[1] + i;

            if (*src != NOFUZZ)
            {
                byte    *dest = screens[0] + i;

                if (!y || *(src - SCREENWIDTH) == NOFUZZ)
                {
                    // top
                    if (!(M_BigRandom() & 3))
                        *dest = fullcolormap[12 * 256 + dest[(fuzztable[i] = FUZZ(-1, 1))]];
                }
                else if (y == h - SCREENWIDTH)
                {
                    // bottom of view
                    *dest = fullcolormap[5 * 256 + dest[(fuzztable[i] = FUZZ(-1, 0))]];
                }
                else if (*(src + SCREENWIDTH) == NOFUZZ)
                {
                    // bottom of post
                    if (!(M_BigRandom() & 3))
                        *dest = fullcolormap[12 * 256 + dest[(fuzztable[i] = FUZZ(-1, 1))]];
                }
                else
                {
                    // middle
                    if (*(src - 1) == NOFUZZ || *(src + 1) == NOFUZZ)
                    {
                        if (!(M_BigRandom() & 3))
                            *dest = fullcolormap[12 * 256 + dest[(fuzztable[i] = FUZZ(-1, 1))]];
                    }
                    else
                        *dest = fullcolormap[6 * 256 + dest[(fuzztable[i] = FUZZ(-1, 1))]];
                }
            }
        }
}

void R_DrawPausedFuzzColumns(void)
{
    const int   w = viewwindowx + viewwidth;
    const int   h = (viewwindowy + viewheight) * SCREENWIDTH;

    for (int x = viewwindowx; x < w; x++)
        for (int y = viewwindowy * SCREENWIDTH; y < h; y += SCREENWIDTH)
        {
            const int   i = x + y;
            byte        *src = screens[1] + i;

            if (*src != NOFUZZ)
            {
                byte    *dest = screens[0] + i;

                if (!y || *(src - SCREENWIDTH) == NOFUZZ)
                {
                    // top
                    if (!fuzztable[i])
                        *dest = fullcolormap[12 * 256 + dest[fuzztable[i]]];
                }
                else if (y == h - SCREENWIDTH)
                {
                    // bottom of view
                    *dest = fullcolormap[5 * 256 + dest[fuzztable[i]]];
                }
                else if (*(src + SCREENWIDTH) == NOFUZZ)
                {
                    // bottom of post
                    if (!fuzztable[i])
                        *dest = fullcolormap[12 * 256 + dest[fuzztable[i]]];
                }
                else
                {
                    // middle
                    if (*(src - 1) == NOFUZZ || *(src + 1) == NOFUZZ)
                    {
                        if (!fuzztable[i])
                            *dest = fullcolormap[12 * 256 + dest[fuzztable[i]]];
                    }
                    else
                        *dest = fullcolormap[6 * 256 + dest[fuzztable[i]]];
                }
            }
        }
}

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//
byte    *dc_translation;
byte    translationtables[256 * 3];

void R_DrawTranslatedColumn(void)
{
    int                 count = dc_yh - dc_yl + 1;
    byte                *dest = ylookup0[dc_yl] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap = dc_colormap[0];

    while (--count)
    {
        *dest = colormap[dc_translation[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dc_translation[dc_source[frac >> FRACBITS]]];
}

void R_DrawDitherTranslatedColumn(void)
{
    int                 y = dc_yl;
    int                 count = dc_yh - y + 1;
    byte                *dest = ylookup0[y] + dc_x;
    fixed_t             frac = dc_texturefrac;
    const lighttable_t  *colormap[2] = { dc_colormap[0], dc_nextcolormap[0] };
    const int           fracz = ((dc_z >> 5) & 255);

    while (--count)
    {
        *dest = colormap[dither(dc_x, y++, fracz)][dc_translation[dc_source[frac >> FRACBITS]]];
        dest += SCREENWIDTH;
        frac += dc_iscale;
    }

    *dest = colormap[dither(dc_x, y, fracz)][dc_translation[dc_source[frac >> FRACBITS]]];
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//
void R_InitTranslationTables(void)
{
    // translate just the 16 green colors
    for (int i = 0; i < 256; i++)
        if (i >= 0x70 && i <= 0x7F)
        {
            // map green ramp to gray, brown, red
            translationtables[i] = 0x60 + (i & 0x0F);
            translationtables[i + 256] = 0x40 + (i & 0x0F);
            translationtables[i + 512] = 0x20 + (i & 0x0F);
        }
        else
        {
            // Keep all other colors as is.
            translationtables[i] = i;
            translationtables[i + 256] = i;
            translationtables[i + 512] = i;
        }
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int             ds_x1;
int             ds_x2;
int             ds_y;
int             ds_z;

lighttable_t    *ds_colormap;
lighttable_t    *ds_nextcolormap;

fixed_t         ds_xfrac;
fixed_t         ds_yfrac;
fixed_t         ds_xstep;
fixed_t         ds_ystep;

// start of a 64x64 tile image
byte            *ds_source;

//
// Draws the actual span.
//
void R_DrawSpan(void)
{
    int     count = ds_x2 - ds_x1;
    byte    *dest = ylookup0[ds_y] + ds_x1;
    fixed_t xfrac = ds_xfrac;
    fixed_t yfrac = ds_yfrac;

    while (--count)
    {
        *dest++ = ds_colormap[ds_source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }

    *dest = ds_colormap[ds_source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]];
}

void R_DrawDitherSpan(void)
{
    int                 x = ds_x1;
    int                 count = ds_x2 - x;
    byte                *dest = ylookup0[ds_y] + x;
    fixed_t             xfrac = ds_xfrac;
    fixed_t             yfrac = ds_yfrac;
    const lighttable_t  *colormap[2] = { ds_colormap, ds_nextcolormap };
    const int           fracz = ((ds_z >> 12) & 255);

    while (--count)
    {
        *dest++ = colormap[dither(x++, ds_y, fracz)][ds_source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }

    *dest = colormap[dither(x, ds_y, fracz)][ds_source[((xfrac >> 16) & 63) | ((yfrac >> 10) & 4032)]];
}

void R_DrawColorSpan(void)
{
    int         count = ds_x2 - ds_x1;
    byte        *dest = ylookup0[ds_y] + ds_x1;
    const byte  color = ds_colormap[NOTEXTURECOLOR];

    while (--count)
        *dest++ = color;

    *dest = color;
}

void R_DrawDitherColorSpan(void)
{
    int                 x = ds_x1;
    int                 count = ds_x2 - x;
    byte                *dest = ylookup0[ds_y] + x;
    const lighttable_t  *colormap[2] = { ds_colormap, ds_nextcolormap };
    const int           fracz = ((ds_z >> 12) & 255);

    while (--count)
        *dest++ = colormap[dither(x++, ds_y, fracz)][NOTEXTURECOLOR];

    *dest = colormap[dither(x, ds_y, fracz)][NOTEXTURECOLOR];
}

//
// R_InitBuffer
//
void R_InitBuffer(int width, int height)
{
    // Handle resize, e.g. smaller view windows with border and/or status bar.
    viewwindowx = (SCREENWIDTH - width) / 2;

    // Same with base row offset.
    viewwindowy = (width == SCREENWIDTH ? 0 : (SCREENHEIGHT - SBARHEIGHT - height) / 2);

    for (int i = 0, y = viewwindowy * SCREENWIDTH + viewwindowx; y < SCREENAREA; i++, y += SCREENWIDTH)
    {
        ylookup0[i] = screens[0] + y;
        ylookup1[i] = screens[1] + y;
    }

    fuzzrange[0] = -SCREENWIDTH;
    fuzzrange[1] = 0;
    fuzzrange[2] = SCREENWIDTH;

    for (int i = 0; i < MAXSCREENAREA; i++)
        fuzztable[i] = FUZZ(-1, 1);
}

void R_FillBezel(void)
{
    byte    *src = (byte *)grnrock;
    byte    *dest = &screens[0][(SCREENHEIGHT - SBARHEIGHT) * SCREENWIDTH];

    for (int y = SCREENHEIGHT - SBARHEIGHT; y < SCREENHEIGHT; y++)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            byte    dot = src[(((y >> 1) & 63) << 6) + ((x >> 1) & 63)];

            *dest++ = dot;
            *dest++ = dot;
        }

    if (st_drawbrdr)
    {
        for (int x = 0; x < (SCREENWIDTH - NONWIDEWIDTH) / 2 / SCREENSCALE; x += 8)
            V_DrawPatch(x - WIDESCREENDELTA, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, brdr_b);

        for (int x = SCREENWIDTH / SCREENSCALE - 8; x >= ((SCREENWIDTH - NONWIDEWIDTH) / 2 + NONWIDEWIDTH) / SCREENSCALE - 8; x -= 8)
            V_DrawPatch(x - WIDESCREENDELTA, VANILLAHEIGHT - VANILLASBARHEIGHT, 0, brdr_b);
    }
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen(void)
{
    byte    *src = (byte *)grnrock;
    byte    *dest = screens[1];

    for (int y = 0; y < SCREENHEIGHT - SBARHEIGHT; y++)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            byte    dot = src[(((y >> 1) & 63) << 6) + ((x >> 1) & 63)];

            *dest++ = dot;
            *dest++ = dot;
        }

    if (st_drawbrdr)
    {
        int x1 = viewwindowx / 2 - WIDESCREENDELTA;
        int y1 = viewwindowy / 2;
        int x2 = viewwidth / 2 + x1;
        int y2 = viewheight / 2 + y1;

        for (int x = x1; x < x2 - 8; x += 8)
        {
            V_DrawPatch(x, y1 - 8, 1, brdr_t);
            V_DrawPatch(x, y2, 1, brdr_b);
        }

        V_DrawPatch(x2 - 8, y1 - 8, 1, brdr_t);
        V_DrawPatch(x2 - 8, y2, 1, brdr_b);

        for (int y = y1; y < y2 - 8; y += 8)
        {
            V_DrawPatch(x1 - 8, y, 1, brdr_l);
            V_DrawPatch(x2, y, 1, brdr_r);
        }

        V_DrawPatch(x1 - 8, y2 - 8, 1, brdr_l);
        V_DrawPatch(x2, y2 - 8, 1, brdr_r);

        V_DrawPatch(x1 - 8, y1 - 8, 1, brdr_tl);
        V_DrawPatch(x2, y1 - 8, 1, brdr_tr);
        V_DrawPatch(x1 - 8, y2, 1, brdr_bl);
        V_DrawPatch(x2, y2, 1, brdr_br);
    }
}

//
// Copy a screen buffer.
//
void R_VideoErase(unsigned int ofs, int count)
{
    memcpy(screens[0] + ofs, screens[1] + ofs, count);
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder(void)
{
    int top = (SCREENHEIGHT - SBARHEIGHT - viewheight) / 2;
    int side = (SCREENWIDTH - viewwidth) / 2;
    int ofs;

    // copy top and one line of left side
    R_VideoErase(0, top * SCREENWIDTH + side);

    // copy one line of right side and bottom
    ofs = (viewheight + top) * SCREENWIDTH - side;
    R_VideoErase(ofs, top * SCREENWIDTH + side);

    // copy sides using wraparound
    ofs = top * SCREENWIDTH + SCREENWIDTH - side;
    side *= 2;

    for (int i = 1; i < viewheight; i++)
    {
        R_VideoErase(ofs, side);
        ofs += SCREENWIDTH;
    }
}
