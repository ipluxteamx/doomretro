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

#if !defined(__R_DRAW_H__)
#define __R_DRAW_H__

#include "m_random.h"

#define FUZZ(a, b)      fuzzrange[M_BigRandomInt(a, b) + 1]

// [BH] Compensate for rounding errors in DOOM's renderer by stretching wall
//  columns by 1px. This eliminates the randomly-colored pixels ("sparkles")
//  that appear at the bottom of some columns.
#define SPARKLEFIX      64

#define NOTEXTURECOLOR  80

extern lighttable_t     *dc_colormap[2];
extern lighttable_t     *dc_nextcolormap[2];
extern int              dc_x;
extern int              dc_yl;
extern int              dc_yh;
extern int              dc_z;
extern fixed_t          dc_iscale;
extern fixed_t          dc_texturemid;
extern fixed_t          dc_texheight;
extern fixed_t          dc_texturefrac;
extern byte             dc_solidblood;
extern byte             *dc_blood;
extern byte             *dc_brightmap;
extern int              dc_floorclip;
extern int              dc_ceilingclip;
extern int              dc_numposts;
extern byte             dc_black;
extern byte             *dc_black33;
extern byte             *dc_black40;

// first pixel in a column
extern byte             *dc_source;

extern int              fuzzpos;
extern int              fuzzrange[3];
extern int              fuzztable[MAXSCREENAREA];

// The span blitting interface.
// Hook in assembler or system specific BLT here.
void R_DrawColumn(void);
void R_DrawDitherColumn(void);
void R_DrawCorrectedColumn(void);
void R_DrawCorrectedDitherColumn(void);
void R_DrawColorColumn(void);
void R_DrawWallColumn(void);
void R_DrawDitherWallColumn(void);
void R_DrawBrightmapWallColumn(void);
void R_DrawBrightmapDitherWallColumn(void);
void R_DrawColorDitherColumn(void);
void R_DrawFlippedSkyColumn(void);
void R_DrawSkyColorColumn(void);
void R_DrawTranslucentColumn(void);
void R_DrawDitherTranslucentColumn(void);
void R_DrawTranslucent50Column(void);
void R_DrawDitherTranslucent50Column(void);
void R_DrawCorrectedTranslucent50Column(void);
void R_DrawCorrectedDitherTranslucent50Column(void);
void R_DrawTranslucent50ColorColumn(void);
void R_DrawDitherTranslucent50ColorColumn(void);
void R_DrawTranslucent33Column(void);
void R_DrawDitherTranslucent33Column(void);
void R_DrawTranslucentGreenColumn(void);
void R_DrawDitherTranslucentGreenColumn(void);
void R_DrawTranslucentRedColumn(void);
void R_DrawDitherTranslucentRedColumn(void);
void R_DrawTranslucentRedWhiteColumn1(void);
void R_DrawDitherTranslucentRedWhiteColumn1(void);
void R_DrawTranslucentRedWhiteColumn2(void);
void R_DrawDitherTranslucentRedWhiteColumn2(void);
void R_DrawTranslucentRedWhite50Column(void);
void R_DrawDitherTranslucentRedWhite50Column(void);
void R_DrawTranslucentBlueColumn(void);
void R_DrawDitherTranslucentBlueColumn(void);
void R_DrawTranslucentGreen33Column(void);
void R_DrawDitherTranslucentGreen33Column(void);
void R_DrawTranslucentRed33Column(void);
void R_DrawDitherTranslucentRed33Column(void);
void R_DrawTranslucentBlue25Column(void);
void R_DrawDitherTranslucentBlue25Column(void);
void R_DrawRedToBlueColumn(void);
void R_DrawDitherRedToBlueColumn(void);
void R_DrawTranslucentRedToBlue33Column(void);
void R_DrawDitherTranslucentRedToBlue33Column(void);
void R_DrawRedToGreenColumn(void);
void R_DrawDitherRedToGreenColumn(void);
void R_DrawTranslucentRedToGreen33Column(void);
void R_DrawDitherTranslucentRedToGreen33Column(void);
void R_DrawPlayerSpriteColumn(void);
void R_DrawShadowColumn(void);
void R_DrawFuzzyShadowColumn(void);
void R_DrawSolidShadowColumn(void);
void R_DrawSolidFuzzyShadowColumn(void);
void R_DrawBloodSplatColumn(void);
void R_DrawSolidBloodSplatColumn(void);

// The Spectre/Invisibility effect.
void R_DrawFuzzColumn(void);
void R_DrawPausedFuzzColumn(void);
void R_DrawFuzzColumns(void);
void R_DrawPausedFuzzColumns(void);

// Draw with color translation tables,
//  for player sprite rendering,
//  green/red/blue/indigo shirts.
void R_DrawTranslatedColumn(void);
void R_DrawDitherTranslatedColumn(void);

void R_VideoErase(unsigned int ofs, int count);

extern int          ds_x1;
extern int          ds_x2;
extern int          ds_y;
extern int          ds_z;

extern lighttable_t *ds_colormap;
extern lighttable_t *ds_nextcolormap;

extern fixed_t      ds_xfrac;
extern fixed_t      ds_yfrac;
extern fixed_t      ds_xstep;
extern fixed_t      ds_ystep;

// start of a 64*64 tile image
extern byte         *ds_source;

extern byte         translationtables[256 * 3];
extern byte         *dc_translation;

// Span blitting for rows, floor/ceiling.
// No Spectre effect needed.
void R_DrawSpan(void);
void R_DrawDitherSpan(void);
void R_DrawColorSpan(void);
void R_DrawDitherColorSpan(void);

void R_InitBuffer(int width, int height);

// Initialize color translation tables,
//  for player rendering etc.
void R_InitTranslationTables(void);

void R_FillBezel(void);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not fullscreen, draws a border around it.
void R_DrawViewBorder(void);

#endif
