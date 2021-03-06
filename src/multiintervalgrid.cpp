/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#include "nrlib/grid/grid.hpp"
#include "nrlib/random/beta.hpp"

#include "src/multiintervalgrid.h"
#include "src/definitions.h"
#include "src/simbox.h"
#include "src/tasklist.h"

MultiIntervalGrid::MultiIntervalGrid(ModelSettings * model_settings,
                                     InputFiles    * input_files,
                                     const Simbox  * estimation_simbox,
                                     SegyGeometry  * segy_geometry,
                                     std::string   & err_text,
                                     bool          & failed)
{

  interval_names_                                                 = model_settings->getIntervalNames();
  n_intervals_                                                    = static_cast<int>(interval_names_.size());
  interval_simboxes_.resize(n_intervals_);
  desired_grid_resolution_.resize(n_intervals_);
  relative_grid_resolution_.resize(n_intervals_);
  erosion_priorities_.resize(n_intervals_+1);
  int n_surfaces                                                  = n_intervals_ + 1;
  eroded_surfaces_.resize(n_surfaces);
  std::vector<std::vector<Surface> > visible_surfaces(n_intervals_); //These surfaces give same visible zone as eroded, but are truncated to uneroded. Gives best zone definition.
  int erosion_priority_top_surface                                = model_settings->getErosionPriorityTopSurface();
  const std::map<std::string,int> erosion_priority_base_surfaces  = model_settings->getErosionPriorityBaseSurfaces();
  const std::map<std::string, double> uncertainty_base_surfaces   = model_settings->getUncertaintyBaseSurfaces();
  dz_min_                                                         = 10000;

  if (model_settings->GetMultipleIntervalSetting() == false) {
    LogKit::WriteHeader("Setting up inversion grid");
    multiple_interval_setting_ = false;
  }
  else {
    LogKit::WriteHeader("Setting up multiple interval grid");
    multiple_interval_setting_ = true;
  }

  uncertainties_.resize(n_intervals_+1); //Two more than needed, but now follows erosion priorities.

  std::vector<Surface>    surfaces(n_intervals_+1);                 //Store surfaces.

  // Temp variables
  std::string             previous_interval_name("");
  std::string             top_surface_file_name_temp("");
  std::string             base_surface_file_name_temp("");
  Surface               * top_surface  = NULL;
  Surface               * base_surface = NULL;

  // 1. ERODE SURFACES AND SET SURFACES OF SIMBOXES -----------------------------------------

  try{
    // if multiple-intervals keyword is used in model settings
    if (multiple_interval_setting_) {
      top_surface_file_name_temp = input_files->getTimeSurfTopFile();
      erosion_priorities_[0] = erosion_priority_top_surface;
      uncertainties_[0] = 0;

      top_surface = MakeSurfaceFromFileName(top_surface_file_name_temp, *estimation_simbox, model_settings, input_files, segy_geometry, err_text);
      surfaces[0] = *top_surface;
      for (int i = 0; i < n_intervals_; i++) {

        std::string interval_name = model_settings->getIntervalName(i);
        base_surface_file_name_temp = input_files->getBaseTimeSurface(interval_name);
        erosion_priorities_[i+1] = erosion_priority_base_surfaces.find(interval_name)->second;

        double uncty = 0.001;
        std::map<std::string, double>::const_iterator it = uncertainty_base_surfaces.find(interval_name);
        if(it != uncertainty_base_surfaces.end())
          uncty = it->second;
        uncertainties_[i+1] = uncty;
        if(i == n_intervals_-1 && uncertainties_[i+1] > 0.001)
          LogKit::LogMessage(LogKit::Warning,"Warning: Uncertainty on last base surface is ignored.\n\n");

        base_surface = MakeSurfaceFromFileName(base_surface_file_name_temp, *estimation_simbox, model_settings, input_files, segy_geometry, err_text);
        surfaces[i+1] =  *base_surface;
      }

      if (!failed) {

        for (int i = 0; i<n_intervals_; i++) {
          desired_grid_resolution_[i] = FindResolution(&surfaces[i], &surfaces[i+1], estimation_simbox,
                                                       model_settings->getTimeNz(interval_names_[i]), model_settings->getTimeDz());

        }

        ErodeAllSurfaces(eroded_surfaces_,
                         erosion_priorities_,
                         surfaces,
                         *estimation_simbox,
                         err_text);

        CreateVisibleSurfaces(surfaces, eroded_surfaces_, visible_surfaces);


        //Check if eroded surfaces have max distance greater than zero.
        //H-TODO Make removal of intervals automatic
        for (int i = 0; i<n_intervals_; i++) {

          double max_distance = eroded_surfaces_[i+1].Max() - eroded_surfaces_[i].Min();

          //If max distance of eroded surfaces is zero we can't set up interval simboxes
          if (max_distance == 0) {
            LogKit::LogMessage(LogKit::Error,"Error: Eroded surfaces for interval " + interval_names_[i] + " has maximum thickness equal to zero. This interval should be removed.\n");
            err_text += "Eroded surfaces for interval " + interval_names_[i] + " has maximum distance equal to zero. This interval should be removed.\n";
            TaskList::addTask("Removed interval " + interval_names_[i]);
            failed = true;
          }
        }

      }
      else {
        err_text += "Erosion of surfaces failed because the interval surfaces could not be set up correctly.\n";
      }
    }
    // if multiple-intervals is NOT used in model settings
    else {

      if (model_settings->getParallelTimeSurfaces() == false) {
        top_surface_file_name_temp = input_files->getTimeSurfTopFile();
        top_surface = MakeSurfaceFromFileName(top_surface_file_name_temp, *estimation_simbox, model_settings, input_files, segy_geometry, err_text);
        eroded_surfaces_[0] = *top_surface;
        surfaces[0]         = *top_surface;

        base_surface_file_name_temp = input_files->getBaseTimeSurface("");
        base_surface = MakeSurfaceFromFileName(base_surface_file_name_temp, *estimation_simbox, model_settings, input_files, segy_geometry, err_text);
        eroded_surfaces_[1] = *base_surface;
        surfaces[1]         = *base_surface;

        std::vector<Surface> v_surf_for_zone(2);
        v_surf_for_zone[0] = *top_surface;
        v_surf_for_zone[1] = *base_surface;
        visible_surfaces[0] = v_surf_for_zone;
      }
      else { //If only one surface-file is used, similar to setup of estimation_simbox.
        top_surface_file_name_temp = input_files->getTimeSurfTopFile();
        top_surface = MakeSurfaceFromFileName(top_surface_file_name_temp, *estimation_simbox, model_settings, input_files, segy_geometry, err_text);
        eroded_surfaces_[0] = *top_surface;
        surfaces[0]         = *top_surface;

        base_surface = new Surface(*top_surface);
        double lz = model_settings->getTimeLz();
        base_surface->Add(lz);
        eroded_surfaces_[1] = *base_surface;
        surfaces[1]         = *base_surface;

        std::vector<Surface> v_surf_for_zone(2);
        v_surf_for_zone[0] = *top_surface;
        v_surf_for_zone[1] = *base_surface;
        visible_surfaces[0] = v_surf_for_zone;

        double dz = model_settings->getTimeDz();
        if (model_settings->getTimeNzs().find("") == model_settings->getTimeNzs().end()) { //Taken from simbox->SetDepth without nz
          int nz = static_cast<int>(0.5+lz/dz);
          model_settings->setTimeNz("", nz);
        }
      }

      int nz = model_settings->getTimeNz("");
      desired_grid_resolution_[0] = FindResolution(top_surface, base_surface, estimation_simbox, nz, model_settings->getTimeDz());
    }
    surfaces_ = surfaces;
  }
  catch(NRLib::Exception & e) {
    failed = true;
    err_text += e.what();
  }

  delete top_surface;
  delete base_surface;

  // 2 SET UP INTERVAL_SIMBOXES ------------------------------------------------------------

  //Set up a vector of simboxes, one per interval.
  if (!failed) {
    try{
        SetupIntervalSimboxes(model_settings,
                              input_files,
                              estimation_simbox,
                              interval_names_,
                              visible_surfaces,
                              interval_simboxes_,
                              input_files->getCorrDirFiles(),
                              input_files->getCorrDirTopSurfaceFiles(),
                              input_files->getCorrDirBaseSurfaceFiles(),
                              model_settings->getCorrDirTopConforms(),
                              model_settings->getCorrDirBaseConforms(),
                              desired_grid_resolution_,
                              relative_grid_resolution_,
                              dz_min_,
                              dz_rel_,
                              segy_geometry,
                              err_text,
                              failed);
    }
    catch(NRLib::Exception & e) {
      failed = true;
      err_text += e.what();
    }
  }

  // 3  WRITE SURFACES ----------------------------------------------------------------------
  if (!failed) {
    for (size_t i = 0 ; i < interval_simboxes_.size(); i++) {
      bool   generate_seismic     = model_settings->getForwardModeling();
      bool   estimation_mode      = model_settings->getEstimationMode();
      bool   generate_background  = model_settings->getGenerateBackground();
      int    output_format        = model_settings->getOutputGridFormat();
      int    output_domain        = model_settings->getOutputGridDomain();
      int    output_grids_elastic = model_settings->getOutputGridsElastic();
      int    output_grids_other   = model_settings->getOutputGridsOther();
      int    output_grids_seismic = model_settings->getOutputGridsSeismic();

      //Add in writing of ascii-files
      if (model_settings->getWriteAsciiSurfaces() && !(output_format & IO::ASCII))
        output_format += IO::ASCII;

      if ((output_domain & IO::TIMEDOMAIN) > 0) {
        std::string top_surf          = IO::PrefixSurface() + IO::PrefixTop() + interval_names_[i] + "_"  + IO::PrefixTime();
        std::string base_surf         = IO::PrefixSurface() + IO::PrefixBase() + interval_names_[i] + "_" + IO::PrefixTime();
        if (interval_simboxes_.size() == 1) {
          top_surf          = IO::PrefixSurface() + IO::PrefixTop()  + IO::PrefixTime();
          base_surf         = IO::PrefixSurface() + IO::PrefixBase() + IO::PrefixTime();
        }
        interval_simboxes_[i]->setTopBotName(top_surf,base_surf, output_format);
        //std::string top_surf_eroded   = IO::PrefixSurface() + IO::PrefixTop() +  IO::PrefixEroded() + interval_names_[i] + "_" + IO::PrefixTime();
        //std::string base_surf_eroded  = IO::PrefixSurface() + IO::PrefixBase() + IO::PrefixEroded() + interval_names_[i] + "_" + IO::PrefixTime();
        //if (interval_simboxes_.size() == 1) {
        //  top_surf_eroded          = IO::PrefixSurface() + IO::PrefixTop() +  IO::PrefixEroded()  + IO::PrefixTime();
        //  base_surf_eroded         = IO::PrefixSurface() + IO::PrefixBase() + IO::PrefixEroded()  + IO::PrefixTime();
        //}
        //interval_simboxes_[i]->SetTopBaseErodedNames(top_surf_eroded, base_surf_eroded, output_format);
        if (generate_seismic) {
          interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                                  IO::PathToSeismicData(), output_format);
          //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
          //                                         IO::PathToSeismicData(), output_format);
        }
        else if (!estimation_mode) {
          if (output_grids_elastic > 0 || output_grids_other > 0 || output_grids_seismic > 0)
            interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                      IO::PathToInversionResults(), output_format);
            //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
            //                          IO::PathToInversionResults(), output_format);
        }
        if ((output_format & IO::STORM) > 0) { // These copies are only needed with the STORM format
          if ((output_grids_elastic & IO::BACKGROUND) > 0 ||
              (output_grids_elastic & IO::BACKGROUND_TREND) > 0 ||
              (estimation_mode && generate_background)) {
            interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                      IO::PathToBackground(), output_format);
            //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
            //                          IO::PathToBackground(), output_format);
          }
          if ((output_grids_other & IO::CORRELATION) > 0) {
            interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                                    IO::PathToCorrelations(), output_format);
            //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
            //                                        IO::PathToCorrelations(), output_format);
          }
          if ((output_grids_seismic & (IO::ORIGINAL_SEISMIC_DATA | IO::SYNTHETIC_SEISMIC_DATA)) > 0) {
            interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                                    IO::PathToSeismicData(), output_format);
            //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
            //                                        IO::PathToSeismicData(), output_format);
          }
          if ((output_grids_other & IO::TIME_TO_DEPTH_VELOCITY) > 0) {
            interval_simboxes_[i]->WriteTopBaseSurfaceGrids(top_surf, base_surf,
                                                    IO::PathToVelocity(), output_format);
            //interval_simboxes_[i]->WriteTopBaseErodedSurfaceGrids(top_surf_eroded, base_surf_eroded,
            //                                        IO::PathToVelocity(), output_format);
          }
        }
      }
    }
  }

  // Add surface files.
  surface_files_.push_back(input_files->getTimeSurfTopFile());

  const std::map<std::string, std::string> & interval_base_time_surfaces = input_files->getBaseTimeSurfaces();
  for (int i = 0; i > n_intervals_; i++) {
    surface_files_.push_back(interval_base_time_surfaces.find(interval_names_[i])->second);
  }

}

