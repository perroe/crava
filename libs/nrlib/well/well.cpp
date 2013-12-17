// $Id: well.cpp 882 2011-09-23 13:10:16Z perroe $

// Copyright (c)  2011, Norwegian Computing Center
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// �  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// �  Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cassert>
#include <fstream>
#include <string>

#include "well.hpp"
#include "norsarwell.hpp"
#include "rmswell.hpp"

#include "nrlib/iotools/logkit.hpp"
//#include "nrlib/stormgrid/stormcontgrid.hpp"

#include "nrlib/surface/surface.hpp"

using namespace NRLib;

Well::Well()
{
  well_rmissing_ = -999.0;
  well_imissing_  = -999;
}


Well::Well(const std::string & name,
           double              rmissing,
           int                 imissing)
{
  well_name_ = name;
  well_rmissing_ = rmissing;
  well_imissing_ = imissing;
}


Well::Well(const std::string & file_name,
           bool              & read_ok,
           const std::string & facies_log)
{
  ReadWell(file_name, read_ok, facies_log);
}


Well::Well(const std::map<std::string,std::vector<double> > & cont_log,
           const std::map<std::string,std::vector<int> >    & disc_log,
           const std::string                                & well_name)
{
  cont_log_       = cont_log;
  disc_log_       = disc_log;
  well_name_      = well_name;
  well_rmissing_  = -999.0;
  well_imissing_  = -999;
}

Well::~Well(){

}


void
Well::ReadWell(const std::string              & file_name,
               bool                           & read_ok,
               const std::string              & facies_log)
{
  if(file_name.find(".nwh",0) != std::string::npos) {
    NRLib::NorsarWell well(file_name);
    well_name_ = well.GetWellName();
    cont_log_ = well.GetContLog();
    n_data_ = well.GetNData();
    disc_log_ = well.GetDiscLog();
    read_ok = true;
    // assume that facies logs from norsar wells are not used
    has_facies_log_ = false;
  }
  else if(file_name.find(".rms",0) != std::string::npos) {
    NRLib::RMSWell well(file_name);
    well_name_ = well.GetWellName();
    n_data_ = well.GetNData();
    cont_log_ = well.GetContLog();
    disc_log_ = well.GetDiscLog();

    std::map<int, std::string> facies_map;
    if(well.HasDiscLog(facies_log))
      facies_map = well.GetDiscNames(facies_log);
    //if(well.HasDiscLog("FACIES"))
    //  facies_map = well.GetDiscNames("FACIES");

    if(facies_map.size() > 0){
      has_facies_log_ = true;
      facies_map_ = facies_map;
    }
    else{
      has_facies_log_ = false;
    }
    read_ok = true;
  }
  else
    read_ok = false;
}


void
Well::AddContLog(const std::string& name, const std::vector<double>& log)
{
  cont_log_[name] = log;
}


bool Well::HasDiscLog(const std::string& name) const{
  std::map<std::string, std::vector<int> >::const_iterator it = disc_log_.find(name);
  if(it != disc_log_.end()){
    return true;
  }
  else{
    return false;
  }
}

bool
Well::HasContLog(const std::string& name) const
{
  //std::map<std::string, std::vector<double> >::const_iterator item = cont_log_.find(name);
  if(cont_log_.find(name) != cont_log_.end())
    return true;
  else
    return false;
}


std::vector<double>&
Well::GetContLog(const std::string& name)
{
  std::map<std::string, std::vector<double> >::iterator item = cont_log_.find(name);
  assert(item != cont_log_.end());
  return item->second;
}


const std::vector<double>&
Well::GetContLog(const std::string& name) const
{
  std::map<std::string, std::vector<double> >::const_iterator item = cont_log_.find(name);
  assert(item != cont_log_.end());
  return item->second;
}


void
Well::RemoveContLog(const std::string& name)
{
  cont_log_.erase(name);
}


void
Well::AddDiscLog(const std::string& name, const std::vector<int>& log)
{
  disc_log_[name] = log;
}


