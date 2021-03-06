/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/


#ifndef COMMONDATA_H
#define COMMONDATA_H

#include "src/simbox.h"
#include "nrlib/well/well.hpp"
#include "nrlib/segy/segy.hpp"
#include "lib/utils.h"
#include "src/blockedlogscommon.h"
#include "src/tasklist.h"
#include "src/seismicstorage.h"
#include "src/background.h"
#include "src/gridmapping.h"
#include "src/analyzelog.h"
#include "src/inputfiles.h"
#include "src/modelsettings.h"
#include "src/timeline.h"
#include "src/cravatrend.h"
#include "src/multiintervalgrid.h"

class MultiIntervalGrid;
class CravaTrend;
class BlockedLogsCommon;
class Wavelet1D;

class CommonData{
public:
  CommonData(ModelSettings * model_settings,
             InputFiles    * input_files);

  ~ CommonData();

  enum                 gridTypes{CTMISSING, DATA, PARAMETER};

  //GET FUNCTIONS

  const Simbox                                                       & GetEstimationSimbox()                            const { return estimation_simbox_                             ;}
  const Simbox                                                       & GetFullInversionSimbox()                         const { return full_inversion_simbox_                         ;}
  const Simbox                                                       & GetOutputSimbox()                                const { return output_simbox_                                 ;}
        Simbox                                                       & GetOutputSimbox()                                      { return output_simbox_                                 ;}
  const Simbox                                                       * GetBgSimbox(int i)                               const { return bg_simboxes_[i]                                ;}
  const std::vector<NRLib::Well *>                                   & GetWells()                                       const { return wells_                                         ;}
        std::vector<NRLib::Well *>                                   & GetWells()                                             { return wells_                                         ;}
  const MultiIntervalGrid                                            * GetMultipleIntervalGrid()                        const { return multiple_interval_grid_                        ;}
  MultiIntervalGrid                                                  * GetMultipleIntervalGrid(void)                          { return multiple_interval_grid_                        ;}

  const std::map<std::string, std::vector<DistributionsRock *> >     & GetDistributionsRock()                           const { return rock_distributions_                            ;}
  const std::map<std::string, std::vector<DistributionWithTrend *> > & GetReservoirVariables()                          const { return reservoir_variables_                           ;}

  TimeLine                                                           * GetTimeLine()                                          { return time_line_                                     ;}
  const std::vector<std::string>                                     & GetFaciesNames()                                 const { return facies_names_                                  ;}
  const std::vector<int>                                             & GetFaciesNr()                                    const { return facies_nr_                                     ;}

  const std::vector<std::vector<float> >                             & GetPriorFacies()                                 const { return prior_facies_                                  ;}
  const std::vector<float>                                           & GetPriorFaciesInterval(int i)                    const { return prior_facies_[i]                               ;}
  const std::vector<NRLib::Grid<float> *>                            & GetPriorFaciesProbCubesInterval(int interval)    const { return prior_facies_prob_cubes_[interval]             ;}
  const NRLib::Grid<float>                                           * GetPriorFaciesProbCube(int interval, int facies) const { return prior_facies_prob_cubes_[interval][facies]     ;}

  const std::map<std::string, BlockedLogsCommon *>                   & GetBlockedLogs()                                 const { return mapped_blocked_logs_                           ;}
  const std::map<std::string, BlockedLogsCommon *>                   & GetBlockedLogsOutput()                           const { return mapped_blocked_logs_output_                    ;}
  const std::map<std::string, BlockedLogsCommon *>                   & GetBlockedLogsForCorr()                          const { return mapped_blocked_logs_for_correlation_           ;}
  const std::map<std::string, BlockedLogsCommon *>                   & GetBlockedLogsInterval(int i)                    const { return mapped_blocked_logs_intervals_.find(i)->second ;}
  const std::map<std::string, BlockedLogsCommon *>                   & GetBgBlockedLogs()                               const { return mapped_bg_blocked_logs_                        ;}

  std::vector<Surface *>                                             & GetFaciesEstimInterval()                               { return facies_estim_interval_                         ;}

  std::vector<float>                                                 & GetGravityObservationUtmxTimeLapse(int time_lapse)     { return observation_location_utmx_[time_lapse]         ;}
  std::vector<float>                                                 & GetGravityObservationUtmyTimeLapse(int time_lapse)     { return observation_location_utmy_[time_lapse]         ;}
  std::vector<float>                                                 & GetGravityObservationDepthTimeLapse(int time_lapse)    { return observation_location_depth_[time_lapse]        ;}
  std::vector<float>                                                 & GetGravityResponseTimeLapse(int time_lapse)            { return gravity_response_[time_lapse]                  ;}
  std::vector<float>                                                 & GetGravityStdDevTimeLapse(int time_lapse)              { return gravity_std_dev_[time_lapse]                   ;}

  GridMapping                                                        * GetTimeDepthMapping()                                  { return time_depth_mapping_                            ;}
  bool                                                                 GetVelocityFromInversion()                             { return velocity_from_inversion_                       ;}

  std::vector<Grid2D *>                                              & GetLocalNoiseScaleTimeLapse(int time_lapse)            { return local_noise_scales_.find(time_lapse)->second   ;}
  std::vector<SeismicStorage *>                                      & GetSeismicDataTimeLapse(int time_lapse)                { return seismic_data_[time_lapse]                      ;}
  std::map<int, std::vector<float> >                                 & GetSNRatio()                                           { return sn_ratios_                                     ;}
  std::vector<float>                                                 & GetSNRatioTimeLapse(int time_lapse)                    { return sn_ratios_.find(time_lapse)->second            ;}

  bool                                                                 HasSeismicData()                                       { return seismic_data_.size() > 0                       ;}

  bool                                                                 GetRefMatFromFileGlobalVpVs()                          { return refmat_from_file_global_vpvs_                  ;}
  NRLib::Matrix                                                      & GetReflectionMatrixTimeLapse(int time_lapse)           { return reflection_matrix_[time_lapse]                 ;}

  std::vector<Wavelet *>                                             & GetWavelet(int time_lapse)                             { return wavelets_.find(time_lapse)->second             ;}
  std::vector<Wavelet1D *>                                           & GetWellWavelets(int angle)                             { return well_wavelets_[angle]                          ;}
  std::vector<std::vector<float> >                                   & GetAngularCorrelation(int time_lapse)                  { return angular_correlations_[time_lapse]              ;}

  bool                                                                 GetPriorCovEst()                                 const { return prior_cov_estimated_or_file_                   ;}
  const std::vector<std::vector<double> >                            & GetTGradX()                                      const { return t_grad_x_                                      ;}
  const std::vector<std::vector<double> >                            & GetTGradY()                                      const { return t_grad_y_                                      ;}
  const NRLib::Grid2D<float>                                         & GetRefTimeGradX()                                const { return ref_time_grad_x_                               ;}
  const NRLib::Grid2D<float>                                         & GetRefTimeGradY()                                const { return ref_time_grad_y_                               ;}

