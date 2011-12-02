/*
 * Movie.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: travel
 */

#include "Movie.hpp"
#include "../io/imageio.hpp"
#include "../utils/pv_random.h"
#include "../include/default_params.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

namespace PV {

Movie::Movie(const char * name, HyPerCol * hc, const char * fileOfFileNames)
     : Image(name, hc)
{
    initializeMovie(name, hc, fileOfFileNames, DISPLAY_PERIOD);
}

Movie::Movie(const char * name, HyPerCol * hc, const char * fileOfFileNames, float defaultDisplayPeriod)
     : Image(name, hc)
{
   initializeMovie(name, hc, fileOfFileNames, defaultDisplayPeriod);
}

//
/*
 * Notes:
 * - offsetX and offsetY are initialized by Image(name,hc)
 * - writeImages is initialized by Image(name,hc)
 */
int Movie::initializeMovie(const char * name, HyPerCol * hc, const char * fileOfFileNames, float defaultDisplayPeriod) {

   PVLayerLoc * loc = &clayer->loc;
   movieOutputPath = NULL;

   if( getParent()->icCommunicator()->commRank()==0 ) {
      fp = fopen(fileOfFileNames, "r");
      if( fp == NULL ) {
         fprintf(stderr, "Unable to open %s\n", fileOfFileNames);
         fprintf(stderr, "Error code %d\n", errno);
         exit(EXIT_FAILURE);
      }
   }

   filename = strdup(getNextFileName());
   assert(filename != NULL);

   // get size info from image so that data buffer can be allocated
   int status = getImageInfo(filename, parent->icCommunicator(), &imageLoc);
   if(status != 0) {
      fprintf(stderr, "Movie: Unable to get image info for \"%s\"\n", filename);
      exit(EXIT_FAILURE);
   }

   // create mpi_datatypes for border transfer
   mpi_datatypes = Communicator::newDatatypes(loc);

//   int N = imageLoc.nx * imageLoc.ny * imageLoc.nBands;
//   imageData = new float [N];
//   for (int i = 0; i < N; ++i) {
//      imageData[i] = 0;
//   }
   imageData = NULL;

   // Image::read takes care of grayscale, so loc->nf should be left alone.
   // loc->nf = imageLoc.nf;

   //
   PVParams * params = hc->parameters();
   this->displayPeriod = params->value(name,"displayPeriod", defaultDisplayPeriod);
   nextDisplayTime = hc->simulationTime() + this->displayPeriod;

   // offsetX = (int) params->value(name,"offsetX", 0); // reading offsetX,offsetY moved to Image
   // offsetY = (int) params->value(name,"offsetY", 0);

   resetPositionInBounds();  // ensure that offsets keep loc within image bounds

   jitterFlag = params->value(name,"jitterFlag", 0) != 0;
   writePosition = 0;
   if( jitterFlag ) {
      stepSize          = (int) params->value(name, "stepSize", 0);
      persistenceProb   = params->value(name,"persistenceProb", 1.0);
      recurrenceProb    = params->value(name,"recurrenceProb", 1.0);
      biasChangeTime    = (int) params->value(name,"biasChangeTime", 1000);
      writePosition     = (int) params->value(name,"writePosition", 1);
      biasX   = offsetX;
      biasY   = offsetY;
   }
   randomMovie       = (int) params->value(name,"randomMovie",0);
   if( randomMovie ) {
      randomMovieProb   = params->value(name,"randomMovieProb", 0.05);  // 100 Hz
   }

   if(randomMovie){
      // random number generator already initialized by HyPerCol
      //unsigned long seed = (unsigned long) time ((time_t *) NULL);
      //pv_srandom(seed); // initialize seed
      randomFrame();
   }else{
      read(filename, offsetX, offsetY);
   }

   // set output path for movie frames
   if(writeImages){
      if ( params->stringPresent(name, "movieOutputPath") ) {
         movieOutputPath = strdup(params->stringValue(name, "movieOutputPath"));
         assert(movieOutputPath != NULL);
      }
      else {
         movieOutputPath = strdup( hc->getOutputPath());
         assert(movieOutputPath != NULL);
         printf("Movie output path is not specified in params file.\n"
               "Movie output path set to default \"%s\"\n",movieOutputPath);
      }
   }

   if(writePosition){
      assert(jitterFlag);
      char file_name[PV_PATH_MAX];
      if (movieOutputPath != NULL){
         //I don't know why someone was saving the return value, but it was
         //generating a compiler warning because it was unused.  Did someone
         //want to use this for something?
         snprintf(file_name, PV_PATH_MAX-1, "%s/image-pos.txt", movieOutputPath);
         //int nchars = snprintf(file_name, PV_PATH_MAX-1, "%s/image-pos.txt", movieOutputPath);
      }
      else{
         snprintf(file_name, PV_PATH_MAX-1, "%s/image-pos.txt", hc->getOutputPath());
         //int nchars = snprintf(file_name, PV_PATH_MAX-1, "%s/image-pos.txt", hc->getOutputPath());
      }
      printf("write position to %s\n",file_name);
      fp_pos = fopen(file_name,"a");
      assert(fp_pos != NULL);
      fprintf(fp_pos,"%f %s: \n%d %d\t\t%f %d %d\n",hc->simulationTime(),filename,biasX,biasY,
            hc->simulationTime(),offsetX,offsetY);
   }

   // grayScale call moved to Image::read, called immediately above
   // if (loc->nf > 1) {
   //    toGrayScale();
   // }

   // exchange border information
   exchange();

   return PV_SUCCESS;
}

Movie::~Movie()
{
   if (imageData != NULL) {
      delete imageData;
      imageData = NULL;
   }
   if (getParent()->icCommunicator()->commRank()==0 && fp != NULL && fp != stdout) {
      fclose(fp);
   }

   if(writePosition){
      if (getParent()->icCommunicator()->commRank()==0 && fp_pos != NULL && fp_pos != stdout) {
            fclose(fp_pos);
         }
   }
}

pvdata_t * Movie::getImageBuffer()
{
//   return imageData;
   return data;
}

PVLayerLoc Movie::getImageLoc()
{
   return imageLoc;
//   return clayer->loc;
   // imageLoc contains size information of the image file being loaded;
   // clayer->loc contains size information of the layer, which may
   // be smaller than the whole image.  To get clayer->loc, use
   // getLayerLoc().  --pete 2011-07-10
}

int Movie::updateState(float time, float dt)
{
  updateImage(time, dt);
  return 0;
}

/**
 * - Update the image buffers
 * - If the time is a multiple of biasChangetime then the position of the bias (biasX, biasY) changes.
 * - With probability persistenceProb the offset position (offsetX, offsetY) remains unchanged.
 * - Otherwise, with probability (1-persistenceProb) the offset position performs a random walk
 *   around the bias position (biasX, biasY).
 *
 * - If the time is a multiple of displayPeriod then load the next image.
 * - If nf=1 then the image is converted to grayscale during the call to read(filename, offsetX, offsetY).
 *   If nf>1 then the image is loaded with color information preserved.
 * - Return true if buffers have changed
 */
bool Movie::updateImage(float time, float dt)
{
   PVLayerLoc * loc = &clayer->loc;

   if(randomMovie){
      randomFrame();
      lastUpdateTime = time;
   } else {
      bool needNewImage = false;
      if (time >= nextDisplayTime) {
         needNewImage = true;
         if (filename != NULL) free(filename);
         filename = strdup(getNextFileName());
         assert(filename != NULL);
         nextDisplayTime += displayPeriod;

         if(writePosition){
            fprintf(fp_pos,"%f %s: \n",time,filename);
         }
         lastUpdateTime = time;
      } // time >= nextDisplayTime

      if( jitterFlag ) {
         // move bias
         if( time > 0 && !(((int)time) % biasChangeTime) ){
            calcBias(stepSize, imageLoc.nx - loc->nx - stepSize);
         }

         // move offset
         double p = pv_random_prob();
         if (p > persistenceProb){
            needNewImage = true;
            calcBiasedOffset(stepSize, imageLoc.nx - loc->nx - stepSize);
            if(writePosition){
               fprintf(fp_pos,"%d %d ",biasX,biasY);
            }
         }
         // ensure that offsets keep loc within image bounds
         resetPositionInBounds();
         if(writePosition){
            fprintf(fp_pos,"\t\t%f %d %d\n",time,offsetX,offsetY);
         }
         lastUpdateTime = time;
      } // jitterFlag

      if( needNewImage ){
         read(filename, offsetX, offsetY);
      }
   } // randomMovie

   // exchange border information
   exchange();

   return true;
}

/**
 * When we play a random frame - in order to perform a reverse correlation analysis -
 * we call writeActivitySparse(time) in order to write the "activity" in the image.
 *
 */
int Movie::outputState(float time, bool last)
{
   if (writeImages) {
      char basicFilename[PV_PATH_MAX + 1];
      snprintf(basicFilename, PV_PATH_MAX, "%s/Movie_%.2f.tif", movieOutputPath, time);
      write(basicFilename);
   }

   int status = PV_SUCCESS;
   if (randomMovie != 0) {
      status = writeActivitySparse(time);
   }

   return status;
}

int Movie::copyReducedImagePortion()
{
   const PVLayerLoc * loc = getLayerLoc();

   const int nx = loc->nx;
   const int ny = loc->ny;

   const int nx0 = imageLoc.nx;
   const int ny0 = imageLoc.ny;

   assert(nx0 <= nx);
   assert(ny0 <= ny);

   const int i0 = nx/2 - nx0/2;
   const int j0 = ny/2 - ny0/2;

   int ii = 0;
   for (int j = j0; j < j0+ny0; j++) {
      for (int i = i0; i < i0+nx0; i++) {
         imageData[ii++] = data[i+nx*j];
      }
   }

   return 0;
}

/**
 * This creates a random image patch (frame) that is used to perform a reverse correlation analysis
 * as the input signal propagates up the visual system's hierarchy.
 * NOTE: Check Image::toGrayScale() method which was the inspiration for this routine
 */
int Movie::randomFrame()
{
   assert(randomMovie); // randomMovieProb was set only if randomMovie is true
   for (int kex = 0; kex < clayer->numExtended; kex++) {
      data[kex] = (pv_random_prob() < randomMovieProb) ? 1: 0;
   }
   return 0;
}

const char * Movie::getNextFileName()
{
   InterColComm * icComm = getParent()->icCommunicator();
   if( icComm->commRank()==0 ) {
      int c;
      size_t len = PV_PATH_MAX;

      // if at end of file (EOF), rewind

      if ((c = fgetc(fp)) == EOF) {
         rewind(fp);
      }
      else {
         ungetc(c, fp);
      }

      char * path = fgets(inputfile, len, fp);

      if (path != NULL) {
         path[PV_PATH_MAX-1] = '\0';
         len = strlen(path);
         if (len > 1) {
            if (path[len-1] == '\n') {
               path[len-1] = '\0';
            }
         }
      }
   }
#ifdef PV_USE_MPI
   MPI_Bcast(inputfile, PV_PATH_MAX, MPI_CHAR, 0, icComm->communicator());
#endif // PV_USE_MPI
   return inputfile;
}


/**
 * The bias position (biasX, biasY) changes here.
 * It can perform a random walk of step 1 or it can perform a random jump up to a maximum length
 * equal to step.
 */
void Movie::calcBias(int step, int sizeLength)
{
   assert(jitterFlag);
   const float dp = 1.0 / step;
   double p;
   const int random_walk = 0;
   const int random_jump = 1;


   if (random_walk) {
      p = pv_random_prob();
      if (p < 0.5) {
         biasX += step;
      } else {
         biasX -= step;
      }
      p = pv_random_prob();
      if (p < 0.5) {
         biasY += step;
      } else {
         biasY -= step;
      }
   } else if (random_jump) {
      p = pv_random_prob();
      for (int i = 0; i < step; i++) {
         if ((i * dp < p) && (p < (i + 1) * dp)) {
            biasX = (pv_random_prob() < 0.5) ? biasX - i : biasX + i;
         }
      }
      p = pv_random_prob();
      for (int i = 0; i < step; i++) {
         if ((i * dp < p) && (p < (i + 1) * dp)) {
            biasY = (pv_random_prob() < 0.5) ? biasY - i : biasY + i;
         }
      }
   }

   biasX = (biasX < 0) ? -biasX : biasX;
   biasX = (biasX > sizeLength) ? sizeLength - (biasX-sizeLength) : biasX;

   biasY = (biasY < 0) ? -biasY : biasY;
   biasY = (biasY > sizeLength) ? sizeLength - (biasY-sizeLength) : biasY;

   return;
}

/**
 * Return an offset that moves randomly around position (biasX, biasY)
 * With probability recurenceProb the offset returns to its bias position
 * (biasX,biasY). Otherwise, with probability (1-recurrenceProb) performa a
 * random jump of maximum length equal to step.
 */
void Movie::calcBiasedOffset(int step, int sizeLength)
{
   assert(jitterFlag); // calcBiasedOffset should only be called when jitterFlag is true
   const float dp = 1.0 / step;
   double p = pv_random_prob();

   if (p > recurrenceProb){
      p = pv_random_prob();
      for (int i = 0; i < step; i++) {
         if ((i * dp < p) && (p < (i + 1) * dp)) {
            offsetX = (pv_random_prob() < 0.5) ? offsetX - i : offsetX + i;
         }
      }
      p = pv_random_prob();
      for (int i = 0; i < step; i++) {
         if ((i * dp < p) && (p < (i + 1) * dp)) {
            offsetY = (pv_random_prob() < 0.5) ? offsetY - i : offsetY + i;
         }
      }
      offsetX = (offsetX < 0) ? -offsetX : offsetX;
      offsetX = (offsetX > sizeLength) ? sizeLength - (offsetX-sizeLength) : offsetX;

      offsetY = (offsetY < 0) ? -offsetY : offsetY;
      offsetY = (offsetY > sizeLength) ? sizeLength - (offsetY-sizeLength) : offsetY;
   } else {
      offsetX = biasX;
      offsetY = biasY;
   }

   return;
}


/**
 * Return an integer between 0 and (step-1)
 */
int Movie::calcPosition(int pos, int step, int sizeLength)
{
   const float dp = 1.0 / step;
   const double p = pv_random_prob();
   const int random_walk = 0;
   const int move_forward = 0;
   const int move_backward = 0;
   const int random_jump = 1;

   if (random_walk) {
      if (p < 0.5) {
         pos += step;
      } else {
         pos -= step;
      }
   } else if (move_forward) {
      pos += step;
   } else if (move_backward) {
      pos -= step;
   } else if (random_jump) {
      for (int i = 0; i < step; i++) {
         if ((i * dp < p) && (p < (i + 1) * dp)) {
            pos = (pv_random_prob() < 0.5) ? pos - i : pos + i;
         }
      }
   }

   pos = (pos < 0) ? -pos : pos;
   pos = (pos > sizeLength) ? sizeLength - (pos-sizeLength) : pos;

   return pos;
}


int Movie::resetPositionInBounds()
{
   PVLayerLoc * loc = &clayer->loc;

   // apply circular boundary conditions
   //
   if (offsetX < 0) offsetX += imageLoc.nx;
   if (offsetY < 0) offsetY += imageLoc.ny;
   offsetX = (imageLoc.nx < offsetX + loc->nx) ? imageLoc.nx - offsetX : offsetX;
   offsetY = (imageLoc.ny < offsetY + loc->ny) ? imageLoc.ny - offsetY : offsetY;

   // could still be out of bounds
   //
   offsetX = (offsetX < 0 || imageLoc.nx < offsetX + loc->nx) ? 0 : offsetX;
   offsetY = (offsetY < 0 || imageLoc.ny < offsetY + loc->ny) ? 0 : offsetY;

   return 0;
}

}
