// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "modules/calculate_sdf/sdf.h"

CalculateSDFModule::CalculateSDFModule() : Module(nRequiredTargets()), analyser_(ProcedureNode::AnalysisContext)
{
    // Set unique name for this instance of the Module
    static int instanceId = 0;
    uniqueName_ = fmt::format("{}{:02d}", type(), instanceId++);

    // Initialise Module - set up keywords etc.
    initialise();
}

CalculateSDFModule::~CalculateSDFModule() {}

/*
 * Instances
 */

// Create instance of this module
Module *CalculateSDFModule::createInstance() const { return new CalculateSDFModule; }