  const std::vector<NRLib::Matrix>                                   & GetPriorAutoCov(int i_interval)                  const { return prior_auto_cov_[i_interval]                    ;}
  const Surface                                                      * GetPriorCorrXY(int i_interval)                   const { return prior_corr_XY_[i_interval]                     ;}
  const NRLib::Matrix                                                & GetPriorParamCov(int i_interval)                 const { return prior_param_cov_[i_interval]                   ;}
  const std::vector<double>                                          & GetPriorCorrT(int i_interval)                    const { return prior_corr_T_[i_interval]                      ;}

  const std::vector<NRLib::Grid<float> *>                            & GetBackgroundParametersInterval(int i)           const { return background_parameters_[i]                      ;}
  double                                                               GetBackgroundVsVpRatioInterval(int i)            const { return background_vs_vp_ratios_[i]                    ;}

  NRLib::Grid<float>                                                 * GetBackgroundVpInterval(int i)                   const { return background_parameters_[i][0]                   ;}
  NRLib::Grid<float>                                                 * GetBackgroundVsInterval(int i)                   const { return background_parameters_[i][1]                   ;}
  NRLib::Grid<float>                                                 * GetBackgroundRhoInterval(int i)                  const { return background_parameters_[i][2]                   ;}
  const NRLib::Grid2D<std::vector<double> >                          & GetBackgroundVerticalTrends()                    const { return background_vertical_trends_                    ;}

  const std::vector<CravaTrend>                                      & GetTrendCubes()                                  const { return trend_cubes_                                   ;}
  const CravaTrend                                                   & GetTrendCube(int i)                              const { return trend_cubes_[i]                                ;}

  const std::string                                                  & GetWaveletEstIntTop()                            const { return wavelet_est_int_top_                           ;}
  const std::string                                                  & GetWaveletEstIntBot()                            const { return wavelet_est_int_bot_                           ;}

  Grid2D                                                             * GetGainGrid(int timelapse, int angle)            const { return gain_grids_[timelapse][angle]                  ;}
  Grid2D                                                             * GetShiftGrid(int timelapse, int angle)           const { return shift_grids_[timelapse][angle]                 ;}

  template<typename T>
  static void         ResampleTrace(const std::vector<T>     & data_trace,
                                    const rfftwnd_plan       & fftplan1,
                                    const rfftwnd_plan       & fftplan2,
                                    fftw_real                * rAmpData,
                                    fftw_real                * rAmpFine,
                                    int                        nt,
                                    int                        cnt,
                                    int                        rnt,
                                    int                        cmt,
                                    int                        rmt)
{
  fftw_complex * cAmpData = reinterpret_cast<fftw_complex*>(rAmpData);
  fftw_complex * cAmpFine = reinterpret_cast<fftw_complex*>(rAmpFine);

  //
  // Fill vector to be FFT'ed
  //
  int n_data = static_cast<int>(data_trace.size());

  for (int i = 0; i < n_data; i++) {
    rAmpData[i] = static_cast<fftw_real>(data_trace[i]);
  }
  // Pad with zeros
  for (int i = n_data; i < rnt; i++) {
    rAmpData[i] = 0.0f;
  }

  //
  // Transform to Fourier domain
  //
  rfftwnd_one_real_to_complex(fftplan1, rAmpData, cAmpData);

  //
  // Fill fine-sampled grid
  //
  for (int i = 0; i < cnt; i++) {
    cAmpFine[i].re = cAmpData[i].re;
    cAmpFine[i].im = cAmpData[i].im;
  }
  // Pad with zeros (cmt is always greater than cnt)
  for (int i = cnt; i < cmt; i++) {
    cAmpFine[i].re = 0.0f;
    cAmpFine[i].im = 0.0f;
  }

  //
  // Fine-sampled grid: Fourier --> Time
  //
  rfftwnd_one_complex_to_real(fftplan2, cAmpFine, rAmpFine);

  //
  // Scale and fill grid_trace
  //
  float scale = 1/static_cast<float>(nt);
  for (int i = 0; i < rmt; i++) {
    rAmpFine[i] = scale*rAmpFine[i];
  }
}

  template<typename T>
  static void         ApplyFilter(std::vector<T> & log_filtered,
                                  std::vector<T> & log_interpolated,
                                  int                   n_time_samples,
                                  double                dt_milliseconds,
                                  float                 max_hz)
{
  //
  // Extract nonmissing part of log
  //
  int i=0;
  while (i < n_time_samples && log_interpolated[i] == RMISSING)
    i++;
  int first_nonmissing = i;
  i = n_time_samples - 1;
  while (i > 0 && log_interpolated[i] == RMISSING)
    i--;
  int last_nonmissing = i;
  int n_time_samples_defined = last_nonmissing - first_nonmissing + 1;

  for (i = 0; i < n_time_samples; i++) {            // Initialise with RMISSING
    log_filtered[i] = RMISSING;
  }

  if (n_time_samples_defined > 0) {
    //
    // Setup FFT
    //
    int   nt  = 2*n_time_samples_defined;
    int   cnt = nt/2 + 1;
    int   rnt = 2*cnt;

    fftw_real*    rAmp = static_cast<fftw_real*>(fftw_malloc(sizeof(float)*rnt));
    fftw_complex* cAmp = reinterpret_cast<fftw_complex*>(rAmp);

    for (i = 0; i < n_time_samples_defined; i++) {          // Array to filter is made symmetric
      rAmp[i]      = static_cast<fftw_real>(log_interpolated[first_nonmissing + i]);
      rAmp[nt-i-1] = rAmp[i];
    }

    //for (int i=0 ; i<nt ; i++) {
    //  printf("i=%d, log_interpolated[i]=%7.4f\n",i,rAmp[i]);
    //}

    //
    // Transform to Fourier domain
    //
    rfftwnd_plan p1 = rfftwnd_create_plan(1, &nt, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE | FFTW_IN_PLACE);
    rfftwnd_one_real_to_complex(p1, rAmp, cAmp);
    fftwnd_destroy_plan(p1);

    //for (int i=0 ; i<cnt ; i++) {
    //  printf("i=%2d, cAmp.re[i]=%11.4f  cAmp.im[i]=%11.4f\n",i,cAmp[i].re,cAmp[i].im);
    //}

    //
    // Filter using Odd's magic vector...
    //
    float dt  = static_cast<float> (dt_milliseconds/1000.0); // Sampling density in seconds
    float T   = (nt - 1)*dt;                                 // Time sample
    float w   = 1/T;                                         // Lowest frequency that can be extracted from log
    int   N   = int(max_hz/w + 0.5f);                        // Number of elements of Fourier vector to keep

    if (cnt < N+1) {
      LogKit::LogMessage(LogKit::Warning, "Warning: The vertical resolution is too low to allow filtering of well logs to %3.1f Hz.\n");
    }

    float * magic_vector = new float[cnt];
    for (i = 0; ((i < N+1) && (i < cnt)); i++) {
      magic_vector[i] = 1.0;
    }
    for (;i < cnt; i++) {
      magic_vector[i] = 0.0;
    }
    for (i = 0; i < cnt; i++) {
      cAmp[i].re *= magic_vector[i];
      cAmp[i].im *= magic_vector[i];
    }

    //for (int i=0 ; i<cnt ; i++) {
    //  printf("i=%2d, cAmp.re[i]=%11.4f  cAmp.im[i]=%11.4f\n",i,cAmp[i].re,cAmp[i].im);
    //}

    //
    // Backtransform to time domain
    //
    rfftwnd_plan p2 = rfftwnd_create_plan(1, &nt, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE | FFTW_IN_PLACE);
    rfftwnd_one_complex_to_real(p2, cAmp, rAmp);
    fftwnd_destroy_plan(p2);

    float scale= float(1.0/nt);
    for (i = 0; i < rnt; i++) {
      rAmp[i] *= scale;
    }

    //
    // Fill log_filtered[]
    //
    for (i = 0; i < n_time_samples_defined; i++) {
      log_filtered[first_nonmissing + i] = rAmp[i];      // Fill with values where defined
    }
    delete [] magic_vector;
    fftw_free(rAmp);

    //for (int i=0 ; i<n_time_samples ; i++) {
    //  printf("i log_interpolated[i] log_filtered[i]  %d  %.3f  %.3f\n",i,log_interpolated[i],log_filtered[i]);
    //}
  }
}

