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

#if !defined(__D_MAIN_H__)
#define __D_MAIN_H__

#include "d_event.h"
#include "doomdef.h"
#include "r_defs.h"

#define PAGETICS    (20 * TICRATE)

extern patch_t  *pagelump;
extern patch_t  *creditlump;
extern char     **episodes[];
extern char     **expansions[];
extern char     **skilllevels[];
extern char     *packageconfig;
extern char     *pwadfile;
extern dboolean advancetitle;
extern dboolean splashscreen;
extern dboolean dowipe;
extern int      logotic;
extern int      pagetic;
extern int      titlesequence;
extern int      fadecount;

void D_Display(void);

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overridden by user input, calls D_AdvanceTitle.
//
void D_DoomMain(void);

// Called by IO functions when input is detected.
void D_PostEvent(event_t *ev);

//
// BASE LEVEL
//
void D_PageTicker(void);
void D_PageDrawer(void);
void D_AdvanceTitle(void);
void D_DoAdvanceTitle(void);
void D_StartTitle(int page);
void D_FadeScreenToBlack(void);
void D_FadeScreen(dboolean screenshot);
dboolean D_IsDOOMIWAD(char *filename);

#endif