//Copy constructor
MultiIntervalGrid::MultiIntervalGrid(const MultiIntervalGrid * multi_interval_grid)
{
  n_intervals_               = multi_interval_grid->n_intervals_;
  multiple_interval_setting_ = multi_interval_grid->multiple_interval_setting_;

  interval_names_            = multi_interval_grid->interval_names_;
  erosion_priorities_        = multi_interval_grid->erosion_priorities_;
  surface_files_             = multi_interval_grid->surface_files_;

  interval_simboxes_         = multi_interval_grid->interval_simboxes_;

  desired_grid_resolution_   = multi_interval_grid->desired_grid_resolution_;
  relative_grid_resolution_  = multi_interval_grid->relative_grid_resolution_;
  dz_rel_                    = multi_interval_grid->dz_rel_;
}

MultiIntervalGrid::~MultiIntervalGrid() {
  for (size_t i = 0; i < interval_simboxes_.size(); i++)
    delete interval_simboxes_[i];
}

// ---------------------------------------------------------------------------------------------------------------
int   MultiIntervalGrid::WhichSimbox(double x, double y, double z) const
{
  int simbox_num = -1;
  for (size_t i = 0; i < interval_simboxes_.size(); i++) {
    if (interval_simboxes_[i]->IsPointBetweenVisibleSurfaces(x,y,z)) {
      simbox_num = static_cast<int>(i);
      break;
    }
  }
  return simbox_num;
}


