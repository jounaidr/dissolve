// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#pragma once

#include "math/range.h"
#include "procedure/nodes/node.h"
#include "procedure/nodes/sequence.h"
#include <memory>

// Forward Declarations
class Element;
class Molecule;
class SiteStack;
class Species;
class SpeciesSite;

// Iterate Selection Node
class IterateSelectionProcedureNode : public ProcedureNode
{
    public:
    explicit IterateSelectionProcedureNode(
        ProcedureNode::NodeContext forEachContext = ProcedureNode::NodeContext::AnalysisContext);

    /*
     * Parameters
     */
    private:
    // Pointers to individual parameters
    std::shared_ptr<ExpressionVariable> nSelectedParameter_, siteIndexParameter_, stackIndexParameter_, indexParameter_;
    // Selection to iterate over
    std::shared_ptr<const SelectProcedureNode> selection_;

    /*
     * Selected Sites
     */
    private:
    // Current Site index
    int currentSiteIndex_;
    // Number of selections made by the node
    int nSelections_;
    // Cumulative number of sites ever selected
    unsigned long int nCumulativeSites_;
    // Current site
    OptionalReferenceWrapper<const Site> currentSite_;

    /*
     * Branch
     */
    private:
    // Branch for ForEach
    ProcedureNodeSequence forEachBranch_;

    public:
    // Return the branch from this node (if it has one)
    OptionalReferenceWrapper<ProcedureNodeSequence> branch() override;

    /*
     * Execute
     */
    public:
    // Prepare any necessary data, ready for execution
    bool prepare(const ProcedureContext &procedureContext) override;
    // Execute node
    bool execute(const ProcedureContext &procedureContext) override;
    // Finalise any necessary data after execution
    bool finalise(const ProcedureContext &procedureContext) override;
};
