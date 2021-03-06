#ifndef RPLIB_DISTRIBUTIONS_DRY_ROCK_STORAGE_HPP
#define RPLIB_DISTRIBUTIONS_DRY_ROCK_STORAGE_HPP

#include "rplib/distributionsdryrock.h"
#include "rplib/distributionwithtrendstorage.h"
#include "nrlib/trend/trendstorage.hpp"
#include "nrlib/trend/trend.hpp"

#include <map>

class DistributionsSolidStorage;

class DistributionsRockStorage;
class DistributionsFluidStorage;

class DistributionsDryRockStorage {
public:
  DistributionsDryRockStorage();

  virtual ~DistributionsDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & /*n_vintages*/,
                                                                           const std::string                                         & /*path*/,
                                                                           const std::vector<std::string>                            & /*trend_cube_parameters*/,
                                                                           const std::vector<std::vector<double> >                   & /*trend_cube_sampling*/,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& /*model_dryrock_storage*/,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & /*model_solid_storage*/,
                                                                           std::string                                               & /*errTxt*/)                    const = 0;

protected:
  std::vector<DistributionsDryRock *>         CreateDistributionsDryRockMix(const int                                                       & n_vintages,
                                                                            const std::string                                               & path,
                                                                            const std::vector<std::string>                                  & trend_cube_parameters,
                                                                            const std::vector<std::vector<double> >                         & trend_cube_sampling,
                                                                            const std::map<std::string, DistributionsDryRockStorage *>      & model_dryrock_storage,
                                                                            const std::map<std::string, DistributionsSolidStorage *>        & model_solid_storage,
                                                                            const std::vector<std::string>                                  & constituent_label,
                                                                            const std::vector<std::vector<DistributionWithTrendStorage *> > & constituent_volume_fraction,
                                                                            DEMTools::MixMethod                                               mix_method,
                                                                            std::string                                                     & errTxt) const;
};

//----------------------------------------------------------------------------------//

class TabulatedVelocityDryRockStorage : public DistributionsDryRockStorage {
public:
  TabulatedVelocityDryRockStorage(std::vector<DistributionWithTrendStorage *> vp,
                                  std::vector<DistributionWithTrendStorage *> vs,
                                  std::vector<DistributionWithTrendStorage *> density,
                                  std::vector<DistributionWithTrendStorage *> correlation_vp_vs,
                                  std::vector<DistributionWithTrendStorage *> correlation_vp_density,
                                  std::vector<DistributionWithTrendStorage *> correlation_vs_density,
                                  std::vector<DistributionWithTrendStorage *> total_porosity,
                                  std::vector<DistributionWithTrendStorage *> mineral_k);

  virtual ~TabulatedVelocityDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& /*model_dryrock_storage*/,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & /*model_solid_storage*/,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::vector<DistributionWithTrendStorage *> vp_;
  std::vector<DistributionWithTrendStorage *> vs_;
  std::vector<DistributionWithTrendStorage *> density_;
  std::vector<DistributionWithTrendStorage *> correlation_vp_vs_;        /// Converted to double
  std::vector<DistributionWithTrendStorage *> correlation_vp_density_;   /// Converted to double
  std::vector<DistributionWithTrendStorage *> correlation_vs_density_;   /// Converted to double
  std::vector<DistributionWithTrendStorage *> total_porosity_;
  std::vector<DistributionWithTrendStorage *> mineral_k_;
};

//----------------------------------------------------------------------------------//
class TabulatedModulusDryRockStorage : public DistributionsDryRockStorage {
public:
  TabulatedModulusDryRockStorage(std::vector<DistributionWithTrendStorage *> bulk_modulus,
                                 std::vector<DistributionWithTrendStorage *> shear_modulus,
                                 std::vector<DistributionWithTrendStorage *> density,
                                 std::vector<DistributionWithTrendStorage *> correlation_bulk_shear,
                                 std::vector<DistributionWithTrendStorage *> correlation_bulk_density,
                                 std::vector<DistributionWithTrendStorage *> correlation_shear_density,
                                 std::vector<DistributionWithTrendStorage *> total_porosity,
                                 std::vector<DistributionWithTrendStorage *> mineral_k);

