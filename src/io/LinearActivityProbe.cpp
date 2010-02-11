/*
 * ProbeActivityLinear.cpp
 *
 *  Created on: Mar 7, 2009
 *      Author: rasmussn
 */

#include "LinearActivityProbe.hpp"

namespace PV {

/**
 * @hc
 * @dim
 * @kLoc
 * @f
 */
LinearActivityProbe::LinearActivityProbe(HyPerCol * hc, PVDimType dim, int linePos, int f)
   : LayerProbe()
{
   this->parent = hc;
   this->dim = dim;
   this->linePos = linePos;
   this->f   = f;
}

/**
 * @filename
 * @hc
 * @dim
 * @kLoc
 * @f
 */
LinearActivityProbe::LinearActivityProbe(const char * filename, HyPerCol * hc, PVDimType dim, int linePos, int f)
    : LayerProbe(filename)
{
   this->parent = hc;
   this->dim = dim;
   this->linePos = linePos;
   this->f   = f;
}

/**
 * @time
 * @l
 */
int LinearActivityProbe::outputState(float time, HyPerLayer * l)
{
   int width, sLine;
   float * line;

   const PVLayer * clayer = l->clayer;

   float * activity = clayer->activity->data;

   const int nx = clayer->loc.nx;
   const int ny = clayer->loc.ny;
   const int nf = clayer->numFeatures;

   const int marginWidth = clayer->loc.nPad;

   float dt = parent->getDeltaTime();

   double sum = 0.0;
   float freq;

   if (dim == DimX) {
      width = nx + 2*marginWidth;
      line = activity + (linePos+marginWidth) * width * nf;
      sLine = nf;
   }
   else {
      width = ny + 2*marginWidth;
      line = activity + (linePos+marginWidth)*nf;
      sLine = nf * (nx + 2*marginWidth);

   }

   for (int k = 0; k < width; k++) {
     float a = line[f + k * sLine];
     sum += a;
   }

   freq = sum / (width * dt * 0.001);
   fprintf(fp, "t=%6.1f sum=%3d f=%6.1f Hz :", time, (int)sum, freq);

   for (int k = 0; k < width; k++) {
     float a = line[f + k * sLine];
     if (a > 0.0) fprintf(fp, "*");
     else         fprintf(fp, " ");
   }
   fprintf(fp, ":\n");
   fflush(fp);

   return 0;
}

} // namespace PV