  static FFTGrid *   CreateFFTGrid(int nx,
                                   int ny,
                                   int nz,
                                   int nxp,
                                   int nyp,
                                   int nzp,
                                   bool file_grid);

  static std::string ConvertIntToString(int number);

  static std::string ConvertInt(int number);

  static std::string ConvertFloatToString(float number);

  static void        GenerateSyntheticSeismicLogs(std::vector<Wavelet *>                   & wavelet,
                                                  std::map<std::string, BlockedLogsCommon *> & blocked_wells,
                                                  const NRLib::Matrix                      & reflection_matrix,
                                                  const Simbox                             * time_simbox);

  NRLib::Matrix      SetupDefaultReflectionMatrix(double                 vsvp,
                                                  const ModelSettings  * model_settings,
                                                  int                    number_of_angles,
                                                  int                    this_time_lapse) const;

  void               SetupCorrectReflectionMatrix(const ModelSettings * model_settings,
                                                  std::vector<NRLib::Matrix > & reflection_matrix) const; //Only for those instances where this is possible.

  double             FindVsVpForZone(int                   i_interval,
                                     const ModelSettings * model_settings,
                                     std::string         & origin) const;

  //Resampling algorithm, from a storm_grid or a segy cube, to either a NRLib::Grid or fftgrid
  //The wanted result grid needs to be defined outside this function, and the other needs to be sent in as an empty grid
  void               FillInData(NRLib::Grid<float>  * grid,          //resample to
                                FFTGrid             * fft_grid,      //resample to
                                const Simbox        * simbox,
                                const StormContGrid * storm_grid,    //resample from
                                const SegY          * segy,          //resample from
                                float                 smooth_length,
                                int                 & missing_traces_simbox,
                                int                 & missing_traces_padding,
                                int                 & dead_traces_simbox,
                                NRLib::Grid2D<bool> * dead_traces_map,
                                int                   grid_type,
                                bool                  scale    = false,
                                bool                  is_segy  = true,
                                bool                  is_storm = false,
                                bool                  is_seismic = false) const;

  void               GetCorrGradIJ(float         & corr_grad_I,
                                   float         & corr_grad_J,
                                   const Simbox  * simbox) const;

  void               ReleaseBackgroundGrids(int i_interval, int elastic_param);

  static void        SetUndefinedCellsToGlobalAverageGrid(NRLib::Grid<float> * grid,
                                                          const float          avg);

  static   void      FindWaveletEstimationInterval(const std::string      & wavelet_est_int_top,
                                                   const std::string      & wavelet_est_int_bot,
                                                   std::vector<Surface *> & wavelet_estim_interval,
                                                   const Simbox           & estimation_simbox,
                                                   std::string            & err_text);

  static int         FindClosestFactorableNumber(int leastint);


private:

  void               LoadWellMoveInterval(const InputFiles             * input_files,
                                          const Simbox                 * estimation_simbox,
                                          std::vector<Surface *>       & well_move_interval,
                                          std::string                  & err_text) const;

  bool               OptimizeWellLocations(ModelSettings                                 * model_settings,
                                           InputFiles                                    * input_files,
                                           const Simbox                                  * estimation_simbox,
                                           const Simbox                                  & inversion_simbox,
                                           std::vector<NRLib::Well *>                    & wells,
                                           std::map<std::string, BlockedLogsCommon *>    & mapped_blocked_logs,
                                           std::vector<std::vector<SeismicStorage *> >   & seismic_data,
                                           const std::vector<NRLib::Matrix>              & reflection_matrix,
                                           std::string                                   & err_text) const;

  void               MoveWell(NRLib::Well  & well,
                              const Simbox * simbox,
                              double         delta_X,
                              double         delta_Y,
                              double         k_move) const;

  void               CalculateDeviation(NRLib::Well         & new_well,
                                        const ModelSettings * const model_settings,
                                        float               & dev_angle,
                                        Simbox              * simbox) const;

  void               CountFaciesInWell(NRLib::Well            & well,
                                       Simbox                 * simbox,
                                       int                      n_facies,
                                       const std::vector<int> & facies_nr,
                                       std::vector<int>       & facies_count) const;

  void               GetGeometryFromGridOnFile(const std::string         grid_file,
                                               const TraceHeaderFormat * thf,
                                               SegyGeometry           *& geometry,
                                               std::string             & err_text) const;

  SegyGeometry *     GetGeometryFromCravaFile(const std::string & file_name) const;

  SegyGeometry *     GetGeometryFromStormFile(const std::string    & file_name,
                                              std::string          & err_text,
                                              bool                   scale = false) const;

  bool               CreateOuterTemporarySimbox(ModelSettings           * model_settings,
                                                InputFiles              * input_files,
                                                Simbox                  & full_inversion_simbox,
                                                SegyGeometry           *& segy_geometry,
                                                std::string             & err_text) const;

  bool               SetupOutputSimbox(Simbox             & output_simbox,
                                       const Simbox       & full_inversion_simbox,
                                       ModelSettings      * model_settings,
                                       const SegyGeometry * segy_geometry,
                                       MultiIntervalGrid  * multi_interval_grid,
                                       std::string        & err_text);

  double             FindDzMin(MultiIntervalGrid * multi_interval_grid);

  void               WriteAreas(const SegyGeometry  * area_params,
                                Simbox              * time_simbox,
                                std::string         & err_text) const;

  void               FindSmallestSurfaceGeometry(const double   x0,
                                                 const double   y0,
                                                 const double   lx,
                                                 const double   ly,
                                                 const double   rot,
                                                 double       & x_min,
                                                 double       & y_min,
                                                 double       & x_max,
                                                 double       & y_max) const;

  int                GetNzFromGridOnFile(ModelSettings     * model_settings,
                                         const std::string & grid_file,
                                         std::string       & err_text) const;

  void               SetSurfaces(const ModelSettings * const model_settings,
                                 Simbox              & full_inversion_simbox,
                                 bool                  multi_surface,
                                 const InputFiles    * input_files,
                                 SegyGeometry        * segy_geometry,
                                 std::string         & err_text) const;

