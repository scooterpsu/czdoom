/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Gamma correction LUT stuff.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomdef.h"
#include "r_main.h"
#include "r_draw.h"
#include "m_bbox.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"
#include "i_video.h"
#include "r_filter.h"
#include "lprintf.h"

// Each screen is [SCREENWIDTH*SCREENHEIGHT];
screeninfo_t screens[NUM_SCREENS];

/* jff 4/24/98 initialize this at runtime */
const byte *colrngs[CR_LIMIT];

int usegamma;

/*
 * V_InitColorTranslation
 *
 * Loads the color translation tables from predefined lumps at game start
 * No return
 *
 * Used for translating text colors from the red palette range
 * to other colors. The first nine entries can be used to dynamically
 * switch the output of text color thru the HUlib_drawText routine
 * by embedding ESCn in the text to obtain color n. Symbols for n are
 * provided in v_video.h.
 *
 * cphipps - constness of crdef_t stuff fixed
 */

typedef struct {
  const char *name;
  const byte **map;
  const byte defaultmap[256];
} crdef_t;

// killough 5/2/98: table-driven approach
static const crdef_t crdefs[] = {
  {"CRBRICK",  &colrngs[CR_BRICK ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 46, 46, 46, 46, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 46, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRTAN",    &colrngs[CR_TAN   ],
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 78, 45, 78, 78, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 78, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRGRAY",   &colrngs[CR_GRAY  ],
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 110, 110, 46, 110, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 110, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108, 110, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRGREEN",  &colrngs[CR_GREEN ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 127, 127, 46, 127, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 127, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRBROWN",  &colrngs[CR_BROWN ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 143, 143, 46, 143, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 143, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRGOLD",   &colrngs[CR_GOLD  ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 167, 167, 46, 167, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 167, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 160, 160, 161, 161, 162, 162, 163, 163, 164, 164, 165, 165, 166, 166, 167, 167, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRRED",    &colrngs[CR_RED   ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 191, 191, 46, 191, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 191, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRBLUE",   &colrngs[CR_BLUE  ],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 203, 203, 46, 203, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 203, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 196, 196, 197, 197, 198, 198, 199, 199, 200, 200, 201, 201, 202, 202, 203, 203, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRORANGE", &colrngs[CR_ORANGE],
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 223, 223, 46, 223, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 223, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRYELLOW", &colrngs[CR_YELLOW],
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 231, 231, 46, 231, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 231, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 224, 224, 225, 225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRBLUE2",  &colrngs[CR_BLUE2],
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 207, 207, 46, 207, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 207, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 }
  },
  {"CRLIGHT", &colrngs[CR_LIGHT],
    {   0,   1,   2,   3,   4,   5,   6,   6,   7,   9,  10,  10,  11,  13,  14,  15,
       16,  17,  18,  19,  20,  21,  22,  23,  24,  24,  25,  25,  26,  26,  27,  27,
       28,  28,  29,  29,  30,  30,  31,  31,  32,  33,  34,  35,  36,  37,  38,  39,
       48,  49,  50,  51,  52,  53,  54,  55,  56,  56,  57,  57,  58,  58,  59,  59,
       60,  60,  61,  61,  62,  62,  63,  63,  64,  65,  66,  67,  68,  69,  70,  71,
       80,  81,  82,  83,  84,  84,  85,  85,  86,  86,  87,  87,  88,  88,  89,  89,
       90,  90,  91,  91,  93,  94,  95,  95,  96,  97,  98,  99, 100, 101, 102, 103,
      112, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119, 120, 121,
      128, 129, 130, 131, 132, 133, 134, 135, 136, 136, 137, 137, 138, 138, 139, 139,
      140, 141, 142, 143, 144, 145, 146, 147, 152, 152, 153, 153, 154, 154, 155, 156,
      226, 227, 228, 161, 162, 163, 164, 165, 168, 169, 170, 171, 172, 172, 173, 173,
      208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
      192, 192, 193, 193, 194, 194, 195, 195, 196, 196, 197, 197, 198, 198, 199, 199,
      224, 225, 226, 227, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 216,
      224, 225, 226, 227, 228, 229, 230, 231, 232, 232, 233, 233, 236, 236, 237, 237,
      240, 241, 241, 242, 242, 243, 243, 244, 248, 249, 250, 251, 251, 252, 252, 255
    }
  },  
  {NULL}
};


const byte gammatable[5][256] = {
  {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
    17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
    33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
    49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
    65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
    81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
    97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
    113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
  },
  {
    2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,
    24,25,26,27,29,30,31,32,33,34,36,37,38,39,40,41,
    42,44,45,46,47,48,49,50,51,52,54,55,56,57,58,59,
    60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,
    77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,
    93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,
    109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
    125,126,127,128,129,129,130,131,132,133,134,135,136,137,138,139,
    140,141,142,143,144,145,146,147,148,148,149,150,151,152,153,154,
    155,156,157,158,159,160,161,162,163,163,164,165,166,167,168,169,
    170,171,172,173,174,175,175,176,177,178,179,180,181,182,183,184,
    185,186,186,187,188,189,190,191,192,193,194,195,196,196,197,198,
    199,200,201,202,203,204,205,205,206,207,208,209,210,211,212,213,
    214,214,215,216,217,218,219,220,221,222,222,223,224,225,226,227,
    228,229,230,230,231,232,233,234,235,236,237,237,238,239,240,241,
    242,243,244,245,245,246,247,248,249,250,251,252,252,253,254,255
  },
  {
    4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,
    33,35,36,38,39,40,42,43,45,46,47,48,50,51,52,54,
    55,56,57,59,60,61,62,63,65,66,67,68,69,70,72,73,
    74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,
    91,92,93,94,95,96,97,98,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,114,115,116,117,118,119,120,121,122,
    123,124,125,126,127,128,129,130,131,132,133,133,134,135,136,137,
    138,139,140,141,142,143,144,144,145,146,147,148,149,150,151,152,
    153,153,154,155,156,157,158,159,160,160,161,162,163,164,165,166,
    166,167,168,169,170,171,172,172,173,174,175,176,177,178,178,179,
    180,181,182,183,183,184,185,186,187,188,188,189,190,191,192,193,
    193,194,195,196,197,197,198,199,200,201,201,202,203,204,205,206,
    206,207,208,209,210,210,211,212,213,213,214,215,216,217,217,218,
    219,220,221,221,222,223,224,224,225,226,227,228,228,229,230,231,
    231,232,233,234,235,235,236,237,238,238,239,240,241,241,242,243,
    244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,255
  },
  {
    8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,
    47,49,50,52,53,55,57,58,60,61,63,64,65,67,68,70,
    71,72,74,75,76,77,79,80,81,82,84,85,86,87,88,90,
    91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,
    124,125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,
    139,140,141,142,143,143,144,145,146,147,148,149,150,150,151,152,
    153,154,155,155,156,157,158,159,160,160,161,162,163,164,165,165,
    166,167,168,169,169,170,171,172,173,173,174,175,176,176,177,178,
    179,180,180,181,182,183,183,184,185,186,186,187,188,189,189,190,
    191,192,192,193,194,195,195,196,197,197,198,199,200,200,201,202,
    202,203,204,205,205,206,207,207,208,209,210,210,211,212,212,213,
    214,214,215,216,216,217,218,219,219,220,221,221,222,223,223,224,
    225,225,226,227,227,228,229,229,230,231,231,232,233,233,234,235,
    235,236,237,237,238,238,239,240,240,241,242,242,243,244,244,245,
    246,246,247,247,248,249,249,250,251,251,252,253,253,254,254,255
  },
  {
    16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,
    66,68,69,71,73,75,76,78,80,81,83,84,86,87,89,90,
    92,93,94,96,97,98,100,101,102,103,105,106,107,108,109,110,
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,128,
    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,156,
    157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
    169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,
    181,182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,
    192,193,193,194,195,195,196,196,197,198,198,199,200,200,201,202,
    202,203,203,204,205,205,206,207,207,208,208,209,210,210,211,211,
    212,213,213,214,214,215,216,216,217,217,218,219,219,220,220,221,
    221,222,223,223,224,224,225,225,226,227,227,228,228,229,229,230,
    230,231,232,232,233,233,234,234,235,235,236,236,237,237,238,239,
    239,240,240,241,241,242,242,243,243,244,244,245,245,246,246,247,
    247,248,248,249,249,250,250,251,251,252,252,253,254,254,255,255
  }
};


// killough 5/2/98: tiny engine driven by table above
void V_InitColorTranslation(void)
{
  register const crdef_t *p;
  for (p=crdefs; p->name; p++) {
    int lump = W_CheckNumForName(p->name);
	if (lump == -1)
	   *p->map = p->defaultmap;
	else
	   *p->map = W_CacheLumpName(p->name);
  }
}

//
// V_CopyRect
//
// Copies a source rectangle in a screen buffer to a destination
// rectangle in another screen buffer. Source origin in srcx,srcy,
// destination origin in destx,desty, common size in width and height.
// Source buffer specfified by srcscrn, destination buffer by destscrn.
//
// Marks the destination rectangle on the screen dirty.
//
// No return.
//
static void FUNC_V_CopyRect(int srcx, int srcy, int srcscrn, int width,
                int height, int destx, int desty, int destscrn,
                enum patch_translation_e flags)
{
  byte *src;
  byte *dest;

  if (flags & VPT_STRETCH)
  {
    srcx=srcx*SCREENWIDTH/320;
    srcy=srcy*SCREENHEIGHT/200;
    width=width*SCREENWIDTH/320;
    height=height*SCREENHEIGHT/200;
    destx=destx*SCREENWIDTH/320;
    desty=desty*SCREENHEIGHT/200;
  }

#ifdef RANGECHECK
  if (srcx<0
      ||srcx+width >SCREENWIDTH
      || srcy<0
      || srcy+height>SCREENHEIGHT
      ||destx<0||destx+width >SCREENWIDTH
      || desty<0
      || desty+height>SCREENHEIGHT)
    I_Error ("V_CopyRect: Bad arguments");
#endif

  src = screens[srcscrn].data+screens[srcscrn].byte_pitch*srcy+srcx*V_GetPixelDepth();
  dest = screens[destscrn].data+screens[destscrn].byte_pitch*desty+destx*V_GetPixelDepth();

  for ( ; height>0 ; height--)
    {
      memcpy (dest, src, width*V_GetPixelDepth());
      src += screens[srcscrn].byte_pitch;
      dest += screens[destscrn].byte_pitch;
    }
}

/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text betwen levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer
 * directly, so this is my code from the equivalent function in f_finale.c
 */
static void FUNC_V_DrawBackground(const char* flatname, int scrn)
{
  /* erase the entire screen to a tiled background */
  const byte *src;
  int         x,y;
  int         width,height;
  int         lump;

  // killough 4/17/98:
  src = W_CacheLumpNum(lump = firstflat + R_FlatNumForName(flatname));

  /* V_DrawBlock(0, 0, scrn, 64, 64, src, 0); */
  width = height = 64;
  if (V_GetMode() == VID_MODE8) {
    byte *dest = screens[scrn].data;

    while (height--) {
      memcpy (dest, src, width);
      src += width;
      dest += screens[scrn].byte_pitch;
    }
  } else if (V_GetMode() == VID_MODE15) {
    unsigned short *dest = (unsigned short *)screens[scrn].data;

    while (height--) {
      int i;
      for (i=0; i<width; i++) {
        dest[i] = VID_PAL15(src[i], VID_COLORWEIGHTMASK);
      }
      src += width;
      dest += screens[scrn].short_pitch;
    }
  } else if (V_GetMode() == VID_MODE16) {
    unsigned short *dest = (unsigned short *)screens[scrn].data;

    while (height--) {
      int i;
      for (i=0; i<width; i++) {
        dest[i] = VID_PAL16(src[i], VID_COLORWEIGHTMASK);
      }
      src += width;
      dest += screens[scrn].short_pitch;
    }
  } else if (V_GetMode() == VID_MODE32) {
    unsigned int *dest = (unsigned int *)screens[scrn].data;

    while (height--) {
      int i;
      for (i=0; i<width; i++) {
        dest[i] = VID_PAL32(src[i], VID_COLORWEIGHTMASK);
      }
      src += width;
      dest += screens[scrn].int_pitch;
    }
  }
  /* end V_DrawBlock */

  for (y=0 ; y<SCREENHEIGHT ; y+=64)
    for (x=y ? 0 : 64; x<SCREENWIDTH ; x+=64)
      V_CopyRect(0, 0, scrn, ((SCREENWIDTH-x) < 64) ? (SCREENWIDTH-x) : 64,
     ((SCREENHEIGHT-y) < 64) ? (SCREENHEIGHT-y) : 64, x, y, scrn, VPT_NONE);
  W_UnlockLumpNum(lump);
}

//
// V_Init
//
// Allocates the 4 full screen buffers in low DOS memory
// No return
//

void V_Init (void)
{
  int  i;

  // reset the all
  for (i = 0; i<NUM_SCREENS; i++) {
    screens[i].data = NULL;
    screens[i].not_on_heap = false;
    screens[i].width = 0;
    screens[i].height = 0;
    screens[i].byte_pitch = 0;
    screens[i].short_pitch = 0;
    screens[i].int_pitch = 0;
  }
}

//
// V_DrawMemPatch
//
// CPhipps - unifying patch drawing routine, handles all cases and combinations
//  of stretching, flipping and translating
//
// This function is big, hopefully not too big that gcc can't optimise it well.
// In fact it packs pretty well, there is no big performance lose for all this merging;
// the inner loops themselves are just the same as they always were
// (indeed, laziness of the people who wrote the 'clones' of the original V_DrawPatch
//  means that their inner loops weren't so well optimised, so merging code may even speed them).
//
static void V_DrawMemPatch(int x, int y, int scrn, const rpatch_t *patch,
        int cm, enum patch_translation_e flags)
{
  const byte *trans;

  if (cm<CR_LIMIT)
    trans=colrngs[cm];
  else
    trans=translationtables + 256*((cm-CR_LIMIT)-1);
  y -= patch->topoffset;
  x -= patch->leftoffset;

  // CPhipps - auto-no-stretch if not high-res
  if (flags & VPT_STRETCH)
    if ((SCREENWIDTH==320) && (SCREENHEIGHT==200))
      flags &= ~VPT_STRETCH;

  // CPhipps - null translation pointer => no translation
  if (!trans)
    flags &= ~VPT_TRANS;

  if (V_GetMode() == VID_MODE8 && !(flags & VPT_STRETCH)) {
    int             col;
    byte           *desttop = screens[scrn].data+y*screens[scrn].byte_pitch+x*V_GetPixelDepth();
    unsigned int    w = patch->width;

    if (y<0 || y+patch->height > ((flags & VPT_STRETCH) ? 200 :  SCREENHEIGHT)) {
      // killough 1/19/98: improved error message:
      lprintf(LO_WARN, "V_DrawMemPatch8: Patch (%d,%d)-(%d,%d) exceeds LFB in vertical direction (horizontal is clipped)\n"
              "Bad V_DrawMemPatch8 (flags=%u)", x, y, x+patch->width, y+patch->height, flags);
      return;
    }

    w--; // CPhipps - note: w = width-1 now, speeds up flipping

    for (col=0 ; (unsigned int)col<=w ; desttop++, col++, x++) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? (w - col) : (col);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);

      if (x < 0)
        continue;
      if (x >= SCREENWIDTH)
        break;

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        // killough 2/21/98: Unrolled and performance-tuned

        const byte *source = column->pixels + post->topdelta;
        byte *dest = desttop + post->topdelta*screens[scrn].byte_pitch;
        int count = post->length;

        if (!(flags & VPT_TRANS)) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              dest[0] = s0;
              dest[screens[scrn].byte_pitch] = s1;
              dest += screens[scrn].byte_pitch*2;
              s0 = source[2];
              s1 = source[3];
              source += 4;
              dest[0] = s0;
              dest[screens[scrn].byte_pitch] = s1;
              dest += screens[scrn].byte_pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = *source++;
              dest += screens[scrn].byte_pitch;
            } while (--count);
        } else {
          // CPhipps - merged translation code here
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = trans[s0];
              s1 = trans[s1];
              dest[0] = s0;
              dest[screens[scrn].byte_pitch] = s1;
              dest += screens[scrn].byte_pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = trans[s0];
              s1 = trans[s1];
              source += 4;
              dest[0] = s0;
              dest[screens[scrn].byte_pitch] = s1;
              dest += screens[scrn].byte_pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = trans[*source++];
              dest += screens[scrn].byte_pitch;
            } while (--count);
        }
      }
    }
  }
  else {
    // CPhipps - move stretched patch drawing code here
    //         - reformat initialisers, move variables into inner blocks

    int   col;
    int   w = (patch->width << 16) - 1; // CPhipps - -1 for faster flipping
    int   left, right, top, bottom;
    int   DX  = (SCREENWIDTH<<16)  / 320;
    int   DXI = (320<<16)          / SCREENWIDTH;
    int   DY  = (SCREENHEIGHT<<16) / 200;
    int   DYI = (200<<16)          / SCREENHEIGHT;
    R_DrawColumn_f colfunc;
    draw_column_vars_t dcvars;
    draw_vars_t olddrawvars = drawvars;

    R_SetDefaultDrawColumnVars(&dcvars);

    drawvars.byte_topleft = screens[scrn].data;
    drawvars.short_topleft = (unsigned short *)screens[scrn].data;
    drawvars.int_topleft = (unsigned int *)screens[scrn].data;
    drawvars.byte_pitch = screens[scrn].byte_pitch;
    drawvars.short_pitch = screens[scrn].short_pitch;
    drawvars.int_pitch = screens[scrn].int_pitch;

    if (!(flags & VPT_STRETCH)) {
      DX = 1 << 16;
      DXI = 1 << 16;
      DY = 1 << 16;
      DYI = 1 << 16;
    }

    if (flags & VPT_TRANS) {
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, drawvars.filterpatch, RDRAW_FILTER_NONE);
      dcvars.translation = trans;
    } else {
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterpatch, RDRAW_FILTER_NONE);
    }

    left = ( x * DX ) >> FRACBITS;
    top = ( y * DY ) >> FRACBITS;
    right = ( (x + patch->width) * DX ) >> FRACBITS;
    bottom = ( (y + patch->height) * DY ) >> FRACBITS;

    dcvars.texheight = patch->height;
    dcvars.iscale = DYI;
    dcvars.drawingmasked = MAX(patch->width, patch->height) > 8;
    dcvars.edgetype = drawvars.patch_edges;

    if (drawvars.filterpatch == RDRAW_FILTER_LINEAR) {
      // bias the texture u coordinate
      if (patch->isNotTileable)
        col = -(FRACUNIT>>1);
      else
        col = (patch->width<<FRACBITS)-(FRACUNIT>>1);
    }
    else {
      col = 0;
    }

    for (dcvars.x=left; dcvars.x<right; dcvars.x++, col+=DXI) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? ((w - col)>>16): (col>>16);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);
      const rcolumn_t *prevcolumn = R_GetPatchColumn(patch, colindex-1);
      const rcolumn_t *nextcolumn = R_GetPatchColumn(patch, colindex+1);

      // ignore this column if it's to the left of our clampRect
      if (dcvars.x < 0)
        continue;
      if (dcvars.x >= SCREENWIDTH)
        break;

      dcvars.texu = ((flags & VPT_FLIP) ? ((patch->width<<FRACBITS)-col) : col) % (patch->width<<FRACBITS);

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        int yoffset = 0;

        dcvars.yl = (((y + post->topdelta) * DY)>>FRACBITS);
        dcvars.yh = (((y + post->topdelta + post->length) * DY - (FRACUNIT>>1))>>FRACBITS);
        dcvars.edgeslope = post->slope;

        if ((dcvars.yh < 0) || (dcvars.yh < top))
          continue;
        if ((dcvars.yl >= SCREENHEIGHT) || (dcvars.yl >= bottom))
          continue;

        if (dcvars.yh >= bottom) {
          dcvars.yh = bottom-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }
        if (dcvars.yh >= SCREENHEIGHT) {
          dcvars.yh = SCREENHEIGHT-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }

        if (dcvars.yl < 0) {
          yoffset = 0-dcvars.yl;
          dcvars.yl = 0;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }
        if (dcvars.yl < top) {
          yoffset = top-dcvars.yl;
          dcvars.yl = top;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }

        dcvars.source = column->pixels + post->topdelta + yoffset;
        dcvars.prevsource = prevcolumn ? prevcolumn->pixels + post->topdelta + yoffset: dcvars.source;
        dcvars.nextsource = nextcolumn ? nextcolumn->pixels + post->topdelta + yoffset: dcvars.source;

        dcvars.texturemid = -((dcvars.yl-centery)*dcvars.iscale);

        colfunc(&dcvars);
      }
    }

    R_ResetColumnBuffer();
    drawvars = olddrawvars;
  }
}