// ---------------------------------------------------------------------------------------------------------------
void   MultiIntervalGrid::SetupIntervalSimboxes(ModelSettings                             * model_settings,
                                                const InputFiles                          * input_files,
                                                const Simbox                              * estimation_simbox,
                                                const std::vector<std::string>            & interval_names,
                                                const std::vector<std::vector<Surface> >  & eroded_surfaces,
                                                std::vector<Simbox *>                     & interval_simboxes,
                                                const std::map<std::string, std::string>  & corr_dir_single_surfaces,
                                                const std::map<std::string, std::string>  & corr_dir_top_surfaces,
                                                const std::map<std::string, std::string>  & corr_dir_base_surfaces,
                                                const std::map<std::string, bool>         & corr_dir_top_conform,
                                                const std::map<std::string, bool>         & corr_dir_base_conform,
                                                std::vector<double>                       & desired_grid_resolution,
                                                std::vector<double>                       & relative_grid_resolution,
                                                double                                    & dz_min,
                                                std::vector<double>                       & dz_rel,
                                                SegyGeometry                              * segy_geometry,
                                                std::string                               & err_text,
                                                bool                                      & failed) const
{
  for (size_t i = 0; i < interval_names.size(); i++) {

    bool                   failed_tmp                             = false;
    std::string            err_text_tmp                           = "";
    bool                   corr_dir                               = false;
    Surface                top_surface                            = eroded_surfaces[i][0];
    Surface                base_surface                           = eroded_surfaces[i][1];
    std::string            interval_name                          = interval_names[i];
    int                    n_layers                               = model_settings->getTimeNz(interval_name);
    double                 dz                                     = model_settings->getTimeDz();
    std::map<std::string, std::string>::const_iterator it_single  = corr_dir_single_surfaces.find(interval_name);
    std::map<std::string, std::string>::const_iterator it_top     = corr_dir_top_surfaces.find(interval_name);
    std::map<std::string, std::string>::const_iterator it_base    = corr_dir_base_surfaces.find(interval_name);
    std::map<std::string, bool>::const_iterator it_top_conform    = corr_dir_top_conform.find(interval_name);
    std::map<std::string, bool>::const_iterator it_base_conform   = corr_dir_base_conform.find(interval_name);
    int                    other_output_flag                      = model_settings->getOtherOutputFlag();
    int                    other_output_domain                    = model_settings->getOutputGridDomain();
    int                    other_output_format                    = model_settings->getOutputGridFormat();

    if (model_settings->getWriteAsciiSurfaces() && !(other_output_format & IO::ASCII))
      other_output_format+= IO::ASCII;

    // Make a simbox for the original interval --------------------------------------------
    //SegyGeometry * geometry = model_settings->getAreaParameters();

    float min_samp_dens = model_settings->getMinSamplingDensity();

    // Make extended interval_simbox for the inversion interval ---------------------------

    // Case 1: Single correlation surface
    if (it_single != corr_dir_single_surfaces.end() && it_top == corr_dir_top_surfaces.end() && it_base == corr_dir_base_surfaces.end()) {
      corr_dir = true;
      Surface * corr_surf  = MakeSurfaceFromFileName(it_single->second,  *estimation_simbox, model_settings, input_files, segy_geometry, err_text_tmp);
      interval_simboxes[i] =  new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, corr_surf,
                                         other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
      delete corr_surf;
    }
    // Case 2: Top and base correlation surfaces
    else if (it_single == corr_dir_single_surfaces.end() && it_top != corr_dir_top_surfaces.end() && it_base != corr_dir_base_surfaces.end()) {
      corr_dir = true;
      Surface * corr_surf_top  = MakeSurfaceFromFileName(it_top->second,  *estimation_simbox, model_settings, input_files, segy_geometry, err_text_tmp);
      Surface * corr_surf_base = MakeSurfaceFromFileName(it_base->second, *estimation_simbox, model_settings, input_files, segy_geometry, err_text_tmp);
      interval_simboxes[i] = new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, corr_surf_top, corr_surf_base,
                                        other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
      delete corr_surf_top;
      delete corr_surf_base;
    }
    // Case 3: Top conform and base correlation surface
    else if (it_top_conform->second == true && it_base != corr_dir_base_surfaces.end()) {
      corr_dir = true;
      Surface * corr_surf_base = MakeSurfaceFromFileName(it_base->second, *estimation_simbox, model_settings, input_files, segy_geometry, err_text_tmp);
      interval_simboxes[i] = new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, &top_surface, corr_surf_base,
                                        other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
      delete corr_surf_base;
    }
    // Case 4: Top correlation surface and base conform
    else if (it_top != corr_dir_top_surfaces.end() && it_base_conform->second == true) {
      corr_dir = true;
      Surface * corr_surf_top = MakeSurfaceFromFileName(it_top->second, *estimation_simbox, model_settings, input_files, segy_geometry, err_text_tmp);
      interval_simboxes[i]    = new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, corr_surf_top, &base_surface,
                                           other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
      delete corr_surf_top;
    }
    // Case 5: Top and base conform
    else if (it_top_conform->second == true && it_base_conform->second == true) {
      interval_simboxes[i] = new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, err_text_tmp, failed_tmp);
    }
    //Case 6: Only top conform: Use top surface as single correlation surface
    else if (it_top_conform->second == true) {
      interval_simboxes[i] =  new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, &top_surface,
                                         other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
    }
    //Case 7: Only base conform: Use base surface as single correlation surface
    else if (it_base_conform->second == true) {
      interval_simboxes[i] =  new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, &base_surface,
                                         other_output_flag, other_output_domain, other_output_format, err_text_tmp, failed_tmp);
    }
    // Case 8: No correlation directions in the xml file - top and base conform
    else if (corr_dir_top_conform.size() == 0 && corr_dir_base_conform.size() == 0) {
      interval_simboxes[i] = new Simbox(estimation_simbox, interval_names[i], n_layers, dz, model_settings->getLzLimit(), top_surface, base_surface, err_text_tmp, failed_tmp);
    }
    // else something is wrong
    else {
      std::string interval_text = "";
      if (interval_name != "")
        interval_text = " for interval " + interval_name[i];

      err_text_tmp += "\nCorrelation directions are not set correctly" + interval_text + ".\n";
    }

    // Calculate Z padding ----------------------------------------------------------------

    if (err_text_tmp == "") {
      // calculated dz should be the same as the desired grid resolution?
      //interval_simboxes[i]->calculateDz(model_settings->getLzLimit(), err_text_tmp);
      relative_grid_resolution[i] = interval_simboxes[i]->getdz() / desired_grid_resolution[i];
      EstimateZPaddingSize(interval_simboxes[i], model_settings);


      if (interval_simboxes[i]->getdz()*interval_simboxes[i]->getMinRelThick() < min_samp_dens) {
        err_text_tmp += "We normally discourage denser sampling than "+NRLib::ToString(min_samp_dens);
        err_text_tmp += "ms in the time grid. If you really need\nthis, please use ";
        err_text_tmp += "<project-settings><advanced-settings><minimum-sampling-density>\n";
      }

      if (interval_simboxes[i]->status() == Simbox::BOXOK) {
        if (corr_dir) {
          LogIntervalInformation(interval_simboxes[i], interval_names[i],
            "Time inversion interval (extended relative to output interval due to correlation):","Two-way-time");
        }
        else{
          LogIntervalInformation(interval_simboxes[i], interval_names[i], "Time output interval","Two-way-time");
        }
      }
      else {
        err_text_tmp += "Could not make the time simulation grid.\n";
      }

    }

    // Calculate XY padding ---------------------------------------------------------------

    if (err_text_tmp == "") {

      //EstimateXYPaddingSizes(&interval_simboxes[i], model_settings);
      interval_simboxes[i]->SetNXpad(estimation_simbox->GetNXpad());
      interval_simboxes[i]->SetNYpad(estimation_simbox->GetNYpad());
      interval_simboxes[i]->SetXPadFactor(estimation_simbox->GetXPadFactor());
      interval_simboxes[i]->SetYPadFactor(estimation_simbox->GetYPadFactor());

      unsigned long long int grid_size = static_cast<unsigned long long int>(interval_simboxes[i]->GetNXpad()*interval_simboxes[i]->GetNYpad()*interval_simboxes[i]->GetNZpad());

      if (grid_size > std::numeric_limits<unsigned int>::max()) {
        float fsize = 4.0f*static_cast<float>(grid_size)/static_cast<float>(1024*1024*1024);
        float fmax  = 4.0f*static_cast<float>(std::numeric_limits<unsigned int>::max()/static_cast<float>(1024*1024*1024));
        err_text_tmp += "Grids as large as "+NRLib::ToString(fsize,1)+"GB cannot be handled. The largest accepted grid size\n";
        err_text_tmp += "is "+NRLib::ToString(fmax)+"GB. Please reduce the number of layers or the lateral resolution.\n";
      }

      if (interval_names.size() == 1)
        LogKit::LogFormatted(LogKit::Low,"\nTime simulation grids: \n");
      else
        LogKit::LogFormatted(LogKit::Low,"\nTime simulation grids for interval \'"+interval_names[i]+"\':\n");
      LogKit::LogFormatted(LogKit::Low,"  Output grid        %4i * %4i * %4i   : %10llu\n",
                            interval_simboxes[i]->getnx(),interval_simboxes[i]->getny(),interval_simboxes[i]->getnz(),
                            static_cast<unsigned long long int>(interval_simboxes[i]->getnx()*interval_simboxes[i]->getny()*interval_simboxes[i]->getnz()));
      LogKit::LogFormatted(LogKit::Low,"  FFT grid           %4i * %4i * %4i   :%11llu\n",
                            interval_simboxes[i]->GetNXpad(),interval_simboxes[i]->GetNYpad(),interval_simboxes[i]->GetNZpad(),
                            static_cast<unsigned long long int>(interval_simboxes[i]->GetNXpad()*interval_simboxes[i]->GetNYpad()*interval_simboxes[i]->GetNZpad()));
    }

    // Check consistency ------------------------------------------------------------------
    if (err_text_tmp == "" && interval_simboxes[i]->getdz() >= 10.0 && model_settings->getFaciesProbFromRockPhysics() == true) {
      err_text_tmp += "dz for interval \'" + interval_names[i] + "\' is too large to generate synthetic well data when estimating facies probabilities using rock physics models. Need dz < 10.";
    }

    if (err_text_tmp != "" || failed_tmp == true) {
      LogKit::LogFormatted(LogKit::Low," failed.\n");
      err_text += err_text_tmp;
      failed = true;
    }

  } // end for loop over intervals

  // Pick the simbox with the finest vertical resolution and set dz_rel relative to this dz
  if (failed == false) {
    dz_rel.resize(interval_names.size());
    dz_min = 10000;
    for (size_t m = 0; m < interval_simboxes.size(); m++) {
      if (interval_simboxes[m]->getdz() < dz_min)
        dz_min = interval_simboxes[m]->getdz();
    }
    for (size_t m = 0; m < interval_simboxes.size(); m++) {
      dz_rel[m] = interval_simboxes[m]->getdz()/dz_min;
    }
  }

}

