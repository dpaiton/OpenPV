/*
 * pv.cpp
 *
 */

#include <columns/buildandrun.hpp>
#include "DelayTestProbe.hpp"

void * addcustomgroup(const char * keyword, const char * groupname, HyPerCol * hc);

int main(int argc, char * argv[]) {
   int status = buildandrun(argc, argv, NULL, NULL, &addcustomgroup);
   return status==PV_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

void * addcustomgroup(const char * keyword, const char * groupname, HyPerCol * hc) {
   int status;
   LayerProbe * addedProbe = NULL;
   HyPerLayer * targetlayer;
   char * message = NULL;
   const char * filename;
   if( !strcmp( keyword, "DelayTestProbe") ) {
      addedProbe =  new DelayTestProbe(groupname, hc);
      checknewobject((void *) addedProbe, keyword, groupname, hc);
      return addedProbe;
   }

   assert(!addedProbe);
   fprintf(stderr, "Unrecognized params keyword \"%s\"\n", keyword);
   return addedProbe;
}
