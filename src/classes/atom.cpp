// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "classes/atom.h"
#include "classes/atomType.h"
#include "classes/molecule.h"
#include "classes/speciesAtom.h"
#include <utility>

// Set coordinates
void Atom::set(const Vec3<double> r) { r_ = r; }

// Set coordinates
void Atom::set(double rx, double ry, double rz) { r_.set(rx, ry, rz); }

// Return coordinates
const Vec3<double> &Atom::r() const { return r_; }

// Return x-coordinate
double Atom::x() const { return r_.x; }

// Return y-coordinate
double Atom::y() const { return r_.y; }

// Return z-coordinate
double Atom::z() const { return r_.z; }

// Set local AtomType index
void Atom::setLocalTypeIndex(int id) { localTypeIndex_ = id; }

// Return local AtomType index
int Atom::localTypeIndex() const { return localTypeIndex_; }

// Set master AtomType index
void Atom::setMasterTypeIndex(int id) { masterTypeIndex_ = id; }

// Return master AtomType index
int Atom::masterTypeIndex() const { return masterTypeIndex_; }

// Return global index of the atom
int Atom::globalIndex() const
{
    assert(molecule_);
    return molecule_->globalAtomIndex(this);
}

/*
 * Location
 */

// Set SpeciesAtom that this Atom represents
void Atom::setSpeciesAtom(const SpeciesAtom *spAtom) { speciesAtom_ = spAtom; }

// Return SpeciesAtom that this Atom represents
const SpeciesAtom *Atom::speciesAtom() const { return speciesAtom_; }

// Set Molecule in which this Atom exists
void Atom::setMolecule(std::shared_ptr<Molecule> mol) { molecule_ = std::move(mol); }

// Return Molecule in which this Atom exists
const std::shared_ptr<Molecule> &Atom::molecule() const { return molecule_; }

// Set cell in which the atom exists
void Atom::setCell(Cell *cell) { cell_ = cell; }

// Return cell in which the atom exists
Cell *Atom::cell() const { return cell_; }

/*
 * Coordinate Manipulation
 */

// Set coordinates
void Atom::setCoordinates(const Vec3<double> &newr) { r_ = newr; }

// Set coordinates
void Atom::setCoordinates(double dx, double dy, double dz) { setCoordinates(Vec3<double>(dx, dy, dz)); }

// Translate coordinates
void Atom::translateCoordinates(const Vec3<double> &delta) { setCoordinates(r_ + delta); }

// Translate coordinates
void Atom::translateCoordinates(double dx, double dy, double dz) { setCoordinates(r_ + Vec3<double>(dx, dy, dz)); }

/*
 * Intramolecular Information
 */

// Return scaling type and factors (electrostatic, van der Waals) to employ with specified Atom
SpeciesAtom::ScaledInteractionDefinition Atom::scaling(const Atom *j) const
{
    assert(speciesAtom_ != nullptr);
    assert(j != nullptr);
    assert(j->speciesAtom() != nullptr);

    return speciesAtom_->scaling(j->speciesAtom());
}

/*
 * Targeted Potentials
 */

// Add targeted potential to this atom
void Atom::addTargetedPotential(const ExternalPotential *potential) { targetedPotentials_.emplace_back(potential); }

// Clear all targeted potentials from this Atom
void Atom::clearTargetedPotentials() { targetedPotentials_.clear(); }

// Return list of targeted potentials for this atom
const std::vector<const ExternalPotential *> &Atom::targetedPotentials() const { return targetedPotentials_; }
