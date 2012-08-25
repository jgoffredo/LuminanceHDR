/**
 * @brief Contrast mapping TMO
 *
 * From:
 * 
 * Rafal Mantiuk, Karol Myszkowski, Hans-Peter Seidel.
 * A Perceptual Framework for Contrast Processing of High Dynamic Range Images
 * In: ACM Transactions on Applied Perception 3 (3), pp. 286-308, 2006
 * http://www.mpi-inf.mpg.de/~mantiuk/contrast_domain/
 *
 * This file is a part of LuminanceHDR package, based on pfstmo.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2007 Grzegorz Krawczyk
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Radoslaw Mantiuk, <radoslaw.mantiuk@gmail.com>
 * @author Rafal Mantiuk, <mantiuk@gmail.com>
 * Updated 2007/12/17 by Ed Brambley <E.J.Brambley@damtp.cam.ac.uk>
 *
 * $Id: pfstmo_mantiuk06.cpp,v 1.9 2008/06/16 22:09:33 rafm Exp $
 */

#include <stdlib.h>
#include <fcntl.h>
#include <cmath>
#include <iostream>

#include "contrast_domain.h"

#include "Libpfs/pfs.h"
#include "Libpfs/colorspace.h"
//#include "Common/msec_timer.h"

namespace
{
// decode SRGB into RGB
template <typename T>
T decode(T value)
{
    if (value <= 0.0031308f )
    {
        return (value *= 12.92f);
    }
    return (1.055f * powf( value, 1.f/2.4f ) - 0.055f);
}

template <typename T>
void decodeVector(T* vector, size_t size)
{
    std::transform(vector, vector + size, vector, decode<T>);
}
}

void pfstmo_mantiuk06(pfs::Frame* frame, float scaleFactor, float saturationFactor, float detailFactor, bool cont_eq, ProgressHelper *ph)
{
  //--- default tone mapping parameters;
  //float scaleFactor = 0.1f;
  //float saturationFactor = 0.8f;
  //bool cont_map = false, cont_eq = false
  //bool cont_map = false;
  int itmax = 200;
  float tol = 1e-3f;
  
  std::cout << "pfstmo_mantiuk06 (";
  if (!cont_eq)
  {
    std::cout << "Mode: Contrast Mapping, ";
  }
  else 
  {
    scaleFactor = -scaleFactor;
    std::cout << "Mode: Contrast Equalization, ";
  }
  
  std::cout << "scaleFactor: " << scaleFactor;
  std::cout << ", saturationFactor: " << saturationFactor;
  std::cout << ", detailFactor: " << detailFactor << ")" << std::endl;
  
  pfs::Channel *inRed, *inGreen, *inBlue;
  frame->getXYZChannels(inRed, inGreen, inBlue);
  
  int cols = frame->getWidth();
  int rows = frame->getHeight();
    
  pfs::Array2D inY( cols, rows );
  pfs::transformRGB2Y(inRed->getChannelData(),
                      inGreen->getChannelData(),
                      inBlue->getChannelData(),
                      &inY);

  tmo_mantiuk06_contmap(cols, rows,
                        inRed->getRawData(),
                        inGreen->getRawData(),
                        inBlue->getRawData(),
                        inY.getRawData(),
                        scaleFactor, saturationFactor, detailFactor, itmax, tol,
                        ph);
  
  if ( !ph->isTerminationRequested() )
  {
      // pfs::transformColorSpace( pfs::CS_RGB, Xr, G, Zr, pfs::CS_XYZ, Xr, Yr, Zr );
      decodeVector(inRed->getRawData(), cols*rows);
      decodeVector(inGreen->getRawData(), cols*rows);
      decodeVector(inBlue->getRawData(), cols*rows);

      frame->getTags().setString("LUMINANCE", "RELATIVE");
  }
}
