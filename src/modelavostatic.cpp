#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include "src/definitions.h"
#include "src/modelgeneral.h"
#include "src/modelavostatic.h"
#include "src/xmlmodelfile.h"
#include "src/modelsettings.h"
#include "src/wavelet1D.h"
#include "src/wavelet3D.h"
#include "src/corr.h"
#include "src/analyzelog.h"
#include "src/vario.h"
#include "src/simbox.h"
#include "src/background.h"
#include "src/welldata.h"
#include "src/blockedlogs.h"
#include "src/fftgrid.h"
#include "src/fftfilegrid.h"
#include "src/gridmapping.h"
#include "src/inputfiles.h"
#include "src/timings.h"
#include "src/io.h"
#include "src/waveletfilter.h"
#include "src/tasklist.h"

#include "lib/utils.h"
#include "lib/random.h"
#include "lib/timekit.hpp"
#include "lib/lib_matr.h"
#include "nrlib/iotools/fileio.hpp"
#include "nrlib/iotools/stringtools.hpp"
#include "nrlib/segy/segy.hpp"
#include "nrlib/surface/surfaceio.hpp"
#include "nrlib/surface/surface.hpp"
#include "nrlib/surface/regularsurface.hpp"
#include "nrlib/iotools/logkit.hpp"
#include "nrlib/stormgrid/stormcontgrid.hpp"


ModelAVOStatic::ModelAVOStatic(ModelSettings      *& modelSettings,
                               const InputFiles    * inputFiles,
                               std::vector<bool>     failedGeneralDetails,
                               Simbox              * timeSimbox,
                               Simbox             *& timeBGSimbox,
                               Simbox              * timeSimboxConstThick)
{
  forwardModeling_        = modelSettings->getForwardModeling();
  numberOfWells_          = modelSettings->getNumberOfWells();

  priorFacies_            = NULL;
  priorFaciesProbCubes_   = NULL;
  wells_                  = NULL;

  bool failedSimbox       = failedGeneralDetails[0];

  failed_                 = false;
  bool failedWells        = false;
  bool failedExtraSurf    = false;
  bool failedPriorFacies  = false;
  bool failedRockPhysics  = false;

  bool failedLoadingModel = false;

  std::string errText("");

  if(!failedSimbox)
  {
    if (modelSettings->getForwardModeling() == false)
    {
      //
      // INVERSION/ESTIMATION
      //
      loadExtraSurfaces(waveletEstimInterval_, faciesEstimInterval_, wellMoveInterval_,
                        timeSimbox, inputFiles, errText, failedExtraSurf);

      processWells(wells_, timeSimbox, timeBGSimbox, timeSimboxConstThick,
                   modelSettings, inputFiles, errText, failedWells);

      checkAvailableMemory(timeSimbox, modelSettings, inputFiles);
      bool estimationMode = modelSettings->getEstimationMode();
      if (estimationMode == false && !failedWells && !failedExtraSurf)
      {
        processPriorFaciesProb(faciesEstimInterval_,
                               priorFacies_,
                               wells_,
                               timeSimbox->getnz(),
                               static_cast<float> (timeSimbox->getdz()),
                               timeSimbox,
                               modelSettings,
                               failedPriorFacies,
                               errText,
                               inputFiles);
      }
    }
  }
  failedLoadingModel = failedWells || failedExtraSurf || failedPriorFacies || failedRockPhysics;

  if (failedLoadingModel) {
    LogKit::WriteHeader("Error(s) while loading data");
    LogKit::LogFormatted(LogKit::Error,"\n"+errText);
    LogKit::LogFormatted(LogKit::Error,"\nAborting\n");
  }

  failed_ = failedLoadingModel;
  failed_details_.push_back(failedWells);
  failed_details_.push_back(failedExtraSurf);
  failed_details_.push_back(failedPriorFacies);
}


ModelAVOStatic::~ModelAVOStatic(void)
{
  if(!forwardModeling_)
  {
    for(int i=0 ; i<numberOfWells_ ; i++)
      if(wells_[i] != NULL)
        delete wells_[i];
    delete [] wells_;
  }

  if(waveletEstimInterval_.size() == 2) {
    if (waveletEstimInterval_[0] != NULL)
      delete waveletEstimInterval_[0];
    if (waveletEstimInterval_[1] != NULL)
      delete waveletEstimInterval_[1];
  }

  if(faciesEstimInterval_.size() == 2) {
    if (faciesEstimInterval_[0] != NULL)
      delete faciesEstimInterval_[0];
    if (faciesEstimInterval_[1] != NULL)
      delete faciesEstimInterval_[1];
  }

  if (priorFacies_ != NULL)
    delete [] priorFacies_;

  if(wellMoveInterval_.size() == 2) {
    if (wellMoveInterval_[0] != NULL)
      delete wellMoveInterval_[0];
    if (wellMoveInterval_[1] != NULL)
      delete wellMoveInterval_[1];
  }
}


