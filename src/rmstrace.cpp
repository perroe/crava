/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#include "src/rmstrace.h"

RMSTrace::RMSTrace(int                 & IL,
                   int                 & XL,
                   int                 & i_index,
                   int                 & j_index,
                   double              & utmx,
                   double              & utmy,
                   std::vector<double> & time,
                   std::vector<double> & velocity)
: IL_(IL),
  XL_(XL),
  i_index_(i_index),
  j_index_(j_index),
  utmx_(utmx),
  utmy_(utmy),
  velocity_(velocity)
{
  int s_to_ms = 1000;

  time_.resize(time.size());
  for(size_t i=0; i<time.size(); i++)
    time_[i] = time[i] * s_to_ms;

}

RMSTrace::RMSTrace()
{
}

RMSTrace::~RMSTrace()
{
}
