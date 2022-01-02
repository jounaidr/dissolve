// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#include "neta/NETAVisitor.h"
#include "data/ff/ff.h"
#include "neta/NETAErrorListeners.h"
#include "neta/bondcount.h"
#include "neta/character.h"
#include "neta/geometry.h"
#include "neta/hydrogencount.h"
#include "neta/or.h"
#include "neta/presence.h"
#include "neta/ring.h"
#include "templates/optionalref.h"

/*
 * Creation Entry Point
 */

// Return the topmost context in the stack
NETANode *NETAVisitor::currentContext()
{
    assert(!contextStack_.empty());

    return contextStack_.back().get();
}

// Construct description within supplied object, from given tree
void NETAVisitor::create(NETADefinition &neta, NETAParser::NetaContext *tree, const Forcefield *associatedForcefield)
{
    // Store pointer to definition and forcefield. and clear the passed definition
    neta_ = &neta;
    associatedForcefield_ = associatedForcefield;

    contextStack_.clear();
    contextStack_.emplace_back(neta_->rootNode());

    // Visit the root sequence generated by the tree
    if (tree->RootNodes)
        neta_->rootNode()->setNodes(visit(tree->RootNodes).as<NETANode::NETASequence>());
}

/*
 * Visitor Overrides
 */

antlrcpp::Any NETAVisitor::visitNeta(NETAParser::NetaContext *context) { return true; }

void NETAVisitor::setContextuals(std::vector<NETAParser::ModifierContext *> modifiers,
                                 std::vector<NETAParser::OptionContext *> options, std::vector<NETAParser::FlagContext *> flags)
{
    for (auto *modifier : modifiers)
        visitModifier(modifier, currentContext());
    for (auto *option : options)
        visitOption(option, currentContext());
    for (auto *flag : flags)
        visitFlag(flag, currentContext());
}

antlrcpp::Any NETAVisitor::visitOrSequence(NETAParser::OrSequenceContext *context)
{
    NETANode::NETASequence sequence;
    auto orNode = std::make_shared<NETAOrNode>(neta_);
    sequence.emplace_back(orNode);

    auto LHS = visit(context->LHS);
    orNode->setNodes(LHS.as<NETANode::NETASequence>());

    auto RHS = visit(context->RHS);
    orNode->setAlternativeNodes(RHS.as<NETANode::NETASequence>());

    return sequence;
}

antlrcpp::Any NETAVisitor::visitSequence(NETAParser::SequenceContext *context)
{
    setContextuals(context->Modifiers, context->Options, context->Flags);

    NETANode::NETASequence sequence;
    for (auto *node : context->Nodes)
        sequence.emplace_back(visit(node).as<std::shared_ptr<NETANode>>());
    return sequence;
}

antlrcpp::Any NETAVisitor::visitRingSequence(NETAParser::RingSequenceContext *context)
{
    setContextuals(context->Modifiers, context->Options, context->Flags);

    NETANode::NETASequence sequence;
    for (auto *node : context->Nodes)
        sequence.emplace_back(visit(node).as<std::shared_ptr<NETANode>>());
    return sequence;
}

antlrcpp::Any NETAVisitor::visitBondCountNode(NETAParser::BondCountNodeContext *context)
{
    auto bondCountNode = std::make_shared<NETABondCountNode>(neta_);

    // Check comparison operator
    if (!NETANode::comparisonOperators().isValid(context->comparisonOperator()->getText()))
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid comparison operator.\n", context->comparisonOperator()->getText())));
    NETANode::ComparisonOperator op = NETANode::comparisonOperators().enumeration(context->comparisonOperator()->getText());

    bondCountNode->set(op, std::stoi(context->Integer()->getText()));

    return std::dynamic_pointer_cast<NETANode>(bondCountNode);
}

antlrcpp::Any NETAVisitor::visitCharacterNode(NETAParser::CharacterNodeContext *context)
{
    auto characterNode = std::make_shared<NETACharacterNode>(neta_);

    // Set target elements / types
    visitTargetList(context->Targets, characterNode.get());

    // Reverse logic?
    if (context->Not())
        characterNode->setReverseLogic();

    return std::dynamic_pointer_cast<NETANode>(characterNode);
}

antlrcpp::Any NETAVisitor::visitConnectionNode(NETAParser::ConnectionNodeContext *context)
{
    auto connectionNode = std::make_shared<NETAConnectionNode>(neta_);

    // Set target elements / types
    visitTargetList(context->Targets, connectionNode.get());

    // Reverse logic?
    if (context->Not())
        connectionNode->setReverseLogic();

    // Bracketed node sequence?
    if (context->Sequence)
    {
        contextStack_.emplace_back(connectionNode);
        auto nodes = visit(context->Sequence);
        connectionNode->setNodes(nodes.as<NETANode::NETASequence>());
        contextStack_.pop_back();
    }

    return std::dynamic_pointer_cast<NETANode>(connectionNode);
}

