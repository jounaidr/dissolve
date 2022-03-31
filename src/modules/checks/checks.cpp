// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#include "modules/checks/checks.h"
#include "keywords/configuration.h"
#include "keywords/double.h"
#include "keywords/geometrylist.h"

ChecksModule::ChecksModule() : Module("Checks")
{
    // Targets
    keywords_.addTarget<ConfigurationKeyword>("Configuration", "Set target configuration for the module", targetConfiguration_);

    // Distance
    keywords_.add<GeometryListKeyword>("Distance", "Distance", "Define a distance between Atoms to be checked", distances_,
                                       Geometry::DistanceType);
    keywords_.add<DoubleKeyword>("Distance", "DistanceThreshold", "Threshold at which distance checks will fail (Angstroms)",
                                 distanceThreshold_, 1.0e-5);

    // Angle
    keywords_.add<GeometryListKeyword>("Angle", "Angle", "Define an angle between Atoms to be checked", angles_,
                                       Geometry::AngleType);
    keywords_.add<DoubleKeyword>("Angle", "AngleThreshold", "Threshold at which angle checks will fail", angleThreshold_,
                                 1.0e-5);
}