// --------------------------------------------------------------------------------
Surface * MultiIntervalGrid::MakeSurfaceFromFileName(const std::string   & file_name,
                                                     const Simbox        & estimation_simbox,
                                                     const ModelSettings * model_settings,
                                                     const InputFiles    * input_files,
                                                     SegyGeometry        * segy_geometry,
                                                     std::string         & err_text) const
{
  Surface * new_surface = NULL;

  if (!NRLib::IsNumber(file_name)) { // If the file name is a string
    if (segy_geometry != NULL) {
        std::vector<int> ilxl_area = FindILXLArea(model_settings, input_files, segy_geometry);
        double lx = segy_geometry->GetDx() * segy_geometry->GetNx();
        double ly = segy_geometry->GetDy() * segy_geometry->GetNy();

        new_surface = new Surface(file_name, NRLib::SURF_UNKNOWN, segy_geometry->GetAngle(), segy_geometry->GetX0(),
                                  segy_geometry->GetY0(), lx, ly, &ilxl_area[0], segy_geometry->GetInLine0(), segy_geometry->GetCrossLine0());
    }
    else if (NRLib::FindSurfaceFileType(file_name) != NRLib::SURF_MULT_ASCII)
      new_surface = new Surface(file_name);
    else
      err_text += "Cannot read multicolumn ascii surface " + file_name + " without segy geometry.";

    RemoveNaNFromSurface(new_surface);
    InterpolateMissing(new_surface);
  }
  else { //If the file name is a value

    double x_min, x_max, y_min, y_max;

    FindSmallestSurfaceGeometry(estimation_simbox.getx0(), estimation_simbox.gety0(),
                                estimation_simbox.getlx(), estimation_simbox.getly(),
                                estimation_simbox.getAngle(), x_min, y_min, x_max, y_max);

    new_surface = new Surface(x_min-100, y_min-100, x_max-x_min+200, y_max-y_min+200, 2, 2, 0.0, atof(file_name.c_str()));
  }

  return new_surface;
}

