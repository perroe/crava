/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/


#ifndef COMMONDATA_H
#define COMMONDATA_H

#include "src/simbox.h"
#include "src/welldata.h"
#include "nrlib/well/well.hpp"
#include "nrlib/segy/segy.hpp"
#include "lib/utils.h"
#include "src/blockedlogscommon.h"
#include "src/tasklist.h"
#include "src/seismicstorage.h"
#include "src/multiintervalgrid.h"

class CravaTrend;
class InputFiles;
class ModelSettings;
class ModelGeneral;


class CommonData{
public:
  CommonData(ModelSettings    * model_settings,
             InputFiles       * input_files);

  ~ CommonData();

  //GET FUNCTIONS

  const Simbox                    & GetEstimationSimbox()     const { return estimation_simbox_     ;}
  const NRLib::Volume             & GetFullInversionVolume()  const { return full_inversion_volume_ ;}
  const std::vector<NRLib::Well>  & GetWells()                const { return wells_                 ;}


private:

  void LoadWellMoveInterval(const InputFiles             * input_files,
                            const Simbox                 * estimation_simbox,
                            std::vector<Surface *>       & well_move_interval,
                            std::string                  & err_text,
                            bool                         & failed);

  void  OptimizeWellLocations(ModelSettings                                 * model_settings,
                              InputFiles                                    * input_files,
                              const Simbox                                  * estimation_simbox,
                              //const NRLib::Volume                           & volume,
                              std::vector<NRLib::Well>                      & wells,
                              std::map<std::string, BlockedLogsCommon *>    & mapped_blocked_logs,
                              std::map<int, std::vector<SeismicStorage> >   & seismic_data,
                              std::map<int, float **>                       & reflection_matrix,
                              std::string                                   & err_text,
                              bool                                          & failed);

  void MoveWell(const NRLib::Well & well,
                const Simbox      * simbox, 
                double              delta_X, 
                double              delta_Y, 
                double              k_move);

  void  CalculateDeviation(NRLib::Well            & new_well,
                           const ModelSettings    * const model_settings,
                           float                  & dev_angle,
                           Simbox                 * simbox,
                           int                      use_for_wavelet_estimation);

  void GetGeometryFromGridOnFile(const std::string         grid_file,
                                 const TraceHeaderFormat * thf,
                                 SegyGeometry           *& geometry,
                                 std::string             & err_text);

  SegyGeometry * GetGeometryFromCravaFile(const std::string & file_name);

  SegyGeometry * GetGeometryFromStormFile(const std::string    & file_name,
                                       std::string          & err_text,
                                       bool                   scale = false);

  bool CreateOuterTemporarySimbox(ModelSettings           * model_settings,
                                  InputFiles              * input_files,
                                  Simbox                  & estimation_simbox,
                                  NRLib::Volume           & full_inversion_interval,
                                  std::string             & err_text);

  void WriteAreas(const SegyGeometry  * area_params,
                  Simbox              * time_simbox,
                  std::string         & err_text);

  void FindSmallestSurfaceGeometry(const double   x0,
                                   const double   y0,
                                   const double   lx,
                                   const double   ly,
                                   const double   rot,
                                   double       & x_min,
                                   double       & y_min,
                                   double       & x_max,
                                   double       & y_max);

  void SetSurfacesMultipleIntervals(const ModelSettings            * const model_settings,
                                    NRLib::Volume                  & full_inversion_volume,
                                    Simbox                         & estimation_simbox,
                                    const InputFiles               * input_files,
                                    std::string                    & err_text,
                                    bool                           & failed);

  void SetSurfacesSingleInterval(const ModelSettings              * const model_settings,
                                 NRLib::Volume                    & full_inversion_volume,
                                 Simbox                           & estimation_simbox,
                                 const std::vector<std::string>   & surf_file,
                                 std::string                      & err_text,
                                 bool                             & failed);

  bool ReadSeismicData(ModelSettings  * modelSettings,
                       InputFiles     * inputFiles);

  bool ReadWellData(ModelSettings                   * model_settings,
                    Simbox                          * estimation_simbox,
                    InputFiles                      * input_files,
                    std::vector<std::string>          & log_names,
                    const std::vector<std::string>  & log_names_from_user,
                    const std::vector<bool>         & inverse_velocity,
                    bool                              facies_log_given,
                    std::string                     & err_text);

  bool BlockWellsForEstimation(const ModelSettings                            * const model_settings,
                               const Simbox                                   & estimation_simbox,
                               std::vector<NRLib::Well>                       & wells,
                               std::map<std::string, BlockedLogsCommon *>     & mapped_blocked_logs_common,
                               std::string                                    & err_text);