void
ModelAVOStatic::processWells(WellData          **& wells,
                             Simbox              * timeSimbox,
                             Simbox              * timeBGSimbox,
                             Simbox              * timeSimboxConstThick,
                             ModelSettings      *& modelSettings,
                             const InputFiles    * inputFiles,
                             std::string         & errText,
                             bool                & failed)
{
  int     nWells         = modelSettings->getNumberOfWells();

  if(nWells > 0) {

    double wall=0.0, cpu=0.0;
    TimeKit::getTime(wall,cpu);

    LogKit::WriteHeader("Reading and processing wells");

    bool    faciesLogGiven = modelSettings->getFaciesLogGiven();
    int     nFacies        = 0;

    int error = 0;

    std::string tmpErrText("");
    wells = new WellData *[nWells];
    for(int i=0 ; i<nWells ; i++) {
      wells[i] = new WellData(inputFiles->getWellFile(i),
        modelSettings->getLogNames(),
        modelSettings->getInverseVelocity(),
        modelSettings,
        modelSettings->getIndicatorFacies(i),
        modelSettings->getIndicatorWavelet(i),
        modelSettings->getIndicatorBGTrend(i),
        modelSettings->getIndicatorRealVs(i),
        faciesLogGiven);
      if(wells[i]->checkError(tmpErrText) != 0) {
        errText += tmpErrText;
        error = 1;
      }
    }

    if (error == 0) {
      if(modelSettings->getFaciesLogGiven()) {
        checkFaciesNames(wells, modelSettings, inputFiles, tmpErrText, error);
        nFacies = modelSettings->getNumberOfFacies(); // nFacies is set in checkFaciesNames()
      }
      if (error>0)
        errText += "Prior facies probabilities failed.\n"+tmpErrText;

      int   * validWells    = new int[nWells];
      bool  * validIndex    = new bool[nWells];
      int   * nMerges       = new int[nWells];
      int   * nInvalidAlpha = new int[nWells];
      int   * nInvalidBeta  = new int[nWells];
      int   * nInvalidRho   = new int[nWells];
      float * rankCorr      = new float[nWells];
      float * devAngle      = new float[nWells];
      int  ** faciesCount   = NULL;

      if(nFacies > 0) {
        faciesCount = new int * [nWells];
        for (int i = 0 ; i < nWells ; i++)
          faciesCount[i] = new int[nFacies];
      }

      int count = 0;
      int nohit=0;
      int empty=0;
      int facieslognotok = 0;
      int upwards=0;
      LogKit::LogFormatted(LogKit::Low,"\n");
      for (int i=0 ; i<nWells ; i++)
      {
        bool skip = false;
        LogKit::LogFormatted(LogKit::Low,wells[i]->getWellname()+" : \n");
        if(wells[i]!=NULL) {
          if(wells[i]->checkSimbox(timeSimbox) == 1) {
            skip = true;
            nohit++;
            TaskList::addTask("Consider increasing the inversion volume such that well "+wells[i]->getWellname()+ " can be included");
          }
          if(wells[i]->getNd() == 0) {
            LogKit::LogFormatted(LogKit::Low,"  IGNORED (no log entries found)\n");
            skip = true;
            empty++;
            TaskList::addTask("Check the log entries in well "+wells[i]->getWellname()+".");
          }
          if(wells[i]->isFaciesOk()==0) {
            LogKit::LogFormatted(LogKit::Low,"   IGNORED (facies log has wrong entries)\n");
            skip = true;
            facieslognotok++;
            TaskList::addTask("Check the facies logs in well "+wells[i]->getWellname()+".\n       The facies logs in this well are wrong and the well is ignored");
          }
          if(wells[i]->removeDuplicateLogEntries(timeSimbox, nMerges[i]) == false) {
            LogKit::LogFormatted(LogKit::Low,"   IGNORED (well is too far from monotonous in time)\n");
            skip = true;
            upwards++;
            TaskList::addTask("Check the TWT log in well "+wells[i]->getWellname()+".\n       The well is moving too much upwards, and the well is ignored");
          }
          if(skip)
            validIndex[i] = false;
          else {
            validIndex[i] = true;
            wells[i]->setWrongLogEntriesUndefined(nInvalidAlpha[i], nInvalidBeta[i], nInvalidRho[i]);
            wells[i]->filterLogs();
            wells[i]->findMeanVsVp(waveletEstimInterval_);
            wells[i]->lookForSyntheticVsLog(rankCorr[i]);
            wells[i]->calculateDeviation(devAngle[i], timeSimbox);
            wells[i]->setBlockedLogsOrigThick( new BlockedLogs(wells[i], timeSimbox, modelSettings->getRunFromPanel()) );
            wells[i]->setBlockedLogsConstThick( new BlockedLogs(wells[i], timeSimboxConstThick) );
            if (timeBGSimbox==NULL)
              wells[i]->setBlockedLogsExtendedBG( new BlockedLogs(wells[i], timeSimbox) ); // Need a copy constructor?
            else
              wells[i]->setBlockedLogsExtendedBG( new BlockedLogs(wells[i], timeBGSimbox) );
            if (nFacies > 0)
              wells[i]->countFacies(timeSimbox,faciesCount[i]);
            validWells[count] = i;
            count++;
          }
        }
      }
      //
      // Write summary.
      //
      LogKit::LogFormatted(LogKit::Low,"\n");
      LogKit::LogFormatted(LogKit::Low,"                                      Invalid                                    \n");
      LogKit::LogFormatted(LogKit::Low,"Well                    Merges      Vp   Vs  Rho  synthVs/Corr    Deviated/Angle \n");
      LogKit::LogFormatted(LogKit::Low,"---------------------------------------------------------------------------------\n");
      for(int i=0 ; i<nWells ; i++) {
        if (validIndex[i])
          LogKit::LogFormatted(LogKit::Low,"%-23s %6d    %4d %4d %4d     %3s / %5.3f      %3s / %4.1f\n",
          wells[i]->getWellname().c_str(),
          nMerges[i],
          nInvalidAlpha[i],
          nInvalidBeta[i],
          nInvalidRho[i],
          (wells[i]->hasSyntheticVsLog() ? "yes" : " no"),
          rankCorr[i],
          (devAngle[i] > modelSettings->getMaxDevAngle() ? "yes" : " no"),
          devAngle[i]);
        else
          LogKit::LogFormatted(LogKit::Low,"%-23s      -       -    -    -       - /     -       -  /    -\n",
          wells[i]->getWellname().c_str());
      }

      //
      // Print facies count for each well
      //
      if(nFacies > 0) {
        //
        // Probabilities
        //
        LogKit::LogFormatted(LogKit::Low,"\nFacies distributions for each well: \n");
        LogKit::LogFormatted(LogKit::Low,"\nWell                    ");
        for (int i = 0 ; i < nFacies ; i++)
          LogKit::LogFormatted(LogKit::Low,"%12s ",modelSettings->getFaciesName(i).c_str());
        LogKit::LogFormatted(LogKit::Low,"\n");
        for (int i = 0 ; i < 24+13*nFacies ; i++)
          LogKit::LogFormatted(LogKit::Low,"-");
        LogKit::LogFormatted(LogKit::Low,"\n");
        for (int i = 0 ; i < nWells ; i++) {
          if (validIndex[i]) {
            float tot = 0.0;
            for (int f = 0 ; f < nFacies ; f++)
              tot += static_cast<float>(faciesCount[i][f]);
            LogKit::LogFormatted(LogKit::Low,"%-23s ",wells[i]->getWellname().c_str());
            for (int f = 0 ; f < nFacies ; f++) {
              if (tot > 0) {
                float faciesProb = static_cast<float>(faciesCount[i][f])/tot;
                LogKit::LogFormatted(LogKit::Low,"%12.4f ",faciesProb);
              }
              else
                LogKit::LogFormatted(LogKit::Low,"         -   ");
            }
            LogKit::LogFormatted(LogKit::Low,"\n");
          }
          else {
            LogKit::LogFormatted(LogKit::Low,"%-23s ",wells[i]->getWellname().c_str());
            for (int f = 0 ; f < nFacies ; f++)
              LogKit::LogFormatted(LogKit::Low,"         -   ");
            LogKit::LogFormatted(LogKit::Low,"\n");

          }
        }
        LogKit::LogFormatted(LogKit::Low,"\n");
        //
        // Counts
        //
        LogKit::LogFormatted(LogKit::Medium,"\nFacies counts for each well: \n");
        LogKit::LogFormatted(LogKit::Medium,"\nWell                    ");
        for (int i = 0 ; i < nFacies ; i++)
          LogKit::LogFormatted(LogKit::Medium,"%12s ",modelSettings->getFaciesName(i).c_str());
        LogKit::LogFormatted(LogKit::Medium,"\n");
        for (int i = 0 ; i < 24+13*nFacies ; i++)
          LogKit::LogFormatted(LogKit::Medium,"-");
        LogKit::LogFormatted(LogKit::Medium,"\n");
        for (int i = 0 ; i < nWells ; i++) {
          if (validIndex[i]) {
            float tot = 0.0;
            for (int f = 0 ; f < nFacies ; f++)
              tot += static_cast<float>(faciesCount[i][f]);
            LogKit::LogFormatted(LogKit::Medium,"%-23s ",wells[i]->getWellname().c_str());
            for (int f = 0 ; f < nFacies ; f++) {
              LogKit::LogFormatted(LogKit::Medium,"%12d ",faciesCount[i][f]);
            }
            LogKit::LogFormatted(LogKit::Medium,"\n");
          }
          else {
            LogKit::LogFormatted(LogKit::Medium,"%-23s ",wells[i]->getWellname().c_str());
            for (int f = 0 ; f < nFacies ; f++)
              LogKit::LogFormatted(LogKit::Medium,"         -   ");
            LogKit::LogFormatted(LogKit::Medium,"\n");

          }
        }
        LogKit::LogFormatted(LogKit::Medium,"\n");
      }

      //
      // Remove invalid wells
      //
      for(int i=0 ; i<nWells ; i++)
        if (!validIndex[i])
          delete wells[i];
      for(int i=0 ; i<count ; i++)
        wells[i] = wells[validWells[i]];
      for(int i=count ; i<nWells ; i++)
        wells[i] = NULL;
      nWells = count;
      modelSettings->setNumberOfWells(nWells);

      delete [] validWells;
      delete [] validIndex;
      delete [] nMerges;
      delete [] nInvalidAlpha;
      delete [] nInvalidBeta;
      delete [] nInvalidRho;
      delete [] rankCorr;
      delete [] devAngle;

      if (nohit>0)
        LogKit::LogFormatted(LogKit::Low,"\nWARNING: %d well(s) do not hit the inversion volume and will be ignored.\n",nohit);
      if (empty>0)
        LogKit::LogFormatted(LogKit::Low,"\nWARNING: %d well(s) contain no log entries and will be ignored.\n",empty);
      if(facieslognotok>0)
        LogKit::LogFormatted(LogKit::Low,"\nWARNING: %d well(s) have wrong facies logs and will be ignored.\n",facieslognotok);
      if(upwards>0)
        LogKit::LogFormatted(LogKit::Low,"\nWARNING: %d well(s) are moving upwards in TWT and will be ignored.\n",upwards);
      if (nWells==0 && modelSettings->getNoWellNedded()==false) {
        LogKit::LogFormatted(LogKit::Low,"\nERROR: There are no wells left for data analysis. Please check that the inversion area given");
        LogKit::LogFormatted(LogKit::Low,"\n       below is correct. If it is not, you probably have problems with coordinate scaling.");
        LogKit::LogFormatted(LogKit::Low,"\n                                   X0          Y0        DeltaX      DeltaY      Angle");
        LogKit::LogFormatted(LogKit::Low,"\n       -------------------------------------------------------------------------------");
        LogKit::LogFormatted(LogKit::Low,"\n       Inversion area:    %11.2f %11.2f   %11.2f %11.2f   %8.3f\n",
          timeSimbox->getx0(), timeSimbox->gety0(),
          timeSimbox->getlx(), timeSimbox->getly(),
          (timeSimbox->getAngle()*180)/M_PI);
        errText += "No wells available for estimation.";
        error = 1;
      }

      if(nFacies > 0) {
        int fc;
        for(int i = 0; i < nFacies; i++){
          fc = 0;
          for(int j = 0; j < nWells; j++){
            fc+=faciesCount[j][i];
          }
          if(fc == 0){
            LogKit::LogFormatted(LogKit::Low,"\nWARNING: Facies %s is not observed in any of the wells, and posterior facies probability can not be estimated for this facies.\n",modelSettings->getFaciesName(i).c_str() );
            TaskList::addTask("In order to estimate prior facies probability for facies "+ modelSettings->getFaciesName(i) + " add wells which contain observations of this facies.\n");
          }
        }
        for (int i = 0 ; i<nWells ; i++)
          delete [] faciesCount[i];
        delete [] faciesCount;
      }

    }
    failed = error > 0;
    Timings::setTimeWells(wall,cpu);
  }
}

