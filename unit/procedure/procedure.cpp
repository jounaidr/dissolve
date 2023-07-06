// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "procedure/procedure.h"
#include "keywords/node.h"
#include "procedure/nodes/add.h"
#include "procedure/nodes/box.h"
#include "procedure/nodes/parameters.h"
#include "procedure/nodes/select.h"
#include <gtest/gtest.h>
#include <string>

namespace UnitTest
{

TEST(ProcedureTest, Context)
{
    Procedure procedure(ProcedureNode::AnalysisContext);

    // Select A
    auto selectA = procedure.createRootNode<SelectProcedureNode>("A");
    auto &forEachA = selectA->branch()->get();
    EXPECT_TRUE(procedure.rootSequence().check());

    // Select B
    auto selectB = forEachA.create<SelectProcedureNode>("B");
    auto &forEachB = selectB->branch()->get();
    EXPECT_TRUE(procedure.rootSequence().check());

    // Wrong context type (in root node)
    EXPECT_THROW(procedure.createRootNode<BoxProcedureNode>("Box"), std::runtime_error);
    EXPECT_TRUE(procedure.rootSequence().check());

    // Wrong context type (in sequence branch)
    EXPECT_THROW(forEachB.create<BoxProcedureNode>("Box"), std::runtime_error);
    EXPECT_TRUE(procedure.rootSequence().check());

    // Node with same name as an existing one
    EXPECT_THROW(procedure.createRootNode<BoxProcedureNode>("A"), std::runtime_error);
}

TEST(ProcedureTest, Scope)
{
    Procedure procedure(ProcedureNode::AnalysisContext);

    // Select A
    auto selectA = procedure.createRootNode<SelectProcedureNode>("A");
    auto forEachA = selectA->branch()->get();
    EXPECT_TRUE(procedure.rootSequence().check());

    // Select B
    auto selectB = forEachA.create<SelectProcedureNode>("B");
    auto forEachB = selectB->branch()->get();
    EXPECT_TRUE(procedure.rootSequence().check());

    // Select C
    auto selectC = procedure.createRootNode<SelectProcedureNode>("C");

    // Node external to procedure
    auto selectExternal = std::make_shared<SelectProcedureNode>();
    selectExternal->setName("X");

    // Valid node keyword argument (in scope)
    EXPECT_TRUE(selectB->keywords().set("ExcludeSameMolecule", ConstNodeVector<SelectProcedureNode>{selectA}));
    EXPECT_TRUE(selectC->keywords().set("ReferenceSite", selectA));

    // Invalid node arguments (out of scope)
    // -- Out of scope (node appears subsequently in sequence)
    EXPECT_FALSE(selectA->keywords().set("ExcludeSameMolecule", ConstNodeVector<SelectProcedureNode>{selectB}));
    // -- Out of scope (node is in a branch in a previous node)
    EXPECT_FALSE(selectC->keywords().set("ReferenceSite", selectB));
    // -- Out of scope (not in same procedure)
    EXPECT_FALSE(selectB->keywords().set("ExcludeSameMolecule", ConstNodeVector<SelectProcedureNode>{selectExternal}));

    // Check removal of bad node references
    // -- Confirm current keyword data
    auto keywordNode = selectC->keywords()
                           .get<std::shared_ptr<const SelectProcedureNode>, NodeKeyword<SelectProcedureNode>>("ReferenceSite")
                           .value()
                           .get();
    EXPECT_EQ(keywordNode, selectA);
    // -- Move 'C' so it is before 'A'
    std::swap(procedure.rootSequence().sequence()[0], procedure.rootSequence().sequence()[1]);
    // -- Validate procedure and re-check keyword data
    procedure.rootSequence().validateNodeKeywords();
    keywordNode = selectC->keywords()
                      .get<std::shared_ptr<const SelectProcedureNode>, NodeKeyword<SelectProcedureNode>>("ReferenceSite")
                      .value()
                      .get();
    EXPECT_EQ(keywordNode, nullptr);
}

TEST(ProcedureTest, Parameters)
{
    Procedure procedure(ProcedureNode::GenerationContext);

    // Parameters (Small Things)
    auto smallThings = procedure.createRootNode<ParametersProcedureNode>("Small");
    smallThings->addParameter("Egg", 1);
    smallThings->addParameter("Marble", -5);
    smallThings->addParameter("Pin", 11.22);

    // Add (Small)
    auto smallHole = procedure.createRootNode<AddProcedureNode>("SmallHole");

    // Parameters (Big Things)
    auto largeThings = procedure.createRootNode<ParametersProcedureNode>("Large");
    largeThings->addParameter("Bus", 1);
    largeThings->addParameter("MeaningOfLife", 2.0);
    largeThings->addParameter("Family", 30);

    // Add (Big Things)
    auto bigHole = procedure.createRootNode<AddProcedureNode>("BigHole");

    EXPECT_TRUE(procedure.rootSequence().check());

    // Check number of available parameters to each Add node
    auto smallVars = smallHole->getParameters();
    EXPECT_EQ(smallVars.size(), 3);
    auto bigVars = bigHole->getParameters();
    EXPECT_EQ(bigVars.size(), 6);

    // Set an equation in A via a NodeValue
    NodeValue simple("20.0");
    NodeValue eggPinFromSmall("1 + Egg * Pin", smallVars), eggPinFromBig("1 + Egg * Pin", smallVars);
    NodeValue eggBus("Egg / Bus", bigVars), busMeaning("Bus + MeaningOfLife", bigVars);
    EXPECT_TRUE(simple.isValid() && eggPinFromSmall.isValid() && eggPinFromBig.isValid() && eggBus.isValid() &&
                busMeaning.isValid());
    EXPECT_TRUE(smallHole->keywords().set("Population", simple));
    EXPECT_TRUE(smallHole->keywords().set("Population", eggPinFromSmall));
    EXPECT_TRUE(smallHole->keywords().set("Population", eggPinFromBig));
    EXPECT_FALSE(smallHole->keywords().set("Population", eggBus));
    EXPECT_FALSE(smallHole->keywords().set("Population", busMeaning));
    EXPECT_TRUE(bigHole->keywords().set("Population", eggPinFromSmall));
    EXPECT_TRUE(bigHole->keywords().set("Population", eggPinFromBig));
    EXPECT_TRUE(bigHole->keywords().set("Population", eggBus));
    EXPECT_TRUE(bigHole->keywords().set("Population", busMeaning));
}

} // namespace UnitTest