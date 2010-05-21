/*
 * Retina.h
 *
 *  Created on: Jul 29, 2008
 *
 */

#ifndef RETINA_HPP_
#define RETINA_HPP_

#include "HyPerLayer.hpp"
#include "Image.hpp"
#include "fileread.h"

namespace PV
{

class Retina: public PV::HyPerLayer
{
public:
   Retina(const char * name, HyPerCol * hc);
#ifdef OBSOLETE
   Retina(const char * name, HyPerCol * hc, Image * img);
   Retina(const char * name, HyPerCol * hc, const char * filename);
#endif

   int initialize(PVLayerType type);
   int setParams(PVParams * params, fileread_params * p);

   virtual int updateState(float time, float dt);
   virtual int writeState(const char * path, float time);
   virtual int spike(float time, float dt, float prev, float prob, float probStim, float * probSpike);

#ifdef OBSOLETE
   Image * getImage()  { return img; }
#endif

private:

#ifdef OBSOLETE
   int updateImage(float time, float dt);
   int copyFromImageBuffer();
#endif

   // For input from a given source input layer, determine which
   // cells in this layer will respond to the input activity.
   // Return the feature vectors for both the input and the sensitive
   // neurons, since most likely we will have to determine those.
   int findPostSynaptic(int dim, int maxSize, int col,
      // input: which layer, which neuron
      HyPerLayer *lSource, float pos[],
      // output: how many of our neurons are connected.
      // an array with their indices.
      // an array with their feature vectors.
      int * nNeurons, int nConnectedNeurons[], float * vPos);

   int calculateWeights(HyPerLayer * lSource, float * pos, float * vPos, float * vfWeights );

#ifdef OBSOLETE
   Image * img;        // image object
   int fireOffPixels;  // controls firing of off (0) pixels
#endif
};

} // namespace PV

#endif /* RETINA_HPP_ */