  bool               ReadSeismicData(ModelSettings                               * modelSettings,
                                     InputFiles                                  * inputFiles,
                                     const Simbox                                & full_inversion_simbox,
                                     Simbox                                      & estimation_simbox,
                                     std::string                                 & err_text,
                                     std::vector<std::vector<SeismicStorage *> > & seismic_data) const;

  bool               ReadWellData(ModelSettings                           * model_settings,
                                  Simbox                                  * full_inv_simbox,
                                  InputFiles                              * input_files,
                                  std::vector<NRLib::Well *>              & wells,
                                  std::vector<bool>                       & facies_log_wells,
                                  std::vector<int>                        & facies_nr,
                                  std::vector<std::string>                & facies_names,
                                  std::string                             & err_text) const;

  void               SetLogsToBeBlocked(ModelSettings                    * model_settings,
                                        const std::vector<NRLib::Well *> & wells,
                                        std::vector<std::string>         & continuous_logs_to_be_blocked,
                                        std::vector<std::string>         & discrete_logs_to_be_blocked) const;

  bool               BlockWellsForEstimation(const ModelSettings                                        * const model_settings,
                                             const Simbox                                               & estimation_simbox,
                                             std::vector<NRLib::Well *>                                 & wells,
                                             std::vector<std::string>                                   & continuous_logs_to_be_blocked,
                                             std::vector<std::string>                                   & discrete_logs_to_be_blocked,
                                             std::map<std::string, BlockedLogsCommon *>                 & mapped_blocked_logs_common,
                                             std::string                                                & err_text,
                                             bool                                                         est_simbox = true) const;

  bool               BlockLogsForCorrelation(const ModelSettings                                        * const model_settings,
                                             const MultiIntervalGrid                                    * multiple_interval_grid,
                                             std::vector<NRLib::Well *>                                 & wells,
                                             std::vector<std::string>                                   & continuous_logs_to_be_blocked,
                                             std::vector<std::string>                                   & discrete_logs_to_be_blocked,
                                             std::map<std::string, BlockedLogsCommon *>                 & mapped_blocked_logs_for_correlation,
                                             std::string                                                & err_text_common) const;

  bool                 BlockLogsForInversion(const ModelSettings                                        * const model_settings,
                                             const MultiIntervalGrid                                    * multiple_interval_grid,
                                             std::vector<NRLib::Well *>                                 & wells,
                                             std::vector<std::string>                                   & continuous_logs_to_be_blocked,
                                             std::vector<std::string>                                   & discrete_logs_to_be_blocked,
                                             std::map<int, std::map<std::string, BlockedLogsCommon *> > & mapped_blocked_logs_intervals,
                                             std::string                                                & err_text_common) const;

  bool               RemoveDuplicateLogEntriesFromWell(NRLib::Well   & well,
                                                       ModelSettings * model_settings,
                                                       const Simbox  * simbox,
                                                       int           & n_merges) const;

  void               MergeCells(const std::string         & name,
                                std::vector<double>       & pos_resampled,
                                const std::vector<double> & pos,
                                int                         ii,
                                int                         istart,
                                int                         iend,
                                bool                        print_to_screen) const;

  void               MergeCellsDiscrete(const std::string      & name,
                                        std::vector<int>       & log_resampled,
                                        const std::vector<int> & log,
                                        int                      ii,
                                        int                      istart,
                                        int                      iend,
                                        bool                     print_to_screen) const;

  void               SetWrongLogEntriesInWellUndefined(NRLib::Well          & well,
                                                       const ModelSettings  * model_settings,
                                                       int                  & count_vp,
                                                       int                  & count_vs,
                                                       int                  & count_rho) const;

  void               LookForSyntheticVsLog(NRLib::Well          & well,
                                           const ModelSettings  * model_settings,
                                           float                & rank_correlation) const;

  void               FilterLogs(NRLib::Well   & well,
                                ModelSettings * model_settings) const;

  bool               ResampleTime(std::vector<double>       & time_resampled,
                                  const std::vector<double> & z_pos,
                                  int                         nd,
                                  double                    & dt) const;

  void               ResampleLog(std::vector<double>       & log_resampled,
                                 const std::vector<double> & log,
                                 const std::vector<double> & time,
                                 const std::vector<double> & time_resampled,
                                 int                         nd,
                                 double                      dt) const;

  void               InterpolateLog(std::vector<double>       & log_interpolated,
                                    const std::vector<double> & log_resampled,
                                    int                         nd) const;

  void               CutWell(std::string           well_file_name,
                             NRLib::Well         & well,
                             const NRLib::Volume & full_inversion_volume) const;

  void               ProcessLogsGeneralWell(NRLib::Well                     & new_well,
                                            const std::vector<std::string>  & log_names,
                                            const std::vector<std::string>  & position_log_names,
                                            const std::vector<bool>         & inverse_velocity,
                                            bool                              relative_coord,
                                            bool                              facies_log_given,
                                            bool                              porosity_log_given,
                                            int                               format,
                                            std::string                     & error_text) const;
  int                CheckWellAgainstSimbox(const Simbox      * simbox,
                                            const NRLib::Well & well) const;

  bool               SetupReflectionMatrix(ModelSettings                    * model_settings,
                                           InputFiles                       * input_files,
                                           std::vector<NRLib::Matrix>       & reflection_matrix,
                                           std::vector<int>                   n_angles,
                                           bool                               refmat_from_file_global_vpvs,
                                           std::string                      & err_text) const;

  void               VsVpFromWells(int          i_interval,
                                   double     & vsvp,
                                   int        & N) const;

  void               FindMeanVsVp(const NRLib::Well            & well,
                                  const NRLib::Surface<double> & top,
                                  const NRLib::Surface<double> & bot,
                                  double                       & mean_vs_vp,
                                  int                          & n_vs_vp) const;


  bool               SetupTemporaryWavelet(ModelSettings                               * model_settings,
                                           std::vector<std::vector<SeismicStorage *> > & seismic_data,
                                           std::vector<Wavelet*>                       & temporary_wavelets,
                                           const std::vector<NRLib::Matrix>            & refl_mat,
                                           std::string                                 & err_text) const;

  bool               WaveletHandling(ModelSettings                               * model_settings,
                                     InputFiles                                  * input_files,
                                     const Simbox                                & estimation_simbox,
                                     const Simbox                                & full_inversion_simbox,
                                     std::map<std::string, BlockedLogsCommon *>  & mapped_blocked_logs,
                                     std::vector<std::vector<SeismicStorage *> > & seismic_data,
                                     std::map<int, std::vector<Wavelet *> >      & wavelets,
                                     std::vector<std::vector<Wavelet1D *> >      & well_wavelets,
                                     std::map<int, std::vector<Grid2D *> >       & local_noise_scale,
                                     std::map<int, std::vector<float> >          & global_noise_estimate,
                                     std::map<int, std::vector<float> >          & sn_ratio,
                                     std::vector<std::vector<double> >           & t_grad_x,
                                     std::vector<std::vector<double> >           & t_grad_y,
                                     NRLib::Grid2D<float>                        & ref_time_grad_x,
                                     NRLib::Grid2D<float>                        & ref_time_grad_y,
                                     std::vector<NRLib::Matrix>                  & refl_mat,
                                     std::string                                 & wavelet_est_int_top,
                                     std::string                                 & wavelet_est_int_bot,
                                     std::vector<std::vector<Grid2D *> >         & shift_grids,
                                     std::vector<std::vector<Grid2D *> >         & gain_grids,
                                     std::string                                 & err_text_common) const;