// CPhipps - some simple, useful wrappers for that function, for drawing patches from wads

// CPhipps - GNU C only suppresses generating a copy of a function if it is
// static inline; other compilers have different behaviour.
// This inline is _only_ for the function below

static void FUNC_V_DrawNumPatch(int x, int y, int scrn, int lump,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch(x, y, scrn, R_CachePatchNum(lump), cm, flags);
  R_UnlockPatchNum(lump);
}

unsigned short *V_Palette15 = NULL;
unsigned short *V_Palette16 = NULL;
unsigned int *V_Palette32 = NULL;
static unsigned short *Palettes15 = NULL;
static unsigned short *Palettes16 = NULL;
static unsigned int *Palettes32 = NULL;
static int currentPaletteIndex = 0;

//
// V_UpdateTrueColorPalette
//
void V_UpdateTrueColorPalette(video_mode_t mode) {
  int i, w, p;
  byte r,g,b;
  int nr,ng,nb;
  float t;
  int paletteNum = (V_GetMode() == VID_MODEGL ? 0 : currentPaletteIndex);
  static int usegammaOnLastPaletteGeneration = -1;
  
  int pplump = W_GetNumForName("PLAYPAL");
  int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
  const byte *pal = W_CacheLumpNum(pplump);
  // opengl doesn't use the gamma
  const byte *const gtable = 
    ((gtlump == -1)? gammatable : (const byte *)W_CacheLumpNum(gtlump)) +
    (V_GetMode() == VID_MODEGL ? 0 : 256*(usegamma))
  ;

  int numPals = W_LumpLength(pplump) / (3*256);
  const float dontRoundAbove = 220;
  float roundUpR, roundUpG, roundUpB;
  
  if (usegammaOnLastPaletteGeneration != usegamma) {
    if (Palettes15) free(Palettes15);
    if (Palettes16) free(Palettes16);
    if (Palettes32) free(Palettes32);
    Palettes15 = NULL;
    Palettes16 = NULL;
    Palettes32 = NULL;
    usegammaOnLastPaletteGeneration = usegamma;      
  }
  
  if (mode == VID_MODE32) {
    if (!Palettes32) {
      // set int palette
      Palettes32 = (int*)malloc(numPals*256*sizeof(int)*VID_NUMCOLORWEIGHTS);
      for (p=0; p<numPals; p++) {
        for (i=0; i<256; i++) {
          r = gtable[pal[(256*p+i)*3+0]];
          g = gtable[pal[(256*p+i)*3+1]];
          b = gtable[pal[(256*p+i)*3+2]];
          
          // ideally, we should always round up, but very bright colors
          // overflow the blending adds, so they don't get rounded.
          roundUpR = (r > dontRoundAbove) ? 0 : 0.5f;
          roundUpG = (g > dontRoundAbove) ? 0 : 0.5f;
          roundUpB = (b > dontRoundAbove) ? 0 : 0.5f;
                  
          for (w=0; w<VID_NUMCOLORWEIGHTS; w++) {
            t = (float)(w)/(float)(VID_NUMCOLORWEIGHTS-1);
            nr = (int)(r*t+roundUpR);
            ng = (int)(g*t+roundUpG);
            nb = (int)(b*t+roundUpB);
            Palettes32[((p*256+i)*VID_NUMCOLORWEIGHTS)+w] = (
              (nr<<16) | (ng<<8) | nb
            );
          }
        }
      }
    }
    V_Palette32 = Palettes32 + paletteNum*256*VID_NUMCOLORWEIGHTS;
  }
  else if (mode == VID_MODE16) {
    if (!Palettes16) {
      // set short palette
      Palettes16 = (short*)malloc(numPals*256*sizeof(short)*VID_NUMCOLORWEIGHTS);
      for (p=0; p<numPals; p++) {
        for (i=0; i<256; i++) {
          r = gtable[pal[(256*p+i)*3+0]];
          g = gtable[pal[(256*p+i)*3+1]];
          b = gtable[pal[(256*p+i)*3+2]];
          
          // ideally, we should always round up, but very bright colors
          // overflow the blending adds, so they don't get rounded.
          roundUpR = (r > dontRoundAbove) ? 0 : 0.5f;
          roundUpG = (g > dontRoundAbove) ? 0 : 0.5f;
          roundUpB = (b > dontRoundAbove) ? 0 : 0.5f;
                   
          for (w=0; w<VID_NUMCOLORWEIGHTS; w++) {
            t = (float)(w)/(float)(VID_NUMCOLORWEIGHTS-1);
            nr = (int)((r>>3)*t+roundUpR);
            ng = (int)((g>>2)*t+roundUpG);
            nb = (int)((b>>3)*t+roundUpB);
            Palettes16[((p*256+i)*VID_NUMCOLORWEIGHTS)+w] = (
              (nr<<11) | (ng<<5) | nb
            );
          }
        }
      }
    }
    V_Palette16 = Palettes16 + paletteNum*256*VID_NUMCOLORWEIGHTS;
  }
  else if (mode == VID_MODE15) {
    if (!Palettes15) {
      // set short palette
      Palettes15 = (short*)malloc(numPals*256*sizeof(short)*VID_NUMCOLORWEIGHTS);
      for (p=0; p<numPals; p++) {
        for (i=0; i<256; i++) {
          r = gtable[pal[(256*p+i)*3+0]];
          g = gtable[pal[(256*p+i)*3+1]];
          b = gtable[pal[(256*p+i)*3+2]];
          
          // ideally, we should always round up, but very bright colors
          // overflow the blending adds, so they don't get rounded.
          roundUpR = (r > dontRoundAbove) ? 0 : 0.5f;
          roundUpG = (g > dontRoundAbove) ? 0 : 0.5f;
          roundUpB = (b > dontRoundAbove) ? 0 : 0.5f;
                   
          for (w=0; w<VID_NUMCOLORWEIGHTS; w++) {
            t = (float)(w)/(float)(VID_NUMCOLORWEIGHTS-1);
            nr = (int)((r>>3)*t+roundUpR);
            ng = (int)((g>>3)*t+roundUpG);
            nb = (int)((b>>3)*t+roundUpB);
            Palettes15[((p*256+i)*VID_NUMCOLORWEIGHTS)+w] = (
              (nr<<10) | (ng<<5) | nb
            );
          }
        }
      }
    }
    V_Palette15 = Palettes15 + paletteNum*256*VID_NUMCOLORWEIGHTS;
  }       
   
  W_UnlockLumpNum(pplump);
  W_UnlockLumpNum(gtlump);
}


