/*
	*** Module Widget
	*** src/modules/partials/gui/modulewidget.h
	Copyright T. Youngs 2007-2017

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

#ifndef DUQ_PARTIALSMODULEWIDGET_H
#define DUQ_PARTIALSMODULEWIDGET_H

#include "modules/partials/gui/ui_modulewidget.h"
#include "gui/modulewidget.h"

// Forward Declarations
class DUQ;
class Module;
class PartialSet;
class UChromaViewWidget;

// Module Widget
class PartialsModuleWidget : public ModuleWidget
{
	// All Qt declarations derived from QObject must include this macro
	Q_OBJECT

	private:
	// Associated Module
	Module* module_;
	// UChromaView contained within this widget
	UChromaViewWidget* uChromaView_;
	// Reference to DUQ
	DUQ& dUQ_;

	public:
	// Constructor / Destructor
	PartialsModuleWidget(QWidget* parent, Module* module, DUQ& dUQ);
	~PartialsModuleWidget();
	// Main form declaration
	Ui::PartialsModuleWidget ui;
	// Update controls within widget
	void updateControls();
	// Data Type Enum
	enum DataType { FullData=1, BoundData=2, UnboundData=3, BraggData=4,
			UnweightedGRData=10, 
			WeightedGRData=20, 
			UnweightedSQData=30, 
			WeightedSQData=40};


	/*
	 * Widgets / Functions
	 */
	private:
	// Add data from PartialSet to tree
	void addPartialSetToTree(PartialSet& partials, QTreeWidgetItem* topLevelItem, PartialsModuleWidget::DataType rootType, const char* rootTitle);
	// Repopulate tree with source data
	void repopulateSourceTree();

	private slots:
};

#endif
