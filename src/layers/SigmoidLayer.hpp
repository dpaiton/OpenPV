/*
 * CloneLayer.hpp
 * can be used to implement Sigmoid junctions
 *
 *  Created on: May 11, 2011
 *      Author: garkenyon
 */

#ifndef SIGMOIDLAYER_HPP_
#define SIGMOIDLAYER_HPP_

#include "HyPerLayer.hpp"
#include "LIF.hpp"

namespace PV {

// CloneLayer can be used to implement Sigmoid junctions between spiking neurons
class SigmoidLayer: public HyPerLayer {
public:
   SigmoidLayer(const char * name, HyPerCol * hc, LIF * clone);
   virtual ~SigmoidLayer();
   int initialize(LIF * clone);
   virtual int updateV();
   virtual int setActivity();
   virtual int resetPhiBuffers();
   LIF * sourceLayer;
};

}

#endif /* CLONELAYER_HPP_ */