//---------------------------------------------------------------------------
// V_DestroyTrueColorPalette
//---------------------------------------------------------------------------
static void V_DestroyTrueColorPalette(video_mode_t mode) {
  if (mode == VID_MODE15) {
    if (Palettes15) free(Palettes15);
    Palettes15 = NULL;
    V_Palette15 = NULL;
  }
  if (mode == VID_MODE16) {
    if (Palettes16) free(Palettes16);
    Palettes16 = NULL;
    V_Palette16 = NULL;
  }
  if (mode == VID_MODE32) {
    if (Palettes32) free(Palettes32);
    Palettes32 = NULL;
    V_Palette32 = NULL;
  }
}

void V_DestroyUnusedTrueColorPalettes(void) {
  if (V_GetMode() != VID_MODE15) V_DestroyTrueColorPalette(VID_MODE15);
  if (V_GetMode() != VID_MODE16) V_DestroyTrueColorPalette(VID_MODE16);
  if (V_GetMode() != VID_MODE32) V_DestroyTrueColorPalette(VID_MODE32);  
}

//
// V_SetPalette
//
// CPhipps - New function to set the palette to palette number pal.
// Handles loading of PLAYPAL and calls I_SetPalette

void V_SetPalette(int pal)
{
  currentPaletteIndex = pal;

  if (V_GetMode() == VID_MODEGL) {
#ifdef GL_DOOM
    gld_SetPalette(pal);
#endif
  } else {
    I_SetPalette(pal);
    if (V_GetMode() == VID_MODE15 || V_GetMode() == VID_MODE16 || V_GetMode() == VID_MODE32) {
      // V_SetPalette can be called as part of the gamma setting before
      // we've loaded any wads, which prevents us from reading the palette - POPE
      if (W_CheckNumForName("PLAYPAL") >= 0) {
        V_UpdateTrueColorPalette(V_GetMode());
      }
    }
  }
}

