/* psplib/pl_gfx.c
   Graphics rendering routines

   Copyright (C) 2007-2009 Akop Karapetyan

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Author contact information: dev@psp.akop.org
*/

#include <psp2/display.h>
#include <vita2d.h>
#include <math.h>

#include "pl_gfx.h"
#include "pl_image.h"

int pl_gfx_init(unsigned int format)
{
  return vita2d_init();
}

void pl_gfx_shutdown()
{
  vita2d_fini();
}
