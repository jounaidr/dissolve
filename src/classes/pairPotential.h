// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#pragma once

#include "data/ff/ff.h"
#include "math/data1D.h"
#include "math/interpolator.h"
#include <memory>

// Forward Declarations
class AtomType;
class SerializablePairPotential;

// PairPotential Definition
class PairPotential
{
    friend class SerializablePairPotential;

    public:
    PairPotential();
    // Coulomb Truncation Scheme enum
    enum CoulombTruncationScheme
    {
        NoCoulombTruncation,     /* No truncation scheme */
        ShiftedCoulombTruncation /* Shifted and truncated */
    };
    // Return enum options for CoulombTruncationScheme
    static EnumOptions<PairPotential::CoulombTruncationScheme> coulombTruncationSchemes();
    // Short-Range Truncation Scheme enum
    enum ShortRangeTruncationScheme
    {
        NoShortRangeTruncation,      /* No truncation scheme */
        ShiftedShortRangeTruncation, /* Shifted and truncated */
        CosineShortRangeTruncation   /* Cosine-multiplied truncation */
    };
    // Return enum options for ShortRangeTruncationScheme
    static EnumOptions<PairPotential::ShortRangeTruncationScheme> shortRangeTruncationSchemes();

    /*
     * Seed Interaction Type
     */
    private:
    // Truncation scheme to apply to short-range part of potential
    static ShortRangeTruncationScheme shortRangeTruncationScheme_;
    // Width of short-range potential over which to truncate (if scheme = Cosine)
    static double shortRangeTruncationWidth_;
    // Short-range energy at cutoff distance (used by truncation scheme)
    double shortRangeEnergyAtCutoff_{0.0};
    // Short-range force at cutoff distance (used by truncation scheme)
    double shortRangeForceAtCutoff_{0.0};
    // Whether atom type charges should be included in the generated potential
    bool includeAtomTypeCharges_{true};
    // Truncation scheme to apply to Coulomb part of potential
    static CoulombTruncationScheme coulombTruncationScheme_;
    // Coulomb energy at cutoff distance (used by truncation scheme)
    double coulombEnergyAtCutoff_{0.0};
    // Coulomb force at cutoff distance (used by truncation scheme)
    double coulombForceAtCutoff_{0.0};

    public:
    // Set short-ranged truncation scheme
    static void setShortRangeTruncationScheme(ShortRangeTruncationScheme scheme);
    // Return short-ranged truncation scheme
    static ShortRangeTruncationScheme shortRangeTruncationScheme();
    // Set width of short-range potential over which to truncate (if scheme = Cosine)
    static void setShortRangeTruncationWidth(double width);
    // Return width of short-range potential over which to truncate (if scheme = Cosine)
    static double shortRangeTruncationWidth();
    // Set whether atom type charges should be included in the generated potential
    void setIncludeAtomTypeCharges(bool b);
    // Return whether atom type charges should be included in the generated potential
    bool includeAtomTypeCharges() const;
    // Set Coulomb truncation scheme
    static void setCoulombTruncationScheme(CoulombTruncationScheme scheme);
    // Return Coulomb truncation scheme
    static CoulombTruncationScheme coulombTruncationScheme();

    /*
     * Source Parameters
     */
    private:
    // Original source AtomTypes
    std::shared_ptr<AtomType> atomTypeI_, atomTypeJ_;
    // Interaction potential
    InteractionPotential<ShortRangeFunctions> interactionPotential_;
    // Charge on I (taken from AtomType)
    double chargeI_{0.0};
    // Charge on J (taken from AtomType)
    double chargeJ_{0.0};

    private:
    // Set Data1D names from source AtomTypes
    void setData1DNames();