//
// V_FillRect
//
// CPhipps - New function to fill a rectangle with a given colour
static void V_FillRect8(int scrn, int x, int y, int width, int height, byte colour)
{
  byte* dest = screens[scrn].data + x + y*screens[scrn].byte_pitch;
  while (height--) {
    memset(dest, colour, width);
    dest += screens[scrn].byte_pitch;
  }
}

static void V_FillRect15(int scrn, int x, int y, int width, int height, byte colour)
{
  unsigned short* dest = (unsigned short *)screens[scrn].data + x + y*screens[scrn].short_pitch;
  int w;
  short c = VID_PAL15(colour, VID_COLORWEIGHTMASK);
  while (height--) {
    for (w=0; w<width; w++) {
      dest[w] = c;
    }
    dest += screens[scrn].short_pitch;
  }
}

static void V_FillRect16(int scrn, int x, int y, int width, int height, byte colour)
{
  unsigned short* dest = (unsigned short *)screens[scrn].data + x + y*screens[scrn].short_pitch;
  int w;
  short c = VID_PAL16(colour, VID_COLORWEIGHTMASK);
  while (height--) {
    for (w=0; w<width; w++) {
      dest[w] = c;
    }
    dest += screens[scrn].short_pitch;
  }
}

static void V_FillRect32(int scrn, int x, int y, int width, int height, byte colour)
{
  unsigned int* dest = (unsigned int *)screens[scrn].data + x + y*screens[scrn].int_pitch;
  int w;
  int c = VID_PAL32(colour, VID_COLORWEIGHTMASK);
  while (height--) {
    for (w=0; w<width; w++) {
      dest[w] = c;
    }
    dest += screens[scrn].int_pitch;
  }
}

