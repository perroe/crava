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
#include "nrlib/iotools/stringtools.hpp"
#include "nrlib/surface/surface.hpp"
//#include "src/definitions.h"

using namespace NRLib;

Well::Well()
{
  well_rmissing_ = -999.0;
  well_imissing_ = -999;
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
    std::string name = NRLib::RemovePath(file_name);
    name = NRLib::ReplaceExtension(name, "");
    well_name_ = name;
    cont_log_  = well.GetContLog();
    n_data_    = well.GetNData();
    disc_log_  = well.GetDiscLog();
    read_ok    = true;

    x_pos0_ = well.GetXPosOrigin()*1000;
    y_pos0_ = well.GetYPosOrigin()*1000;

    //Norsar wells has its own missing value in the well-log
    SetMissing(well.GetContMissing());

    // assume that facies logs from norsar wells are not used
    has_facies_log_ = false;
  }
  else if(file_name.find(".rms",0) != std::string::npos || file_name.find(".txt",0) != std::string::npos) {
    NRLib::RMSWell well(file_name);
    well_name_ = well.GetWellName();
    n_data_    = well.GetNData();
    cont_log_  = well.GetContLog();
    disc_log_  = well.GetDiscLog();
    x_pos0_    = well.GetXPos0();
    y_pos0_    = well.GetYPos0();

    std::map<int, std::string> facies_map;
    if(well.HasDiscLog(facies_log))
      facies_map = well.GetDiscNames(facies_log);

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

void
Well::AddContLogSeismicResolution(const std::string& name, const std::vector<double>& log)
{
  cont_log_seismic_resolution_[name] = log;
}

void
Well::AddContLogBackgroundResolution(const std::string& name, const std::vector<double>& log)
{
  cont_log_background_resolution_[name] = log;
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

std::vector<double>&
Well::GetContLogSeismicResolution(const std::string& name)
{
  std::map<std::string, std::vector<double> >::iterator item = cont_log_seismic_resolution_.find(name);
  assert(item != cont_log_.end());
  return item->second;
}

std::vector<double>&
Well::GetContLogBackgroundResolution(const std::string& name)
{
  std::map<std::string, std::vector<double> >::iterator item = cont_log_background_resolution_.find(name);
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

const std::vector<double>&
Well::GetContLogSeismicResolution(const std::string& name) const
{
  std::map<std::string, std::vector<double> >::const_iterator item = cont_log_seismic_resolution_.find(name);
  assert(item != cont_log_.end());
  return item->second;
}

const std::vector<double>&
Well::GetContLogBackgroundResolution(const std::string& name) const
{
  std::map<std::string, std::vector<double> >::const_iterator item = cont_log_background_resolution_.find(name);
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