  void               CheckThatDataCoverGrid(ModelSettings                               * model_settings,
                                            std::vector<std::vector<SeismicStorage *> > & seismic_data,
                                            MultiIntervalGrid                           * multiple_interval_grid,
                                            std::string                                 & err_text) const;

  bool               CheckThatDataCoverGrid(const SegY  * segy,
                                            float         offset,
                                            double        top_grid,
                                            double        bot_grid,
                                            float         guard_zone,
                                            std::string & err_text) const;

  bool               CheckThatDataCoverGrid(StormContGrid * stormgrid,
                                            double          top_grid,
                                            double          bot_grid,
                                            float           guard_zone,
                                            std::string   & err_text,
                                            bool            scale = false) const;

  void               ProcessLogsRMSWell(NRLib::Well                     & new_well,
                                        std::string                     & error_text,
                                        bool                            & failed) const;

  void               ReadFaciesNamesFromWellLogs(NRLib::Well              & well,
                                                 std::vector<int>         & facies_nr,
                                                 std::vector<std::string> & facies_names) const;

  void               SetFaciesNamesFromWells(const ModelSettings                          * model_settings,
                                             const std::vector<std::vector<int> >         & facies_log_wells,
                                             const std::vector<std::vector<std::string> > & facies_names_wells,
                                             std::vector<int>                             & facies_nr,
                                             std::vector<std::string>                     & facies_names,
                                             std::string                                  & err_text) const;

  void               GetMinMaxFnr(int            & min,
                                  int            & max,
                                  const int        n_facies,
                                  std::vector<int> facies_nr) const;


  float **           ReadMatrix(const std::string & file_name,
                                int                 n1,
                                int                 n2,
                                const std::string & read_reason,
                                std::string       & err_text) const;

  int                Process1DWavelet(const ModelSettings                        * modelSettings,
                                      const InputFiles                           * inputFiles,
                                      SeismicStorage                             * seismic_data,
                                      std::map<std::string, BlockedLogsCommon *> & mapped_blocked_logs,
                                      const std::vector<Surface *>               & waveletEstimInterval,
                                      const Simbox                               & estimation_simbox,
                                      const Simbox                               & full_inversion_simbox,
                                      const NRLib::Matrix                        & reflection_matrix,
                                      std::string                                & err_text,
                                      Wavelet                                   *& wavelet,
                                      std::vector<Wavelet1D *>                   & well_wavelets,
                                      Grid2D                                    *& shift_grid,
                                      Grid2D                                    *& gain_grid,
                                      Grid2D                                    *& local_noise_scale,
                                      unsigned int                                 i_timelapse,
                                      unsigned int                                 j_angle,
                                      const float                                  angle,
                                      float                                      & sn_ratio,
                                      bool                                         estimate_wavlet,
                                      bool                                         use_ricker_wavelet) const;

  int                Process3DWavelet(const ModelSettings                      * model_settings,
                                      const InputFiles                         * input_files,
                                      SeismicStorage                           * seismic_data,
                                      std::map<std::string, BlockedLogsCommon *> & mapped_blocked_logs,
                                      const std::vector<Surface *>             & wavelet_estim_interval,
                                      const Simbox                             & estimation_simbox,
                                      const Simbox                             & full_inversion_simbox,
                                      const NRLib::Matrix                      & reflection_matrix,
                                      std::string                              & err_text,
                                      Wavelet                                 *& wavelet,
                                      unsigned int                               i_timelapse,
                                      unsigned int                               j_angle,
                                      float                                      angle,
                                      float                                      sn_ratio,
                                      const NRLib::Grid2D<float>               & ref_time_grad_x,
                                      const NRLib::Grid2D<float>               & ref_time_grad_y,
                                      const std::vector<std::vector<double> >  & t_grad_x,
                                      const std::vector<std::vector<double> >  & t_grad_y,
                                      bool                                       estimate_wavelet) const;

  void               ComputeStructureDepthGradient(double                 v0,
                                                   double                 radius,
                                                   const Surface        * t0_surf,
                                                   const Surface        * correlation_direction,
                                                   const Simbox         & full_inversion_simbox,
                                                   const Simbox         & estimation_simbox,
                                                   NRLib::Grid2D<float> & structure_depth_grad_x,
                                                   NRLib::Grid2D<float> & structure_depth_grad_y) const;

  void               ComputeReferenceTimeGradient(const Surface        * t0_surf,
                                                  const Simbox         & estimation_simbox,
                                                  NRLib::Grid2D<float> & ref_time_grad_x,
                                                  NRLib::Grid2D<float> & ref_time_grad_y) const;

  void               CalculateSmoothGrad(const Surface * surf, double x, double y, double radius, double ds,  double& gx, double& gy) const;


  void               ResampleSurfaceToGrid2D(const Surface * surface,
                                             Grid2D        * outgrid,
                                             const Simbox  & simbox) const;

  int                GetWaveletFileFormat(const std::string & fileName, std::string & errText) const;

  void               ReadAndWriteLocalGridsToFile(const std::string   & fileName,
                                                  const std::string   & type,
                                                  const float           scaleFactor,
                                                  const ModelSettings * modelSettings,
                                                  const Simbox        & simbox,
                                                  const Grid2D        * grid,
                                                  const float           angle) const;

  void               ResampleGrid2DToSurface(const Simbox   * simbox,
                                             const Grid2D   * grid,
                                             Surface       *& surface) const;

  bool               SetupTrendCubes(ModelSettings                     * model_settings,
                                     InputFiles                        * input_files,
                                     MultiIntervalGrid                 * multiple_interval_grid,
                                     const Simbox                      * full_inversion_simbox,
                                     std::vector<CravaTrend>           & trend_cubes,
                                     std::string                       & error_text) const;

  bool               SetupRockPhysics(const ModelSettings                                           * model_settings,
                                      const InputFiles                                              * input_files,
                                      const MultiIntervalGrid                                       * multiple_interval_grid,
                                      const Simbox                                                  & estimation_simbox,
                                      const std::vector<CravaTrend>                                 & trend_cubes,
                                      std::vector<NRLib::Well *>                                    & wells,
                                      std::map<std::string, std::vector<DistributionWithTrend *> >  & reservoir_variables,
                                      std::map<std::string, std::vector<DistributionsRock *> >      & rock_distributions,
                                      const std::vector<std::string>                                & cont_logs_to_be_blocked,
                                      const std::vector<std::string>                                & disc_logs_to_be_blocked,
                                      std::string                                                   & error_text) const;

  void               PrintExpectationAndCovariance(const std::vector<double>   & expectation,
                                                   const NRLib::Grid2D<double> & covariance,
                                                   const bool                  & has_trend) const;

  bool               EstimateWaveletShape() const;

