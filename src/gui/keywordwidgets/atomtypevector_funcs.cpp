// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "classes/atomtype.h"
#include "classes/atomtypemix.h"
#include "classes/coredata.h"
#include "gui/helpers/listwidgetupdater.h"
#include "gui/keywordwidgets/atomtypevector.h"
#include "gui/keywordwidgets/dropdown.h"
#include "templates/algorithms.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QString>

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<AtomType>)

AtomTypeVectorKeywordWidget::AtomTypeVectorKeywordWidget(QWidget *parent, KeywordBase *keyword, const CoreData &coreData)
    : KeywordDropDown(this), KeywordWidgetBase(coreData)
{
    // Create and set up the UI for our widget in the drop-down's widget container
    ui_.setupUi(dropWidget());
    ui_.AtomTypeList->setModel(&atomTypeModel_);

    // Connect signals / slots
    connect(&atomTypeModel_, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(modelDataChanged(const QModelIndex &, const QModelIndex &)));
    // Cast the pointer up into the parent class type
    keyword_ = dynamic_cast<AtomTypeVectorKeyword *>(keyword);
    if (!keyword_)
        Messenger::error("Couldn't cast base keyword '{}' into AtomTypeVectorKeyword.\n", keyword->name());
    atomTypeModel_.setCheckStateData(keyword_->data());
}

/*
 * Widgets
 */

void AtomTypeVectorKeywordWidget::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (refreshing_)
        return;

    keyword_->setAsModified();

    updateSummaryText();

    emit(keywordValueChanged(keyword_->optionMask()));
}

/*
 * Update
 */

// Update value displayed in widget
void AtomTypeVectorKeywordWidget::updateValue() { updateWidgetValues(coreData_); }

// Update widget values data based on keyword data
void AtomTypeVectorKeywordWidget::updateWidgetValues(const CoreData &coreData)
{
    refreshing_ = true;

    atomTypeModel_.setData(coreData.atomTypes());
    atomTypeModel_.setCheckStateData(keyword_->data());

    updateSummaryText();

    refreshing_ = false;
}

// Update keyword data based on widget values
void AtomTypeVectorKeywordWidget::updateKeywordData()
{
    // Handled by model
}

// Update summary text
void AtomTypeVectorKeywordWidget::updateSummaryText()
{
    if (keyword_->data().empty())
        setSummaryText("<None>");
    else
        setSummaryText(
            QString::fromStdString(joinStrings(keyword_->data(), ", ", [](const auto &at) { return at.get()->name(); })));
}