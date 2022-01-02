// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "procedure/nodes/calculatebase.h"

// Forward Declarations
class SelectProcedureNode;

// Calculate Angle Node
class CalculateAngleProcedureNode : public CalculateProcedureNodeBase
{
    public:
    CalculateAngleProcedureNode(std::shared_ptr<SelectProcedureNode> site0 = nullptr,
                                std::shared_ptr<SelectProcedureNode> site1 = nullptr,
                                std::shared_ptr<SelectProcedureNode> site2 = nullptr);
    ~CalculateAngleProcedureNode() override = default;

    /*
     * Observable Target (implements virtuals in CalculateProcedureNodeBase)
     */
    public:
    // Return number of sites required to calculate observable
    int nSitesRequired() const override;
    // Return dimensionality of calculated observable
    int dimensionality() const override;

    /*
     * Execute
     */
    public:
    // Execute node, targetting the supplied Configuration
    bool execute(ProcessPool &procPool, Configuration *cfg, std::string_view prefix, GenericList &targetList) override;
};