  bool               SetupPriorFaciesProb(ModelSettings                                                    * model_settings,
                                          const InputFiles                                                 * input_files,
                                          const MultiIntervalGrid                                          * multiple_interval_grid,
                                          std::vector<std::vector<NRLib::Grid<float> *> >                  & prior_facies_prob_cubes,
                                          std::vector<std::vector<float> >                                 & prior_facies,
                                          std::vector<Surface *>                                           & facies_estim_interval,
                                          std::vector<std::string>                                         & facies_names,
                                          std::vector<int>                                                 & facies_nr,
                                          const std::map<int, std::map<std::string, BlockedLogsCommon *> > & mapped_blocked_logs_intervals,
                                          const Simbox                                                     & full_inversion_simbox,
                                          std::string                                                      & err_text_common) const;

  void               FindFaciesEstimationInterval(const InputFiles             * input_files,
                                                  std::vector<Surface *> & facies_estim_interval,
                                                  const Simbox           & estimation_simbox,
                                                  std::string            & err_text) const;

  void               CheckFaciesNamesConsistency(const ModelSettings     * model_settings,
                                                 const InputFiles        * input_files,
                                                 std::string             & tmp_err_text) const;

  void               SetFaciesNamesFromRockPhysics(std::vector<std::string>                                   & facies_names,
                                                   std::vector<int>                                           & facies_nr) const;

  void               ReadPriorFaciesProbCubes(const InputFiles                                 * input_files,
                                              ModelSettings                                    * model_settings,
                                              std::vector<std::vector<NRLib::Grid<float> *> >  & prior_facies_prob_cubes,
                                              const std::vector<Simbox *>                      & interval_simboxes,
                                              const Simbox                                     & full_inverion_simbox,
                                              std::string                                      & err_text) const;

  void               ReadGridFromFile(const std::string                  & file_name,
                                      const std::string                  & par_name,
                                      const float                          offset,
                                      std::vector<NRLib::Grid<float> *>  & grid,
                                      const SegyGeometry                *& geometry,
                                      const TraceHeaderFormat            * format,
                                      int                                  grid_type,
                                      const std::vector<Simbox *>          & interval_simboxes,
                                      const Simbox                       * inversion_simbox,
                                      const ModelSettings                * model_settings,
                                      std::string                        & err_text,
                                      bool                                 nopadding = false) const;

  void               GetZPaddingFromCravaFile(const std::string & file_name,
                                              std::string       & err_text,
                                              int               & nz_pad) const;

  void               ReadSegyFile(const std::string                 & file_name,
                                  std::vector<NRLib::Grid<float> *> & interval_grids,
                                  const std::vector<Simbox *>       & interval_simboxes,
                                  const NRLib::Volume               * volume, //Defines reading interval.
                                  const ModelSettings               * model_settings,
                                  const SegyGeometry               *& geometry,
                                  int                                 grid_type,
                                  const std::string                 & par_name,
                                  float                               offset,
                                  const TraceHeaderFormat           * format,
                                  std::string                       & err_text,
                                  bool                                nopadding = false) const;

  int                GetFillNumber(int i, int n, int np) const;

  //int                FindClosestFactorableNumber(int leastint) const;

  void               SmoothTraceInGuardZone(std::vector<float> & data_trace,
                                            float                dz_data,
                                            float                smooth_length) const;

  void               InterpolateGridValues(std::vector<float> & grid_trace,
                                           float                z0_grid,
                                           float                dz_grid,
                                           fftw_real          * rAmpFine,
                                           float                z0_data,
                                           float                dz_fine,
                                           int                  n_fine,
                                           int                  nz,
                                           int                  nzp) const;

  void               InterpolateAndShiftTrend(std::vector<float>       & interpolated_trend,
                                              float                      z0_grid,
                                              float                      dz_grid,
                                              const std::vector<float> & trend_long,
                                              float                      z0_data,
                                              float                      dz_fine,
                                              int                        n_fine,
                                              int                        nz,
                                              int                        nzp) const;

  int                GetZSimboxIndex(int k,
                                     int nz,
                                     int nzp) const;

  void               SetTrace(const std::vector<float> & trace,
                              NRLib::Grid<float>       * grid,
                              int                        i,
                              int                        j) const;

  void               SetTrace(float                value,
                              NRLib::Grid<float> * grid,
                              int                  i,
                              int                  j) const;

  void               SetTrace(const std::vector<float> & trace,
                              FFTGrid                  * grid,
                              int                        i,
                              int                        j) const;

  void               SetTrace(float     value,
                              FFTGrid * grid,
                              int       i,
                              int       j) const;

  void               ReadStormFile(const std::string                 & file_name,
                                   std::vector<NRLib::Grid<float> *> & interval_grids,
                                   const int                           grid_type,
                                   const std::string                 & par_name,
                                   const std::vector<Simbox *>         & interval_simboxes,
                                   const ModelSettings               * model_settings,
                                   std::string                       & err_text,
                                   bool                                is_storm = true,
                                   bool                                nopadding = true) const;

  bool               SetupDepthConversion(ModelSettings * model_settings,
                                          InputFiles    * input_files,
                                          Simbox        & simbox,
                                          GridMapping  *& time_depth_mapping,
                                          bool          & velocity_from_inversion,
                                          std::string   & err_text_common) const;

  bool               SetupBackgroundModel(ModelSettings                                              * model_settings,
                                          InputFiles                                                 * input_files,
                                          const std::vector<NRLib::Well *>                           & wells,
                                          std::map<int, std::map<std::string, BlockedLogsCommon *> > & mapped_blocked_logs_intervals,
                                          std::map<std::string, BlockedLogsCommon *>                 & bg_blocked_logs,
                                          std::vector<Simbox *>                                      & bg_simboxes,
                                          const std::vector<std::string>                             & cont_logs_to_be_blocked,
                                          const std::vector<std::string>                             & disc_logs_to_be_blocked,
                                          MultiIntervalGrid                                          * multi_interval_grid,
                                          Simbox                                                     * inversion_simbox,
                                          std::vector<std::vector<NRLib::Grid<float> *> >            & background_parameters,
                                          NRLib::Grid2D<std::vector<double> >                        & vertical_trends,
                                          std::vector<double>                                        & background_vs_vp_ratios,
                                          const std::vector<CravaTrend>                              & trend_cubes,
                                          std::string                                                & err_text_common) const;

  double             FindMeanVsVp(const NRLib::Grid<float> * vp,
                                  const NRLib::Grid<float> * vs) const;

  void               SubtractGrid(NRLib::Grid<float>       * to_grid,
                                  const NRLib::Grid<float> * from_grid) const;

  void               ChangeSignGrid(NRLib::Grid<float> * grid) const;

  void               LoadVelocity(NRLib::Grid<float>  * velocity,
                                  Simbox              * interval_simbox,
                                  const ModelSettings * model_settings,
                                  const std::string   & velocity_field,
                                  bool                & velocity_from_inversion,
                                  std::string         & err_text) const;

  std::map<std::string, DistributionsRock *> GetRockDistributionTime0() const;

