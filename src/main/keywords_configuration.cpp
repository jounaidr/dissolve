/*
	*** Keyword Parsing - Configuration Block
	*** src/main/keywords_configuration.cpp
	Copyright T. Youngs 2012-2016

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

#include "main/keywords.h"
#include "main/duq.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include "base/sysfunc.h"

// Configuration Block Keywords
KeywordData ConfigurationBlockData[] = {
	{ "BoxNormalisationFile", 	1,	"Specifies a file from which to load the RDF normalisation array" },
	{ "BraggMaximumQ",		1,	"Sets the maximum Q value for Bragg calculation" },
	{ "CellAngles", 		3,	"Gives the angles of the unit cell" },
	{ "CellLengths",		3,	"Gives the relative lengths of the unit cell" },
	{ "Density",			2,	"Specifies the density of the Configuration, along with its units" },
	{ "EndConfiguration",		0,	"Signals the end of the Configuration block" },
	{ "InputCoordinates",		1,	"Specifies the file which contains the starting coordinates" },
	{ "Module",			1,	"Starts the setup of a Module for this configuration" },
	{ "Multiplier",			1,	"Specifies the factor by which relative populations are multiplied when generating the Configuration data" },
	{ "NonPeriodic",		0,	"States that the simulation should be treated as non-periodic" },
	{ "OutputCoordinates",		1,	"Specifies the file which should contain output coordinates" },
	{ "RDFBinWidth",		1,	"Specified bin width for all radial distribution functions" },
	{ "RDFRange",			1,	"Requested extent for calculated radial distribution functions" },
	{ "Species",			2,	"Specifies a Species and its relative population to add to this Configuration" },
	{ "Temperature",		1,	"Defines the temperature of the Configuration" },
	{ "UseOutputAsInput",		0,	"Use output coordinates file as input (if it exists)" }
};

// Convert text string to ConfigurationKeyword
Keywords::ConfigurationKeyword Keywords::configurationKeyword(const char* s)
{
	for (int n=0; n<Keywords::nConfigurationKeywords; ++n) if (strcmp(s,ConfigurationBlockData[n].name) == 0) return (Keywords::ConfigurationKeyword) n;
	return Keywords::nConfigurationKeywords;
}

// Convert ConfigurationKeyword to text string
const char* Keywords::configurationKeyword(Keywords::ConfigurationKeyword id)
{
	return ConfigurationBlockData[id].name;
}

// Return minimum number of expected arguments
int Keywords::configurationBlockNArguments(Keywords::ConfigurationKeyword id)
{
	return ConfigurationBlockData[id].nArguments;
}

// Parse Configuration block
bool Keywords::parseConfigurationBlock(LineParser& parser, DUQ* duq, Configuration* cfg)
{
	Messenger::print("Parsing %s block\n", Keywords::inputBlock(Keywords::ConfigurationBlock));

	Sample* sam;
	Species* sp;
	Module* module;
	bool blockDone = false, error = false;

	while (!parser.eofOrBlank(duq->worldPool()))
	{
		// Read in a line, which should contain a keyword and a minimum number of arguments
		parser.getArgsDelim(duq->worldPool(), LineParser::SkipBlanks+LineParser::UseQuotes);
		Keywords::ConfigurationKeyword conKeyword = Keywords::configurationKeyword(parser.argc(0));
		if ((conKeyword != Keywords::nConfigurationKeywords) && ((parser.nArgs()-1) < Keywords::configurationBlockNArguments(conKeyword)))
		{
			Messenger::error("Not enough arguments given to '%s' keyword.\n", Keywords::configurationKeyword(conKeyword));
			error = true;
			break;
		}
		switch (conKeyword)
		{
			case (Keywords::BoxNormalisationFileKeyword):
				cfg->setBoxNormalisationFile(parser.argc(1));
				break;
			case (Keywords::BraggMaximumQKeyword):
				cfg->setBraggMaximumQ(parser.argd(1));
				break;
			case (Keywords::CellAnglesKeyword):
				cfg->setBoxAngles(Vec3<double>(parser.argd(1),  parser.argd(2), parser.argd(3)));
				break;
			case (Keywords::CellLengthsKeyword):
				cfg->setRelativeBoxLengths(Vec3<double>(parser.argd(1), parser.argd(2), parser.argd(3)));
				break;
			case (Keywords::DensityKeyword):
				// Determine units given
				if (strcmp(parser.argc(2),"atoms/A3") == 0) cfg->setAtomicDensity(parser.argd(1));
				else if (strcmp(parser.argc(2),"g/cm3") == 0) cfg->setChemicalDensity(parser.argd(1));
				else
				{
					Messenger::error("Unrecognised density unit given - '%s'\nValid values are 'atoms/A3' or 'g/cm3'.\n", parser.argc(2));
					error = true;
				}
				break;
			case (Keywords::EndConfigurationKeyword):
				Messenger::print("Found end of %s block.\n", Keywords::inputBlock(Keywords::ConfigurationBlock));
				blockDone = true;
				break;
			case (Keywords::InputCoordinatesKeyword):
				cfg->setInputCoordinatesFile(parser.argc(1));
				cfg->setRandomConfiguration(false);
				Messenger::print("--> Initial coordinates will be loaded from file '%s'\n", parser.argc(1));
				break;
			case (Keywords::ModuleAddKeyword):
				// The argument following the keyword is the module name
				module = duq->findModule(parser.argc(1));
				if (!module)
				{
					Messenger::error("No Module named '%s' exists.\n", parser.argc(1));
					error = true;
					break;
				}

				// Try to add this module (or an instance of it) to the current Configuration
				module = cfg->addModule(module);
				if (!module)
				{
					Messenger::error("Failed to add module '%s' to configuration.\n", parser.argc(1));
					error = true;
					break;
				}
				else
				{
					// Check dependencies of the new Module - loop over dependent Module names
					LineParser moduleParser;
					moduleParser.getArgsDelim(LineParser::Defaults, module->dependentModules());
					for (int n=0; n<moduleParser.nArgs(); ++n)
					{
						// First, make sure this is an actual Module name
						Module* dependentModule = duq->findModule(moduleParser.argc(n));
						if (!dependentModule)
						{
							Messenger::error("Module lists a dependent module '%s', but this Module doesn't exist.\n", moduleParser.argc(n));
							error = true;
							break;
						}

						// Find dependentModule in the previously-defined list of Modules for this Configuration
						Module* existingModule;
						RefListIterator<Module,bool> moduleIterator(cfg->modules());
						while (existingModule = moduleIterator.iterate()) if (DUQSys::sameString(existingModule->name(),dependentModule->name())) break;
						if (existingModule)
						{
							module->addDependentModule(existingModule);
							Messenger::print("Added dependent Module '%s' to Module '%s'.\n", existingModule->uniqueName(), module->uniqueName());
						}
						else
						{
							// No Module exists in the Configuration already
							Messenger::error("The Module '%s' requires the Module '%s' to run prior to it, but the '%s' Module has not been associated to the Configuration.\n", module->name(), dependentModule->name(), dependentModule->name());
							error = true;
							break;
						}
					}
				}
				if (error) break;

				// Parse rest of Module block
				if (!parseModuleBlock(parser, duq, module, cfg, NULL)) error = true;
				break;
			case (Keywords::MultiplierKeyword):
				cfg->setMultiplier(parser.argd(1));
				Messenger::print("--> Set Configuration contents multiplier to %i\n", cfg->multiplier());
				break;
			case (Keywords::NonPeriodicKeyword):
				cfg->setNonPeriodic(true);
				Messenger::print("--> Flag set for a non-periodic calculation.\n");
				break;
			case (Keywords::OutputCoordinatesKeyword):
				cfg->setOutputCoordinatesFile(parser.argc(1));
				Messenger::print("--> Output coordinates will be saved to file '%s'\n", parser.argc(1));
				break;
			case (Keywords::RDFBinWidthKeyword):
				cfg->setRDFBinWidth(parser.argd(1));
				break;
			case (Keywords::RDFRangeKeyword):
				cfg->setRequestedRDFRange(parser.argd(1));
				break;
			case (Keywords::SpeciesAddKeyword):
				sp = duq->findSpecies(parser.argc(1));
				if (sp == NULL)
				{
					Messenger::error("Configuration refers to Species '%s', but no such Species is defined.\n", parser.argc(1));
					error = true;
				}
				else
				{
					// Add this species to the list of those used by the Configuration
					if (!cfg->addUsedSpecies(sp, parser.argd(2)))
					{
						Messenger::error("Configuration already uses Species '%s' - cannot add it a second time.\n", sp->name());
						error = true;
					}
					else Messenger::print("--> Set composition for Species '%s' (%f relative population).\n", sp->name(), parser.argd(2));
				}
				break;
			case (Keywords::TemperatureKeyword):
				cfg->setTemperature(parser.argd(1));
				break;
			case (Keywords::UseOutputAsInputKeyword):
				cfg->setUseOutputCoordinatesAsInput(true);
				break;
			case (Keywords::nConfigurationKeywords):
				Messenger::error("Unrecognised %s block keyword found - '%s'\n", Keywords::inputBlock(Keywords::ConfigurationBlock), parser.argc(0));
				Keywords::printValidKeywords(Keywords::ConfigurationBlock);
				error = true;
				break;
			default:
				printf("DEV_OOPS - %s block keyword '%s' not accounted for.\n", Keywords::inputBlock(Keywords::ConfigurationBlock), Keywords::configurationKeyword(conKeyword));
				error = true;
				break;
		}
		
		// Error encountered?
		if (error) break;
		
		// End of block?
		if (blockDone) break;
	}

	return (!error);
}