// --------------------------------------------------------------------------------
void MultiIntervalGrid::FindSmallestSurfaceGeometry(const double   x0,
                                                    const double   y0,
                                                    const double   lx,
                                                    const double   ly,
                                                    const double   rot,
                                                    double       & x_min,
                                                    double       & y_min,
                                                    double       & x_max,
                                                    double       & y_max) const
{
  x_min = x0 - ly*sin(rot);
  x_max = x0 + lx*cos(rot);
  y_min = y0;
  y_max = y0 + lx*sin(rot) + ly*cos(rot);
  if (rot < 0) {
    x_min = x0;
    x_max = x0 + lx*cos(rot) - ly*sin(rot);
    y_min = y0 + lx*sin(rot);
    y_max = y0 + ly*cos(rot);
  }
}

// --------------------------------------------------------------------------------
void  MultiIntervalGrid::ErodeAllSurfaces(std::vector<Surface>       & eroded_surfaces,
                                          const std::vector<int>     & erosion_priorities,
                                          const std::vector<Surface> & surfaces,
                                          const Simbox               & simbox,
                                          std::string                & err_text) const{
  double    min_product     = 1000000000;
  bool      simbox_covered  = true;
  int       surf_max_res    = 0;
  int       n_surf          = static_cast<int>(eroded_surfaces.size());

  for (int i = 0; i < n_surf; i++) {
    if (!simbox.CheckSurface(surfaces[i])) {
      simbox_covered = false;
      err_text += "Surface " + surfaces[i].GetName() + " does not cover the inversion volume.";
    }
  }

  if (simbox_covered) {
    // Find the surface with the highest resolution
    for (int i = 0; i < n_surf; i++) {
      if (surfaces[i].GetDX()*surfaces[i].GetDY() < min_product) {
        min_product     = surfaces[i].GetDX()*surfaces[i].GetDY();
        surf_max_res    = i;
      }
    }

    for (int i=0; i<n_surf; i++) {
      int l=0;
      //For surface[0] find the priority 1 surface, for surface[1] find the priority 2 surface etc.
      //All priority 1 to i surfaces have been found at this point
      while(i+1 != erosion_priorities[l])
        l++;

      Surface temp_surface = Surface(surfaces[l]);

      //Find closest eroded surface downward
      for (int k=l+1; k<n_surf; k++) {
        if (eroded_surfaces[k].GetN() > 0) {
          ErodeSurface(temp_surface, eroded_surfaces[k], surfaces[surf_max_res], false);
          break;
        }
      }
      //Find closest eroded surface upward
      for (int k=l-1; k>=0; k--) {
        if (eroded_surfaces[k].GetN() > 0) {
          ErodeSurface(temp_surface, eroded_surfaces[k], surfaces[surf_max_res], true);
          break;
        }
      }
      eroded_surfaces[l] = temp_surface;
    }
  }
  else{
    err_text += "Could not process surface erosion since one or more of the surfaces does not cover the inversion area.";
  }
}