static void WRAP_V_DrawLine(fline_t* fl, int color);
static void V_PlotPixel8(int scrn, int x, int y, byte color);
static void V_PlotPixel15(int scrn, int x, int y, byte color);
static void V_PlotPixel16(int scrn, int x, int y, byte color);
static void V_PlotPixel32(int scrn, int x, int y, byte color);

#ifdef GL_DOOM
static void WRAP_gld_FillRect(int scrn, int x, int y, int width, int height, byte colour)
{
  gld_FillBlock(x,y,width,height,colour);
}
static void WRAP_gld_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, enum patch_translation_e flags)
{
}
static void WRAP_gld_DrawBackground(const char *flatname, int n)
{
  gld_DrawBackground(flatname);
}
static void WRAP_gld_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags)
{
  gld_DrawNumPatch(x,y,lump,cm,flags);
}
static void WRAP_gld_DrawBlock(int x, int y, int scrn, int width, int height, const byte *src, enum patch_translation_e flags)
{
}
static void V_PlotPixelGL(int scrn, int x, int y, byte color) {
  gld_DrawLine(x-1, y, x+1, y, color);
  gld_DrawLine(x, y-1, x, y+1, color);
}
static void WRAP_gld_DrawLine(fline_t* fl, int color)
{
  gld_DrawLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, color);
}
#endif

