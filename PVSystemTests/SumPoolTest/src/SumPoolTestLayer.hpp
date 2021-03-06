#ifndef SUMPOOLTESTLAYER_HPP_ 
#define SUMPOOLTESTLAYER_HPP_

#include <layers/ANNLayer.hpp>

namespace PV {

class SumPoolTestLayer: public PV::ANNLayer{
public:
	SumPoolTestLayer(const char* name, HyPerCol * hc);
//   virtual int checkpointRead(const char * cpDir, double* timef);
//   virtual int checkpointWrite(const char * cpDir);

protected:
   int updateState(double timef, double dt);

private:
};

} /* namespace PV */
#endif
