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
#include "m_config.h"
#include "m_menu.h"
#include "p_local.h"
#include "r_sky.h"
#include "w_wad.h"

#define MAXVISPLANES    1024                    // must be a power of 2

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:
#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

static visplane_t   *visplanes[MAXVISPLANES];   // killough
static visplane_t   *freetail;                  // killough
static visplane_t   **freehead = &freetail;     // killough
visplane_t          *floorplane;
visplane_t          *ceilingplane;

int                 *openings;                  // dropoff overflow
int                 *lastopening;               // dropoff overflow

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
int                 floorclip[MAXWIDTH];        // dropoff overflow
int                 ceilingclip[MAXWIDTH];      // dropoff overflow

// texture mapping
static lighttable_t **planezlight;
static fixed_t      planeheight;

static fixed_t      xoffset, yoffset;           // killough 02/28/98: flat offsets

fixed_t             *yslope;
fixed_t             yslopes[LOOKDIRS][MAXHEIGHT];

static fixed_t      cachedheight[MAXHEIGHT];

dboolean            r_liquid_current = r_liquid_current_default;
dboolean            r_liquid_swirl = r_liquid_swirl_default;

static dboolean     updateswirl;

//
// R_MapPlane
//
static void R_MapPlane(int y, int x1)
{
    static fixed_t  cacheddistance[MAXHEIGHT];
    static fixed_t  cachedviewcosdistance[MAXHEIGHT];
    static fixed_t  cachedviewsindistance[MAXHEIGHT];
    static fixed_t  cachedxstep[MAXHEIGHT];
    static fixed_t  cachedystep[MAXHEIGHT];
    fixed_t         distance;
    fixed_t         viewcosdistance;
    fixed_t         viewsindistance;
    int             dx;

    if (planeheight != cachedheight[y])
    {
        int dy = ABS(centery - y);

        if (!dy)
            return;

        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
        viewcosdistance = cachedviewcosdistance[y] = FixedMul(viewcos, distance);
        viewsindistance = cachedviewsindistance[y] = FixedMul(viewsin, distance);
        ds_xstep = cachedxstep[y] = FixedMul(viewsin, planeheight) / dy;
        ds_ystep = cachedystep[y] = FixedMul(viewcos, planeheight) / dy;
    }
    else
    {
        distance = cacheddistance[y];
        viewcosdistance = cachedviewcosdistance[y];
        viewsindistance = cachedviewsindistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;
    ds_xfrac = viewx + xoffset + viewcosdistance + dx * ds_xstep;
    ds_yfrac = -viewy + yoffset - viewsindistance + dx * ds_ystep;

    if (fixedcolormap)
    {
        ds_colormap = fixedcolormap;
        ds_nextcolormap = fixedcolormap;
        ds_z = 0;
    }
    else
    {
        ds_colormap = planezlight[BETWEEN(0, distance >> LIGHTZSHIFT, MAXLIGHTZ - 1)];
        ds_nextcolormap = planezlight[BETWEEN(0, (distance >> LIGHTZSHIFT) + 1, MAXLIGHTZ - 1)];
        ds_z = distance;
    }

    ds_y = y;
    ds_x1 = x1;

    spanfunc();
}

//
// R_ClearPlanes
// At beginning of frame.
//
void R_ClearPlanes(void)
{
    // opening/clipping determination
    for (int i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (int i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead;)
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));
}

// New function, by Lee Killough
static visplane_t *new_visplane(unsigned int hash)
{
    visplane_t  *check = freetail;

    if (!check)
        check = calloc(1, sizeof(*check));
    else if (!(freetail = freetail->next))
        freehead = &freetail;

    check->next = visplanes[hash];
    visplanes[hash] = check;

    return check;
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t x, fixed_t y)
{
    visplane_t      *check;
    unsigned int    hash;                                       // killough

    if (picnum == skyflatnum || (picnum & PL_SKYFLAT))          // killough 10/98
    {
        height = 0;                                             // killough 07/19/98: most skies map together
        lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);

    for (check = visplanes[hash]; check; check = check->next)   // killough
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel
            && x == check->xoffset && y == check->yoffset)
            return check;

    check = new_visplane(hash);                                 // killough

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->xoffset = x;
    check->yoffset = y;
    check->left = viewwidth;
    check->right = -1;
    check->modified = false;

    memset(check->top, UINT_MAX, sizeof(check->top));

    return check;
}

