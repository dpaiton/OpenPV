/*
 * RandomPatchMovie.cpp
 *
 *      Author: pschultz
 * Like Movie, but whenever the next image is loaded, offsetX and offsetY
 * are chosen randomly from the set of allowable values
 * (offset >= 0 && offsetX < imagesize - patchsize)
 */

#include "RandomPatchMovie.hpp"

namespace PV {

RandomPatchMovie::RandomPatchMovie() {
   initialize_base();
}

RandomPatchMovie::RandomPatchMovie(const char * name, HyPerCol * hc) {
   initialize_base();
   initialize(name, hc);
}

int RandomPatchMovie::initialize_base() {
   displayPeriod = DISPLAY_PERIOD;
   numImageFiles = 0;
   imageFilenameIndices = NULL;
   imageListPath = NULL;
   listOfImageFiles = NULL;
   patchposfile = NULL;
   return PV_SUCCESS;
}

int RandomPatchMovie::initialize(const char * name, HyPerCol * hc) {
   Image::initialize(name, hc);
#ifdef PV_USE_MPI
   int rank = parent->icCommunicator()->commRank();
#else // PV_USE_MPI
   int rank = 0;
#endif // PV_USE_MPI

   PVParams * params = parent->parameters();
   int rootproc = 0;
   if( rank == rootproc) {
      FILE * fp = fopen(imageListPath, "rb");
      if( fp == NULL ) {
         fprintf(stderr, "RandomPatchMovie \"%s\" error opening \"%s\" for reading: %s\n", name, imageListPath, strerror(errno));
         fprintf(stderr, "Error code %d\n", errno);
         exit(EXIT_FAILURE);
      }
      int status = fseek(fp, 0L, SEEK_END);
      if( status != 0 ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": unable to find end of file \"%s\"\n", name, imageListPath);
         fclose(fp);
         exit(EXIT_FAILURE);
      }
      errno = 0;
      long int filelength = ftell(fp);
      if( filelength == -1L && errno ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": unable to determine length of file \"%s\"\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      if( filelength < 0 || filelength > INT_MAX) {
         fprintf(stderr, "RandomPatchMovie \"%s\": file \"%s\" is too long.\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      fseek(fp, 0L, SEEK_SET);
      size_t filesize = (size_t) filelength;
      if( filesize == 0 ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": file of filenames \"%s\" is empty.\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      listOfImageFiles = (char *) malloc( (filesize+1) * sizeof(char) );
      if( listOfImageFiles == NULL ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": unable to allocate memory to hold file \"%s\".\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      size_t charsread = fread(listOfImageFiles, sizeof(char), filesize, fp);
      if( charsread != filesize ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": unable to read file \"%s\".\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      fclose(fp);
      fp = NULL;

      patchposfilename = params->stringValue(name, "patchPosFilename", true);
      if( patchposfilename && patchposfilename[0] != 0 ) {
         char patchpospath[PV_PATH_MAX];
         int actuallength = snprintf(patchpospath, PV_PATH_MAX, "%s/%s", parent->getOutputPath(), patchposfilename);
         if( actuallength >= PV_PATH_MAX ) {
            fprintf(stderr, "Error: RandomPatchMovie \"%s\": patchposfilename \"%s\" too long.  Exiting.\n", name, patchposfilename);
            exit(EXIT_FAILURE);
         }
         patchposfile = fopen(patchpospath, "w");
         if( patchposfile == NULL ) {
            fprintf(stderr, "RandomPatchMovie \"%s\": Unable to open filename \"%s\" for writing patch filenames and positions.  Exiting.", name, filename);
            exit(EXIT_FAILURE);
         }
      }

      skipLowContrastPatchProb = params->value(name, "skipLowContrastPatchProb", 0.0f);
      if( skipLowContrastPatchProb > 0 ) {
    	  lowContrastThreshold = params->value(name, "lowContrastThreshold", 0.0f);
      }

      numImageFiles = 0;
      if( listOfImageFiles[0] == '\n' ) listOfImageFiles[0] = 0;
      for( int n=1; n<(int) filesize; n++ ) {
         if( listOfImageFiles[n] == '\n' ) {
            listOfImageFiles[n] = 0;
            if( listOfImageFiles[n-1] != 0 ) numImageFiles++;
         }
      }
      // Make sure the last file is null-terminated even if imageListPath does not end with a linefeed.
      if( listOfImageFiles[filesize-1] != 0) {
         listOfImageFiles[filesize] = 0;
         numImageFiles++;
      }
      listOfImageFiles[filesize] = 0;
      if( numImageFiles == 0 ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": file \"%s\" has no non-empty lines.\n", name, imageListPath);
         exit(EXIT_FAILURE);
      }
      imageFilenameIndices = (int *) malloc(numImageFiles * sizeof(int) );
      if( imageFilenameIndices == NULL ) {
         fprintf(stderr, "RandomPatchMovie \"%s\": unable to allocate memory for filename pointers.\n", name);
         exit(EXIT_FAILURE);
      }
      int fileindex = 0;
      if( listOfImageFiles[0] != 0 ){ imageFilenameIndices[0] = 0; fileindex++; }
      for( int n=1; n<(int) filesize; n++ ) {
         if( listOfImageFiles[n] != 0 && listOfImageFiles[n-1] == 0 ) {
            assert(fileindex < numImageFiles);
            imageFilenameIndices[fileindex] = n;
            fileindex++;
         }
      }
   }
   free(filename);
   filename = strdup(getRandomFilename());

   GDALColorInterp * colorbandtypes = NULL;
   int status = getImageInfo(filename, parent->icCommunicator(), &imageLoc, &colorbandtypes);
   if(status != 0) {
      fprintf(stderr, "Movie: Unable to get image info for \"%s\"\n", filename);
      exit(EXIT_FAILURE);
   }

   // create mpi_datatypes for border transfer
   PVLayerLoc * loc = &getCLayer()->loc;
   mpi_datatypes = Communicator::newDatatypes(loc);

   imageData = NULL;
   this->displayPeriod = params->value(name,"displayPeriod", displayPeriod);
   nextDisplayTime = hc->simulationTime() + this->displayPeriod;

   unsigned int seed = parent->getObjectSeed(1);
   cl_random_init(&rng, 1UL, seed);
   retrieveRandomPatch();
   readImage(filename,getOffsetX(),getOffsetY(), colorbandtypes);

   // exchange border information
   exchange();
   return PV_SUCCESS;
}

int RandomPatchMovie::ioParamsFillGroup(enum ParamsIOFlag ioFlag) {
   int status = Image::ioParamsFillGroup(ioFlag);
   ioParam_imageListPath(ioFlag);
   ioParam_displayPeriod(ioFlag);
   return status;
}

void RandomPatchMovie::ioParam_imagePath(enum ParamsIOFlag ioFlag) {
   if (ioFlag == PARAMS_IO_READ) {
      filename = NULL;
      parent->parameters()->handleUnnecessaryStringParameter(name, "imagePath");
   }
}

void RandomPatchMovie::ioParam_imageListPath(enum ParamsIOFlag ioFlag) {
   parent->ioParamStringRequired(ioFlag, name, "imageListPath", &imageListPath);
}

void RandomPatchMovie::ioParam_displayPeriod(enum ParamsIOFlag ioFlag) {
   parent->ioParamValue(ioFlag, name, "displayPeriod", &displayPeriod, displayPeriod);
}

RandomPatchMovie::~RandomPatchMovie() {
   if( patchposfile != NULL ) {
      assert(patchposfilename != NULL);
      int status = fclose(patchposfile);
      if( status != 0 ) {
         fprintf(stderr, "Warning: closing file \"%s\" returned error %d\n", patchposfilename, errno);
      }
      patchposfilename = NULL; // Don't free; it belongs to parent->params
   }
}

int RandomPatchMovie::readOffsets() {
   // offsets are generated randomly each time an image is produced.
   offsets[0] = -1;
   offsets[1] = -1;
   return PV_SUCCESS;
}

int RandomPatchMovie::retrieveRandomPatch() {
   bool finished = false;
   GDALColorInterp * colorbandtypes = NULL;
   while(!finished) {
      free(filename);
      filename = strdup(getRandomFilename());
      getImageInfo(filename, parent->icCommunicator(), &imageLoc, &colorbandtypes);
      getRandomOffsets(&imageLoc, &offsets[0], &offsets[1]);
      readImage(filename, getOffsetX(), getOffsetY(), colorbandtypes);
      if( skipLowContrastPatchProb > 0 ) {
         for( int k=0; k<getNumNeurons(); k++ ) {
            const PVLayerLoc * loc = getLayerLoc();
            int kex = kIndexExtended(k, loc->nx, loc->ny, loc->nf, loc->nb);
            if( fabsf(data[kex])>lowContrastThreshold ) {
    	       finished = true;
    	       break;
            }
         }
         if( !finished ) {
            rng = cl_random_get(rng);
            double p = rng.s0/cl_random_max();
            if( p > skipLowContrastPatchProb ) {
               finished = true;
            }
         }
      }
      else {
    	  finished = true;
      }
   }

   if( patchposfile != NULL) {
      fprintf(patchposfile, "time = %f, offsetX = %d, offsetY = %d, filename = \"%s\"\n", parent->simulationTime(), getOffsetX(), getOffsetY(), filename);
   }

   return PV_SUCCESS;
}

int RandomPatchMovie::outputState(double timed, bool last)
{
   if (writeImages) {
      char basicFilename[PV_PATH_MAX + 1];
      snprintf(basicFilename, PV_PATH_MAX, "%s/Movie_%f.%s", parent->getOutputPath(), timed, writeImagesExtension);
      write(basicFilename);
   }

   for (int i = 0; i < numProbes; i++) {
      probes[i]->outputState(timed);
   }

   return HyPerLayer::outputState(timed, last);
}

int RandomPatchMovie::updateState(double timed, double dt)
{
   updateImage(timed, dt);
   return 0;
}

bool RandomPatchMovie::updateImage(double timed, double dt) {
   bool needNewImage = timed >= nextDisplayTime;
   if( needNewImage ) {
      nextDisplayTime += displayPeriod;
      lastUpdateTime = timed;
      retrieveRandomPatch();
   }
   exchange();

   return true;
}

#define RANDOMPATCHMOVIE_ROOTPROC 0

char * RandomPatchMovie::getRandomFilename() {
   char * randfname;
#ifdef PV_USE_MPI
   InterColComm * icComm = parent->icCommunicator();
   int rank = icComm->commRank();
   const MPI_Comm mpi_comm = icComm->communicator();

#else // PV_USE_MPI
   int rank = 0;
#endif // PV_USE_MPI
   int rootproc = RANDOMPATCHMOVIE_ROOTPROC;
   int namelength;
   if( rank == rootproc ) {
      fileIndex = getRandomFileIndex();
      assert(fileIndex >= 0 && fileIndex < numImageFiles );
      randfname = listOfImageFiles + imageFilenameIndices[fileIndex];
   }
   namelength = (int) strlen(randfname);
#ifdef PV_USE_MPI
   MPI_Bcast(&namelength, 1, MPI_INT, rootproc, mpi_comm);
   MPI_Bcast(randfname, namelength+1, MPI_CHAR, rootproc, mpi_comm);
#endif // PV_USE_MPI
   assert( randfname != NULL );
   return randfname;
}

int RandomPatchMovie::getRandomOffsets(const PVLayerLoc * imgloc, int * offsetXptr, int * offsetYptr) {
   int xdimension = imgloc->nxGlobal - getLayerLoc()->nxGlobal;
   int ydimension = imgloc->nyGlobal - getLayerLoc()->nyGlobal;
   rng = cl_random_get(rng);
   double x = ((double)rng.s0 * xdimension)/((double) cl_random_max());
   *offsetXptr = (int) x;
   double y = ((double)rng.s0 * ydimension)/((double) cl_random_max());
   *offsetYptr = (int) y;
   return PV_SUCCESS;
}

int RandomPatchMovie::getRandomFileIndex() {
#ifdef PV_USE_MPI
   assert(parent->icCommunicator()->commRank()==RANDOMPATCHMOVIE_ROOTPROC);
#endif // PV_USE_MPI
   double x = ((double)rng.s0 * numImageFiles)/((double) cl_random_max());
   int idx = (int) x;
   return idx;
}

}  // end namespace PV
