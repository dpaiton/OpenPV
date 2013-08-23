/*
 * Random.hpp
 *
 *  Created on: Aug 23, 2013
 *      Author: pschultz
 *
 *  A class to manage Tausworth random number generators so that
 *  random number generation is not affected by details of the
 *  MPI configuration.
 *
 *  Random(HyPerCol * hc, const PVLayerLoc * locptr, bool isExtended)
 *  creates an array of RNG states.
 *  The size of the array is determined by the local layer size specified in locptr,
 *  but the initial seeds are determined by global index.
 *  Hence the sequence of random numbers generated by a specific RNG is determined
 *  by global index, not local index, so it is independent of MPI configuration.
 */

#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include "HyPerCol.hpp"
#include "../layers/PVLayer.h"
#include "../utils/cl_random.h"

namespace PV {

class Random {
public:
   Random(HyPerCol * hc, unsigned int numBlocks, unsigned int blockLength, unsigned int numGlobalBlocks, unsigned int globalBlockLength, unsigned int startIndex);
   Random(HyPerCol * hc, int count);
   Random(HyPerCol * hc, const PVLayerLoc * locptr, bool isExtended);
   virtual ~Random();

   HyPerCol * getParentHyPerCol() {return parentHyPerCol;}
   unsigned int getNumBlocks() {return numBlocks;}
   unsigned int getBlockLength() {return blockLength;}
   unsigned int getNumGlobalBlocks() {return numGlobalBlocks;}
   unsigned int getGlobalBlockLength() {return globalBlockLength;}
   unsigned int getStartIndex() {return startIndex;}
   size_t getRNGArraySize() {return rngArraySize;}
   uint4 * getRNG(int index) {return &rngArray[index];}
   float uniformRandom(int localIndex);
   float uniformRandom(int localIndex, float min, float max) {return min+uniformRandom(localIndex)*(max-min);}
   void uniformRandom(float * values, int localIndex, int count=1) {for (int k=0; k<count; k++) values[k] = uniformRandom(localIndex+k);}
   void uniformRandom(float * values, int localIndex, int count, float min, float max) {for (int k=0; k<count; k++) values[k] = uniformRandom(localIndex+k,min,max);}

protected:
   Random();
   int initialize(HyPerCol * hc, unsigned int numBlocks, unsigned int blockLength, unsigned int numGlobalBlocks, unsigned int globalBlockLength, unsigned int startIndex);

private:
   int initialize_base();

// Member variables
protected:
   HyPerCol * parentHyPerCol;
   unsigned int numBlocks;
   unsigned int blockLength;
   unsigned int numGlobalBlocks;
   unsigned int globalBlockLength;
   unsigned int startIndex;
   uint4 * rngArray;
   size_t rngArraySize;
};

} /* namespace PV */
#endif /* RANDOM_HPP_ */