//
// R_DupPlane
//
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{
    visplane_t  *new_pl = new_visplane(visplane_hash(pl->picnum, pl->lightlevel, pl->height));

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->xoffset = pl->xoffset;
    new_pl->yoffset = pl->yoffset;
    new_pl->left = start;
    new_pl->right = stop;
    new_pl->modified = false;

    memset(new_pl->top, UINT_MAX, sizeof(new_pl->top));

    return new_pl;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
    int intrl;
    int intrh;
    int unionl;
    int unionh;
    int x;

    if (start < pl->left)
    {
        intrl = pl->left;
        unionl = start;
    }
    else
    {
        unionl = pl->left;
        intrl = start;
    }

    if (stop > pl->right)
    {
        intrh = pl->right;
        unionh = stop;
    }
    else
    {
        unionh = pl->right;
        intrh = stop;
    }

    for (x = intrl; x <= intrh && pl->top[x] == UINT_MAX; x++);

    if (x > intrh)
    {
        pl->left = unionl;
        pl->right = unionh;

        return pl;
    }

    // make a new visplane
    return R_DupPlane(pl, start, stop);
}

//
// R_MakeSpans
//
static void R_MakeSpans(visplane_t *pl)
{
    // spanstart holds the start of a plane span
    // initialized to 0 at start
    static int  spanstart[MAXHEIGHT];
    int         stop = pl->right + 1;

    if (terraintypes[pl->picnum] != SOLID && r_liquid_current)
    {
        xoffset = animatedliquidxoffs;
        yoffset = animatedliquidyoffs;
    }
    else
    {
        xoffset = pl->xoffset;
        yoffset = pl->yoffset;
    }

    planeheight = ABS(pl->height - viewz);
    planezlight = zlight[BETWEEN(0, (pl->lightlevel >> LIGHTSEGSHIFT) + extralight, LIGHTLEVELS - 1)];
    pl->top[pl->left - 1] = UINT_MAX;
    pl->top[stop] = UINT_MAX;

    for (ds_x2 = pl->left; ds_x2 <= stop; ds_x2++)
    {
        unsigned int    t1 = pl->top[ds_x2 - 1];
        unsigned int    b1 = pl->bottom[ds_x2 - 1];
        unsigned int    t2 = pl->top[ds_x2];
        unsigned int    b2 = pl->bottom[ds_x2];

        for (; t1 < t2 && t1 <= b1; t1++)
            R_MapPlane(t1, spanstart[t1]);

        for (; b1 > b2 && b1 >= t1; b1--)
            R_MapPlane(b1, spanstart[b1]);

        while (t2 < t1 && t2 <= b2)
            spanstart[t2++] = ds_x2;

        while (b2 > b1 && b2 >= t2)
            spanstart[b2--] = ds_x2;
    }
}

// Ripple Effect from SMMU (r_ripple.cpp) by Simon Howard
#define SPEED           40

// swirl factors determine the number of waves per flat width
// 1 cycle per 64 units
#define SWIRLFACTOR     (8192 / 64)

// 1 cycle per 32 units (2 in 64)
#define SWIRLFACTOR2    (8192 / 32)

static int  offsets[1024 * 4096];

//
// R_DistortedFlat
// Generates a distorted flat from a normal one using a two-dimensional sine wave pattern.
// [crispy] Optimized to precalculate offsets
//
static byte *R_DistortedFlat(int flatnum)
{
    static byte distortedflat[4096];
    static int  prevgametime = -1;
    static int  prevflatnum = -1;
    static byte *normalflat;
    static int  *offset = offsets;

    if (prevgametime != gametime && updateswirl)
    {
        offset = &offsets[(gametime & 1023) << 12];
        prevgametime = gametime;

        if (prevflatnum != flatnum)
        {
            normalflat = lumpinfo[firstflat + flatnum]->cache;
            prevflatnum = flatnum;
        }

        for (int i = 0; i < 4096; i++)
            distortedflat[i] = normalflat[offset[i]];
    }
    else if (prevflatnum != flatnum)
    {
        normalflat = lumpinfo[firstflat + flatnum]->cache;
        prevflatnum = flatnum;

        for (int i = 0; i < 4096; i++)
            distortedflat[i] = normalflat[offset[i]];
    }

    return distortedflat;
}

