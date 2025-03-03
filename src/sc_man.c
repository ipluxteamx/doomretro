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

#include "c_console.h"
#include "m_misc.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAX_STRING_SIZE 1024
#define ASCII_COMMENT1  ';'
#define ASCII_COMMENT2  '/'
#define ASCII_QUOTE     '"'
#define ASCII_ESCAPE    '\\'

char            *sc_String;
int             sc_Number;
int             sc_Line;

static char     *ScriptBuffer;
static char     *ScriptPtr;
static char     *ScriptEndPtr;
static int      ScriptLumpNum;
static dboolean sc_End;
static dboolean ScriptOpen;
static dboolean AlreadyGot;

void SC_Open(char *name)
{
    static char StringBuffer[MAX_STRING_SIZE];

    SC_Close();
    ScriptLumpNum = W_GetNumForName(name);
    ScriptBuffer = W_CacheLumpNum(ScriptLumpNum);
    ScriptPtr = ScriptBuffer;
    ScriptEndPtr = ScriptPtr + W_LumpLength(ScriptLumpNum);
    sc_Line = 1;
    sc_End = false;
    ScriptOpen = true;
    sc_String = StringBuffer;
    AlreadyGot = false;
}

void SC_Close(void)
{
    if (ScriptOpen)
    {
        if (ScriptLumpNum >= 0)
            W_ReleaseLumpNum(ScriptLumpNum);
        else
            Z_Free(ScriptBuffer);

        ScriptOpen = false;
    }
}

dboolean SC_GetString(void)
{
    char        *text;
    dboolean    foundToken = false;

    if (AlreadyGot)
    {
        AlreadyGot = false;
        return true;
    }

    if (ScriptPtr >= ScriptEndPtr)
    {
        sc_End = true;
        return false;
    }

    while (!foundToken)
    {
        while (ScriptPtr < ScriptEndPtr && (*ScriptPtr <= 32 || *ScriptPtr == '=' || (*ScriptPtr == ',' && *(ScriptPtr - 1) != '\'')))
            if (*ScriptPtr++ == '\n')
                sc_Line++;

        if (ScriptPtr >= ScriptEndPtr)
        {
            sc_End = true;
            return false;
        }

        if (*ScriptPtr != ASCII_COMMENT1 && *ScriptPtr != ASCII_COMMENT2 && *(ScriptPtr + 1) != ASCII_COMMENT2)
            foundToken = true;
        else
        {
            while (*ScriptPtr++ != '\n')
                if (ScriptPtr >= ScriptEndPtr)
                {
                    sc_End = true;
                    return false;
                }

            sc_Line++;
        }
    }

    text = sc_String;

    if (*ScriptPtr == ASCII_QUOTE)
    {
        ScriptPtr++;

        while (*ScriptPtr != ASCII_QUOTE || *(ScriptPtr - 1) == ASCII_ESCAPE)
        {
            if (*ScriptPtr != ASCII_ESCAPE)
                *text++ = *ScriptPtr++;
            else
                ScriptPtr++;

            if (ScriptPtr == ScriptEndPtr || text == &sc_String[MAX_STRING_SIZE - 1])
                break;
        }

        ScriptPtr++;
    }
    else
        while (*ScriptPtr > 32 && *ScriptPtr != ASCII_COMMENT1 && *ScriptPtr != ASCII_COMMENT2 && *(ScriptPtr + 1) != ASCII_COMMENT2)
        {
            if (*ScriptPtr == '{' || *ScriptPtr == '}')
                ScriptPtr++;

            *text++ = *ScriptPtr++;

            if (*ScriptPtr == '=' || (*ScriptPtr == ',' && *(ScriptPtr - 1) != '\''))
            {
                ScriptPtr++;
                break;
            }

            if (ScriptPtr == ScriptEndPtr || text == &sc_String[MAX_STRING_SIZE - 1])
                break;
        }

    *text = '\0';
    return true;
}

void SC_MustGetString(void)
{
    if (!SC_GetString())
        SC_ScriptError();
}

dboolean SC_GetNumber(void)
{
    if (SC_GetString())
    {
        sc_Number = strtol(sc_String, NULL, 0);
        return true;
    }
    else
        return false;
}

void SC_MustGetNumber(void)
{
    if (!SC_GetNumber())
        SC_ScriptError();
}

void SC_UnGet(void)
{
    AlreadyGot = true;
}

int SC_MatchString(char **strings)
{
    for (int i = 0; *strings; i++)
        if (SC_Compare(*strings++))
            return i;

    return -1;
}

dboolean SC_Compare(char *text)
{
    return M_StringCompare(text, sc_String);
}

static void SC_ScriptError(void)
{
    char    *temp = commify(sc_Line);

    C_Warning(1, "Line %s in the " BOLD("MAPINFO") " lump is invalid.", temp);
    free(temp);
}
