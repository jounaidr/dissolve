// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/atom.h"
#include "classes/box.h"

OrthorhombicBox::OrthorhombicBox(const Vec3<double> lengths) : Box()
{
    type_ = Box::BoxType::Orthorhombic;

    // Construct axes_
    axes_.setColumn(0, lengths.x, 0.0, 0.0);
    axes_.setColumn(1, 0.0, lengths.y, 0.0);
    axes_.setColumn(2, 0.0, 0.0, lengths.z);

    // Store Box lengths
    a_ = lengths.x;
    b_ = lengths.y;
    c_ = lengths.z;
    ra_ = 1.0 / a_;
    rb_ = 1.0 / b_;
    rc_ = 1.0 / c_;

    // Finalise associated data
    finalise();
}

/*
 * Coordinate Conversion
 */

// Convert specified fractional coordinates to real-space coordinates
void OrthorhombicBox::toReal(Vec3<double> &r) const
{
    r.x *= a_;
    r.y *= b_;
    r.z *= c_;
}

// Convert specified real-space coordinates to fractional coordinates
void OrthorhombicBox::toFractional(Vec3<double> &r) const
{
    r.x *= ra_;
    r.y *= rb_;
    r.z *= rc_;
}

/*
 * Minimum Image Calculation
 */

// Return minimum image coordinates of r1 with respect to r2
Vec3<double> OrthorhombicBox::minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    Vec3<double> v21 = r1 - r2;
    toFractional(v21);
    wrap(v21);
    toReal(v21);
    v21 += r2;
    return v21;
}

// Return minimum image distance from r1 to r2
double OrthorhombicBox::minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    Vec3<double> v12 = r2 - r1;
    toFractional(v12);
    wrap(v12);
    toReal(v12);
    return v12.magnitude();
}

// Return minimum image vector from r1 to r2
Vec3<double> OrthorhombicBox::minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    Vec3<double> v12 = r2 - r1;
    toFractional(v12);
    wrap(v12);
    toReal(v12);
    return v12;
}

// Return minimum image squared distance from r1 to r2
double OrthorhombicBox::minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    Vec3<double> v12 = r2 - r1;
    toFractional(v12);
    wrap(v12);
    toReal(v12);
    return v12.magnitudeSq();
}