// --------------------------------------------------------------------------------
void  MultiIntervalGrid::ErodeSurface(Surface       &  surface,
                                      const Surface &  priority_surface,
                                      const Surface &  resolution_surface,
                                      const bool    &  compare_upward) const{

  int nx    = static_cast<int>(resolution_surface.GetNI());
  int ny    = static_cast<int>(resolution_surface.GetNJ());
  double x0 = resolution_surface.GetXMin();
  double y0 = resolution_surface.GetYMin();
  double lx = resolution_surface.GetLengthX();
  double ly = resolution_surface.GetLengthY();

  NRLib::Grid2D<double> eroded_surface(nx,ny,0);
  double x;
  double y;
  double z;
  double z_priority;

  double missing = surface.GetMissingValue();
  for (int i=0; i<nx; i++) {
    for (int j=0; j<ny; j++) {
      resolution_surface.GetXY(i, j, x, y);

      //simbox.getXYCoord(i,j,x,y);

      z_priority = priority_surface.GetZ(x,y);
      z          = surface.GetZ(x,y);

      if (compare_upward) {
        if (z < z_priority && z != missing)
          eroded_surface(i,j) = z_priority;
        else
          eroded_surface(i,j) = z;
      }

      else {
        if (z > z_priority && z_priority != missing)
          eroded_surface(i,j) = z_priority;
        else
          eroded_surface(i,j) = z;
      }
    }
  }

  std::string name = surface.GetName();
  double angle     = surface.GetAngle();

  surface = Surface(x0, y0, lx, ly, eroded_surface, angle);
  surface.SetName(name);

}

// --------------------------------------------------------------------------------
void MultiIntervalGrid::CreateVisibleSurfaces(const std::vector<Surface> & surfaces,
                                              const std::vector<Surface> & eroded_surfaces,
                                              std::vector<std::vector<Surface> > & visible_surfaces)
{
  for(size_t i=0;i<surfaces.size()-1;i++) {
    visible_surfaces[i].resize(2);
    visible_surfaces[i][0] = eroded_surfaces[i];
    visible_surfaces[i][1] = eroded_surfaces[i+1];
    for(int j=0;j<2;j++) {
      std::vector<double>::const_iterator above = surfaces[i].begin();
      std::vector<double>::const_iterator below = surfaces[i+1].begin();
      std::vector<double>::iterator       surf  = visible_surfaces[i][j].begin();
      for(;surf != visible_surfaces[i][j].end();++surf) {
        if(*surf < *above)
          *surf = *above;
        else if(*surf > *below)
          *surf = *below;

        ++above;
        ++below;
      }
    }
  }
}


// --------------------------------------------------------------------------------
void MultiIntervalGrid::EstimateZPaddingSize(Simbox          * simbox,
                                             ModelSettings   * model_settings) {
  int    nz             = simbox->getnz();
  double min_lz         = simbox->getlz()*simbox->getMinRelThick();
  double z_pad_fac      = model_settings->getZPadFac();
  double z_pad          = z_pad_fac*min_lz;

  if (model_settings->getEstimateZPadding())
  {
    double w_length    = static_cast<double>(model_settings->getDefaultWaveletLength());
    double p_fac      = 1.0;
    z_pad             = w_length/p_fac;                               // Use half a wavelet as padding
    z_pad_fac         = std::min(1.0, z_pad/min_lz);                  // More than 100% padding is not sensible
  }
  int nz_pad          = FindPaddingSize(nz, z_pad_fac);
  z_pad_fac           = static_cast<double>(nz_pad - nz)/static_cast<double>(nz);

  std::string text2;
  int logLevel = LogKit::Medium;
  if (model_settings->getEstimateZPadding()) {
    text2 = " estimated from an assumed wavelet length";
    logLevel = LogKit::Low;
  }

  simbox->SetNZpad(nz_pad);
  simbox->SetZPadFactor(z_pad_fac);

  std::string output_name = "";
  if (simbox->GetIntervalName() != "")
    output_name = " for interval \'" + simbox->GetIntervalName() + "\'";

  LogKit::LogFormatted(logLevel,"\nZ padding sizes" + text2 + output_name + ":\n");
  LogKit::LogFormatted(logLevel,"  zPad, zPadFac, nz, nzPad                 : %5.0fms, %5.3f, %5d, %4d\n",
                       z_pad, z_pad_fac, nz, nz_pad);
}

// --------------------------------------------------------------------------------
int MultiIntervalGrid::FindPaddingSize(int     nx,
                                       double  px) {

  int leastint    = static_cast<int>(ceil(nx*(1.0f+px)));
  int closestprod = FindClosestFactorableNumber(leastint);
  return(closestprod);
}

