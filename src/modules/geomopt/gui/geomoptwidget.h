// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "modules/geomopt/gui/ui_geomoptwidget.h"
#include "modules/widget.h"

// Forward Declarations
class GeometryOptimisationModule;

// Module Widget
class GeometryOptimisationModuleWidget : public ModuleWidget
{
    // All Qt declarations derived from QObject must include this macro
    Q_OBJECT

    private:
    // Associated Module
    GeometryOptimisationModule *module_;

    public:
    GeometryOptimisationModuleWidget(QWidget *parent, GeometryOptimisationModule *module, Dissolve &dissolve);

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::GeometryOptimisationModuleWidget ui_;
};