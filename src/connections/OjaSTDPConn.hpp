/*
 * OjaSTDPConn.h
 *
 *  Created on: Sep 27, 2012
 *      Author: dpaiton et slundquist
 */

#ifndef OJASTDPCONN_H_
#define OJASTDPCONN_H_

#include "HyPerConn.hpp"
#include "../include/default_params.h"
#include <stdio.h>

namespace PV {

class OjaSTDPConn: HyPerConn {
public:
   OjaSTDPConn();
   OjaSTDPConn(const char * name, HyPerCol * hc, HyPerLayer * pre, HyPerLayer * post,
            const char * filename=NULL, bool stdpFlag=true,
            InitWeights *weightInit=NULL);
   virtual ~OjaSTDPConn();

   int setParams(PVParams * params);

   virtual int initializeThreadBuffers();
   virtual int initializeThreadKernels();

   virtual float maxWeight(int axonID);

   virtual int updateState(float time, float dt);
   virtual int updateWeights(int axonID);
   virtual int outputState(float time, bool last=false);
   virtual int writeTextWeightsExtra(FILE * fd, int k, int axonID);

protected:

   int initialize_base();
   int initialize(const char * name, HyPerCol * hc,
                  HyPerLayer * pre, HyPerLayer * post,
                  const char * filename, bool stdpFlag, InitWeights *weightInit);
   virtual int initPlasticityPatches();

   PVLayerCube * post_tr;      // plasticity decrement variable for postsynaptic layer
   PVLayerCube * pre_tr;       // plasticity increment variable for presynaptic layer

   bool stdpFlag;              // presence of spike timing dependent plasticity

   // STDP parameters for modifying weights
   float ampLTP; // long term potentiation amplitude
   float ampLTD; // long term depression amplitude
   float tauLTP;
   float tauLTD;
   float tauLTPLong;
   float tauLTDLong;
   float weightDecay;
   float dWMax;
   float ojaScale;
   float STDPScale;

   bool  synscalingFlag;
   float synscaling_v;

#ifdef OBSOLETE_STDP
   PVPatch       *** dwPatches;      // list of stdp patches Psij variable
#endif
#ifdef OBSOLETE
   int pvpatch_update_weights_localWMax(int nk, float * RESTRICT w, const float * RESTRICT m,
                              const float * RESTRICT p, float aPre,
                              const float * RESTRICT aPost, float dWMax, float wMin, float * RESTRICT Wmax);
#endif // OBSOLETE

private:
   int deleteWeights();

};

}

#endif /* OJASTDPCONN_H_ */
