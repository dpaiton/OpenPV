/*
 * CopyConn.hpp
 *
 *  Created on: Nov 19, 2014
 *      Author: pschultz
 */

#ifndef COPYCONN_HPP_
#define COPYCONN_HPP_

#include "HyPerConn.hpp"

namespace PV {

class CopyConn: public HyPerConn {
public:
   CopyConn(char const * name, HyPerCol * hc);
   virtual ~CopyConn();
   virtual int communicateInitInfo();
   virtual bool needUpdate(double time, double dt);

   char const * getOriginalConnName() { return originalConnName; }
   HyPerConn * getOriginalConn() { return originalConn; }

protected:
   CopyConn();
   int initialize(char const * name, HyPerCol * hc);
   virtual int ioParamsFillGroup(enum ParamsIOFlag ioFlag);
   /**
    * List of parameters needed from the CopyConn class
    * @name CopyConn Parameters
    * @{
    */

   /**
    * @brief CopyConn inherits sharedWeights from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_sharedWeights(enum ParamsIOFlag ioFlag);

   /**
    * @brief weightInitType is not used by CopyConn.
    */
   virtual void ioParam_weightInitType(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits nxp from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_nxp(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits nyp from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_nyp(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits nxpShrunken from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_nxpShrunken(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits nypShrunken from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_nypShrunken(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits nfp from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_nfp(enum ParamsIOFlag ioFlag);

   /**
    * @brief initializeFromCheckpointFlag is not used by CopyConn.
    */
   virtual void ioParam_initializeFromCheckpointFlag(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits numAxonalArbors from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_numAxonalArbors(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits plasticityFlag from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_plasticityFlag(enum ParamsIOFlag ioFlag);

   /**
    * @brief CopyConn inherits triggerFlag from the original connection, instead of reading it from parameters
    */
   virtual void ioParam_triggerFlag(enum ParamsIOFlag ioFlag);

   /**
    * @brief weightUpdatePeriod is not used by CopyConn.
    */
   virtual void ioParam_weightUpdatePeriod(enum ParamsIOFlag ioFlag);

   /**
     * @brief initialWeightUpdateTime is not used by CopyConn.
     */
   virtual void ioParam_initialWeightUpdateTime(enum ParamsIOFlag ioFlag);

   /**
    * @brief originalConnName (required): The name of the connection the weights will be copied from
    */
   virtual void ioParam_originalConnName(enum ParamsIOFlag ioFlag);
   /** @} */

   virtual int setPatchSize();

   virtual int setInitialValues();
   virtual PVPatch *** initializeWeights(PVPatch *** arbors, pvwdata_t ** dataStart);

   virtual int updateWeights(int arborId = 0);
   int copy(int arborId = 0);

   char * originalConnName;
   HyPerConn * originalConn;

private:
   int initialize_base();
};

} /* namespace PV */

#endif /* COPYCONN_HPP_ */
