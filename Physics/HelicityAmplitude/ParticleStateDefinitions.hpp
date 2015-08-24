//-------------------------------------------------------------------------------
// Copyright (c) 2013 Stefan Pflueger.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//   Stefan Pflueger - initial API and implementation
//-------------------------------------------------------------------------------

#ifndef PHYSICS_HELICITYAMPLITUDE_PARTICLESTATEDEFINITIONS_HPP_
#define PHYSICS_HELICITYAMPLITUDE_PARTICLESTATEDEFINITIONS_HPP_

#include <set>

#include "boost/property_tree/ptree.hpp"

#include "qft++.h"

namespace HelicityFormalism {

typedef boost::property_tree::ptree DynamicalInfo;

struct HelicityAngles {
  double theta_;
  double phi_;

  HelicityAngles(double theta, double phi) :
      theta_(theta), phi_(phi) {
  }
};

struct SpinInfo {
  Spin J_;
  Spin M_;

  bool operator==(const SpinInfo &rhs) const {
    if (this->J_.Numerator() != rhs.J_.Numerator())
      return false;
    if (this->J_.Denominator() != rhs.J_.Denominator())
      return false;
    if (this->M_.Numerator() != rhs.M_.Numerator())
      return false;
    if (this->M_.Denominator() != rhs.M_.Denominator())
      return false;

    return true;
  }
  bool operator!=(const SpinInfo &rhs) const {
    return !(*this == rhs);
  }

  bool operator<(const SpinInfo &rhs) const {
    if (this->J_.Numerator() > rhs.J_.Numerator())
      return false;
    else if (this->J_.Numerator() < rhs.J_.Numerator())
      return true;
    if (this->J_.Denominator() > rhs.J_.Denominator())
      return false;
    else if (this->J_.Denominator() < rhs.J_.Denominator())
      return true;
    if (this->M_.Numerator() > rhs.M_.Numerator())
      return false;
    else if (this->M_.Numerator() < rhs.M_.Numerator())
      return true;
    if (this->M_.Denominator() > rhs.M_.Denominator())
      return false;

    return true;
  }
  bool operator>(const SpinInfo &rhs) const {
    return (rhs < *this);
  }
};

struct ParticleStateInfo {
  unsigned int id_;
  int particle_id_;
  std::string name_;
  SpinInfo spin_information_;
  DynamicalInfo dynamical_information_;

  bool operator==(const ParticleStateInfo &rhs) const {
    if (this->id_ != rhs.id_)
      return false;
    if (this->particle_id_ != rhs.particle_id_)
      return false;
    if (this->name_ != rhs.name_)
      return false;
    if (this->spin_information_ != rhs.spin_information_)
      return false;

    return true;
  }

  bool operator<(const ParticleStateInfo &rhs) const {
    if (this->id_ > rhs.id_)
      return false;
    else if (this->id_ < rhs.id_)
      return true;
    if (this->particle_id_ > rhs.particle_id_)
      return false;
    else if (this->particle_id_ < rhs.particle_id_)
      return true;
    if (this->name_ > rhs.name_)
      return false;
    else if (this->name_ < rhs.name_)
      return true;
    if (this->spin_information_ > rhs.spin_information_)
      return false;

    return true;
  }
  bool operator>(const ParticleStateInfo &rhs) const {
    return (rhs < *this);
  }
};

struct ParticleStateIDComparison {
  unsigned int ps_id_;

  ParticleStateIDComparison() {
  }
  ParticleStateIDComparison(unsigned int ps_id) :
      ps_id_(ps_id) {
  }

  bool operator()(const ParticleStateInfo& ps) {
    return ps.id_ == ps_id_;
  }

  bool operator()(const ParticleStateInfo& lhs, const ParticleStateInfo& rhs) {
    return lhs.id_ < rhs.id_;
  }
};

typedef std::pair<SpinInfo, SpinInfo> SpinInfoPair;
typedef std::pair<DynamicalInfo, DynamicalInfo> DynamicalInfoPair;

typedef std::vector<unsigned int> IndexList;
typedef std::pair<unsigned int, unsigned int> IndexPair;
typedef std::map<unsigned int, unsigned int> IndexMapping;

struct TwoBodyDecayIndices {
  unsigned int mother_index_;
  IndexPair decay_products_;

  bool operator<(const TwoBodyDecayIndices &rhs) const {
    if (this->mother_index_ > rhs.mother_index_)
      return false;
    else if (this->mother_index_ < rhs.mother_index_)
      return true;
    if (this->decay_products_ > rhs.decay_products_)
      return false;

    return true;
  }

  bool operator==(const TwoBodyDecayIndices &rhs) const {
    if (this->mother_index_ != rhs.mother_index_)
      return false;
    if (this->decay_products_ != rhs.decay_products_)
      return false;

    return true;
  }

};

struct TwoBodyDecaySpinInformation {
  SpinInfo initial_state_;
  SpinInfoPair final_state_;

  bool operator<(const TwoBodyDecaySpinInformation &rhs) const {
    if (this->initial_state_ > rhs.initial_state_)
      return false;
    else if (this->initial_state_ < rhs.initial_state_)
      return true;
    if (this->final_state_ > rhs.final_state_)
      return false;

    return true;
  }

  bool operator>(const TwoBodyDecaySpinInformation &rhs) const {
    return (rhs < *this);
  }
};

struct TwoBodyDecayDynamicalInformation {
  DynamicalInfo initial_state_;

  bool operator<(const TwoBodyDecayDynamicalInformation &rhs) const {
    if (generateMap(this->initial_state_) > generateMap(rhs.initial_state_))
      return false;

    return true;
  }

  bool operator>(const TwoBodyDecayDynamicalInformation &rhs) const {
    return (rhs < *this);
  }

  std::map<std::string, std::string> generateMap(
      const DynamicalInfo& tree) const {
    std::map<std::string, std::string> return_map;
    DynamicalInfo::const_iterator tree_iter;
    for (tree_iter = tree.begin(); tree_iter != tree.end(); ++tree_iter) {
      return_map[tree_iter->first] = tree_iter->second.get_value<std::string>();
    }
    return return_map;
  }
};

struct TwoBodyDecayInformation {
  TwoBodyDecaySpinInformation spin_info_;
  TwoBodyDecayDynamicalInformation dynamical_info_;

  bool operator<(const TwoBodyDecayInformation &rhs) const {
    if (this->spin_info_ > rhs.spin_info_)
      return false;
    else if (this->spin_info_ < rhs.spin_info_)
      return true;
    if (this->dynamical_info_ > rhs.dynamical_info_)
      return false;

    return true;
  }
};

struct DecayTopology {
  // set of final state particle lists corresponding to particle states
  std::vector<std::vector<ParticleStateInfo> > final_state_content_lists_;

  std::vector<TwoBodyDecayIndices> decay_node_infos_;

  bool operator<(const DecayTopology &rhs) const {
    if (this->decay_node_infos_ > rhs.decay_node_infos_)
      return false;
    else if (this->decay_node_infos_ < rhs.decay_node_infos_)
      return true;
    if (this->final_state_content_lists_ > rhs.final_state_content_lists_)
      return false;

    return true;
  }
};

}

#endif /* PHYSICS_HELICITYAMPLITUDE_PARTICLESTATEDEFINITIONS_HPP_ */