void ModelAVOStatic::checkFaciesNames(WellData ** wells,
                             ModelSettings     *& modelSettings,
                             const InputFiles   * inputFiles,
                             std::string        & tmpErrText,
                             int                & error)
{
  //Compare facies names from wells with facies names given in model file or read from proability cubes
  int min,max;
  int globalmin = 0;
  int globalmax = 0;
  bool first = true;
  for (int w = 0; w < modelSettings->getNumberOfWells(); w++) {
    if(wells[w]->isFaciesLogDefined())
    {
      wells[w]->getMinMaxFnr(min,max);
      if(first==true)
      {
        globalmin = min;
        globalmax = max;
        first = false;
      }
      else
      {
        if(min<globalmin)
          globalmin = min;
        if(max>globalmax)
          globalmax = max;
      }
    }
  }

  int nnames = globalmax - globalmin + 1;
  std::vector<std::string> names(nnames);

  for(int w=0 ; w<modelSettings->getNumberOfWells() ; w++)
  {
    if(wells[w]->isFaciesLogDefined())
    {
      for(int i=0 ; i < wells[w]->getNFacies() ; i++)
      {
        std::string name = wells[w]->getFaciesName(i);
        int         fnr  = wells[w]->getFaciesNr(i) - globalmin;

        if(names[fnr] == "") {
          names[fnr] = name;
        }
        else if(names[fnr] != name)
        {
          tmpErrText += "Problem with facies logs. Facies names and numbers are not uniquely defined.\n";
          error++;
        }
      }
    }
  }

  LogKit::LogFormatted(LogKit::Low,"\nFaciesLabel      FaciesName           ");
  LogKit::LogFormatted(LogKit::Low,"\n--------------------------------------\n");
  for(int i=0 ; i<nnames ; i++)
    if(names[i] != "")
      LogKit::LogFormatted(LogKit::Low,"    %2d           %-20s\n",i+globalmin,names[i].c_str());

  int nFacies = 0;
  for(int i=0 ; i<nnames ; i++)
    if(names[i] != "")
      nFacies++;

  for(int i=0 ; i<nnames ; i++)
  {
    if(names[i] != "")
    {
      modelSettings->addFaciesName(names[i]);
      modelSettings->addFaciesLabel(globalmin + i);
    }
  }

  // Compare names in wells with names given in .xml-file
  if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_MODEL_FILE)
  {
    typedef std::map<std::string,float> mapType;
    mapType myMap = modelSettings->getPriorFaciesProb();

    for(int i=0;i<nFacies;i++)
    {
      mapType::iterator iter = myMap.find(modelSettings->getFaciesName(i));
      if (iter==myMap.end())
      {
        tmpErrText += "Problem with facies logs. Facies "+modelSettings->getFaciesName(i)+" is not one of the facies given in the xml-file.\n";
        error++;
      }
    }
  }
  // Compare names in wells with names given as input in proability cubes
  else if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_CUBES)
  {
    typedef std::map<std::string,std::string> mapType;
    mapType myMap = inputFiles->getPriorFaciesProbFile();

    for(int i=0;i<nFacies;i++)
    {
      mapType::iterator iter = myMap.find(modelSettings->getFaciesName(i));
      if (iter==myMap.end())
      {
        tmpErrText += "Problem with facies logs. Facies "+modelSettings->getFaciesName(i)+" is not one of the facies given in the xml-file.\n";
        error++;
      }
    }
  }
}

