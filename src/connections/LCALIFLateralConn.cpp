/*
 * LCALIFLateralConn.cpp
 *
 *  Created on: Oct 3, 2012
 *      Author: pschultz
 */

#include "LCALIFLateralConn.hpp"

namespace PV {

LCALIFLateralConn::LCALIFLateralConn()
{
   initialize_base();
}

LCALIFLateralConn::LCALIFLateralConn(const char * name, HyPerCol * hc, HyPerLayer * pre, HyPerLayer * post,
      const char * filename, InitWeights *weightInit) {
   initialize_base();
   initialize(name, hc, pre, post, filename, weightInit);
}

LCALIFLateralConn::~LCALIFLateralConn()
{
   free(integratedSpikeCount); integratedSpikeCount = NULL;
}

int LCALIFLateralConn::initialize_base() {
   integratedSpikeCount = NULL;
   return PV_SUCCESS;
}

int LCALIFLateralConn::initialize(const char * name, HyPerCol * hc, HyPerLayer * pre, HyPerLayer * post,
      const char * filename, InitWeights * weightInit) {
   int status = HyPerConn::initialize(name, hc, pre, post, filename, weightInit);
   LCALIFLayer * lcapre = dynamic_cast<LCALIFLayer * >(pre);
   LCALIFLayer * lcapost = dynamic_cast<LCALIFLayer *>(post);
   if (lcapre == NULL || lcapost == NULL) {
      if (parent->columnId()==0) {
         fprintf(stderr, "LCALIFLateralConn \"%s\": Presynaptic layer \"%s\" and postsynaptic layer \"%s\" must both be LCALIFLayers.\n", name, pre->getName(), post->getName());
      }
      abort();
   }
   const PVLayerLoc * preloc = pre->getLayerLoc();
   const PVLayerLoc * postloc = post->getLayerLoc();
   int nxpre = preloc->nx; int nxpost = postloc->nx;
   int nypre = preloc->ny; int nypost = postloc->ny;
   int nfpre = preloc->nf; int nfpost = postloc->nf;
   int nbpre = preloc->nb; int nbpost = postloc->nb;
   if (nxpre!=nxpost || nypre!=nypost || nfpre!=nfpost || nbpre!=nbpost) {
      if (parent->columnId()==0) {
         fprintf(stderr, "LCALIFLateralConn: pre- and post-synaptic layers must have the same geometry (including margin width)\n");
         fprintf(stderr, "  Pre:  nx=%d, ny=%d, nf=%d, nb=%d\n", nxpre, nypre, nfpre, nbpre);
         fprintf(stderr, "  Post: nx=%d, ny=%d, nf=%d, nb=%d\n", nxpost, nypost, nfpost, nbpost);
      }
      abort();
   }
   integratedSpikeCount = (float *) calloc(pre->getNumExtended(), sizeof(float)); // Spike counts initialized to 0
   return status;
}

int LCALIFLateralConn::setParams(PVParams * params) {
   int status = HyPerConn::setParams(params);
   integrationTimeConstant = readIntegrationTimeConstant();
   inhibitionTimeConstant = readAdaptationTimeConstant();
   return status;
}

int LCALIFLateralConn::calc_dW(int axonId) {
   assert(axonId>=0 && axonId < numberOfAxonalArborLists());
   updateIntegratedSpikeCount();
   pvdata_t * gSyn_buffer_start = post->getChannel(channel);
   LCALIFLayer * lcapre = dynamic_cast<LCALIFLayer *>(pre);
   assert(lcapre != NULL);
   pvdata_t targetRate = lcapre->getTargetRate();
   pvdata_t target_rate_sq = targetRate * targetRate;
   float dt_inh = parent->getDeltaTime()/inhibitionTimeConstant;
   float tausq = integrationTimeConstant*integrationTimeConstant;
   const PVPatchStrides * strides  = getPostNonextStrides();
   const int sx = strides->sx;
   const int sy = strides->sy;
   const int sf = strides->sf;
   const PVLayerLoc * postloc = post->getLayerLoc();
   const int nxpost = postloc->nx;
   const int nypost = postloc->ny;
   const int nfpost = postloc->nf;
   const int nbpost = postloc->nb;

   for (int kPre=0; kPre<getNumWeightPatches(); kPre++) {
      // The weight p(x,y,f) connects a presynaptic neuron in extended space to a postsynaptic neuron in restricted space.
      // We need to get the indices.  The presynaptic index is k.  To get the postsynaptic index, find
      // The memory location this weight is mapped to and subtract it from the start of the postsynaptic GSyn buffer.
      pvdata_t * gSyn_patch_start = getGSynPatchStart(kPre, axonId);
      int patch_start_index = gSyn_patch_start - gSyn_buffer_start;
      const PVPatch * p = getWeights(kPre, axonId);
      pvdata_t * dw_data = get_dwData(axonId,kPre);
      int nx = p->nx;
      int ny = p->ny;
      for (int y=0; y<ny; y++) {
         for (int x=0; x<nx; x++) {
            for (int f=0; f<nfp; f++) {
               int postindex = patch_start_index + sy*y + sx*x + sf*f;
               int postindexext = kIndexExtended(postindex, nxpost, nypost, nfpost, nbpost);
               pvdata_t delta_weight = dt_inh*(integratedSpikeCount[kPre]*integratedSpikeCount[postindexext]/tausq/target_rate_sq-1.0);
               dw_data[sxp*x + syp*y + sfp*f] = delta_weight;
            }
         }
      }
   }
   return PV_SUCCESS;
}

int LCALIFLateralConn::updateWeights(int axonId) {
   if (plasticityFlag) {
      for (int kPre=0; kPre<getNumWeightPatches(); kPre++) {
         const PVPatch * p = getWeights(kPre, axonId);
         pvdata_t * dw_data = get_dwData(axonId,kPre);
         pvdata_t * w_data = get_wData(axonId,kPre);
         int nx = p->nx;
         int ny = p->ny;
         for (int y=0; y<ny; y++) {
            for (int x=0; x<nx; x++) {
               for (int f=0; f<nfp; f++) {
                  w_data[sxp*x + syp*y + sfp*f] += dw_data[sxp*x + syp*y + sfp*f];
               }
            }
         }
      }
   }
   return PV_SUCCESS;
}

int LCALIFLateralConn::updateIntegratedSpikeCount() {
   float exp_dt_tau = exp(-parent->getDeltaTime()/integrationTimeConstant);
   pvdata_t * activity = pre->getActivity();
   for (int k=0; k<getNumWeightPatches(); k++) {
      integratedSpikeCount[k] = exp_dt_tau*(integratedSpikeCount[k]+activity[k]);
   }
   return PV_SUCCESS;
}

int LCALIFLateralConn::checkpointWrite(const char * cpDir) {
   int status = HyPerConn::checkpointWrite(cpDir);
   // This is kind of hacky, but we save the extended buffer integratedSpikeCount as if it were a nonextended buffer of size (nx+2*nb)-by-(ny+2*nb)
   char filename[PV_PATH_MAX];
   int chars_needed = snprintf(filename, PV_PATH_MAX, "%s/%s_integratedSpikeCount.pvp", cpDir, name);
   if (chars_needed >= PV_PATH_MAX) {
      fprintf(stderr, "LCALIFLateralConn::checkpointWrite error.  Path \"%s/%s_integratedSpikeCount.pvp\" is too long.\n", cpDir, name);
      abort();
   }
   PVLayerLoc loc;
   memcpy(&loc, pre->getLayerLoc(), sizeof(PVLayerLoc));
   loc.nx += 2*loc.nb;
   loc.ny += 2*loc.nb;
   loc.nxGlobal = loc.nx * parent->icCommunicator()->numCommColumns();
   loc.nyGlobal = loc.ny * parent->icCommunicator()->numCommRows();
   loc.nb = 0;
   write_pvdata(filename, parent->icCommunicator(), (double) parent->simulationTime(), integratedSpikeCount, &loc, PV_FLOAT_TYPE, /*extended*/ false, /*contiguous*/ false);
   return status;
}

int LCALIFLateralConn::checkpointRead(const char * cpDir, float* timef) {
   int status = HyPerConn::checkpointRead(cpDir, timef);
   char filename[PV_PATH_MAX];
   int chars_needed = snprintf(filename, PV_PATH_MAX, "%s/%s_integratedSpikeCount.pvp", cpDir, name);
   if (chars_needed >= PV_PATH_MAX) {
      fprintf(stderr, "LCALIFLateralConn::checkpointWrite error.  Path \"%s/%s_integratedSpikeCount.pvp\" is too long.\n", cpDir, name);
      abort();
   }
   double timed;
   PVLayerLoc loc;
   memcpy(&loc, pre->getLayerLoc(), sizeof(PVLayerLoc));
   loc.nx += 2*loc.nb;
   loc.ny += 2*loc.nb;
   loc.nxGlobal = loc.nx * parent->icCommunicator()->numCommColumns();
   loc.nyGlobal = loc.ny * parent->icCommunicator()->numCommRows();
   loc.nb = 0;
   read_pvdata(filename, parent->icCommunicator(), &timed, integratedSpikeCount, &loc, PV_FLOAT_TYPE, /*extended*/ false, /*contiguous*/ false);
   if( (float) timed != *timef && parent->icCommunicator()->commRank() == 0 ) {
      fprintf(stderr, "Warning: %s and %s_A.pvp have different timestamps: %f versus %f\n", filename, name, (float) timed, *timef);
   }

   return status;
}

} /* namespace PV */
