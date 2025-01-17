// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "modules/atomShake/atomShake.h"
#include "keywords/configuration.h"
#include "keywords/double.h"
#include "keywords/integer.h"
#include "keywords/optionalDouble.h"

AtomShakeModule::AtomShakeModule() : Module(ModuleTypes::AtomShake)
{
    keywords_.addTarget<ConfigurationKeyword>("Configuration", "Set target configuration for the module", targetConfiguration_);

    keywords_.setOrganisation("Options", "Control");
    keywords_.add<IntegerKeyword>("ShakesPerAtom", "Number of shakes to attempt per atom", nShakesPerAtom_, 1);
    keywords_.add<DoubleKeyword>("TargetAcceptanceRate", "Target acceptance rate for Monte Carlo moves", targetAcceptanceRate_,
                                 0.01, 1.0);

    keywords_.setOrganisation("Options", "Step Size");
    keywords_.addRestartable<DoubleKeyword>("StepSize", "Step size in Angstroms to use in Monte Carlo moves", stepSize_, 0.001);
    keywords_.add<DoubleKeyword>("StepSizeMax", "Maximum allowed value for step size, in Angstroms", stepSizeMax_, 0.01);
    keywords_.add<DoubleKeyword>("StepSizeMin", "Minimum allowed value for step size, in Angstroms", stepSizeMin_, 0.001);

    keywords_.setOrganisation("Advanced");
    keywords_.add<OptionalDoubleKeyword>(
        "CutoffDistance", "Interatomic cutoff distance to use for energy calculation (0.0 to use pair potential range)",
        cutoffDistance_, 0.0, std::nullopt, 0.1, "Use PairPotential Range");
}