  virtual ~TabulatedModulusDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& /*model_dryrock_storage*/,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & /*model_solid_storage*/,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::vector<DistributionWithTrendStorage *> bulk_modulus_;
  std::vector<DistributionWithTrendStorage *> shear_modulus_;
  std::vector<DistributionWithTrendStorage *> density_;
  std::vector<DistributionWithTrendStorage *> correlation_bulk_shear_;      /// Converted to double
  std::vector<DistributionWithTrendStorage *> correlation_bulk_density_;    /// Converted to double
  std::vector<DistributionWithTrendStorage *> correlation_shear_density_;   /// Converted to double
  std::vector<DistributionWithTrendStorage *> total_porosity_;
  std::vector<DistributionWithTrendStorage *> mineral_k_;
};

//----------------------------------------------------------------------------------//
class ReussDryRockStorage : public DistributionsDryRockStorage {
public:
  ReussDryRockStorage(std::vector<std::string>                                  constituent_label,
                      std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction);

  virtual ~ReussDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& model_dryrock_storage,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::vector<std::string>                                    constituent_label_;
  std::vector<std::vector<DistributionWithTrendStorage *> >   constituent_volume_fraction_;
};

//----------------------------------------------------------------------------------//
class VoigtDryRockStorage : public DistributionsDryRockStorage {
public:
  VoigtDryRockStorage(std::vector<std::string>                                  constituent_label,
                      std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction);

  virtual ~VoigtDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& model_dryrock_storage,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::vector<std::string>                                    constituent_label_;
  std::vector<std::vector<DistributionWithTrendStorage *> >   constituent_volume_fraction_;
};

//----------------------------------------------------------------------------------//
class HillDryRockStorage : public DistributionsDryRockStorage {
public:
  HillDryRockStorage(std::vector<std::string>                                  constituent_label,
                     std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction);

  virtual ~HillDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& model_dryrock_storage,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::vector<std::string >                                   constituent_label_;
  std::vector<std::vector<DistributionWithTrendStorage *> >   constituent_volume_fraction_;
};

//----------------------------------------------------------------------------------//

class DEMDryRockStorage : public DistributionsDryRockStorage {
public:
  DEMDryRockStorage(std::string                                               host_label,
                    std::vector<DistributionWithTrendStorage *>               host_volume_fraction,
                    std::vector<std::string>                                  inclusion_label,
                    std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_volume_fraction,
                    std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_aspect_ratio);

  virtual ~DEMDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& model_dryrock_storage,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                                                           std::string                                               & errTxt)                    const;

private:
  std::string                                               host_label_;
  std::vector<DistributionWithTrendStorage *>               host_volume_fraction_;
  std::vector<std::string>                                  inclusion_label_;
  std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_volume_fraction_;
  std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_aspect_ratio_;
};

//--------------------------------------------------------------------------------------------------------------------------------------//
class WaltonDryRockStorage : public DistributionsDryRockStorage {
public:
  WaltonDryRockStorage(const std::string                                                & solid_label,
                       std::vector<DistributionWithTrendStorage *>                      & distr_friction_weight,
                       std::vector<DistributionWithTrendStorage *>                      & distr_pressure,
                       std::vector<DistributionWithTrendStorage *>                      & distr_porosity,
                       std::vector<DistributionWithTrendStorage *>                      & distr_coord_number);

  virtual ~WaltonDryRockStorage();

  virtual std::vector<DistributionsDryRock *> GenerateDistributionsDryRock(const int                                                 & n_vintages,
                                                                           const std::string                                         & path,
                                                                           const std::vector<std::string>                            & trend_cube_parameters,
                                                                           const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                                           const std::map<std::string, DistributionsDryRockStorage *>& /*model_dryrock_storage*/,
                                                                           const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                                                           std::string                                               & errTxt) const;

private:
  std::string                                   solid_label_;
  std::vector<DistributionWithTrendStorage *>   distr_friction_weight_;
  std::vector<DistributionWithTrendStorage *>   distr_pressure_;
  std::vector<DistributionWithTrendStorage *>   distr_porosity_;
  std::vector<DistributionWithTrendStorage *>   distr_coord_number_;

};

#endif