void
ModelAVOStatic::checkAvailableMemory(Simbox        * timeSimbox,
                                   ModelSettings * modelSettings,
                                   const InputFiles    * inputFiles)
{
  LogKit::WriteHeader("Estimating amount of memory needed");
  //
  // Find the size of first seismic volume
  //
  float memOneSeis = 0.0f;
  if (inputFiles->getNumberOfSeismicFiles(0) > 0 && inputFiles->getSeismicFile(0,0) != "") {
    memOneSeis = static_cast<float> (NRLib::FindFileSize(inputFiles->getSeismicFile(0,0)));
  }

  //
  // Find the size of one grid
  //
  FFTGrid * dummyGrid = new FFTGrid(timeSimbox->getnx(),
                                    timeSimbox->getny(),
                                    timeSimbox->getnz(),
                                    modelSettings->getNXpad(),
                                    modelSettings->getNYpad(),
                                    modelSettings->getNZpad());
  long long int gridSizePad = static_cast<long long int>(4)*dummyGrid->getrsize();

  delete dummyGrid;
  dummyGrid = new FFTGrid(timeSimbox->getnx(),
                          timeSimbox->getny(),
                          timeSimbox->getnz(),
                          timeSimbox->getnx(),
                          timeSimbox->getny(),
                          timeSimbox->getnz());
  long long int gridSizeBase = 4*dummyGrid->getrsize();
  delete dummyGrid;
  int nGridParameters  = 3;                                      // Vp + Vs + Rho, padded
  int nGridBackground  = 3;                                      // Vp + Vs + Rho, padded
  int nGridCovariances = 6;                                      // Covariances, padded
  int nGridSeismicData = modelSettings->getNumberOfAngles(0);     // One for each angle stack, padded

  int nGridFacies       = modelSettings->getNumberOfFacies()+1;   // One for each facies, one for undef, unpadded.
  int nGridHistograms   = modelSettings->getNumberOfFacies();     // One for each facies, 2MB.
  int nGridKriging      = 1;                                      // One grid for kriging, unpadded.
  int nGridCompute      = 1;                                      // Computation grid, padded (for convenience)
  int nGridFileMode     = 1;                                      // One grid for intermediate file storage

  int nGrids;
  long long int gridMem;
  if(modelSettings->getForwardModeling() == true) {
    if (modelSettings->getFileGrid())  // Use disk buffering
      nGrids = nGridFileMode;
    else
      nGrids = nGridParameters + nGridSeismicData;

    gridMem = nGrids*gridSizePad;
  }
  else {
    if (modelSettings->getFileGrid()) { // Use disk buffering
      nGrids = nGridFileMode;
      if(modelSettings->getKrigingParameter() > 0) {
        nGrids += nGridKriging;
      }
      if(modelSettings->getNumberOfSimulations() > 0)
        nGrids = nGridParameters;
      if(modelSettings->getUseLocalNoise(0)) {
        nGrids = 2*nGridParameters;
      }

      gridMem = nGrids*gridSizePad;
    }
    else {
      //baseP and baseU are the padded and unpadde grids allocated at each peak.
      int baseP = nGridParameters + nGridCovariances;
      if(modelSettings->getUseLocalNoise(0) == true || (modelSettings->getEstimateFaciesProb() && modelSettings->getFaciesProbRelative()))
        baseP += nGridBackground;
      int baseU = 0;
      if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_CUBES)
        baseU += modelSettings->getNumberOfFacies();

      //First peak: At inversion
      int peak1P = baseP + nGridSeismicData; //Need seismic data as well here.
      int peak1U = baseU;

      long long int peakGridMem = peak1P*gridSizePad + peak1U*gridSizeBase; //First peak must be currently largest.
      int peakNGrid   = peak1P;                                             //Also in number of padded grids

      if(modelSettings->getNumberOfSimulations() > 0) { //Second possible peak when simulating.
        int peak2P = baseP + 3; //Three extra parameter grids for simulated parameters.
        if(modelSettings->getUseLocalNoise(0) == true &&
           (modelSettings->getEstimateFaciesProb() == false || modelSettings->getFaciesProbRelative() == false))
          peak2P -= nGridBackground; //Background grids are released before simulation in this case.
        int peak2U = baseU;     //Base level is the same, but may increase.
        bool computeGridUsed = ((modelSettings->getOutputGridsElastic() & (IO::AI + IO::LAMBDARHO + IO::LAMELAMBDA + IO::LAMEMU + IO::MURHO + IO::POISSONRATIO + IO::SI + IO::VPVSRATIO)) > 0);
        if(computeGridUsed == true)
          peak2P += nGridCompute;
        else if(modelSettings->getKrigingParameter() > 0) //Note the else, since this grid will use same memory as computation grid if both are active.
          peak2U += nGridKriging;

        if(peak2P > peakNGrid)
          peakNGrid = peak2P;

        long long int peak2Mem = peak2P*gridSizePad + peak2U*gridSizeBase;
        if(peak2Mem > peakGridMem)
          peakGridMem = peak2Mem;
      }

      if(modelSettings->getEstimateFaciesProb() == true) {//Third possible peak when computing facies prob.
        int peak3P = baseP;                //No extra padded grids, so this one can not peak here.
        int peak3U = baseU + nGridFacies;  //But this one will, and may trigger new memory max.
        if((modelSettings->getOtherOutputFlag() & IO::FACIES_LIKELIHOOD) > 0)
          peak3U += 1; //Also needs to store seismic likelihood.

        long long int peak3Mem = peak3P*gridSizePad + peak3U*gridSizeBase + 2000000*nGridHistograms; //These are 2MB when Vs is used.
        if(peak3Mem > peakGridMem)
          peakGridMem = peak3Mem;
      }
      nGrids  = peakNGrid;
      gridMem = peakGridMem;
    }
  }
  FFTGrid::setMaxAllowedGrids(nGrids);
  if(modelSettings->getDebugFlag()>0)
    FFTGrid::setTerminateOnMaxGrid(true);

  int   workSize    = 2500 + static_cast<int>( 0.65*gridSizePad); //Size of memory used beyond grids.

  float mem0        = 4.0f * workSize;
  float mem1        = static_cast<float>(gridMem);
  float mem2        = static_cast<float>(modelSettings->getNumberOfAngles(0))*gridSizePad + memOneSeis; //Peak memory when reading seismic, overestimated.

  float neededMem   = mem0 + std::max(mem1, mem2);

  float megaBytes   = neededMem/(1024.f*1024.f);
  float gigaBytes   = megaBytes/1024.f;

  LogKit::LogFormatted(LogKit::High,"\nMemory needed for reading seismic data       : %10.2f MB\n",mem2/(1024.f*1024.f));
  LogKit::LogFormatted(LogKit::High,  "Memory needed for holding internal grids (%2d): %10.2f MB\n",nGrids, mem1/(1024.f*1024.f));
  LogKit::LogFormatted(LogKit::High,  "Memory needed for holding other entities     : %10.2f MB\n",mem0/(1024.f*1024.f));

  if (gigaBytes < 1.0f)
    LogKit::LogFormatted(LogKit::Low,"\nMemory needed by CRAVA:  %.2f megaBytes\n",megaBytes);
  else
    LogKit::LogFormatted(LogKit::Low,"\nMemory needed by CRAVA:  %.2f gigaBytes\n",gigaBytes);

  if(mem2>mem1)
    LogKit::LogFormatted(LogKit::Low,"\n This estimate is too high because seismic data are cut to fit the internal grid\n");
  if (!modelSettings->getFileGrid()) {
    //
    // Check if we can hold everything in memory.
    //
    modelSettings->setFileGrid(false);
    char ** memchunk  = new char*[nGrids];

    int i = 0;
    try {
      for(i = 0 ; i < nGrids ; i++)
        memchunk[i] = new char[static_cast<size_t>(gridSizePad)];
    }
    catch (std::bad_alloc& ) //Could not allocate memory
    {
      modelSettings->setFileGrid(true);
      LogKit::LogFormatted(LogKit::Low,"Not enough memory to hold all grids. Using file storage.\n");
    }

    for(int j=0 ; j<i ; j++)
      delete [] memchunk[j];
    delete [] memchunk;
  }
}