    public:
    // Set up PairPotential parameters from specified AtomTypes
    bool setUp(const std::shared_ptr<AtomType> &typeI, const std::shared_ptr<AtomType> &typeJ, bool includeCharges);
    // Return interaction potential
    InteractionPotential<ShortRangeFunctions> &interactionPotential();
    const InteractionPotential<ShortRangeFunctions> &interactionPotential() const;
    // Return first AtomType name
    std::string_view atomTypeNameI() const;
    // Return second AtomType name
    std::string_view atomTypeNameJ() const;
    // Return first source AtomType
    std::shared_ptr<AtomType> atomTypeI() const;
    // Return second source AtomType
    std::shared_ptr<AtomType> atomTypeJ() const;
    // Set charge I
    void setChargeI(double value);
    // Return charge I
    double chargeI() const;
    // Set charge J
    void setChargeJ(double value);
    // Return charge J
    double chargeJ() const;

    /*
     * Tabulated Potential
     */
    private:
    // Number of points to tabulate
    int nPoints_{0};
    // Maximum distance of potential
    double range_{0.0};
    // Distance between points in tabulated potentials
    double delta_{-1.0}, rDelta_{0.0};
    // Tabulated original potential, calculated from AtomType parameters
    Data1D uOriginal_;
    // Additional potential, generated by some means
    Data1D uAdditional_;
    // Full potential (original plus additional), used in simulations
    Data1D uFull_;
    // Interpolation of full potential
    Interpolator uFullInterpolation_;
    // Tabulated derivative of full potential
    Data1D dUFull_;
    // Interpolation of derivative of full potential
    Interpolator dUFullInterpolation_;

    private:
    // Return analytic short range potential energy
    double analyticShortRangeEnergy(
        double r, PairPotential::ShortRangeTruncationScheme truncation = PairPotential::shortRangeTruncationScheme()) const;
    // Return analytic short range force
    double analyticShortRangeForce(
        double r, PairPotential::ShortRangeTruncationScheme truncation = PairPotential::shortRangeTruncationScheme()) const;
    // Calculate full potential
    void calculateUFull();
    // Calculate derivative of potential
    void calculateDUFull();

    public:
    // Generate energy and force tables
    bool tabulate(double maxR, double delta);
    // Return number of tabulated points in potential
    int nPoints() const;
    // Return range of potential
    double range() const;
    // Return spacing between points
    double delta() const;
    // (Re)generate original potential (uOriginal) from current parameters
    void calculateUOriginal(bool recalculateUFull = true);
    // Return potential at specified r
    double energy(double r);
    // Return analytic potential at specified r, including Coulomb term from local atomtype charges
    double analyticEnergy(double r) const;
    // Return analytic potential at specified r, including Coulomb term from supplied charge product
    double analyticEnergy(double qiqj, double r, double elecScale = 1.0, double vdwScale = 1.0,
                          PairPotential::CoulombTruncationScheme truncation = PairPotential::coulombTruncationScheme()) const;
    // Return analytic coulomb potential energy of specified charge product
    double
    analyticCoulombEnergy(double qiqj, double r,
                          PairPotential::CoulombTruncationScheme truncation = PairPotential::coulombTruncationScheme()) const;
    // Return derivative of potential at specified r
    double force(double r);
    // Return analytic force at specified r, including Coulomb term from local atomtype charges
    double analyticForce(double r) const;
    // Return analytic force at specified r, including Coulomb term from supplied charge product
    double analyticForce(double qiqj, double r, double elecScale = 1.0, double vdwScale = 1.0,
                         PairPotential::CoulombTruncationScheme truncation = PairPotential::coulombTruncationScheme()) const;
    // Return analytic coulomb force of specified charge product
    double
    analyticCoulombForce(double qiqj, double r,
                         PairPotential::CoulombTruncationScheme truncation = PairPotential::coulombTruncationScheme()) const;
    // Return full tabulated potential (original plus additional)
    Data1D &uFull();
    const Data1D &uFull() const;
    // Return full tabulated derivative
    Data1D &dUFull();
    const Data1D &dUFull() const;
    // Return original potential
    Data1D &uOriginal();
    const Data1D &uOriginal() const;
    // Return additional potential
    Data1D &uAdditional();
    const Data1D &uAdditional() const;
    // Zero additional potential
    void resetUAdditional();
    // Set additional potential
    void setUAdditional(Data1D &newUAdditional);
    // Adjust additional potential, and recalculate UFull and dUFull
    void adjustUAdditional(const Data1D &deltaU, double factor = 1.0);
};
