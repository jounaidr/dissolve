// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "procedure/nodes/calculatedistance.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include "procedure/nodes/select.h"

CalculateDistanceProcedureNode::CalculateDistanceProcedureNode(SelectProcedureNode *site0, SelectProcedureNode *site1)
    : CalculateProcedureNodeBase(ProcedureNode::CalculateDistanceNode, site0, site1)
{
    // Create keywords - store the pointers to the superclasses for later use
    siteKeywords_[0] = new NodeKeyword<SelectProcedureNode>(this, ProcedureNode::SelectNode, true, site0);
    keywords_.add("Control", siteKeywords_[0], "I", "Site that represents 'i' in the distance i-j");
    siteKeywords_[1] = new NodeKeyword<SelectProcedureNode>(this, ProcedureNode::SelectNode, true, site1);
    keywords_.add("Control", siteKeywords_[1], "J", "Site that represents 'j' in the distance i-j");
}

CalculateDistanceProcedureNode::~CalculateDistanceProcedureNode() {}

/*
 * Observable Target
 */

// Return number of sites required to calculate observable
int CalculateDistanceProcedureNode::nSitesRequired() const { return 2; }

// Return dimensionality of calculated observable
int CalculateDistanceProcedureNode::dimensionality() const { return 1; }

/*
 * Execute
 */

// Execute node, targetting the supplied Configuration
ProcedureNode::NodeExecutionResult CalculateDistanceProcedureNode::execute(ProcessPool &procPool, Configuration *cfg,
                                                                           std::string_view prefix, GenericList &targetList)
{
#ifdef CHECKS
    for (auto n = 0; n < nSitesRequired(); ++n)
    {
        if (sites_[n]->currentSite() == nullptr)
        {
            Messenger::error("Observable {} has no current site.\n", n);
            return ProcedureNode::Failure;
        }
    }
#endif
    // Determine the value of the observable
    value_.x = cfg->box()->minimumDistance(sites_[0]->currentSite()->origin(), sites_[1]->currentSite()->origin());

    return ProcedureNode::Success;
}