antlrcpp::Any NETAVisitor::visitGeometryNode(NETAParser::GeometryNodeContext *context)
{
    auto geometryNode = std::make_shared<NETAGeometryNode>(neta_);

    // Check comparison operator and geometry
    if (!NETANode::comparisonOperators().isValid(context->EqualityOperator()->getText()))
        throw(NETAExceptions::NETASyntaxException(fmt::format("'{}' is not a valid comparison operator (for this context).\n",
                                                              context->EqualityOperator()->getText())));
    NETANode::ComparisonOperator op = NETANode::comparisonOperators().enumeration(context->EqualityOperator()->getText());
    if (!SpeciesAtom::geometries().isValid(context->geometry->getText()))
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid geometry type.\n", context->geometry->getText())));
    SpeciesAtom::AtomGeometry geom = SpeciesAtom::geometries().enumeration(context->geometry->getText());

    geometryNode->set(op, geom);

    return std::dynamic_pointer_cast<NETANode>(geometryNode);
}

antlrcpp::Any NETAVisitor::visitHydrogenCountNode(NETAParser::HydrogenCountNodeContext *context)
{
    auto hydrogenCountNode = std::make_shared<NETAHydrogenCountNode>(neta_);

    // Check comparison operator
    if (!NETANode::comparisonOperators().isValid(context->comparisonOperator()->getText()))
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid comparison operator.\n", context->comparisonOperator()->getText())));
    NETANode::ComparisonOperator op = NETANode::comparisonOperators().enumeration(context->comparisonOperator()->getText());

    hydrogenCountNode->set(op, std::stoi(context->Integer()->getText()));

    return std::dynamic_pointer_cast<NETANode>(hydrogenCountNode);
}

antlrcpp::Any NETAVisitor::visitPresenceNode(NETAParser::PresenceNodeContext *context)
{
    auto presenceNode = std::make_shared<NETAPresenceNode>(neta_);

    // Set target elements / types
    visitTargetList(context->Targets, presenceNode.get());

    // Reverse logic?
    if (context->Not())
        presenceNode->setReverseLogic();

    // Bracketed node sequence?
    if (context->Sequence)
    {
        contextStack_.emplace_back(presenceNode);
        auto nodes = visit(context->Sequence);
        presenceNode->setNodes(nodes.as<NETANode::NETASequence>());
        contextStack_.pop_back();
    }

    return std::dynamic_pointer_cast<NETANode>(presenceNode);
}

antlrcpp::Any NETAVisitor::visitRingNode(NETAParser::RingNodeContext *context)
{
    auto ringNode = std::make_shared<NETARingNode>(neta_);

    // Reverse logic?
    if (context->Not())
        ringNode->setReverseLogic();

    // Bracketed node sequence?
    if (context->Sequence)
    {
        contextStack_.emplace_back(ringNode);
        auto nodes = visit(context->Sequence);
        ringNode->setNodes(nodes.as<NETANode::NETASequence>());
        contextStack_.pop_back();
    }

    return std::dynamic_pointer_cast<NETANode>(ringNode);
}

antlrcpp::Any NETAVisitor::visitElementOrType(NETAParser::ElementOrTypeContext *context)
{
    if (context->Element())
    {
        auto el = Elements::element(context->Element()->getText());
        if (!el)
            throw(NETAExceptions::NETASyntaxException(
                fmt::format("'{}' is not a recognised element.\n", context->Element()->getText())));
        return el;
    }
    else if (context->FFTypeName())
    {
        // Is a forcefield available to search?
        if (!associatedForcefield_)
            throw(NETAExceptions::NETASyntaxException(
                "No associated forcefield, so can't use atom type targets in NETA definition."));

        // Search for the atom type with the name specified (after removing leading '&' from string)
        auto at = associatedForcefield_->atomTypeByName(context->FFTypeName()->getText().substr(1));
        if (!at)
            throw(NETAExceptions::NETASyntaxException(
                fmt::format("No forcefield atom type with name {} exists in forcefield '{}', so can't add it as a target.",
                            context->FFTypeName()->getText(), associatedForcefield_->name())));

        return std::reference_wrapper(*at);
    }
    else if (context->FFTypeIndex())
    {
        // Is a forcefield available to search?
        if (!associatedForcefield_)
            throw(NETAExceptions::NETASyntaxException(
                "No associated forcefield, so can't use atom type targets in NETA definition."));

        // Search for the atom type with the index specified (after removing leading '&' from string)
        auto id = std::stoi(context->FFTypeIndex()->getText().substr(1));
        auto at = associatedForcefield_->atomTypeById(id);
        if (!at)
            throw(NETAExceptions::NETASyntaxException(
                fmt::format("No forcefield atom type with id {} exists in forcefield '{}', so can't add it as a target.", id,
                            associatedForcefield_->name())));

        return std::reference_wrapper(*at);
    }

    throw(NETAExceptions::NETASyntaxException(
        fmt::format("'{]' is not an element symbol, type name, or type index.", context->getText())));
}

