// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "classes/potentialMap.h"
#include "base/messenger.h"
#include "classes/atom.h"
#include "classes/atomType.h"
#include "classes/molecule.h"
#include "classes/pairPotential.h"
#include "classes/species.h"
#include "math/constants.h"
#include <cmath>
#include <new>
using namespace std;

PotentialMap::PotentialMap() = default;

PotentialMap::~PotentialMap() = default;

// Clear all data
void PotentialMap::clear() { potentialMatrix_.clear(); }

/*
 * Source Parameters
 */

// Initialise maps
bool PotentialMap::initialise(const std::vector<std::shared_ptr<AtomType>> &masterAtomTypes,
                              const std::vector<std::unique_ptr<PairPotential>> &pairPotentials, double pairPotentialRange)
{
    // Clear old data first
    clear();

    // Create PairPotential matrix
    nTypes_ = masterAtomTypes.size();
    potentialMatrix_.initialise(nTypes_, nTypes_);

    // Loop over defined PairPotentials
    int indexI, indexJ;
    for (auto &pot : pairPotentials)
    {
        indexI = pot->atomTypeI()->index();
        indexJ = pot->atomTypeJ()->index();
        if (indexI == -1)
            return Messenger::error("Couldn't find AtomType '{}' in typeIndex.\n", pot->atomTypeI()->name());
        if (indexJ == -1)
            return Messenger::error("Couldn't find AtomType '{}' in typeIndex.\n", pot->atomTypeJ()->name());

        // Store PairPotential pointer
        if (indexI == indexJ)
        {
            Messenger::print("Linking self-interaction PairPotential for '{}' (index {},{} in matrix).\n",
                             pot->atomTypeI()->name(), indexI, indexJ);
            potentialMatrix_[{indexI, indexI}] = pot.get();
        }
        else
        {
            Messenger::print("Linking PairPotential between '{}' and '{}' (indices {},{} and {},{} in matrix).\n",
                             pot->atomTypeI()->name(), pot->atomTypeJ()->name(), indexI, indexJ, indexJ, indexI);
            potentialMatrix_[{indexI, indexJ}] = pot.get();
            potentialMatrix_[{indexJ, indexI}] = pot.get();
        }
    }

    // Store potential range
    range_ = pairPotentialRange;

    return true;
}

// Return PairPotential range
double PotentialMap::range() const { return range_; }

/*
 * Energy / Force
 */

// Return energy between Atoms at distance specified
double PotentialMap::energy(const Atom &i, const Atom &j, double r) const
{
    assert(r >= 0.0);
    assert(i.speciesAtom() && j.speciesAtom());

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i.masterTypeIndex(), j.masterTypeIndex()}];
    return pp->energy(r) + (pp->includeAtomTypeCharges()
                                ? 0
                                : pp->analyticCoulombEnergy(i.speciesAtom()->charge() * j.speciesAtom()->charge(), r));
}

// Return energy between Atoms at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::energy(const Atom &i, const Atom &j, double r, double elecScale, double vdwScale) const
{
    assert(r >= 0.0);
    assert(i.speciesAtom() && j.speciesAtom());

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i.masterTypeIndex(), j.masterTypeIndex()}];
    return pp->includeAtomTypeCharges()
               ? pp->energy(r) * elecScale
               : pp->energy(r) * vdwScale +
                     pp->analyticCoulombEnergy(i.speciesAtom()->charge() * j.speciesAtom()->charge(), r) * elecScale;
}

// Return energy between SpeciesAtoms at distance specified
double PotentialMap::energy(const SpeciesAtom *i, const SpeciesAtom *j, double r) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->atomType()->index(), j->atomType()->index()}];
    return pp->energy(r) + (pp->includeAtomTypeCharges() ? 0 : pp->analyticCoulombEnergy(i->charge() * j->charge(), r));
}

// Return energy between SpeciesAtoms at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::energy(const SpeciesAtom *i, const SpeciesAtom *j, double r, double elecScale, double vdwScale) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->atomType()->index(), j->atomType()->index()}];
    return pp->includeAtomTypeCharges()
               ? pp->energy(r) * elecScale
               : pp->energy(r) * vdwScale + pp->analyticCoulombEnergy(i->charge() * j->charge(), r) * elecScale;
}

