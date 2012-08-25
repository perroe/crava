#include "rplib/distributionsrockinclusion.h"

#include "rplib/distributionwithtrend.h"

#include "nrlib/statistics/statistics.hpp"

DistributionsRockInclusion::DistributionsRockInclusion(DistributionsSolid                           * distr_solid,
                                                       DistributionsFluid                           * distr_fluid,
                                                       std::vector< DistributionWithTrend * >       & distr_incl_spectrum,
                                                       std::vector< DistributionWithTrend * >       & distr_incl_concentration,
                                                       DistributionWithTrend                        * distr_porosity,
                                                       DistributionsRockInclusionEvolution          * distr_evolution)
: DistributionsRock()
{
  assert( distr_incl_spectrum.size() == distr_incl_concentration.size() );

  distr_solid_              = distr_solid;
  distr_fluid_              = distr_fluid;
  distr_incl_spectrum_      = distr_incl_spectrum;
  distr_incl_concentration_ = distr_incl_concentration;
  distr_porosity_           = distr_porosity;
  distr_evolution_          = distr_evolution;

  SampleVpVsRhoExpectationAndCovariance(expectation_, covariance_);
}

DistributionsRockInclusion::~DistributionsRockInclusion(){}

Rock *
DistributionsRockInclusion::GenerateSample(const std::vector<double> & trend_params) const
{
  Solid * solid = distr_solid_->GenerateSample(trend_params);
  Fluid * fluid = distr_fluid_->GenerateSample(trend_params);
  size_t n_incl = distr_incl_spectrum_.size();
  std::vector<double> inclusion_spectrum(n_incl);
  std::vector<double> inclusion_concentration(n_incl);

  for (size_t i = 0; i < n_incl; ++i) {
    inclusion_spectrum[i] = distr_incl_spectrum_[i]->ReSample(trend_params[0], trend_params[1]);
    inclusion_concentration[i] = distr_incl_concentration_[i]->ReSample(trend_params[0], trend_params[1]);
  }
  double porosity = distr_porosity_->ReSample(trend_params[0], trend_params[1]);
  Rock * new_rock = new RockInclusion(solid, fluid, inclusion_spectrum, inclusion_concentration, porosity, distr_evolution_);

  // Deep copy taken by constructor of RockInclusion, hence delete
  // solid and fluid here:
  delete solid;
  delete fluid;

  return new_rock;
}

std::vector<double>
DistributionsRockInclusion::GetExpectation(const std::vector<double> & /*trend_params*/) const
{
  return(expectation_);
}

NRLib::Grid2D<double>
DistributionsRockInclusion::GetCovariance(const std::vector<double> & /*trend_params*/)  const
{
  return(covariance_);
}

Pdf3D *
DistributionsRockInclusion::GeneratePdf(void) const
{
  Pdf3D * pdf3D = NULL;
  return pdf3D;
}

bool
DistributionsRockInclusion::HasDistribution() const
{
  bool dummy = false;
  return(dummy);
}

std::vector<bool>
DistributionsRockInclusion::HasTrend() const
{
  std::vector<bool> dummy(2);

  for(int i=0; i<2; i++)
    dummy[i] = false;

  return(dummy);
}

void
DistributionsRockInclusion::SampleVpVsRhoExpectationAndCovariance(std::vector<double>   & expectation,
                                                                  NRLib::Grid2D<double> & covariance)
{
  int nsamples = 100;

  std::vector< NRLib::Vector > m(3);
  NRLib::Vector vp(nsamples);
  NRLib::Vector vs(nsamples);
  NRLib::Vector rho(nsamples);

  std::vector<double> dummy(2, 0.0);
  for (int i = 0; i < nsamples; ++i) {
    Rock * rock = GenerateSample(dummy);
    rock->ComputeSeismicParams(vp(i), vs(i), rho(i));
    delete rock;
  }
  m[0] = vp;
  m[1] = vs;
  m[2] = rho;

  expectation.resize(3, 0.0);
  covariance.Resize(3,3,0.0);
  for (int i = 0; i < 3; ++i) {
    expectation[i] = NRLib::Mean(m[i]);
    for (int j = 0; j < 3; ++j)
      covariance(i,j) = NRLib::Cov(m[i], m[j]);
  }
}