  void               GenerateRockPhysics3DBackground(const std::vector<DistributionsRock *> & rock_distribution,
                                                     const std::vector<float>               & probability,
                                                     NRLib::Grid<float>                    *& vp,
                                                     NRLib::Grid<float>                    *& vs,
                                                     NRLib::Grid<float>                    *& rho,
                                                     const Simbox                           & simbox,
                                                     const CravaTrend                       & trend_cube) const;

  void               SetupExtendedBackgroundSimbox(const Simbox * simbox,
                                                   Surface      * corr_surf,
                                                   Simbox      *& bg_simbox,
                                                   int            output_format,
                                                   int            output_domain,
                                                   int            other_output,
                                                   std::string    interval_name) const;

  void               SetupExtendedBackgroundSimbox(const Simbox * simbox,
                                                   Surface      * top_corr_surf,
                                                   Surface      * bot_corr_surf,
                                                   Simbox      *& bg_simbox,
                                                   int            output_format,
                                                   int            output_domain,
                                                   int            other_output,
                                                   std::string    interval_name) const;

  bool               SetupPriorCorrelation(const ModelSettings                                              * model_settings,
                                           const InputFiles                                                 * input_files,
                                           const std::vector<NRLib::Well *>                                 & wells,
                                           const std::map<int, std::map<std::string, BlockedLogsCommon *> > & mapped_blocked_logs_intervals,
                                           const std::map<std::string, BlockedLogsCommon *>                 & mapped_blocked_logs_for_correlation,
                                           const std::vector<Simbox *>                                      & interval_simboxes,
                                           const std::vector<std::string>                                   & facies_names,
                                           const std::vector<CravaTrend>                                    & trend_cubes,
                                           const std::vector<std::vector<NRLib::Grid<float> *> >            & background,
                                           double                                                             dz_min,
                                           std::vector<std::vector<double> >                                & prior_corr_T,
                                           std::vector<NRLib::Matrix>                                       & prior_param_cov,
                                           std::vector<Surface *>                                           & prior_corr_XY,
                                           std::vector<std::vector<NRLib::Matrix> >                         & prior_auto_cov,
                                           bool                                                             & prior_cov_estimated,
                                           std::string                                                      & err_text_common) const;

  void               ReadPriorAutoCovariance(const std::string                        & file_name,
                                             std::vector<std::vector<NRLib::Matrix> > & prior_auto_cov,
                                             int                                        nz_pad,
                                             int                                        i_interval,
                                             std::string                              & err_txt) const;

  void               ResampleAutoCovToCorrectDz(const std::vector<NRLib::Matrix>                      & prior_auto_cov_dz_min,
                                                double                                                  dz_min,
                                                std::vector<NRLib::Matrix>                            & prior_auto_cov,
                                                double                                                  dz) const;

  void               CalculateCovarianceFromRockPhysics(const std::vector<DistributionsRock *>           & rock_distribution,
                                                        const std::vector<float>                         & probability,
                                                        const CravaTrend                                 & trend_cubes,
                                                        NRLib::Matrix                                    & param_cov,
                                                        std::string                                      & err_txt) const;

  void               CalculateCovarianceInTrendPosition(const std::vector<DistributionsRock *> & rock_distribution,
                                                        const std::vector<float>               & probability,
                                                        const std::vector<double>              & trend_position,
                                                        NRLib::Grid2D<double>                  & sigma_sum) const;

  void               EstimateXYPaddingSizes(Simbox          * interval_simbox,
                                            ModelSettings   * model_settings) const;

  void               ValidateCovarianceMatrix(float               ** C,
                                              const ModelSettings *  model_settings,
                                              std::string         &  err_txt) const;

  void               ValidatePriorAutoCovMatrices(const std::vector<std::vector<NRLib::Matrix> > & prior_auto_cov,
                                                  const ModelSettings                            * model_settings,
                                                  std::string                                    & err_txt) const;

  Surface *          FindCorrXYGrid(const Simbox           * time_simbox,
                                    const ModelSettings    * model_settings) const;

  bool               SetupTimeLine(ModelSettings          * model_settings,
                                   TimeLine              *& time_line,
                                   std::string            & err_text_common) const;

  bool               SetupGravityInversion(const ModelSettings                * model_settings,
                                           const InputFiles                   * input_files,
                                           std::vector<std::vector<float> >   & observation_location_utmx,
                                           std::vector<std::vector<float> >   & observation_location_utmy,
                                           std::vector<std::vector<float> >   & observation_location_depth,
                                           std::vector<std::vector<float> >   & gravity_response,
                                           std::vector<std::vector<float> >   & gravity_std_dev,
                                           std::string                        & err_text_common) const;

  void               ReadGravityDataFile(const std::string   & file_name,
                                         const std::string   & read_reason,
                                         int                   n_obs,
                                         int                   n_columns,
                                         std::vector <float> & obs_loc_utmx,
                                         std::vector <float> & obs_loc_utmy,
                                         std::vector <float> & obs_loc_depth,
                                         std::vector <float> & gravity_response,
                                         std::vector <float> & gravity_std_dev,
                                         std::string         & err_text) const;

  void               ProcessHorizons(std::vector<Surface>   & horizons,
                                     const InputFiles       * input_files,
                                     std::string            & err_text,
                                     bool                   & failed,
                                     int                      i_timelapse) const;

  void               CheckCovarianceParameters(NRLib::Matrix            & param_cov) const;

  /*
  bool               SetupTravelTimeInversion(ModelSettings * model_settings,
                                              InputFiles    * input_files,
                                              Simbox        * inversion_simbox,
                                              std::string   & err_text_common) const;
                                              */

  //void               WriteFilePriorVariances(const ModelSettings      * model_settings,
  //                                           const std::vector<double> & prior_corr_T,
  //                                           const Surface             * prior_corr_XY,
  //                                           const float               & dt) const;

  void               WriteFilePriorVariances(const ModelSettings               * model_settings,
                                             const NRLib::Matrix               & prior_param_cov,
                                             const std::vector<double>         & prior_corr_T,
                                             const Surface                     * prior_corr_XY,
                                             const std::string                 & interval_name,
                                             const double                      & dz) const;

  void               WriteFilePriorVariances(const ModelSettings                * model_settings,
                                             const std::vector<NRLib::Matrix>   & prior_auto_cov,
                                             const Surface                      * prior_corr_XY,
                                             const std::string                  & interval_name,
                                             double                               dz) const;

  int                ComputeTime(int year, int month, int day) const;

  void               PrintPriorVariances(const NRLib::Matrix & prior_param_cov) const;

  void               ReadAngularCorrelations(ModelSettings                                  * model_settings,
                                             std::vector<std::vector<std::vector<float> > > & angular_correlations) const;

  void               SetDebugLevel(ModelSettings    * model_settings) const;

  void               PrintSettings(const ModelSettings    * model_settings,
                                   const InputFiles       * input_files) const;

  void               WriteOutputSurfaces(ModelSettings * model_settings,
                                         Simbox        & simbox) const;

  void               DumpVector(const std::vector<float> data,
                                const std::string        name) const;

  void               DumpVector(const fftw_real   * data,
                                int                 n_data,
                                const std::string   name) const;

  // CLASS VARIABLES ---------------------------------------------------

