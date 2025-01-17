// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "gui/keywordWidgets/producers.h"
#include "gui/keywordWidgets/sectionHeader.h"
#include "gui/keywordWidgets/widget.hui"
#include "gui/signals.h"
#include "main/dissolve.h"
#include "module/module.h"
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>

KeywordsWidget::KeywordsWidget(QWidget *parent) : QScrollArea(parent)
{
    refreshing_ = false;

    setWidgetResizable(true);
}

/*
 * Controls
 */

// Set up controls for specified keywords
void KeywordsWidget::setUp(KeywordStore::KeywordStoreIndexInfo keywordIndexInfo,
                           const KeywordStore::KeywordStoreMap &keywordMap, const CoreData &coreData)
{
    keywordWidgets_.clear();

    // Get group map
    auto &groupMap = keywordMap.at(keywordIndexInfo.first);

    // Create a new QWidget and layout for the next group?
    auto *groupWidget = new QWidget(parentWidget());
    auto *groupLayout = new QGridLayout(groupWidget);
    auto row = 0;
    for (auto sectionName : keywordIndexInfo.second)
    {
        // Get keyword map
        auto &keywords = groupMap.at(sectionName);

        // Create a widget for the section name
        if (!sectionName.empty())
        {
            auto *sectionLabel = new SectionHeaderWidget(QString::fromStdString(std::string(sectionName)));

            if (row != 0)
                sectionLabel->setContentsMargins(0, 15, 0, 0);
            groupLayout->addWidget(sectionLabel, row++, 0, 1, 2);
        }

        // Loop over keywords in the group and add them to our groupbox
        for (auto *keyword : keywords)
        {
            // Try to create a suitable widget
            auto [widget, base] = KeywordWidgetProducer::create(keyword, coreData);
            if (!widget || !base)
            {
                fmt::print("No widget created for keyword '{}'.\n", keyword->name());
                continue;
            }

            // Connect signals
            connect(widget, SIGNAL(keywordDataChanged(int)), this, SLOT(keywordDataChanged(int)));

            // Put the widget in a horizontal layout with a stretch to absorb extra space
            auto *w = new QHBoxLayout;
            w->addWidget(widget, 0);
            w->addStretch(1);

            // Create a label and add it and the widget to our layout
            auto *nameLabel = new QLabel(QString::fromStdString(std::string(keyword->name())));
            nameLabel->setToolTip(QString::fromStdString(std::string(keyword->description())));
            nameLabel->setContentsMargins(10, 5, 0, 0);
            nameLabel->setAlignment(Qt::AlignTop);
            groupLayout->addWidget(nameLabel, row, 0);
            groupLayout->addLayout(w, row++, 1);

            // Push onto our reference list
            keywordWidgets_.push_back(base);
        }
    }

    // Add vertical spacer to the end of the group
    groupLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), row, 0);
    groupWidget->setLayout(groupLayout);
    setWidget(groupWidget);
}

// Create a suitable button for the named group
std::pair<QPushButton *, bool> KeywordsWidget::buttonForGroup(std::string_view groupName)
{
    // Create basic button
    auto *b = new QPushButton(QString::fromStdString(std::string(groupName)));
    b->setCheckable(true);
    b->setAutoExclusive(true);
    b->setFlat(true);

    const std::vector<std::tuple<std::string_view, QString, bool>> knownButtons = {
        {"Options", ":/general/icons/options.svg", false},
        {"Export", ":/general/icons/save.svg", false},
        {"Advanced", ":/general/icons/advanced.svg", true},
    };

    // Apply icons / alignment to recognised buttons
    bool alignRight = false;
    auto it = std::find_if(knownButtons.begin(), knownButtons.end(),
                           [groupName](const auto &btnData) { return std::get<0>(btnData) == groupName; });
    if (it != knownButtons.end())
    {
        b->setIcon(QIcon(std::get<1>(*it)));
        alignRight = std::get<2>(*it);
    }

    return {b, alignRight};
}

// Update controls within widget
void KeywordsWidget::updateControls(int dataMutationFlags)
{
    refreshing_ = true;

    Flags<DissolveSignals::DataMutations> mutationFlags(dataMutationFlags);

    // Update all our keyword widgets
    for (auto *keywordWidget : keywordWidgets_)
        keywordWidget->updateValue(mutationFlags);

    refreshing_ = false;
}

/*
 * Signals / Slots
 */

// Keyword data changed
void KeywordsWidget::keywordDataChanged(int keywordSignalMask) { emit(keywordChanged(keywordSignalMask)); }
