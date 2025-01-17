// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#pragma once

#include "base/units.h"
#include "procedure/nodeValue.h"
#include "procedure/nodes/node.h"

// Forward Declarations
class CoordinateSetsProcedureNode;
class Species;
class RegionProcedureNodeBase;

// Add Node
class AddProcedureNode : public ProcedureNode
{
    public:
    explicit AddProcedureNode(const Species *sp = nullptr, const NodeValue &population = 0, const NodeValue &density = 0.1,
                              Units::DensityUnits densityUnits = Units::AtomsPerAngstromUnits);
    explicit AddProcedureNode(std::shared_ptr<const CoordinateSetsProcedureNode> sets, const NodeValue &population = 0,
                              const NodeValue &density = 0.1, Units::DensityUnits densityUnits = Units::AtomsPerAngstromUnits);
    ~AddProcedureNode() override = default;

    private:
    // Set up keywords for node
    void setUpKeywords();

    /*
     * Identity
     */
    public:
    // Return whether a name for the node must be provided
    bool mustBeNamed() const override;

    /*
     * Node Data
     */
    public:
    // Box Action Style
    enum class BoxActionStyle
    {
        None,        /* Box geometry / volume will remain unchanged */
        AddVolume,   /* Increase Box volume to accommodate new species, according to supplied density */
        ScaleVolume, /* Scale current Box volume to give, after addition of the current species, the supplied density */
        Set          /* Set the Box geometry to that specified in the Species */
    };
    // Return enum option info for BoxActionStyle
    static EnumOptions<BoxActionStyle> boxActionStyles();
    // Positioning Type
    enum class PositioningType
    {
        Central, /* Position the Species at the centre of the Box */
        Current, /* Use current Species coordinates */
        Random,  /* Set position randomly */
        Region   /* Set position randomly within a specified region */
    };
    // Return enum option info for PositioningType
    static EnumOptions<PositioningType> positioningTypes();

    private:
    // The default box action if none is specified
    static constexpr AddProcedureNode::BoxActionStyle defaultBoxAction_ = AddProcedureNode::BoxActionStyle::AddVolume;
    // Action to take on the Box geometry / volume on addition of the species
    AddProcedureNode::BoxActionStyle boxAction_{defaultBoxAction_};
    // Coordinate set source for Species (if any)
    std::shared_ptr<const CoordinateSetsProcedureNode> coordinateSets_{nullptr};
    // Target density when adding molecules
    std::pair<NodeValue, Units::DensityUnits> density_{1.0, Units::GramsPerCentimetreCubedUnits};
    // Population of molecules to add
    NodeValue population_{1.0};
    // The default positioning if none is specified
    static constexpr AddProcedureNode::PositioningType defaultPositioningType_ = AddProcedureNode::PositioningType::Random;
    // Positioning type for individual molecules
    AddProcedureNode::PositioningType positioningType_{defaultPositioningType_};
    // Region into which we will add molecules (if any)
    std::shared_ptr<const RegionProcedureNodeBase> region_{nullptr};
    // Whether to rotate molecules on insertion
    bool rotate_{true};
    // The default scaling settings
    static constexpr bool defaultScale_{true};
    // iFlags controlling box axis scaling
    bool scaleA_{defaultScale_}, scaleB_{defaultScale_}, scaleC_{defaultScale_};
    // Species to be added
    const Species *species_{nullptr};

    /*
     * Execute
     */
    public:
    // Prepare any necessary data, ready for execution
    bool prepare(const ProcedureContext &procedureContext) override;
    // Execute node
    bool execute(const ProcedureContext &procedureContext) override;
};
