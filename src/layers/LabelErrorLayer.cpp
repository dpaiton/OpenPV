/*
 * LabelErrorLayer.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: garkenyon
 */

#include "LabelErrorLayer.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void LabelErrorLayer_update_state(
    const int numNeurons,
    const int nx,
    const int ny,
    const int nf,
    const int nb,

    float * V,
    const float Vth,
    const float VMax,
    const float VMin,
    const float VShift,
    float * GSynHead,
    float * activity,
    float errScale,
    int isBinary);


#ifdef __cplusplus
}
#endif

namespace PV {

LabelErrorLayer::LabelErrorLayer()
{
   initialize_base();
}

LabelErrorLayer::LabelErrorLayer(const char * name, HyPerCol * hc, int num_channels)
{
   initialize_base();
   assert(num_channels == 2);
   initialize(name, hc, num_channels);
}

LabelErrorLayer::LabelErrorLayer(const char * name, HyPerCol * hc)
{
   initialize_base();
   initialize(name, hc, 2);
}

LabelErrorLayer::~LabelErrorLayer()
{
}

int LabelErrorLayer::initialize_base()
{
   errScale = 1;
   isBinary = 1;
   return PV_SUCCESS;
}

int LabelErrorLayer::initialize(const char * name, HyPerCol * hc, int num_channels)
{
   int status = ANNLayer::initialize(name, hc, num_channels);
   PVParams * params = parent->parameters();
   status |= readErrScale(params);
   return status;
}

int LabelErrorLayer::setParams(PVParams * params){
   int status = ANNLayer::setParams(params);
   status |= readErrScale(params);
   status |= readIsBinary(params);
   return status;
}

int LabelErrorLayer::readErrScale(PVParams * params){
    errScale = params->value(name, "errScale", errScale);
    return PV_SUCCESS;
}

int LabelErrorLayer::readIsBinary(PVParams * params){
    isBinary = params->value(name, "isBinary", isBinary);
    return PV_SUCCESS;
}

int LabelErrorLayer::doUpdateState(double time, double dt, const PVLayerLoc * loc, pvdata_t * A,
      pvdata_t * V, int num_channels, pvdata_t * gSynHead, bool spiking,
      unsigned int * active_indices, unsigned int * num_active)
{
   update_timer->start();
#ifdef PV_USE_OPENCL
   if(gpuAccelerateFlag) {
      updateStateOpenCL(time, dt);
      //HyPerLayer::updateState(time, dt);
   }
   else {
#endif
      int nx = loc->nx;
      int ny = loc->ny;
      int nf = loc->nf;
      int num_neurons = nx*ny*nf;
    	  LabelErrorLayer_update_state(num_neurons, nx, ny, nf, loc->nb, V, VThresh,
    			  VMax, VMin, VShift, gSynHead, A, errScale, isBinary);
      if (this->writeSparseActivity){
         updateActiveIndices();  // added by GTK to allow for sparse output, can this be made an inline function???
      }
#ifdef PV_USE_OPENCL
   }
#endif

   update_timer->stop();
   return PV_SUCCESS;
}

} /* namespace PV */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef PV_USE_OPENCL
#  include "../kernels/LabelErrorLayer_update_state.cl"
#else
#  undef PV_USE_OPENCL
#  include "../kernels/LabelErrorLayer_update_state.cl"
#  define PV_USE_OPENCL
#endif

#ifdef __cplusplus
}
#endif