void ModelAVOStatic::processPriorFaciesProb(const std::vector<Surface *> & faciesEstimInterval,
                                            float                       *& priorFacies,
                                            WellData                    ** wells,
                                            int                            nz,
                                            float                          dz,
                                            Simbox                       * timeSimbox,
                                            ModelSettings                * modelSettings,
                                            bool                         & failed,
                                            std::string                  & errTxt,
                                            const InputFiles                   * inputFiles)
{
  if (modelSettings->getEstimateFaciesProb())
  {
    LogKit::WriteHeader("Prior Facies Probabilities");
    int nFacies = modelSettings->getNumberOfFacies();

    if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_WELLS)
    {
      if (nFacies > 0)
      {
        int nWells  = modelSettings->getNumberOfWells();
        int nFacies = modelSettings->getNumberOfFacies();
        int ndata   = nWells*nz;

        int ** faciesCount = new int * [nWells];
        for (int w = 0 ; w < nWells ; w++)
          faciesCount[w] = new int[nFacies];

        for (int w = 0 ; w < nWells ; w++)
          for (int i = 0 ; i < nFacies ; i++)
            faciesCount[w][i] = 0;

        int * faciesLog = new int[ndata];   // NB! *internal* log numbering (0, 1, 2, ...)
        for (int i = 0 ; i < ndata ; i++)
          faciesLog[i] = IMISSING;

        float * vtAlpha   = new float[nz];  // vt = vertical trend
        float * vtBeta    = new float[nz];
        float * vtRho     = new float[nz];
        int   * vtFacies  = new int[nz];

        int nUsedWells = 0;

        for (int w = 0 ; w < nWells ; w++)
        {
          if(wells[w]->getNFacies() > 0) // Well has facies log
          {
            //
            // Note that we use timeSimbox to calculate prior facies probabilities
            // instead of the simbox with parallel top and base surfaces. This
            // will make the prior probabilities slightly different, but that
            // should not be a problem.
            //
            BlockedLogs * bl = wells[w]->getBlockedLogsOrigThick();
            int nBlocks = bl->getNumberOfBlocks();
            //
            // Set facies data outside facies estimation interval IMISSING
            //
            int * blFaciesLog = new int[nBlocks];
            Utils::copyVector(bl->getFacies(), blFaciesLog, nBlocks);

            if (faciesEstimInterval.size() > 0) {
              const double * xPos  = bl->getXpos();
              const double * yPos  = bl->getYpos();
              const double * zPos  = bl->getZpos();
              for (int i = 0 ; i < nBlocks ; i++) {
                const double zTop  = faciesEstimInterval[0]->GetZ(xPos[i],yPos[i]);
                const double zBase = faciesEstimInterval[1]->GetZ(xPos[i],yPos[i]);
                if ( (zPos[i] - 0.5*dz) < zTop || (zPos[i] + 0.5*dz) > zBase)
                  blFaciesLog[i] = IMISSING;
              }
            }

            bl->getVerticalTrend(bl->getAlpha(),vtAlpha);
            bl->getVerticalTrend(bl->getBeta(),vtBeta);
            bl->getVerticalTrend(bl->getRho(),vtRho);
            bl->getVerticalTrend(blFaciesLog,vtFacies);
            delete [] blFaciesLog;

            for(int i=0 ; i<nz ; i++)
            {
              int facies;
              if(vtAlpha[i] != RMISSING && vtBeta[i] != RMISSING && vtRho[i] != RMISSING)
                facies = vtFacies[i];
              else
                facies = IMISSING;

              faciesLog[w*nz + i] = facies;
              if(facies != IMISSING)
                faciesCount[w][facies]++;
            }
            nUsedWells++;
          }
        }
        delete [] vtAlpha;
        delete [] vtBeta;
        delete [] vtRho;
        delete [] vtFacies;

        if (nUsedWells > 0) {
          //
          // Probabilities
          //
          LogKit::LogFormatted(LogKit::Low,"\nFacies distributions for each blocked well: \n");
          LogKit::LogFormatted(LogKit::Low,"\nBlockedWell              ");
          for (int i = 0 ; i < nFacies ; i++)
            LogKit::LogFormatted(LogKit::Low,"%12s ",modelSettings->getFaciesName(i).c_str());
          LogKit::LogFormatted(LogKit::Low,"\n");
          for (int i = 0 ; i < 24+13*nFacies ; i++)
            LogKit::LogFormatted(LogKit::Low,"-");
          LogKit::LogFormatted(LogKit::Low,"\n");
          for (int w = 0 ; w < nWells ; w++)
          {
            if(wells[w]->getNFacies() > 0) // Well has facies log
            {
              float tot = 0.0;
              for (int i = 0 ; i < nFacies ; i++) {
                tot += static_cast<float>(faciesCount[w][i]);
              }

              LogKit::LogFormatted(LogKit::Low,"%-23s ",wells[w]->getWellname().c_str());
              for (int i = 0 ; i < nFacies ; i++) {
                float faciesProb = static_cast<float>(faciesCount[w][i])/tot;
                LogKit::LogFormatted(LogKit::Low," %12.4f",faciesProb);
              }
              LogKit::LogFormatted(LogKit::Low,"\n");
            }
          }
          LogKit::LogFormatted(LogKit::Low,"\n");
          //
          // Counts
          //
          LogKit::LogFormatted(LogKit::Medium,"\nFacies counts for each blocked well: \n");

          LogKit::LogFormatted(LogKit::Medium,"\nBlockedWell              ");
          for (int i = 0 ; i < nFacies ; i++)
            LogKit::LogFormatted(LogKit::Medium,"%12s ",modelSettings->getFaciesName(i).c_str());
          LogKit::LogFormatted(LogKit::Medium,"\n");
          for (int i = 0 ; i < 24+13*nFacies ; i++)
            LogKit::LogFormatted(LogKit::Medium,"-");
          LogKit::LogFormatted(LogKit::Medium,"\n");
          for (int w = 0 ; w < nWells ; w++)
          {
            if(wells[w]->getNFacies() > 0)
            {
              float tot = 0.0;
              for (int i = 0 ; i < nFacies ; i++)
                tot += static_cast<float>(faciesCount[w][i]);
              LogKit::LogFormatted(LogKit::Medium,"%-23s ",wells[w]->getWellname().c_str());
              for (int i = 0 ; i < nFacies ; i++) {
                LogKit::LogFormatted(LogKit::Medium," %12d",faciesCount[w][i]);
              }
              LogKit::LogFormatted(LogKit::Medium,"\n");
            }
          }
          LogKit::LogFormatted(LogKit::Medium,"\n");

          for (int w = 0 ; w < nWells ; w++)
            delete [] faciesCount[w];
          delete [] faciesCount;

          //
          // Make prior facies probabilities
          //
          float sum = 0.0f;
          int * nData = new int[nFacies];
          for(int i=0 ; i<nFacies ; i++)
            nData[i] = 0;

          for(int i=0 ; i<ndata ; i++) {
            if(faciesLog[i] != IMISSING) {
              nData[faciesLog[i]]++;
            }
          }
          delete [] faciesLog;

          for(int i=0 ; i<nFacies ; i++)
            sum += nData[i];

          if (sum > 0) {
            LogKit::LogFormatted(LogKit::Low,"Facies probabilities based on all blocked wells:\n\n");
            LogKit::LogFormatted(LogKit::Low,"Facies         Probability\n");
            LogKit::LogFormatted(LogKit::Low,"--------------------------\n");
            priorFacies = new float[nFacies];
            for(int i=0 ; i<nFacies ; i++) {
              priorFacies[i] = float(nData[i])/sum;
              LogKit::LogFormatted(LogKit::Low,"%-15s %10.4f\n",modelSettings->getFaciesName(i).c_str(),priorFacies[i]);
            }
          }
          else {
            LogKit::LogFormatted(LogKit::Warning,"\nWARNING: No valid facies log entries have been found\n");
            modelSettings->setEstimateFaciesProb(false);
            TaskList::addTask("Consider using a well containing facies log entries to be able to estimate facies probabilities.");

          }
          delete [] nData;
        }
        else
        {
          LogKit::LogFormatted(LogKit::Warning,"\nWARNING: Estimation of facies probabilites have been requested, but there");
          LogKit::LogFormatted(LogKit::Warning,"\n         are no wells with facies available and CRAVA will therefore not");
          LogKit::LogFormatted(LogKit::Warning,"\n         be able to estimate these probabilities...\n");
          modelSettings->setEstimateFaciesProb(false);

          TaskList::addTask("Consider using a well containing facies log entries to be able to estimate facies probabilities.");
        }
      }
      else
      {
        LogKit::LogFormatted(LogKit::Warning,"\nWARNING: Estimation of facies probabilites have been requested, but no facies");
        LogKit::LogFormatted(LogKit::Warning,"\n         have been found and CRAVA will therefore not be able to estimate");
        LogKit::LogFormatted(LogKit::Warning,"\n         these probabilities...\n");
        modelSettings->setEstimateFaciesProb(false);
        TaskList::addTask("Consider using a well containing facies log entries to be able to estimate facies probabilities.");
      }
    }
    else if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_MODEL_FILE)
    {
      priorFacies = new float[nFacies];
      typedef std::map<std::string,float> mapType;
      mapType myMap = modelSettings->getPriorFaciesProb();

      for(int i=0;i<nFacies;i++)
      {
        mapType::iterator iter = myMap.find(modelSettings->getFaciesName(i));
        if(iter!=myMap.end())
          priorFacies[i] = iter->second;
        else
        {
          LogKit::LogFormatted(LogKit::Warning,"\nWARNING: No prior facies probability found for facies %12s\n",modelSettings->getFaciesName(i).c_str());
          modelSettings->setEstimateFaciesProb(false);
          TaskList::addTask("Check that facies " +NRLib::ToString(modelSettings->getFaciesName(i).c_str())+" is given a prior probability in the xml-file");
        }
      }
      LogKit::LogFormatted(LogKit::Low,"Facies         Probability\n");
      LogKit::LogFormatted(LogKit::Low,"--------------------------\n");
      for(int i=0 ; i<nFacies ; i++) {
        LogKit::LogFormatted(LogKit::Low,"%-15s %10.4f\n",modelSettings->getFaciesName(i).c_str(),priorFacies[i]);
      }

    }
    else if(modelSettings->getIsPriorFaciesProbGiven()==ModelSettings::FACIES_FROM_CUBES)
    {
      readPriorFaciesProbCubes(inputFiles,
                                modelSettings,
                                priorFaciesProbCubes_,
                                timeSimbox,
                                errTxt,
                                failed);

       typedef std::map<std::string,std::string> mapType;
       mapType myMap = inputFiles->getPriorFaciesProbFile();

       LogKit::LogFormatted(LogKit::Low,"Facies         Probability in file\n");
       LogKit::LogFormatted(LogKit::Low,"----------------------------------\n");
       for(mapType::iterator it=myMap.begin();it!=myMap.end();it++)
         LogKit::LogFormatted(LogKit::Low,"%-15s %10s\n",(it->first).c_str(),(it->second).c_str());

    }
  }
}
void ModelAVOStatic::readPriorFaciesProbCubes(const InputFiles      * inputFiles,
                                              ModelSettings   * modelSettings,
                                              FFTGrid       **& priorFaciesProbCubes,
                                              Simbox          * timeSimbox,
                                              std::string     & errTxt,
                                              bool            & failed)
{
  int nFacies = modelSettings->getNumberOfFacies();
  priorFaciesProbCubes = new FFTGrid*[nFacies];

  typedef std::map<std::string,std::string> mapType;
  mapType myMap = inputFiles->getPriorFaciesProbFile();
  for(int i=0;i<nFacies;i++)
  {
    mapType::iterator iter = myMap.find(modelSettings->getFaciesName(i));

    if(iter!=myMap.end())
    {
      const std::string & faciesProbFile = iter->second;
      const SegyGeometry      * dummy1 = NULL;
      const TraceHeaderFormat * dummy2 = NULL;
      const float               offset = modelSettings->getSegyOffset(0); //Facies estimation only allowed for one time lapse
      std::string errorText("");
      int outsideTraces = 0;
      ModelGeneral::readGridFromFile(faciesProbFile,
                       "priorfaciesprob",
                       offset,
                       priorFaciesProbCubes[i],
                       dummy1,
                       dummy2,
                       FFTGrid::PARAMETER,
                       timeSimbox,
                       modelSettings,
                       outsideTraces,
                       errorText,
                       true);
      if(errorText != "")
      {
        errorText += "Reading of file \'"+faciesProbFile+"\' for prior facies probability for facies \'"
                     +modelSettings->getFaciesName(i)+"\' failed\n";
        errTxt += errorText;
        failed = true;
      }
    }
    else
    {
      LogKit::LogFormatted(LogKit::Warning,"\nWARNING: No prior facies probability found for facies %12s\n",
                           modelSettings->getFaciesName(i).c_str());
      TaskList::addTask("Check that facies "+NRLib::ToString(modelSettings->getFaciesName(i).c_str())+" is given prior probability in the xml-file");
      modelSettings->setEstimateFaciesProb(false);
      break;
    }
  }
}