std::vector<int> &
Well::GetDiscLog(const std::string& name)
{
  std::map<std::string, std::vector<int> >::iterator item = disc_log_.find(name);
  assert(item != disc_log_.end());
  return item->second;
}


const std::vector<int> &
Well::GetDiscLog(const std::string& name) const
{
  std::map<std::string, std::vector<int> >::const_iterator item = disc_log_.find(name);
  assert(item != disc_log_.end());
  return item->second;
}


void
Well::RemoveDiscLog(const std::string& name)
{
  disc_log_.erase(name);
}


void Well::SetWellName(const std::string& wellname)
{
  well_name_ = wellname;
}


bool Well::IsMissing(double x)const
{
  if (x == well_rmissing_)
    return true;
  else
    return false;
}

bool Well::IsMissing(int n)const
{
  if (n == well_imissing_)
    return true;
  else
    return false;
}


int Well::GetDiscValue(size_t index, const std::string& logname) const
{
  std::map<std::string, std::vector<int> >::const_iterator item = disc_log_.find(logname);
  assert(item != disc_log_.end());
  const std::vector<int>& log = item->second;
  assert(index < log.size());
  return log[index];
}


double Well::GetContValue(size_t index, const std::string& logname) const
{
  std::map<std::string,std::vector<double> >::const_iterator item = cont_log_.find(logname);
  assert(item != cont_log_.end());
  const std::vector<double>& log = item->second;
  assert(index < log.size());
  return log[index];
}


void Well::SetDiscValue(int value, size_t index, const std::string& logname)
{
  std::map<std::string,std::vector<int> >::iterator item = disc_log_.find(logname);
  assert(item != disc_log_.end());
  assert(index < item->second.size());
  (item->second)[index] = value;
}


void Well::SetContValue(double value, size_t index, const std::string& logname)
{
  std::map<std::string,std::vector<double> >::iterator item = cont_log_.find(logname);
  assert(item != cont_log_.end());
  assert(index < item->second.size());
  (item->second)[index] = value;
}


size_t Well::GetNlog() const
{
  return cont_log_.size() + disc_log_.size();
}


size_t Well::GetNContLog() const
{
  return cont_log_.size();
}


size_t Well::GetContLogLength(const std::string& logname) const
{
  std::map<std::string,std::vector<double> >::const_iterator item = cont_log_.find(logname);
  assert(item != cont_log_.end());

  return (item->second).size();
}

int Well::CheckStormgrid(StormContGrid & stormgrid) const
{
  bool insideArea = false;
  int  error      = 1;

  //if(timemissing_ == 0) { //Timemissing not stored when reading logs in commondata ReadWell?

    NRLib::Surface<double> & top  = stormgrid.GetTopSurface();
    NRLib::Surface<double> & base = stormgrid.GetBotSurface();

    std::vector<double> x_pos = GetContLog().find("X_pos")->second; //H Works for both RMS and Norsar wells? Use blocked logs?
    std::vector<double> y_pos = GetContLog().find("Y_pos")->second;
    std::vector<double> z_pos = GetContLog().find("TVD")->second;

    for(unsigned int i=0; i < n_data_nonmissing_; i++) { //nd_

      if(stormgrid.IsInside(x_pos[i], y_pos[i])) {
        insideArea = true;
        //
        // Correct handling of top and base checking.
        //
        double z_top  = top.GetZ(x_pos[i], y_pos[i]);
        double z_base = base.GetZ(x_pos[i], y_pos[i]);

        if(z_pos[i] > z_top && z_pos[i] < z_base) {
          error = 0;
          break;
        }
      }
    }
  //}
  //else
  //  error = 0;

  if (error) {
    if (insideArea) {
      LogKit::LogFormatted(LogKit::Low," \nWell "+well_name_+": ");
      LogKit::LogFormatted(LogKit::Low,"IGNORED. Well does not hit the inversion volume.\n");
    }
    else {
      LogKit::LogFormatted(LogKit::Low," \nWell "+well_name_+": ");
      LogKit::LogFormatted(LogKit::Low,"IGNORED. Well is not inside the inversion area.\n");
    }
  }
  return(error);
}
