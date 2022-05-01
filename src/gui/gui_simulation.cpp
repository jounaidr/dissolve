// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2022 Team Dissolve and contributors

#include "gui/gui.h"
#include "main/dissolve.h"

// Disable editing
void DissolveWindow::preventEditing()
{
    // Disable necessary simulation menu items
    ui_.SimulationRunAction->setEnabled(false);
    ui_.SimulationStopAction->setEnabled(true);
    ui_.SimulationStepAction->setEnabled(false);
    ui_.SimulationStepFiveAction->setEnabled(false);
    ui_.SimulationSaveRestartPointAction->setEnabled(false);
    ui_.SimulationDataManagerAction->setEnabled(false);

    // Disable necessary menus
    ui_.SpeciesMenu->setEnabled(false);
    ui_.ConfigurationMenu->setEnabled(false);
    ui_.LayerMenu->setEnabled(false);

    // Disable editing in all tabs
    ui_.MainTabs->preventEditing();
}

// Allow editing
void DissolveWindow::allowEditing()
{
    // Enable necessary simulation menu items
    ui_.SimulationRunAction->setEnabled(true);
    ui_.SimulationStopAction->setEnabled(false);
    ui_.SimulationStepAction->setEnabled(true);
    ui_.SimulationStepFiveAction->setEnabled(true);
    ui_.SimulationSaveRestartPointAction->setEnabled(true);
    ui_.SimulationDataManagerAction->setEnabled(true);

    // Enable necessary menus
    ui_.SpeciesMenu->setEnabled(true);
    ui_.ConfigurationMenu->setEnabled(true);
    ui_.LayerMenu->setEnabled(true);

    // Allow editing in all tabs
    ui_.MainTabs->allowEditing();
}

// All iterations requested are complete
void DissolveWindow::iterationsComplete()
{
    allowEditing();
    Renderable::setSourceDataAccessEnabled(true);
    dissolveIterating_ = false;

    fullUpdate();
}

/*
 * Tabs
 */

void DissolveWindow::on_MainTabs_currentChanged(int index)
{
    if (refreshing_)
        return;

    // Retrieve the widget corresponding to the index provided - it will be a MainTab widget, from which all our tab widgets
    // are derived
    MainTab *currentTab = dynamic_cast<MainTab *>(ui_.MainTabs->widget(index));
    if (!currentTab)
    {
        Messenger::print("Can't cast this tab widget (index {}) into a MainTab.\n", index);
        return;
    }

    // Update contents
    currentTab->updateControls();

    // Update menu items
    updateMenus();
}

// Return whether the GUI is currently running a simulation
bool DissolveWindow::dissolveIterating() const { return dissolveIterating_; }

// Return list of all current tabs
const std::vector<MainTab *> DissolveWindow::allTabs() const { return ui_.MainTabs->allTabs(); }

void DissolveWindow::closeTab(QWidget *page)
{
    auto *tab = ui_.MainTabs->findTab(page);
    if (!tab)
        return;

    // Determine what we need to delete...
    if (tab->type() == MainTab::TabType::Configuration)
    {
        auto *cfg = dynamic_cast<ConfigurationTab *>(tab)->configuration();
        ui_.MainTabs->removeByPage(page);
        dissolve_.removeConfiguration(cfg);
        setModified({DissolveSignals::ConfigurationsMutated});
    }
    else if (tab->type() == MainTab::TabType::Layer)
    {
        auto *layerTab = dynamic_cast<LayerTab *>(tab);
        layerTab->removeModuleControlWidgets();
        ui_.MainTabs->removeByPage(page);
        dissolve_.removeProcessingLayer(layerTab->moduleLayer());
        setModified({DissolveSignals::ModulesMutated});
    }
    else if (tab->type() == MainTab::TabType::Species)
    {
        auto *sp = dynamic_cast<SpeciesTab *>(tab)->species();
        ui_.MainTabs->removeByPage(page);
        dissolve_.removeSpecies(sp);
        setModified();
    }
    else
        return;

    fullUpdate();
}