static void NULL_FillRect(int scrn, int x, int y, int width, int height, byte colour) {}
static void NULL_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, enum patch_translation_e flags) {}
static void NULL_DrawBackground(const char *flatname, int n) {}
static void NULL_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags) {}
static void NULL_DrawBlock(int x, int y, int scrn, int width, int height, const byte *src, enum patch_translation_e flags) {}
static void NULL_PlotPixel(int scrn, int x, int y, byte color) {}
static void NULL_DrawLine(fline_t* fl, int color) {}

const char *default_videomode;
static video_mode_t current_videomode = VID_MODE8;

V_CopyRect_f V_CopyRect = NULL_CopyRect;
V_FillRect_f V_FillRect = NULL_FillRect;
V_DrawNumPatch_f V_DrawNumPatch = NULL_DrawNumPatch;
V_DrawBackground_f V_DrawBackground = NULL_DrawBackground;
V_PlotPixel_f V_PlotPixel = NULL_PlotPixel;
V_DrawLine_f V_DrawLine = NULL_DrawLine;

//
// V_InitMode
//
void V_InitMode(video_mode_t mode) {
#ifndef GL_DOOM
  if (mode == VID_MODEGL)
    mode = VID_MODE8;
#endif
  switch (mode) {
    case VID_MODE8:
      lprintf(LO_INFO, "V_InitMode: using 8 bit video mode\n");
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRect = V_FillRect8;
      V_DrawNumPatch = FUNC_V_DrawNumPatch;
      V_DrawBackground = FUNC_V_DrawBackground;
      V_PlotPixel = V_PlotPixel8;
      V_DrawLine = WRAP_V_DrawLine;
      current_videomode = VID_MODE8;
      break;
    case VID_MODE15:
      lprintf(LO_INFO, "V_InitMode: using 15 bit video mode\n");
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRect = V_FillRect15;
      V_DrawNumPatch = FUNC_V_DrawNumPatch;
      V_DrawBackground = FUNC_V_DrawBackground;
      V_PlotPixel = V_PlotPixel15;
      V_DrawLine = WRAP_V_DrawLine;
      current_videomode = VID_MODE15;
      break;
    case VID_MODE16:
      lprintf(LO_INFO, "V_InitMode: using 16 bit video mode\n");
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRect = V_FillRect16;
      V_DrawNumPatch = FUNC_V_DrawNumPatch;
      V_DrawBackground = FUNC_V_DrawBackground;
      V_PlotPixel = V_PlotPixel16;
      V_DrawLine = WRAP_V_DrawLine;
      current_videomode = VID_MODE16;
      break;
    case VID_MODE32:
      lprintf(LO_INFO, "V_InitMode: using 32 bit video mode\n");
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRect = V_FillRect32;
      V_DrawNumPatch = FUNC_V_DrawNumPatch;
      V_DrawBackground = FUNC_V_DrawBackground;
      V_PlotPixel = V_PlotPixel32;
      V_DrawLine = WRAP_V_DrawLine;
      current_videomode = VID_MODE32;
      break;
#ifdef GL_DOOM
    case VID_MODEGL:
      lprintf(LO_INFO, "V_InitMode: using OpenGL video mode\n");
      V_CopyRect = WRAP_gld_CopyRect;
      V_FillRect = WRAP_gld_FillRect;
      V_DrawNumPatch = WRAP_gld_DrawNumPatch;
      V_DrawBackground = WRAP_gld_DrawBackground;
      V_PlotPixel = V_PlotPixelGL;
      V_DrawLine = WRAP_gld_DrawLine;
      current_videomode = VID_MODEGL;
      break;
#endif
  }
  R_FilterInit();
}