// --------------------------------------------------------------------------------
int MultiIntervalGrid::FindClosestFactorableNumber(int leastint) {
  int i,j,k,l,m,n;
  int factor   =       1;

  int maxant2    = static_cast<int>(ceil(static_cast<double>(log(static_cast<float>(leastint))) / log(2.0f) ));
  int maxant3    = static_cast<int>(ceil(static_cast<double>(log(static_cast<float>(leastint))) / log(3.0f) ));
  int maxant5    = static_cast<int>(ceil(static_cast<double>(log(static_cast<float>(leastint))) / log(5.0f) ));
  int maxant7    = static_cast<int>(ceil(static_cast<double>(log(static_cast<float>(leastint))) / log(7.0f) ));
  int maxant11   = 0;
  int maxant13   = 0;
  int closestprod= static_cast<int>(pow(2.0f,maxant2));

  /* kan forbedres ved aa trekke fra i endepunktene.i for lokkene*/
  for (i=0;i<maxant2+1;i++)
    for (j=0;j<maxant3+1;j++)
      for (k=0;k<maxant5+1;k++)
        for (l=0;l<maxant7+1;l++)
          for (m=0;m<maxant11+1;m++)
            for (n=maxant11;n<maxant13+1;n++)
            {
              factor = static_cast<int>(pow(2.0f,i)*pow(3.0f,j)*pow(5.0f,k)*
                pow(7.0f,l)*pow(11.0f,m)*pow(13.0f,n));
              if ((factor >=  leastint) &&  (factor <  closestprod))
              {
                closestprod=factor;
              }
            }
            return closestprod;
}

// --------------------------------------------------------------------------------
void  MultiIntervalGrid::LogIntervalInformation(const Simbox      * simbox,
                                                const std::string & interval_name,
                                                const std::string & header_text1,
                                                const std::string & header_text2) const{
  LogKit::LogFormatted(LogKit::Low,"\n"+header_text1+ ":\n");
  double zmin, zmax;
  simbox->getMinMaxZ(zmin,zmax);
  if (interval_name != "")
    LogKit::LogFormatted(LogKit::Low," Interval name: "+ interval_name +"\n");
  LogKit::LogFormatted(LogKit::Low," %13s          avg / min / max    : %7.1f /%7.1f /%7.1f\n",
                       header_text2.c_str(),
                       zmin+simbox->getlz()*simbox->getAvgRelThick()*0.5,
                       zmin,zmax);
  LogKit::LogFormatted(LogKit::Low,"  Interval thickness    avg / min / max    : %7.1f /%7.1f /%7.1f\n",
                       simbox->getlz()*simbox->getAvgRelThick(),
                       simbox->getlz()*simbox->getMinRelThick(),
                       simbox->getlz());
  LogKit::LogFormatted(LogKit::Low,"  Sampling density      avg / min / max    : %7.2f /%7.2f /%7.2f\n",
                       simbox->getdz()*simbox->getAvgRelThick(),
                       simbox->getdz(),
                       simbox->getdz()*simbox->getMinRelThick());
}

void MultiIntervalGrid::LogIntervalInformation(const Simbox      * simbox,
                                               const std::string & header_text1,
                                               const std::string & header_text2) const{
  LogKit::LogFormatted(LogKit::Low,"\n"+header_text1+"\n");
  double zmin, zmax;
  simbox->getMinMaxZ(zmin,zmax);
  LogKit::LogFormatted(LogKit::Low," %13s          avg / min / max    : %7.1f /%7.1f /%7.1f\n",
                       header_text2.c_str(),
                       zmin+simbox->getlz()*simbox->getAvgRelThick()*0.5,
                       zmin,zmax);
  LogKit::LogFormatted(LogKit::Low,"  Interval thickness    avg / min / max    : %7.1f /%7.1f /%7.1f\n",
                       simbox->getlz()*simbox->getAvgRelThick(),
                       simbox->getlz()*simbox->getMinRelThick(),
                       simbox->getlz());
  LogKit::LogFormatted(LogKit::Low,"  Sampling density      avg / min / max    : %7.2f /%7.2f /%7.2f\n",
                       simbox->getdz()*simbox->getAvgRelThick(),
                       simbox->getdz(),
                       simbox->getdz()*simbox->getMinRelThick());
}

// --------------------------------------------------------------------------------
double  MultiIntervalGrid::FindResolution(const Surface * top_surface,
                                          const Surface * base_surface,
                                          const Simbox  * estimation_simbox,
                                          int             n_layers,
                                          double          ext_dz) const{
  int nx = static_cast<int>(top_surface->GetNI());
  int ny = static_cast<int>(base_surface->GetNJ());

  double maximum_dz = 0;
  double resolution = 0;

  if (n_layers < 0) {
    return(ext_dz);
  }
  else {
    for (int i = 0; i < nx; i++) {
      for (int j = 0; j < ny; j++) {
        double x,y;
        estimation_simbox->getXYCoord(i,j,x,y);
        double z_top  = top_surface->GetZ(x,y);
        double z_base = base_surface->GetZ(x,y);
        if (z_top != WELLMISSING && z_base != WELLMISSING) {
          resolution    = (z_base - z_top) / static_cast<double>(n_layers);
          if (resolution > maximum_dz) {
            maximum_dz = resolution;
          }
        }
      }
    }
  }

  return maximum_dz;
}

void
MultiIntervalGrid::FindZoneProbGrid(std::vector<StormContGrid> & zone_prob_grid)
{
  size_t n_surf = surfaces_.size();
  std::vector<NRLib::Beta> uc_dist(n_surf-2); //Top and base are certain. Shifted 1 compared to surf.

  for(size_t i=0;i<n_surf-2;i++) {
    uc_dist[i] = NRLib::Beta(-uncertainties_[i+1],uncertainties_[i+1],2,2);
  }

  std::vector<double> prob_zone(n_surf-1); //All zones
  std::vector<double> rel_z(n_surf-1);     //Dummy for first surface, lacking for last.

  for(size_t i=0; i<zone_prob_grid[0].GetNI();i++) {
    for(size_t j=0; j<zone_prob_grid[0].GetNJ();j++) {
      for(size_t k=0; k<zone_prob_grid[0].GetNK();k++) {
        double x,y,z;
        zone_prob_grid[0].FindCenterOfCell(i, j, k, x, y, z);
        for(size_t s=1;s<n_surf-1;s++)
          rel_z[s] = z-surfaces_[s].GetZ(x,y);
        ComputeZoneProbability(rel_z, uc_dist, erosion_priorities_, prob_zone);
        for(size_t s=0;s<prob_zone.size();s++)
          zone_prob_grid[s](i,j,k) = static_cast<float>(prob_zone[s]);
      }
    }
  }
}

