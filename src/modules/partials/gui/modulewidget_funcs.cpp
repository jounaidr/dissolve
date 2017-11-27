/*
	*** Partials Module Widget - Functions
	*** src/modules/partials/gui/modulewidget_funcs.cpp
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

#include "modules/partials/gui/modulewidget.h"
#include "gui/uchroma/gui/uchromaview.h"
#include "main/duq.h"
#include "classes/atomtype.h"
#include "templates/variantpointer.h"

// Constructor
PartialsModuleWidget::PartialsModuleWidget(QWidget* parent, Module* module, DUQ& dUQ) : ModuleWidget(parent), module_(module), dUQ_(dUQ)
{
	// Set up user interface
	ui.setupUi(this);

	// Add a UChromaWidget to the PlotWidget
	QVBoxLayout* layout = new QVBoxLayout;
	uChromaView_ = new UChromaViewWidget;
	layout->addWidget(uChromaView_);
	ui.PlotWidget->setLayout(layout);

	// Start a new, empty session
	uChromaView_->startNewSession(true);

	// Set up the view pane
	ViewPane* viewPane = uChromaView_->currentViewPane();
	viewPane->setViewType(ViewPane::FlatXYView);
	viewPane->axes().setTitle(0, "\\it{r}, Angstroms");
	viewPane->axes().setMax(0, 10.0);
	viewPane->axes().setTitle(1, "g(r) / S(Q)");
	viewPane->axes().setMin(1, -1.0);
	viewPane->axes().setMax(1, 1.0);

	refreshing_ = false;
}

PartialsModuleWidget::~PartialsModuleWidget()
{
}

/*
 * Widgets / Functions
 */

// Add data from PartialSet to tree
void PartialsModuleWidget::addPartialSetToTree(PartialSet& partials, QTreeWidgetItem* topLevelItem, PartialsModuleWidget::DataType rootType, const char* rootTitle)
{
	// Create item to contain data
	QTreeWidgetItem* setItem = new QTreeWidgetItem(rootType);
	setItem->setText(0, rootTitle);
	setItem->setData(0, Qt::UserRole, VariantPointer<PartialSet>(&partials));

	if (topLevelItem) topLevelItem->addChild(setItem);
	else ui.SourcesTree->addTopLevelItem(setItem);

	// Loop over AtomType pairs
	AtomTypeData* atd1 = partials.atomTypes().first();
	for (int n=0; n<partials.atomTypes().nItems(); ++n, atd1 = atd1->next)
	{
		AtomTypeData* atd2 = atd1;
		for (int m=n; m<partials.atomTypes().nItems(); ++m, atd2 = atd2->next)
		{
			// Create item to contain full data
			QTreeWidgetItem* fullItem = new QTreeWidgetItem(setItem, rootType+PartialsModuleWidget::FullData);
			fullItem->setText(0, CharString("%s-%s", atd1->atomTypeName(), atd2->atomTypeName()).get());
			fullItem->setData(0, Qt::UserRole, VariantPointer<XYData>(&partials.partial(n,m)));

			// Add subitems with other data
			// -- Bound partial
			QTreeWidgetItem* subItem = new QTreeWidgetItem(fullItem, rootType+PartialsModuleWidget::BoundData);
			subItem->setText(0, "Bound");
			subItem->setData(0, Qt::UserRole, VariantPointer<XYData>(&partials.boundPartial(n,m)));
			// -- Unbound partial
			subItem = new QTreeWidgetItem(fullItem, rootType+PartialsModuleWidget::UnboundData);
			subItem->setText(0, "Unbound");
			subItem->setData(0, Qt::UserRole, VariantPointer<XYData>(&partials.unboundPartial(n,m)));
			// -- Bragg partial
			subItem = new QTreeWidgetItem(fullItem, rootType+PartialsModuleWidget::BraggData);
			subItem->setText(0, "Bragg");
			subItem->setData(0, Qt::UserRole, VariantPointer<XYData>(&partials.braggPartial(n,m)));
		}
	}

}

// Repopulate tree with source data
void PartialsModuleWidget::repopulateSourceTree()
{
	// Clear the tree
	ui.SourcesTree->clear();

	// Add on source data as appropriate - we will need to see what we can find in the relevant Module data
	GenericList& moduleData = module_->configurationLocal() ? module_->targetConfigurations().firstItem()->moduleData() : dUQ_.processingModuleData();

	QTreeWidgetItem* cfgItem, *setItem, *fullItem, *subItem;

	// Loop over Configurations targetted by Module
	RefListIterator<Configuration,bool> configIterator(module_->targetConfigurations());
	while (Configuration* cfg = configIterator.iterate())
	{
		// Add on an item for the Configuration, unless we are local to a configuration in which case we'll add items at the top level
		if (module_->configurationLocal())
		{
			cfgItem = new QTreeWidgetItem();
			cfgItem->setText(0, cfg->name());
			ui.SourcesTree->addTopLevelItem(cfgItem);
		}
		else cfgItem = NULL;

		// Unweighted partial g(r)
		if (cfg->moduleData().contains("UnweightedGR", "Partials"))
		{
			// Get the PartialSet
			PartialSet& unweightedgr = GenericListHelper<PartialSet>::retrieve(cfg->moduleData(), "UnweightedGR", "Partials");

			addPartialSetToTree(unweightedgr, cfgItem, PartialsModuleWidget::UnweightedGRData, "Unweighted g(r)");
		}

		// Unweighted partial S(Q)
		if (cfg->moduleData().contains("UnweightedSQ", "Partials"))
		{
			// Get the PartialSet
			PartialSet& unweightedsq = GenericListHelper<PartialSet>::retrieve(cfg->moduleData(), "UnweightedSQ", "Partials");

			addPartialSetToTree(unweightedsq, cfgItem, PartialsModuleWidget::UnweightedSQData, "Unweighted S(Q)");
		}
	}

}