//
// V_GetMode
//
video_mode_t V_GetMode(void) {
  return current_videomode;
}

//
// V_GetModePixelDepth
//
int V_GetModePixelDepth(video_mode_t mode) {
  switch (mode) {
    case VID_MODE8: return 1;
    case VID_MODE15: return 2;
    case VID_MODE16: return 2;
    case VID_MODE32: return 4;
    default: return 0;
  }
}

//
// V_GetNumPixelBits
//
int V_GetNumPixelBits(void) {
  switch (current_videomode) {
    case VID_MODE8: return 8;
    case VID_MODE15: return 15;
    case VID_MODE16: return 16;
    case VID_MODE32: return 32;
    default: return 0;
  }
}

//
// V_GetPixelDepth
//
int V_GetPixelDepth(void) {
  return V_GetModePixelDepth(current_videomode);
}

//
// V_AllocScreen
//
void V_AllocScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap)
    if ((scrn->byte_pitch * scrn->height) > 0)
      scrn->data = malloc(scrn->byte_pitch*scrn->height);
}

//
// V_AllocScreens
//
void V_AllocScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_AllocScreen(&screens[i]);
}

//
// V_FreeScreen
//
void V_FreeScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap) {
    free(scrn->data);
    scrn->data = NULL;
  }
}

