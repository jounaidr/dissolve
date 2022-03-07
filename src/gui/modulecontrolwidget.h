// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#pragma once

#include "base/lock.h"
#include "gui/ui_modulecontrolwidget.h"

// Forward Declarations
class ConfigurationVectorKeyword;
class ConfigurationVectorKeywordWidget;
class Dissolve;
class DissolveWindow;
class Module;
class ModuleWidget;
class ModuleBlock;

// Module Control Widget
class ModuleControlWidget : public QWidget
{
    // All Qt declarations derived from QObject must include this macro
    Q_OBJECT

    public:
    ModuleControlWidget(QWidget *parent = nullptr);
    ~ModuleControlWidget() = default;

    private:
    // Lock for widget refresh
    Lock refreshLock_;

    /*
     * Module Target
     */
    private:
    // Pointer to Dissolve
    Dissolve *dissolve_;
    // Associated Module
    Module *module_;

    public:
    // Set target Module to display
    void setModule(Module *module, Dissolve *dissolve);
    // Return target Module for the widget
    Module *module() const;

    /*
     * Update
     */
    public:
    // Update controls within widget
    void updateControls();
    // Disable sensitive controls
    void disableSensitiveControls();
    // Enable sensitive controls
    void enableSensitiveControls();

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::ModuleControlWidget ui_;
    // Reference vector of "Target" keywords
    std::vector<KeywordWidgetBase *> targetKeywordWidgets_;
    // Additional controls widget for the Module (if any)
    ModuleWidget *moduleWidget_;

    private slots:
    void on_ModuleControlsButton_clicked(bool checked);
    void on_ModuleOutputButton_clicked(bool checked);
    void on_EnabledButton_clicked(bool checked);
    void on_FrequencySpin_valueChanged(int value);
    void moduleKeywordChanged(int signalMask);

    signals:
    // Notify that the Module's data has been modified in some way
    void dataModified();
    // Notify that the active status of a module has changed
    void statusChanged();
};