  bool       CheckThatDataCoverGrid(const SegY            * segy,
                                    float                   offset,
                                    const Simbox          * timeCutSimbox,
                                    float                   guard_zone);

  void ProcessLogsNorsarWell(NRLib::Well                      & new_well,
                             std::vector<std::string>         & log_names_from_user,
                             const std::vector<bool>          & inverse_velocity,
                             bool                               facies_log_given,
                             std::string                      & error_text,
                             bool                             & failed);

  void ProcessLogsRMSWell(NRLib::Well                     & new_well,
                          std::vector<std::string>  & log_names_from_user,
                          const std::vector<bool>         & inverse_velocity,
                          bool                              facies_log_given,
                          std::string                     & error_text,
                          bool                            & failed);


  bool  SetupReflectionMatrixAndTempWavelet(ModelSettings  * model_settings,
                                            InputFiles     * input_files);

  float  ** ReadMatrix(const std::string                  & fileName,
                       int                                  n1,
                       int                                  n2,
                       const std::string                  & readReason,
                       std::string                        & errText);

  void  SetupDefaultReflectionMatrix(float              **& reflectionMatrix,
                                     double                 vsvp,
                                     const ModelSettings  * modelSettings,
                                     int                    numberOfAngles,
                                     int                    thisTimeLapse);

  bool  WaveletHandling(ModelSettings                     * model_settings,
                        InputFiles                        * input_files);

  void  SetupTrendCubes(ModelSettings                     * model_settings, 
                        InputFiles                        * input_files, 
                        MultiIntervalGrid                 * multiple_interval_grid,
                        std::string                       & error_text,
                        bool                              & failed);

  void SetupRockPhysics(const ModelSettings                                 * model_settings,
                        const InputFiles                                    * input_files,
                        const MultiIntervalGrid                             * multiple_interval_grid,
                        const std::vector<CravaTrend>                       & trend_cubes,
                        const std::map<std::string, BlockedLogsCommon *>    & mapped_blocked_logs,
                        int                                                   n_trend_cubes,
                        std::string                                         & error_text,
                        bool                                                & failed);

  void PrintExpectationAndCovariance(const std::vector<double>   & expectation,
                                     const NRLib::Grid2D<double> & covariance,
                                     const bool                  & has_trend) const;

  bool EstimateWaveletShape();
  bool EstimatePriorCorrelation();

  int ComputeTime(int year, int month, int day) const;

  // CLASS VARIABLES ---------------------------------------------------

  // Bool variables indicating whether the corresponding data processing
  // succeeded
  bool outer_temp_simbox_;
  bool read_seismic_;
  bool read_wells_;
  bool block_wells_;
  bool setup_reflection_matrix_;
  bool temporary_wavelet_;
  bool optimize_well_location_;
  bool wavelet_estimation_shape_;
  bool prior_corr_estimation_;
  bool setup_estimation_rock_physics_;
  bool setup_multigrid_;
  bool setup_trend_cubes_;

  MultiIntervalGrid       * multiple_interval_grid_;
  Simbox                    estimation_simbox_;
  NRLib::Volume             full_inversion_volume_;

  std::map<int, std::vector<SeismicStorage> >   seismic_data_;

  // Well logs
  std::vector<std::string>                      log_names_;
  std::vector<NRLib::Well>                      wells_;

  // Blocked well logs
  std::map<std::string, BlockedLogsCommon *>    mapped_blocked_logs_; //
  std::vector<std::string>                      continuous_logs_to_be_blocked_; // Continuous logs that should be blocked
  std::vector<std::string>                      discrete_logs_to_be_blocked_;   // Discrete logs that should be blocked

  // trend cubes
  int                                                           n_trend_cubes_;
  std::vector<CravaTrend>                                       trend_cubes_;
  std::vector<std::map<std::string, std::vector<DistributionsRock *> > >    rock_distributions_;     ///< Rocks used in rock physics model, one map for each interval
  std::vector<std::map<std::string, std::vector<DistributionWithTrend *> > >  reservoir_variables_;    ///< Reservoir variables used in the rock physics model; one map for each interval

  //Well variables not contained in NRlib::Well
  //std::map<std::string, int>                        timemissing_;
  //std::map<std::string, double>                     xpos0_;
  //std::map<std::string, double>                     ypos0_;
  //std::map<std::string, std::vector<std::string> >  faciesnames_;
  //std::map<std::string, std::vector<int> >          faciesNr_;
  //std::map<std::string, int>                        faciesok_; //Bool?
  //std::map<std::string, int>                        nFacies_;


  std::map<int, float **>   reflection_matrix_;
  bool                      reflection_matrix_from_file_; //False: created from global vp/vs

  std::vector<Wavelet*>     temporary_wavelets_; //One wavelet per angle

  // facies
  std::vector<std::string> facies_names_;

};
#endif
