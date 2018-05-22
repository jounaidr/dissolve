/*
	*** Module Tab
	*** src/gui/moduletab.h
	Copyright T. Youngs 2012-2018

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_MODULETAB_H
#define DUQ_MODULETAB_H

#include "gui/ui_moduletab.h"
#include "gui/maintab.h"
#include "gui/widgets/subwidget.h"

// Forward Declarations
class Module;

// Module Tab
class ModuleTab : public QWidget, public MainTab
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	public:
	// Constructor / Destructor
	ModuleTab(DUQWindow* duqWindow, DUQ& duq, QTabWidget* parent, Module* module, const char* title);
	~ModuleTab();
	// Main form declaration
	Ui::ModuleTab ui;


	/*
	 * Module Target
	 */
	public:
	// Module displayed in this tab
	Module* module_;


	/*
	 * SubWidget / SubWindow Handling
	 */
	public:
	// Return the tab's SubWindow area, if it has one
	QMdiArea* subWindowArea();
	// Return the tab's SubWidget layout, if it has one
	QLayout* subWidgetLayout();


	/*
	 * Update
	 */
	protected:
	// Update controls in tab
	void updateControls();
	// Disable sensitive controls within tab, ready for main code to run
	void disableSensitiveControls();
	// Enable sensitive controls within tab, ready for main code to run
	void enableSensitiveControls();


	/*
	 * State
	 */
	public:
	// Write widget state through specified LineParser
	bool writeState(LineParser& parser);

	/*
	 * Widget Functions
	 */
	public slots:
	void on_ToggleKeywordsButton_clicked(bool checked);
	void on_RunButton_clicked(bool checked);
	void on_EnabledButton_clicked(bool checked);

	signals:
	void moduleRun();
};

#endif