//
// V_FreeScreens
//
void V_FreeScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_FreeScreen(&screens[i]);
}

static void V_PlotPixel8(int scrn, int x, int y, byte color) {
  screens[scrn].data[x+screens[scrn].byte_pitch*y] = color;
}

static void V_PlotPixel15(int scrn, int x, int y, byte color) {
  ((unsigned short *)screens[scrn].data)[x+screens[scrn].short_pitch*y] = VID_PAL15(color, VID_COLORWEIGHTMASK);
}

static void V_PlotPixel16(int scrn, int x, int y, byte color) {
  ((unsigned short *)screens[scrn].data)[x+screens[scrn].short_pitch*y] = VID_PAL16(color, VID_COLORWEIGHTMASK);
}

static void V_PlotPixel32(int scrn, int x, int y, byte color) {
  ((unsigned int *)screens[scrn].data)[x+screens[scrn].int_pitch*y] = VID_PAL32(color, VID_COLORWEIGHTMASK);
}

//
// WRAP_V_DrawLine()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
static void WRAP_V_DrawLine(fline_t* fl, int color)
{
  register int x;
  register int y;
  register int dx;
  register int dy;
  register int sx;
  register int sy;
  register int ax;
  register int ay;
  register int d;

#ifdef RANGECHECK         // killough 2/22/98
  static int fuck = 0;

  // For debugging only
  if
  (
       fl->a.x < 0 || fl->a.x >= SCREENWIDTH
    || fl->a.y < 0 || fl->a.y >= SCREENHEIGHT
    || fl->b.x < 0 || fl->b.x >= SCREENWIDTH
    || fl->b.y < 0 || fl->b.y >= SCREENHEIGHT
  )
  {
    //jff 8/3/98 use logical output routine
    lprintf(LO_DEBUG, "fuck %d \r", fuck++);
    return;
  }
#endif

#define PUTDOT(xx,yy,cc) V_PlotPixel(0,xx,yy,(byte)cc)

  dx = fl->b.x - fl->a.x;
  ax = 2 * (dx<0 ? -dx : dx);
  sx = dx<0 ? -1 : 1;

  dy = fl->b.y - fl->a.y;
  ay = 2 * (dy<0 ? -dy : dy);
  sy = dy<0 ? -1 : 1;

  x = fl->a.x;
  y = fl->a.y;

  if (ax > ay)
  {
    d = ay - ax/2;
    while (1)
    {
      PUTDOT(x,y,color);
      if (x == fl->b.x) return;
      if (d>=0)
      {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {
    d = ax - ay/2;
    while (1)
    {
      PUTDOT(x, y, color);
      if (y == fl->b.y) return;
      if (d >= 0)
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}

// V_DrawBox()
//
// Draw a box in the frame buffer, stretched to current resolution.
//
// Pass the coordinates of diagonal line of the box, and the color to be drawn
// Returns nothing
//
void V_DrawBox(fline_t* fl, int color)
{
  int x, y, ax, ay, bx, by;

  if (fl->a.x > fl->b.x) {
    ax = fl->b.x;
    bx = fl->a.x;
  } else {
    ax = fl->a.x;
    bx = fl->b.x;
  }
  if (fl->a.y > fl->b.y) {
    ay = fl->b.y;
    by = fl->a.y;
  } else {
    ay = fl->a.y;
    by = fl->b.y;
  }
  ax = ax * SCREENWIDTH / 320;
  bx = bx * SCREENWIDTH / 320;
  ay = ay * SCREENHEIGHT / 200;
  by = by * SCREENHEIGHT / 200;

  for (x = ax; x < bx; x++)
	 for (y = ay; y < by; y++)
        V_PlotPixel(0, x,y, (byte)color);
}
