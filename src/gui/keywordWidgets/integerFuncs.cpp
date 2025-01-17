// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "gui/helpers/mouseWheelAdjustmentGuard.h"
#include "gui/keywordWidgets/integer.hui"

IntegerKeywordWidget::IntegerKeywordWidget(QWidget *parent, IntegerKeyword *keyword, const CoreData &coreData)
    : QSpinBox(parent), KeywordWidgetBase(coreData), keyword_(keyword)
{
    // Set minimum and maximum values
    setRange(keyword_->minimumLimit() ? keyword_->minimumLimit().value() : -1e6,
             keyword_->maximumLimit() ? keyword_->maximumLimit().value() : 1e6);

    // Set current value
    setValue(keyword_->data());

    // Connect the
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(spinBoxValueChanged(int)));

    // Set event filtering so that we do not blindly accept mouse wheel events (problematic since we will exist in a
    // QScrollArea)
    installEventFilter(new MouseWheelWidgetAdjustmentGuard(this));
}

/*
 * Signals / Slots
 */

// Spin box value changed
void IntegerKeywordWidget::spinBoxValueChanged(int newValue)
{
    if (refreshing_)
        return;

    keyword_->setData(newValue);

    emit(keywordDataChanged(keyword_->editSignals()));
}

/*
 * Update
 */

// Update value displayed in widget
void IntegerKeywordWidget::updateValue(const Flags<DissolveSignals::DataMutations> &mutationFlags)
{
    refreshing_ = true;

    setValue(keyword_->data());

    refreshing_ = false;
}