antlrcpp::Any NETAVisitor::visitTargetList(NETAParser::TargetListContext *context, NETANode *node)
{
    for (auto elementOrType : context->targets)
    {
        auto target = visitElementOrType(elementOrType);
        if (target.is<Elements::Element>())
        {
            auto Z = target.as<Elements::Element>();
            if (!node->addElementTarget(Z))
                throw(NETAExceptions::NETASyntaxException("Failed to add element to target list."));
        }
        else if (target.is<std::reference_wrapper<const ForcefieldAtomType>>())
        {
            auto atomTypeRef = target.as<std::reference_wrapper<const ForcefieldAtomType>>();
            if (!node->addFFTypeTarget(atomTypeRef.get()))
                throw(NETAExceptions::NETASyntaxException("Failed to add atom type to target list."));
        }
        else
            throw(NETAExceptions::NETASyntaxException("Unrecognised item in target list."));
    }

    return true;
}

void NETAVisitor::visitModifier(NETAParser::ModifierContext *context, NETANode *node)
{
    // Check comparison operator
    if (!NETANode::comparisonOperators().isValid(context->comparisonOperator()->getText()))
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid comparison operator.\n", context->comparisonOperator()->getText())));
    NETANode::ComparisonOperator op = NETANode::comparisonOperators().enumeration(context->comparisonOperator()->getText());
    if (node->isValidModifier(context->Keyword()->getText()))
    {
        if (!node->setModifier(context->Keyword()->getText(), op, std::stoi(context->value->getText())))
            throw(NETAExceptions::NETASyntaxException(fmt::format("Failed to set modifier '{}' for the current context ({}).",
                                                                  context->Keyword()->getText(),
                                                                  NETANode::nodeTypes().keyword(node->nodeType()))));
    }
    else
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid modifier keyword for the current context ({}).", context->Keyword()->getText(),
                        NETANode::nodeTypes().keyword(node->nodeType()))));
}

void NETAVisitor::visitOption(NETAParser::OptionContext *context, NETANode *node)
{
    // Check comparison operator - must be either '=' or '!='
    if (!NETANode::comparisonOperators().isValid(context->comparisonOperator()->getText()))
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid comparison operator.\n", context->comparisonOperator()->getText())));
    NETANode::ComparisonOperator op = NETANode::comparisonOperators().enumeration(context->comparisonOperator()->getText());
    if (op != NETANode::ComparisonOperator::EqualTo && op != NETANode::ComparisonOperator::NotEqualTo)
        throw(NETAExceptions::NETASyntaxException(fmt::format(
            "Option '{}' may only use the equal to ('=') or not equal to ('!=') operators.", context->opt->getText())));

    if (node->isValidOption(context->opt->getText()))
    {
        if (!node->setOption(context->opt->getText(), op, context->value->getText()))
            throw(NETAExceptions::NETASyntaxException(fmt::format("Failed to set option '{}' for the current context ({}).",
                                                                  context->opt->getText(),
                                                                  NETANode::nodeTypes().keyword(node->nodeType()))));
    }
    else
        throw(NETAExceptions::NETASyntaxException(
            fmt::format("'{}' is not a valid option keyword for the current context ({}).", context->opt->getText(),
                        NETANode::nodeTypes().keyword(node->nodeType()))));
}

void NETAVisitor::visitFlag(NETAParser::FlagContext *context, NETANode *node)
{
    if (node->isValidFlag(context->Keyword()->getText()))
    {
        if (!node->setFlag(context->Keyword()->getText(), true))
            throw(NETAExceptions::NETASyntaxException(fmt::format("Failed to set flag '{}' for the current context ({}).",
                                                                  context->Keyword()->getText(),
                                                                  NETANode::nodeTypes().keyword(node->nodeType()))));
    }
    else
        throw(NETAExceptions::NETASyntaxException(fmt::format("'{}' is not a valid flag for the current context ({}).",
                                                              context->Keyword()->getText(),
                                                              NETANode::nodeTypes().keyword(node->nodeType()))));
}
