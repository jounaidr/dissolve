// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include <utility>

#include "classes/atomType.h"
#include "main/dissolve.h"

// Set maximum distance for tabulated PairPotentials
void Dissolve::setPairPotentialRange(double range) { pairPotentialRange_ = range; }

// Return maximum distance for tabulated PairPotentials
double Dissolve::pairPotentialRange() const { return pairPotentialRange_; }

// Set delta to use in tabulations
void Dissolve::setPairPotentialDelta(double delta) { pairPotentialDelta_ = delta; }

// Return delta to use in tabulations
double Dissolve::pairPotentialDelta() const { return pairPotentialDelta_; }

// Set whether to automatically determine charge source
void Dissolve::setAutomaticChargeSource(bool b) { automaticChargeSource_ = b; };

// Return whether to automatically determine charge source
bool Dissolve::automaticChargeSource() const { return automaticChargeSource_; }

// Set whether to force the use of the specified charge source (if not automatic choice)
void Dissolve::setForceChargeSource(bool b) { forceChargeSource_ = b; }

// Return whether to force the use of the specified charge source (if not automatic choice)
bool Dissolve::forceChargeSource() const { return forceChargeSource_; }

// Set whether charges from atom types are to be used (and included in PairPotentials)
void Dissolve::setAtomTypeChargeSource(bool b) { atomTypeChargeSource_ = b; }

// Return whether charges from atom types are to be used (and included in PairPotentials)
bool Dissolve::atomTypeChargeSource() const { return atomTypeChargeSource_; }

// Return index of specified PairPotential
int Dissolve::indexOf(PairPotential *pp)
{
    auto result = std::find_if(pairPotentials_.begin(), pairPotentials_.end(), [pp](auto &p) { return pp == p.get(); });

    return result == pairPotentials_.end() ? -1 : result - pairPotentials_.begin();
}

// Return number of defined PairPotentials
int Dissolve::nPairPotentials() const { return pairPotentials_.size(); }

// Add new pair potential to list
PairPotential *Dissolve::addPairPotential(std::shared_ptr<AtomType> at1, std::shared_ptr<AtomType> at2)
{
    auto &pp = pairPotentials_.emplace_back(std::make_unique<PairPotential>());
    pp->setUp(std::move(at1), std::move(at2), atomTypeChargeSource_);

    return pp.get();
}

// Return first PairPotential in list
const std::vector<std::unique_ptr<PairPotential>> &Dissolve::pairPotentials() const { return pairPotentials_; }

// Return nth PairPotential in list
PairPotential *Dissolve::pairPotential(int n) { return pairPotentials_[n].get(); }

// Return specified PairPotential (if defined)
PairPotential *Dissolve::pairPotential(const std::shared_ptr<AtomType> &at1, const std::shared_ptr<AtomType> &at2) const
{
    auto it = std::find_if(pairPotentials_.begin(), pairPotentials_.end(),
                           [at1, at2](const auto &pp) {
                               return (pp->atomTypeI() == at1 && pp->atomTypeJ() == at2) ||
                                      (pp->atomTypeI() == at2 && pp->atomTypeJ() == at1);
                           });
    return it != pairPotentials_.end() ? it->get() : nullptr;
}

PairPotential *Dissolve::pairPotential(std::string_view at1, std::string_view at2) const
{
    auto it = std::find_if(
        pairPotentials_.begin(), pairPotentials_.end(),
        [at1, at2](const auto &pp)
        {
            return (DissolveSys::sameString(pp->atomTypeNameI(), at1) && DissolveSys::sameString(pp->atomTypeNameJ(), at2)) ||
                   (DissolveSys::sameString(pp->atomTypeNameI(), at2) && DissolveSys::sameString(pp->atomTypeNameJ(), at1));
        });
    return it != pairPotentials_.end() ? it->get() : nullptr;
}

// Return map for PairPotentials
const PotentialMap &Dissolve::potentialMap() const { return potentialMap_; }

// Clear and regenerate all PairPotentials, replacing those currently defined
bool Dissolve::regeneratePairPotentials()
{
    Messenger::print("Regenerating pair potentials...\n");
    potentialMap_.clear();
    pairPotentials_.clear();

    // Create a pair potential for each unique atom type pair
    auto success =
        for_each_pair_early(coreData_.atomTypes().begin(), coreData_.atomTypes().end(),
                            [&](int typeI, const auto &at1, int typeJ, const auto &at2) -> EarlyReturn<bool>
                            {
                                Messenger::printVerbose("Adding new PairPotential for interaction between '{}' and '{}'...\n",
                                                        at1->name(), at2->name());
                                auto *pot = addPairPotential(at1, at2);

                                // Tabulate the basic potential
                                if (!pot->tabulate(pairPotentialRange_, pairPotentialDelta_))
                                    return false;

                                // Retrieve additional potential from the processing module data, if present
                                auto itemName =
                                    fmt::format("Potential_{}-{}_Additional", pot->atomTypeNameI(), pot->atomTypeNameJ());
                                if (processingModuleData_.contains(itemName, "Dissolve"))
                                    pot->setUAdditional(processingModuleData_.retrieve<Data1D>(itemName, "Dissolve"));

                                return EarlyReturn<bool>::Continue;
                            });
    if (!success.value_or(true))
        return false;

    // Update the potential map
    return potentialMap_.initialise(coreData_.atomTypes(), pairPotentials_, pairPotentialRange_);
}

// Revert potentials to reference state, clearing additional potentials
void Dissolve::revertPairPotentials()
{
    for (auto &pp : pairPotentials_)
    {
        pp->resetUAdditional();

        // Clear entry in processing module data if it exists
        auto itemName = fmt::format("Potential_{}-{}_Additional", pp->atomTypeNameI(), pp->atomTypeNameJ());
        if (processingModuleData_.contains(itemName, "Dissolve"))
            processingModuleData_.remove(itemName, "Dissolve");
    }
}
