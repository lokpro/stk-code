//  $Id: SkidMark.h,v 1.4 2004/08/14 20:59:27 straver Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_SKIDMARK_CXX
#define HEADER_SKIDMARK_CXX

#include <plib/ssg.h>

/** Helper class to manage a skidmark */
class SkidMark : public ssgVtxTable
{
public:
  SkidMark();
  ~SkidMark();

  /** Add a position where the skidmark is */
  void add(sgCoord* coord);
  void addBreak (sgCoord *coord);

  void recalcBSphere();
  
  int newSkidmark;
};

#endif

/* EOF */
