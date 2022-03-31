// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "modules/calculate_cn/gui/ui_calculatecnwidget.h"
#include "modules/widget.h"

// Forward Declarations
class CalculateCNModule;

// Module Widget
class CalculateCNModuleWidget : public ModuleWidget
{
    // All Qt declarations derived from QObject must include this macro
    Q_OBJECT

    public:
    CalculateCNModuleWidget(QWidget *parent, CalculateCNModule *module, Dissolve &dissolve);

    /*
     * Data
     */
    private:
    // Associated Module
    CalculateCNModule *module_;

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::CalculateCNModuleWidget ui_;
    // RDF data from which we are calculating the coordination number
    OptionalReferenceWrapper<const Data1D> rdfData_;
    // DataViewer for RDF plot
    DataViewer *rdfGraph_;

    public:
    // Update controls within widget
    void updateControls(const Flags<ModuleWidget::UpdateFlags> &updateFlags = {}) override;

    /*
     * Widgets / Functions
     */
    private:
    // Whether the RDF data to plot has been succesfully located
    bool rdfDataLocated_;
};