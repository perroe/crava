#include "nrlib/trend/trendstorage.hpp"
#include "nrlib/trend/trend.hpp"
#include "nrlib/grid/grid2d.hpp"

#include "rplib/rockphysicsstorage.h"
#include "rplib/distributionsrockt0.h"
#include "rplib/trinormalwith2dtrend.h"
#include "rplib/multinormaldistributedrockt0.h"

RockPhysicsStorage::RockPhysicsStorage()
{
}

RockPhysicsStorage::~RockPhysicsStorage()
{
}

GaussianRockPhysicsStorage::GaussianRockPhysicsStorage(NRLib::TrendStorage *mean_vp,
                                                       NRLib::TrendStorage *mean_vs,
                                                       NRLib::TrendStorage *mean_density,
                                                       NRLib::TrendStorage *variance_vp,
                                                       NRLib::TrendStorage *variance_vs,
                                                       NRLib::TrendStorage *variance_density,
                                                       NRLib::TrendStorage *correlation_vp_vs,
                                                       NRLib::TrendStorage *correlation_vp_density,
                                                       NRLib::TrendStorage *correlation_vs_density)
: mean_vp_(mean_vp),
  mean_vs_(mean_vs),
  mean_density_(mean_density),
  variance_vp_(variance_vp),
  variance_vs_(variance_vs),
  variance_density_(variance_density),
  correlation_vp_vs_(correlation_vp_vs),
  correlation_vp_density_(correlation_vp_density),
  correlation_vs_density_(correlation_vs_density)
{
}

GaussianRockPhysicsStorage::~GaussianRockPhysicsStorage()
{
}

DistributionsRockT0 *
GaussianRockPhysicsStorage::GenerateRockPhysics(const std::string & path,
                                                std::string       & errTxt) const
{
  NRLib::Trend * mean_vp_trend                = mean_vp_               ->GenerateTrend(path,errTxt);
  NRLib::Trend * mean_vs_trend                = mean_vs_               ->GenerateTrend(path,errTxt);
  NRLib::Trend * mean_density_trend           = mean_density_          ->GenerateTrend(path,errTxt);
  NRLib::Trend * variance_vp_trend            = variance_vp_           ->GenerateTrend(path,errTxt);
  NRLib::Trend * variance_vs_trend            = variance_vs_           ->GenerateTrend(path,errTxt);
  NRLib::Trend * variance_density_trend       = variance_density_      ->GenerateTrend(path,errTxt);
  NRLib::Trend * correlation_vp_vs_trend      = correlation_vp_vs_     ->GenerateTrend(path,errTxt);
  NRLib::Trend * correlation_vp_density_trend = correlation_vp_density_->GenerateTrend(path,errTxt);
  NRLib::Trend * correlation_vs_density_trend = correlation_vs_density_->GenerateTrend(path,errTxt);

  //Marit: Transponer variablene her
  //Marit: Resample trendverdiene til de som trengs, vha global min/max
  //Marit: Sjekk at referanseparametre og kuber har samme navn

  TriNormalWith2DTrend multi(mean_vp_trend,
                             mean_vs_trend,
                             mean_density_trend,
                             variance_vp_trend,
                             variance_vs_trend,
                             variance_density_trend,
                             correlation_vp_vs_trend,
                             correlation_vp_density_trend,
                             correlation_vs_density_trend);

  DistributionsRockT0 * rock = new MultiNormalDistributedRockT0(multi);

  return(rock);
}