void
ModelAVOStatic::loadExtraSurfaces(std::vector<Surface *> & waveletEstimInterval,
                                  std::vector<Surface *> & faciesEstimInterval,
                                  std::vector<Surface *> & wellMoveInterval,
                                  Simbox                 * timeSimbox,
                                  const InputFiles             * inputFiles,
                                  std::string            & errText,
                                  bool                   & failed)
{
  const double x0 = timeSimbox->getx0();
  const double y0 = timeSimbox->gety0();
  const double lx = timeSimbox->getlx();
  const double ly = timeSimbox->getly();
  const int    nx = timeSimbox->getnx();
  const int    ny = timeSimbox->getny();
  //
  // Get wavelet estimation interval
  //
  const std::string & topWEI  = inputFiles->getWaveletEstIntFileTop(0); //Same for all time lapses
  const std::string & baseWEI = inputFiles->getWaveletEstIntFileBase(0);//Same for all time lapses

  if (topWEI != "" && baseWEI != "") {
    waveletEstimInterval.resize(2);
    try {
      if (NRLib::IsNumber(topWEI))
        waveletEstimInterval[0] = new Surface(x0,y0,lx,ly,nx,ny,atof(topWEI.c_str()));
      else {
        Surface tmpSurf(topWEI);
        waveletEstimInterval[0] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }

    try {
      if (NRLib::IsNumber(baseWEI))
        waveletEstimInterval[1] = new Surface(x0,y0,lx,ly,nx,ny,atof(baseWEI.c_str()));
      else {
        Surface tmpSurf(baseWEI);
        waveletEstimInterval[1] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }
  }
  //
  // Get facies estimation interval
  //
  const std::string & topFEI  = inputFiles->getFaciesEstIntFile(0);
  const std::string & baseFEI = inputFiles->getFaciesEstIntFile(1);

  if (topFEI != "" && baseFEI != "") {
    faciesEstimInterval.resize(2);
    try {
      if (NRLib::IsNumber(topFEI))
        faciesEstimInterval[0] = new Surface(x0,y0,lx,ly,nx,ny,atof(topFEI.c_str()));
      else {
        Surface tmpSurf(topFEI);
        faciesEstimInterval[0] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }

    try {
      if (NRLib::IsNumber(baseFEI))
        faciesEstimInterval[1] = new Surface(x0,y0,lx,ly,nx,ny,atof(baseFEI.c_str()));
      else {
        Surface tmpSurf(baseFEI);
        faciesEstimInterval[1] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }
  }
  //
  // Get well move interval
  //
  const std::string & topWMI  = inputFiles->getWellMoveIntFile(0);
  const std::string & baseWMI = inputFiles->getWellMoveIntFile(1);

  if (topWMI != "" && baseWMI != "") {
    wellMoveInterval.resize(2);
    try {
      if (NRLib::IsNumber(topWMI))
        wellMoveInterval[0] = new Surface(x0,y0,lx,ly,nx,ny,atof(topWMI.c_str()));
      else {
        Surface tmpSurf(topWMI);
        wellMoveInterval[0] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }

    try {
      if (NRLib::IsNumber(baseWMI))
        wellMoveInterval[1] = new Surface(x0,y0,lx,ly,nx,ny,atof(baseWMI.c_str()));
      else {
        Surface tmpSurf(baseWMI);
        wellMoveInterval[1] = new Surface(tmpSurf);
      }
    }
    catch (NRLib::Exception & e) {
      errText += e.what();
      failed = true;
    }
  }
}

void ModelAVOStatic::writeWells(WellData ** wells, ModelSettings * modelSettings) const
{
  int nWells  = modelSettings->getNumberOfWells();
  for(int i=0;i<nWells;i++)
    wells[i]->writeWell(modelSettings->getWellFormatFlag());
}

void ModelAVOStatic::writeBlockedWells(WellData ** wells, ModelSettings * modelSettings) const
{
  int nWells  = modelSettings->getNumberOfWells();
  for(int i=0;i<nWells;i++)
    wells[i]->getBlockedLogsOrigThick()->writeWell(modelSettings);
}

void ModelAVOStatic::addSeismicLogs(WellData     ** wells,
                                    FFTGrid      ** seisCube,
                                    ModelSettings * modelSettings,
                                    int             nAngles)
{
  int nWells  = modelSettings->getNumberOfWells();

    for (int iAngle = 0 ; iAngle < nAngles ; iAngle++)
    {
      seisCube[iAngle]->setAccessMode(FFTGrid::RANDOMACCESS);
      for(int i=0;i<nWells;i++)
        wells[i]->getBlockedLogsOrigThick()->setLogFromGrid(seisCube[iAngle],iAngle,nAngles,"SEISMIC_DATA");
      seisCube[iAngle]->endAccess();
  }
}

void ModelAVOStatic::generateSyntheticSeismic(Wavelet      ** wavelet,
                                              WellData     ** wells,
                                              float        ** reflectionMatrix,
                                              Simbox        * timeSimbox,
                                              ModelSettings * modelSettings,
                                              int             nAngles)
{
  int nWells  = modelSettings->getNumberOfWells();
  int nzp     = modelSettings->getNZpad();
  int nz      = timeSimbox->getnz();

  int i;

  for( i=0; i<nWells; i++ )
  {
    if( wells[i]->isDeviated() == false )
      wells[i]->getBlockedLogsOrigThick()->generateSyntheticSeismic(reflectionMatrix,nAngles,wavelet,nz,nzp,timeSimbox);
  }
}

void
ModelAVOStatic::processWellLocation(FFTGrid                     ** seisCube,
                                    WellData                    ** wells,
                                    float                       ** reflectionMatrix,
                                    Simbox                       * timeSimbox,
                                    ModelSettings                * modelSettings,
                                    const std::vector<Surface *> & interval)
{
  LogKit::WriteHeader("Estimating optimized well location");

  double  deltaX, deltaY;
  float   sum;
  float   kMove;
  float   moveAngle;
  int     iMove;
  int     jMove;
  int     i,j,w;
  int     iMaxOffset;
  int     jMaxOffset;
  int     nMoveAngles = 0;
  int     nWells      = modelSettings->getNumberOfWells();
  int     nAngles     = modelSettings->getNumberOfAngles(0);//Well location is not estimated when using time lapse data
  float   maxShift    = modelSettings->getMaxWellShift();
  float   maxOffset   = modelSettings->getMaxWellOffset();
  double  angle       = timeSimbox->getAngle();
  double  dx          = timeSimbox->getdx();
  double  dy          = timeSimbox->getdx();
  std::vector<float> seismicAngle = modelSettings->getAngle(0); //Use first time lapse as this not is allowed in 4D

  std::vector<float> angleWeight(nAngles);
  LogKit::LogFormatted(LogKit::Low,"\n");
  LogKit::LogFormatted(LogKit::Low,"  Well             Shift[ms]       DeltaI   DeltaX[m]   DeltaJ   DeltaY[m] \n");
  LogKit::LogFormatted(LogKit::Low,"  ----------------------------------------------------------------------------------\n");

  for (w = 0 ; w < nWells ; w++) {
    if( wells[w]->isDeviated()==true )
      continue;

    BlockedLogs * bl = wells[w]->getBlockedLogsOrigThick();
    nMoveAngles = modelSettings->getNumberOfWellAngles(w);

    if( nMoveAngles==0 )
      continue;

    for( i=0; i<nAngles; i++ )
      angleWeight[i] = 0;

    for( i=0; i<nMoveAngles; i++ ){
      moveAngle   = modelSettings->getWellMoveAngle(w,i);

      for( j=0; j<nAngles; j++ ){
        if( moveAngle == seismicAngle[j]){
          angleWeight[j] = modelSettings->getWellMoveWeight(w,i);
          break;
        }
      }
    }

    sum = 0;
    for( i=0; i<nAngles; i++ )
      sum += angleWeight[i];
    if( sum == 0 )
      continue;

    iMaxOffset = static_cast<int>(std::ceil(maxOffset/dx));
    jMaxOffset = static_cast<int>(std::ceil(maxOffset/dy));

    bl->findOptimalWellLocation(seisCube,timeSimbox,reflectionMatrix,nAngles,angleWeight,maxShift,iMaxOffset,jMaxOffset,interval,iMove,jMove,kMove);

    deltaX = iMove*dx*cos(angle) - jMove*dy*sin(angle);
    deltaY = iMove*dx*sin(angle) + jMove*dy*cos(angle);
    wells[w]->moveWell(timeSimbox,deltaX,deltaY,kMove);
    wells[w]->deleteBlockedLogsOrigThick();
    wells[w]->setBlockedLogsOrigThick( new BlockedLogs(wells[w], timeSimbox, modelSettings->getRunFromPanel()) );
    LogKit::LogFormatted(LogKit::Low,"  %-13s %11.2f %12d %11.2f %8d %11.2f \n",
    wells[w]->getWellname().c_str(), kMove, iMove, deltaX, jMove, deltaY);
  }

   for (w = 0 ; w < nWells ; w++){
     nMoveAngles = modelSettings->getNumberOfWellAngles(w);

    if( wells[w]->isDeviated()==true && nMoveAngles > 0 )
    {
      LogKit::LogFormatted(LogKit::Warning,"\nWARNING: Well %7s is treated as deviated and can not be moved.\n",
          wells[w]->getWellname().c_str());
      TaskList::addTask("Well "+NRLib::ToString(wells[w]->getWellname())+" can not be moved. Remove <optimize-location-to> for this well");
    }
   }
}

void ModelAVOStatic::deleteDynamicWells(WellData ** wells,
                                        int         nWells)
{
  for(int i=0; i<nWells; i++){
    wells[i]->getBlockedLogsConstThick()->deleteDynamicBlockedLogs();
    wells[i]->getBlockedLogsExtendedBG()->deleteDynamicBlockedLogs();
    wells[i]->getBlockedLogsOrigThick()->deleteDynamicBlockedLogs();
  }
}
