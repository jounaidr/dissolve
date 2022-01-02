// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "templates/reflist.h"
#include <vector>

// Forward Declarations
class SpeciesAtom;
class Species;
class MasterIntra;

// Base class for intramolecular interactions within Species
class SpeciesIntra
{
    public:
    explicit SpeciesIntra(int form = 0);
    virtual ~SpeciesIntra() = default;
    SpeciesIntra(const SpeciesIntra &source);
    SpeciesIntra(SpeciesIntra &&source) = delete;
    SpeciesIntra &operator=(const SpeciesIntra &source);
    SpeciesIntra &operator=(SpeciesIntra &&source) = delete;

    /*
     * SpeciesAtom Information
     * */
    public:
    // Return vector of involved atoms
    virtual std::vector<const SpeciesAtom *> atoms() const;

    /*
     * Interaction Parameters
     */
    public:
    // Interaction Type
    enum class InteractionType
    {
        Bond,    /* Interaction is a bond between two atoms  */
        Angle,   /* Interaction is an angle between three atoms */
        Torsion, /* Interaction is a torsion between four atoms */
        Improper /* Interaction is an improper torsion between four atoms */
    };

    protected:
    // Linked master from which parameters should be taken (if relevant)
    const MasterIntra *masterParameters_{nullptr};
    // Index of functional form of interaction
    int form_{0};
    // Parameters for interaction
    std::vector<double> parameters_;

    public:
    // Set linked master from which parameters should be taken
    void setMasterParameters(const MasterIntra *master);
    // Return linked master from which parameters should be taken
    const MasterIntra *masterParameters() const;
    // Detach from MasterIntra, if we are currently referencing one
    void detachFromMasterIntra();
    // Set functional form index of interaction
    void setForm(int form);
    // Return functional form index of interaction
    int form() const;
    // Add parameter to interaction
    void addParameter(double param);
    // Set all parameters
    void setParameters(const std::vector<double> &params);
    // Set existing parameter
    void setParameter(int id, double value);
    // Set form and parameters
    void setFormAndParameters(int form, const std::vector<double> &params);
    // Return number of parameters defined
    int nParameters() const;
    // Return nth parameter
    double parameter(int id) const;
    // Return array of parameters
    const std::vector<double> &parameters() const;
    // Set up any necessary parameters
    virtual void setUp() = 0;
    // Calculate and return fundamental frequency for the interaction
    virtual double fundamentalFrequency(double reducedMass) const = 0;
    // Return type of this interaction
    virtual InteractionType type() const = 0;

    /*
     * Connections
     */
    private:
    // Number of SpeciesAtoms attached to termini (number of items stored in attached_ arrays)
    std::vector<int> attached_[2];
    // Whether the term is contained within a cycle
    bool inCycle_{false};

    public:
    // Set attached SpeciesAtoms for terminus specified
    void setAttachedAtoms(int terminus, const std::vector<int> atoms);
    // Set attached SpeciesAtoms for terminus specified (single SpeciesAtom)
    void setAttachedAtoms(int terminus, int index);
    // Return vector of attached indices for terminus specified
    const std::vector<int> &attachedAtoms(int terminus) const;
    // Set whether the term is contained within a cycle
    void setInCycle(bool b);
    // Return whether the term is contained within a cycle
    bool inCycle() const;
};