  // Bool variables indicating whether the corresponding data processing
  // succeeded
  bool outer_temp_simbox_;
  bool read_seismic_;
  bool read_wells_;
  bool block_wells_;
  bool block_wells_output_;
  bool inversion_wells_;
  bool correlation_wells_;
  bool setup_reflection_matrix_;
  bool temporary_wavelet_;
  bool optimize_well_location_;
  bool wavelet_handling_;
  bool setup_multigrid_;
  bool setup_output_simbox_;
  bool setup_trend_cubes_;
  bool setup_estimation_rock_physics_;
  bool setup_prior_facies_probabilities_;
  bool setup_depth_conversion_;
  bool setup_background_model_;
  bool setup_prior_correlation_;
  bool setup_timeline_;
  bool setup_gravity_inversion_;
  bool setup_traveltime_inversion_;

  MultiIntervalGrid                                          * multiple_interval_grid_;
  Simbox                                                       estimation_simbox_;
  Simbox                                                       output_simbox_;         //Output simbox for writing. Top and bot surfaces are the visible surfaces (eroded surfaces)
  Simbox                                                       full_inversion_simbox_; //This simbox holds upper and lower surface, and xy-resolution.
                                                                                       //Not to be used for any z-resolution purposes.

  std::vector<std::vector<SeismicStorage *> >                  seismic_data_; //Vector timelapse, vector angles

  // Well logs
  std::vector<NRLib::Well* >                                   wells_;

  // Blocked well logs
  std::map<std::string, BlockedLogsCommon *>                   mapped_blocked_logs_;                 ///< Blocked logs with estimation simbox
  std::map<std::string, BlockedLogsCommon *>                   mapped_blocked_logs_output_;          ///< Blocked logs with output simbox
  std::map<std::string, BlockedLogsCommon *>                   mapped_blocked_logs_for_correlation_; ///< Blocked logs for estimation of vertical corr
  std::map<int, std::map<std::string, BlockedLogsCommon *> >   mapped_blocked_logs_intervals_;       ///< Blocked logs to interval simboxes
  std::map<std::string, BlockedLogsCommon *>                   mapped_bg_blocked_logs_;              ///< Blocked logs for extended background simbox
  std::vector<std::string>                                     continuous_logs_to_be_blocked_;       ///< Continuous logs that should be blocked
  std::vector<std::string>                                     discrete_logs_to_be_blocked_;         ///< Discrete logs that should be blocked
  // Trend cubes and rock physics
  std::vector<CravaTrend>                                      trend_cubes_;            //Trend cubes per interval.
  std::map<std::string, std::vector<DistributionsRock *> >     rock_distributions_;     ///< Rocks used in rock physics model. Outer map: Inner vector:
  std::map<std::string, std::vector<DistributionWithTrend *> > reservoir_variables_;    ///< Reservoir variables used in the rock physics model. Outer map: Inner vector:

  // prior facies
  std::vector<std::vector<NRLib::Grid<float> *> >              prior_facies_prob_cubes_;
  std::vector<std::vector<float> >                             prior_facies_;                  ///< Prior facies probabilities. outer vector (intervals) inner vector(parameters)
  std::vector<Surface *>                                       facies_estim_interval_;

  // background model
  std::vector<std::vector<NRLib::Grid<float> *> >              background_parameters_;      // outer vector: intervals, inner vector: parameters
  NRLib::Grid2D<std::vector<double> >                          background_vertical_trends_; // First index is interval, other is vp, vs, rho.
  std::vector<double>                                          background_vs_vp_ratios_;    // vs_vp_ratios from generation of backgroundmodel
  std::vector<Simbox *>                                        bg_simboxes_;                // Extended background simboxes

  // Timeline
  TimeLine                                                   * time_line_;

  bool                                                         forward_modeling_;

  std::vector<NRLib::Matrix>                                   reflection_matrix_;             // reflection matrix per timelapse
  std::vector<int>                                             n_angles_;                      // Number of angles for each timelapse
  bool                                                         refmat_from_file_global_vpvs_;  // True if reflection matrix is from file or set up from global vp/vs value.

  // Wavelet
  std::vector<Wavelet*>                                        temporary_wavelets_;            ///< Wavelet per angle
  std::map<int, std::vector<Wavelet *> >                       wavelets_;                      //Map time_lapse, vector angles
  std::map<int, std::vector<Grid2D *> >                        local_noise_scales_;
  std::map<int, std::vector<float> >                           global_noise_estimates_;
  std::map<int, std::vector<float> >                           sn_ratios_;
  std::string                                                  wavelet_est_int_top_; //Filename for wavelet estimation interval
  std::string                                                  wavelet_est_int_bot_ ;
  std::vector<std::vector<Grid2D *> >                          shift_grids_; //vector timelapse, vector angles
  std::vector<std::vector<Grid2D *> >                          gain_grids_;  //vector timelapse, vector angles
  std::vector<std::vector<Wavelet1D *> >                       well_wavelets_; //Optimal wavelets per well.

  std::vector<std::vector<double> >                            t_grad_x_;
  std::vector<std::vector<double> >                            t_grad_y_;
  NRLib::Grid2D<float>                                         ref_time_grad_x_; ///< Time gradient in x-direction for reference time surface (t0)
  NRLib::Grid2D<float>                                         ref_time_grad_y_; ///< Time gradient in x-direction for reference time surface (t0)

  // Facies
  std::vector<bool>                                            facies_log_wells_;   ///< True if this well has a facies log

  std::vector<std::string>                                     facies_names_; ///< Facies names combined for wells.
  std::vector<int>                                             facies_nr_;

  // Prior correlation
  bool                                                         prior_corr_per_interval_;       ///< If there is not enough data to estimate per interval, this is false
  bool                                                         prior_cov_estimated_or_file_;   ///< CRA-257: If prior covariances are estimated, time corr is included in this estimation. This is also true if prior_auto_cov_ is read from file.
  std::vector<Surface *>                                       prior_corr_XY_;
  std::vector<std::vector<NRLib::Matrix> >                     prior_auto_cov_;                ///< CRA-257: New estimation of prior autocovariance - in this case prior_param_corr is not used. The first vector is intervals.
  std::vector<NRLib::Matrix>                                   prior_param_cov_;
  std::vector<std::vector<double> >                            prior_corr_T_;

  //Gravimetry parameters per timelapse
  std::vector<std::vector<float> >                             observation_location_utmx_;     ///< Vectors to store observation location coordinates
  std::vector<std::vector<float> >                             observation_location_utmy_;
  std::vector<std::vector<float> >                             observation_location_depth_;
  std::vector<std::vector<float> >                             gravity_response_;              ///< Vector to store base line gravity response
  std::vector<std::vector<float> >                             gravity_std_dev_;               ///< Vector to store base line gravity standard deviation

  //Traveltime parameters per timelapse
  std::vector<std::vector<Surface> >                           horizons_;                      ///< Horizons used for horizon inversion
  std::vector<NRLib::Grid<float> *>                            rms_data_;                      ///< RMS data U^2

  //Depth conversion
  GridMapping                                                * time_depth_mapping_;
  bool                                                         velocity_from_inversion_;

  //Angular correlations
  std::vector<std::vector<std::vector<float> > >               angular_correlations_;

};
#endif
