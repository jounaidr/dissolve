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
 * Minimum Image Routines (virtual implementations)
 */

// Return minimum image coordinates of r1 with respect to r2
Vec3<double> OrthorhombicBox::minimumImage(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    auto mimVec = r1;
    mimVec -= r2;
    mimVec.x -= int(mimVec.x * ra_ + (mimVec.x < 0.0 ? -0.5 : 0.5)) * a_;
    mimVec.y -= int(mimVec.y * rb_ + (mimVec.y < 0.0 ? -0.5 : 0.5)) * b_;
    mimVec.z -= int(mimVec.z * rc_ + (mimVec.z < 0.0 ? -0.5 : 0.5)) * c_;

    return mimVec + r2;
}

// Return minimum image vector from r1 to r2
Vec3<double> OrthorhombicBox::minimumVector(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    auto mimVec = r2;
    mimVec -= r1;

    mimVec.x -= int(mimVec.x * ra_ + (mimVec.x < 0.0 ? -0.5 : 0.5)) * a_;
    mimVec.y -= int(mimVec.y * rb_ + (mimVec.y < 0.0 ? -0.5 : 0.5)) * b_;
    mimVec.z -= int(mimVec.z * rc_ + (mimVec.z < 0.0 ? -0.5 : 0.5)) * c_;

    return mimVec;
}

// Return minimum image distance from r1 to r2
double OrthorhombicBox::minimumDistance(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    auto mimVec = r2;
    mimVec -= r1;

    mimVec.x -= int(mimVec.x * ra_ + (mimVec.x < 0.0 ? -0.5 : 0.5)) * a_;
    mimVec.y -= int(mimVec.y * rb_ + (mimVec.y < 0.0 ? -0.5 : 0.5)) * b_;
    mimVec.z -= int(mimVec.z * rc_ + (mimVec.z < 0.0 ? -0.5 : 0.5)) * c_;

    return mimVec.magnitude();
}

// Return minimum image squared distance from r1 to r2
double OrthorhombicBox::minimumDistanceSquared(const Vec3<double> &r1, const Vec3<double> &r2) const
{
    auto mimVec = r2;
    mimVec -= r1;

    mimVec.x -= int(mimVec.x * ra_ + (mimVec.x < 0.0 ? -0.5 : 0.5)) * a_;
    mimVec.y -= int(mimVec.y * rb_ + (mimVec.y < 0.0 ? -0.5 : 0.5)) * b_;
    mimVec.z -= int(mimVec.z * rc_ + (mimVec.z < 0.0 ? -0.5 : 0.5)) * c_;

    return mimVec.magnitudeSq();
}

/*
 * Utility Routines (Virtual Implementations)
 */

// Return random coordinate inside Box
Vec3<double> OrthorhombicBox::randomCoordinate() const
{
    Vec3<double> pos;
    pos.x = a_ * DissolveMath::random();
    pos.y = b_ * DissolveMath::random();
    pos.z = c_ * DissolveMath::random();
    return pos;
}

// Return folded coordinate (i.e. inside current Box)
Vec3<double> OrthorhombicBox::fold(const Vec3<double> &r) const
{
    // Convert coordinate to fractional coords
    Vec3<double> frac(r.x * ra_, r.y * rb_, r.z * rc_);

    // Fold into Box and divide by integer Cell sizes
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);
    frac.x *= a_;
    frac.y *= b_;
    frac.z *= c_;
    return frac;
}

// Return folded fractional coordinate (i.e. inside current Box)
Vec3<double> OrthorhombicBox::foldFrac(const Vec3<double> &r) const
{
    // Convert coordinate to fractional coords
    Vec3<double> frac(r.x * ra_, r.y * rb_, r.z * rc_);

    // Fold into Box and divide by integer Cell sizes
    frac.x -= floor(frac.x);
    frac.y -= floor(frac.y);
    frac.z -= floor(frac.z);
    return frac;
}

// Convert supplied fractional coordinates to real space
Vec3<double> OrthorhombicBox::fracToReal(const Vec3<double> &r) const
{
    // Multiply by box lengths
    return Vec3<double>(r.x * a_, r.y * b_, r.z * c_);
}