//
// R_InitDistortedFlats
// [BH] Moved to separate function and called at startup
//
void R_InitDistortedFlats(void)
{
    for (int i = 0, *offset = offsets; i < 1024 * SPEED; i += SPEED, offset += 4096)
        for (int y = 0; y < 64; y++)
            for (int x = 0; x < 64; x++)
            {
                int x1, y1;
                int sinvalue, sinvalue2;

                sinvalue = finesine[(y * SWIRLFACTOR + i * 5 + 900) & 8191] * 2;
                sinvalue2 = finesine[(x * SWIRLFACTOR2 + i * 4 + 300) & 8191] * 2;
                x1 = x + 128 + (sinvalue >> FRACBITS) + (sinvalue2 >> FRACBITS);
                sinvalue = finesine[(x * SWIRLFACTOR + i * 3 + 700) & 8191] * 2;
                sinvalue2 = finesine[(y * SWIRLFACTOR2 + i * 4 + 1200) & 8191] * 2;
                y1 = y + 128 + (sinvalue >> FRACBITS) + (sinvalue2 >> FRACBITS);

                offset[(y << 6) + x] = ((y1 & 63) << 6) + (x1 & 63);
            }
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    if (r_liquid_swirl)
        updateswirl = (!consoleactive && !inhelpscreens && !paused && !freeze);

    dc_colormap[0] = (viewplayer->fixedcolormap == INVERSECOLORMAP && r_textures ? fixedcolormap : fullcolormap);

    for (int i = 0; i < MAXVISPLANES; i++)
        for (visplane_t *pl = visplanes[i]; pl; pl = pl->next)
            if (pl->modified && pl->left <= pl->right)
            {
                int picnum = pl->picnum;

                if (picnum == skyflatnum || (picnum & PL_SKYFLAT))
                {
                    // sky flat
                    int             texture;
                    angle_t         flip = 0U;
                    const rpatch_t  *tex_patch;
                    int             skyoffset;

                    // killough 10/98: allow skies to come from sidedefs.
                    // Allows scrolling and/or animated skies, as well as
                    // arbitrary multiple skies per level without having
                    // to use info lumps.
                    angle_t         an = viewangle;

                    if (picnum & PL_SKYFLAT)
                    {
                        // Sky linedef
                        const line_t    *l = lines + (picnum & ~PL_SKYFLAT);

                        // Sky transferred from first sidedef
                        const side_t    *s = sides + *l->sidenum;

                        if (s->missingtoptexture)
                        {
                            for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                                if ((dc_yl = pl->top[dc_x]) != UINT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                                    R_DrawColorColumn();

                            continue;
                        }

                        // Texture comes from upper texture of reference sidedef
                        texture = texturetranslation[s->toptexture];

                        // Horizontal offset is turned into an angle offset,
                        // to allow sky rotation as well as careful positioning.
                        // However, the offset is scaled very small, so that it
                        // allows a long-period of sky rotation.
                        an += s->textureoffset;

                        // Vertical offset allows careful sky positioning.
                        dc_texturemid = s->rowoffset - 28 * FRACUNIT;

                        dc_texheight = textureheight[texture] >> FRACBITS;

                        if (canmouselook)
                            dc_texturemid = dc_texturemid * dc_texheight / SKYSTRETCH_HEIGHT;

                        // We sometimes flip the picture horizontally.

                        // DOOM always flipped the picture, so we make it optional,
                        // to make it easier to use the new feature, while to still
                        // allow old sky textures to be used.
                        if (l->special != TransferSkyTextureToTaggedSectors_Flipped)
                            flip = ~0U;
                    }
                    else
                    {
                        // Normal DOOM sky, only one allowed per level
                        texture = skytexture;
                        dc_texheight = textureheight[texture] >> FRACBITS;
                        dc_texturemid = skytexturemid;
                    }

                    dc_iscale = skyiscale;
                    tex_patch = R_CacheTextureCompositePatchNum(texture);
                    skyoffset = skycolumnoffset >> FRACBITS;

                    for (dc_x = pl->left; dc_x <= pl->right; dc_x++)
                        if ((dc_yl = pl->top[dc_x]) != UINT_MAX && dc_yl <= (dc_yh = pl->bottom[dc_x]))
                        {
                            dc_source = R_GetTextureColumn(tex_patch,
                                (((an + xtoviewangle[dc_x]) ^ flip) >> ANGLETOSKYSHIFT) + skyoffset);

                            skycolfunc();
                        }
                }
                else
                {
                    // regular flat
                    ds_source = (terraintypes[picnum] != SOLID && r_liquid_swirl ?
                        R_DistortedFlat(picnum) : lumpinfo[flattranslation[picnum]]->cache);

                    R_MakeSpans(pl);
                }
            }
}
