// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/atom.h"
#include "classes/box.h"

MonoclinicBox::MonoclinicBox(const Vec3<double> lengths, double beta) : Box()
{
    type_ = Box::BoxType::Monoclinic;

    // Construct axes
    alpha_ = 90.0;
    beta_ = beta;
    gamma_ = 90.0;
    // Assume that A lays along x-axis and C lays along the z-axis (since gamma = 90)
    axes_.setColumn(0, 1.0, 0.0, 0.0);
    axes_.setColumn(1, 0.0, 1.0, 0.0);
    // The C vector will only have components in x and z
    double cosBeta = cos(beta_ / DEGRAD);
    axes_.setColumn(2, cosBeta, 0.0, sqrt(1.0 - cosBeta * cosBeta));

    // Multiply the unit vectors to have the correct lengths
    axes_.columnMultiply(0, lengths.x);
    axes_.columnMultiply(1, lengths.y);
    axes_.columnMultiply(2, lengths.z);

    // Store Box lengths
    a_ = lengths.x;
    b_ = lengths.y;
    c_ = lengths.z;

    // Finalise associated data
    finalise();
}

/*
 * Minimum Image Routines (virtual implementations)
 */

// Return minimum image coordinates of 'i' with respect to 'j'
Vec3<double> MonoclinicBox::minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    // TODO Can speed this up since we know which matrix elements are zero
    auto mim = inverseAxes_ * (r2 - r1);
    if (mim.x < -0.5)
        mim.x += 1.0;
    else if (mim.x > 0.5)
        mim.x -= 1.0;
    if (mim.y < -0.5)
        mim.y += 1.0;
    else if (mim.y > 0.5)
        mim.y -= 1.0;
    if (mim.z < -0.5)
        mim.z += 1.0;
    else if (mim.z > 0.5)
        mim.z -= 1.0;
    return axes_ * mim + r2;
}

// Return minimum image vector from r1 to r2
Vec3<double> MonoclinicBox::minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    // TODO Can speed this up since we know which matrix elements are zero
    auto mim = inverseAxes_ * (r2 - r1);
    if (mim.x < -0.5)
        mim.x += 1.0;
    else if (mim.x > 0.5)
        mim.x -= 1.0;
    if (mim.y < -0.5)
        mim.y += 1.0;
    else if (mim.y > 0.5)
        mim.y -= 1.0;
    if (mim.z < -0.5)
        mim.z += 1.0;
    else if (mim.z > 0.5)
        mim.z -= 1.0;
    return axes_ * mim;
}

// Return minimum image distance from r1 to r2
double MonoclinicBox::minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    return minimumVector(r1, r2).magnitude();
}

// Return minimum image squared distance from r1 to r2
double MonoclinicBox::minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    return minimumVector(r1, r2).magnitudeSq();
}

/*
 * Utility Routines (Virtual Implementations)
 */

// Return random coordinate inside Box
Vec3<double> MonoclinicBox::randomCoordinate() const
{
    Vec3<double> pos(DissolveMath::random(), DissolveMath::random(), DissolveMath::random());
    return axes_ * pos;
}

// Return folded coordinate (i.e. inside current Box)
Vec3<double> MonoclinicBox::fold(const Vec3<double> &r) const
{
    // TODO Can speed this up part since we know which matrix elements are zero

    // Convert coordinate to fractional coords
    auto frac = inverseAxes_ * r;

    // Fold into Box and remultiply by inverse matrix
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);

    return axes_ * frac;
}

// Return folded fractional coordinate (i.e. inside current Box)
Vec3<double> MonoclinicBox::foldFrac(const Vec3<double> &r) const
{
    // TODO Can speed this up part since we know which matrix elements are zero

    // Convert coordinate to fractional coords
    auto frac = inverseAxes_ * r;

    // Fold into Box and remultiply by inverse matrix
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);

    return frac;
}

// Convert supplied fractional coordinates to real space
Vec3<double> MonoclinicBox::fracToReal(const Vec3<double> &r) const
{
    // Multiply by axes matrix
    // TODO Can speed this up part since we know which matrix elements are zero
    return axes_ * r;
}
