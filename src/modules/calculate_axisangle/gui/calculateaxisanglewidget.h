// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "modules/calculate_axisangle/gui/ui_calculateaxisanglewidget.h"
#include "modules/widget.h"

// Forward Declarations
class CalculateAxisAngleModule;
class DataViewer;

// Module Widget
class CalculateAxisAngleModuleWidget : public ModuleWidget
{
    // All Qt declarations derived from QObject must include this macro
    Q_OBJECT

    private:
    // Associated Module
    CalculateAxisAngleModule *module_;

    public:
    CalculateAxisAngleModuleWidget(QWidget *parent, CalculateAxisAngleModule *module, Dissolve &dissolve);

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::CalculateAxisAngleModuleWidget ui_;
    // DataViewers contained within this widget
    DataViewer *rdfGraph_, *angleGraph_, *dAngleGraph_;

    public:
    // Update controls within widget
    void updateControls(const Flags<ModuleWidget::UpdateFlags> &updateFlags = {}) override;
};
