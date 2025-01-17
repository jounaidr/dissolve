// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "classes/coreData.h"
#include "classes/species.h"
#include "gui/keywordWidgets/speciesVector.h"
#include "templates/algorithms.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QString>

SpeciesVectorKeywordWidget::SpeciesVectorKeywordWidget(QWidget *parent, SpeciesVectorKeyword *keyword, const CoreData &coreData)
    : KeywordDropDown(this), KeywordWidgetBase(coreData), keyword_(keyword)
{
    // Create and set up the UI for our widget in the drop-down's widget container
    ui_.setupUi(dropWidget());

    // Set up the model
    ui_.SpeciesList->setModel(&speciesModel_);
    speciesModel_.setCheckStateData(keyword_->data());
    resetModelData();

    // Connect signals / slots
    connect(&speciesModel_, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(modelDataChanged(const QModelIndex &, const QModelIndex &)));
}

/*
 * Widgets
 */

void SpeciesVectorKeywordWidget::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (refreshing_)
        return;

    updateSummaryText();

    emit(keywordDataChanged(keyword_->editSignals()));
}

/*
 * Update
 */

// Reset model data
void SpeciesVectorKeywordWidget::resetModelData()
{
    refreshing_ = true;

    speciesModel_.setData(coreData_.species());

    updateSummaryText();

    refreshing_ = false;
}

// Update value displayed in widget
void SpeciesVectorKeywordWidget::updateValue(const Flags<DissolveSignals::DataMutations> &mutationFlags)
{
    if (mutationFlags.isSet(DissolveSignals::SpeciesMutated))
        resetModelData();
}

// Update summary text
void SpeciesVectorKeywordWidget::updateSummaryText()
{
    if (keyword_->data().empty())
        setSummaryText("<None>");
    else
        setSummaryText(QString::fromStdString(joinStrings(keyword_->data(), ", ", [](const auto &sp) { return sp->name(); })));
}