// Return analytic energy between Atom types at distance specified
double PotentialMap::analyticEnergy(const Atom *i, const Atom *j, double r) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being local to the atom
    // types
    auto *pp = potentialMatrix_[{i->masterTypeIndex(), j->masterTypeIndex()}];
    return pp->includeAtomTypeCharges() ? pp->analyticEnergy(r)
                                        : pp->analyticEnergy(i->speciesAtom()->charge() * j->speciesAtom()->charge(), r);
}

// Return analytic energy between Atom types at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::analyticEnergy(const Atom *i, const Atom *j, double r, double elecScale, double vdwScale) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being local to the atom
    // types
    auto *pp = potentialMatrix_[{i->masterTypeIndex(), j->masterTypeIndex()}];
    return pp->includeAtomTypeCharges()
               ? pp->analyticEnergy(r) * elecScale
               : pp->analyticEnergy(i->speciesAtom()->charge() * j->speciesAtom()->charge(), r, elecScale, vdwScale);
}

// Return force between Atoms at distance specified
double PotentialMap::force(const Atom &i, const Atom &j, double r) const
{
    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i.masterTypeIndex(), j.masterTypeIndex()}];
    return pp->includeAtomTypeCharges()
               ? pp->force(r)
               : pp->force(r) + pp->analyticCoulombForce(i.speciesAtom()->charge() * j.speciesAtom()->charge(), r);
}

// Return force between Atoms at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::force(const Atom &i, const Atom &j, double r, double elecScale, double vdwScale) const
{
    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i.masterTypeIndex(), j.masterTypeIndex()}];
    return pp->includeAtomTypeCharges()
               ? pp->force(r) * elecScale
               : pp->force(r) * vdwScale +
                     pp->analyticCoulombForce(i.speciesAtom()->charge() * j.speciesAtom()->charge(), r) * elecScale;
}

// Return force between SpeciesAtoms at distance specified
double PotentialMap::force(const SpeciesAtom *i, const SpeciesAtom *j, double r) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->atomType()->index(), j->atomType()->index()}];
    return pp->includeAtomTypeCharges() ? pp->force(r) : pp->force(r) + pp->analyticCoulombForce(i->charge() * j->charge(), r);
}

// Return force between SpeciesAtoms at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::force(const SpeciesAtom *i, const SpeciesAtom *j, double r, double elecScale, double vdwScale) const
{
    assert(r >= 0.0);
    assert(i && j);

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->atomType()->index(), j->atomType()->index()}];
    return pp->includeAtomTypeCharges()
               ? pp->force(r) * elecScale
               : pp->force(r) * vdwScale + pp->analyticCoulombForce(i->charge() * j->charge(), r) * elecScale;
}

// Return analytic force between Atom types at distance specified
double PotentialMap::analyticForce(const Atom *i, const Atom *j, double r) const
{
    assert(r >= 0.0);
    assert(i && j);
    assert(i->speciesAtom() && j->speciesAtom());

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->masterTypeIndex(), j->masterTypeIndex()}];
    return pp->includeAtomTypeCharges() ? pp->analyticForce(r)
                                        : pp->analyticForce(i->speciesAtom()->charge() * j->speciesAtom()->charge(), r);
}

// Return analytic force between Atom types at distance specified, scaling electrostatic and van der Waals components
double PotentialMap::analyticForce(const Atom *i, const Atom *j, double r, double elecScale, double vdwScale) const
{
    assert(r >= 0.0);
    assert(i && j);
    assert(i->speciesAtom() && j->speciesAtom());

    // Check to see whether Coulomb terms should be calculated from atomic charges, rather than them being included in the
    // interpolated potential
    auto *pp = potentialMatrix_[{i->masterTypeIndex(), j->masterTypeIndex()}];
    return pp->includeAtomTypeCharges()
               ? pp->analyticForce(r) * elecScale
               : pp->analyticForce(i->speciesAtom()->charge() * j->speciesAtom()->charge(), r, elecScale, vdwScale);
}