void
MultiIntervalGrid::ComputeZoneProbability(const std::vector<double>      & z,
                                          const std::vector<NRLib::Beta> & horizon_distributions,
                                          const std::vector<int>         & erosion_priority,
                                          std::vector<double>            & zone_probability) const
{

  int n_zones = static_cast<int>(zone_probability.size());

  std::vector<double> horizon_cdf(n_zones+1, 0);
  horizon_cdf[0]       = 1;      //The upper surface has cdf 1, whereas the lower surface has cdf 0
  horizon_cdf[n_zones] = 0;

  for(int i=1; i<n_zones; i++)
    horizon_cdf[i] = horizon_distributions[i-1].Cdf(z[i]);

  for(int zone=0; zone<n_zones; zone++) {
    //Initialize with probability that we are below top surface for zone
    double prob = horizon_cdf[zone];

    //Multiply with probability that we are above base surface for zone
    prob *= (1-horizon_cdf[zone+1]);

    //We may be eroded from above. Must consider the surfaces that
    //1. Are above top in the standard sequence.
    //2. Have lower erosion priority number than the top.
    //3. Have no horizons with lower erosion priority number between it and top.
    int min_erosion = erosion_priority[zone];
    for(int prev_hor = zone-1; prev_hor >=0; prev_hor--) {
      if(erosion_priority[prev_hor] < min_erosion) {
        prob        *= horizon_cdf[prev_hor];
        min_erosion  = erosion_priority[prev_hor]; //Those with higher number stop in this
      }
    }

    //We may be eroded from below. Must consider the surfaces that
    //1. Are below base in the standard sequence.
    //2. Have lower erosion priority number than the base.
    //3. Have no horizons with lower erosion priority number between it and base.
    min_erosion = erosion_priority[zone+1];
    for(int late_hor = zone+2; late_hor < n_zones+1; late_hor++) {
      if(erosion_priority[late_hor] < min_erosion) {
        prob        *= (1-horizon_cdf[late_hor]);
        min_erosion  = erosion_priority[late_hor]; //Those with higher number stop in this
      }
    }

    zone_probability[zone] = prob;
  }
}

void MultiIntervalGrid::RemoveNaNFromSurface(Surface *& surface)
{
  int nx    = static_cast<int>(surface->GetNI());
  int ny    = static_cast<int>(surface->GetNJ());
  double z  = 0.0;
  int n_nan = 0;

  double missing = surface->GetMissingValue();
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {

      z = (*surface)(i,j);

      if (z != z) {
        (*surface)(i,j) = missing;
        n_nan++;
      }

    }
  }

  std::string name = surface->GetName();
  if (n_nan > 0) {
    LogKit::LogFormatted(LogKit::Low,"Found " + NRLib::ToString(n_nan) + " undefined values in" + name + ". These are set as missing.\n");
  }

}

void MultiIntervalGrid::InterpolateMissing(Surface *& surface)
{
  //Interpolate single missing cells
  int ni = static_cast<int>(surface->GetNI());
  int nj = static_cast<int>(surface->GetNJ());
  double missing = surface->GetMissingValue();

  Surface * new_surface = new Surface(*surface);
  double old_value  = 0.0;
  int n_interpolate = 0;

  for (int i = 0; i < ni; i++) {
    for (int j = 0; j < nj; j++) {

      old_value = (*surface)(i,j);
      double new_value = missing;

      if (old_value == missing) {

        //Require all four neighbour non-missing, unless we are at the edge of the surface
        //H-TODO should interpolate from less than 4 corner, but still allow missing values outside the surface,
        // setting n_req to 1 will interpolate one cell along the edge of the surface
        int n_req = 4;
        if (i == 0 || i == ni-1)
          n_req--;
        if (j == 0 || j == nj-1)
          n_req--;

        int n = 0;
        if (i > 0 && (*surface)(i-1, j) != missing) {
          new_value += (*surface)(i-1, j);
          n++;
        }
        if (j < nj-1 && (*surface)(i, j+1) != missing) {
          new_value += (*surface)(i, j+1);
          n++;
        }
        if (j > 0 && (*surface)(i, j-1)  != missing) {
          new_value += (*surface)(i, j-1);
          n++;
        }
        if (i < ni-1 && (*surface)(i+1, j) != missing) {
          new_value += (*surface)(i+1, j);
          n++;
        }

        if (n == n_req) {
          new_value /= n;
          n_interpolate++;
        }
        else
          new_value = missing;

        (*new_surface)(i,j) = new_value;

      }
    }
  }

  std::string name = surface->GetName();
  if (n_interpolate > 0) {
    LogKit::LogFormatted(LogKit::Low,"Interpolated " + NRLib::ToString(n_interpolate) + " cells in " + name + ".\n");
  }

  delete surface;
  surface = new_surface;

}

std::vector<int>
MultiIntervalGrid::FindILXLArea(const ModelSettings * model_settings,
                                const InputFiles    * input_files,
                                SegyGeometry        * segy_geometry)
{
  std::vector<int> ilxl_area;

  if (model_settings->getAreaILXL().size() > 0)
    ilxl_area = model_settings->getAreaILXL();
  else if (model_settings->getSnapGridToSeismicData()) {

    SegyGeometry * template_geometry = NULL;
    if (model_settings->getAreaSpecification() == ModelSettings::AREA_FROM_GRID_DATA_AND_UTM) {
      template_geometry = model_settings->getAreaParameters();
    }
    else if (model_settings->getAreaSpecification() == ModelSettings::AREA_FROM_GRID_DATA_AND_SURFACE) {
      Surface surf(input_files->getAreaSurfaceFile());
      template_geometry = new SegyGeometry(surf);
    }

    ilxl_area = segy_geometry->findAreaILXL(template_geometry);
  }
  else
    ilxl_area = segy_geometry->findAreaILXL(segy_geometry);

  return ilxl_area;
}
