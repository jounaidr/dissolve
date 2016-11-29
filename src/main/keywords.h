/*
	*** Keyword Definitions
	*** src/main/keywords.h
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

#ifndef DUQ_KEYWORDS_H
#define DUQ_KEYWORDS_H

// Forward Declarations
class LineParser;
class DUQ;
class Sample;
class Configuration;
class Species;
class Module;

// Keyword Data
class KeywordData
{
	public:
	// Keyword name
	const char* name;
	// Number of arguments expected by keyword
	int nArguments;
	// Argument description
	const char* argumentDescription;
};

/*
 * Main Keyword Blocks
 */
namespace InputBlocks
{
	/*
	 * Input Block Keywords
	 */
	// Input File Block Keyword Enum
	enum InputBlock
	{
		AtomTypesBlock,			/* 'AtomTypes' - Contains definitions of AtomTypes over all Species */
		ConfigurationBlock,		/* 'Configuration' - Defines a single Configuration for use in the simulation */
		ModuleBlock,			/* 'Module' - Sets up a Module within a Configuration */
		PairPotentialsBlock,		/* 'PairPotentials' - Contains definitions of the PairPotentials for the simulation */
		SampleBlock,			/* 'Sample' - Begins a definition of a Sample */
		SimulationBlock,		/* 'Simulation' - Setting of simulation variables affecting the calculation */
		SpeciesBlock,			/* 'Species' - Begins a definition of a Species */
		nInputBlocks			/* Number of defined InputBlock keywords */
	};
	// Convert text string to InputBlock
	InputBlock inputBlock(const char* s);
	// Convert InputBlock to text string
	const char* inputBlock(InputBlock id);
	// Print list of valid keywords for InputBlock specified
	void printValidKeywords(InputBlock block);
};


/*
 * AtomTypes Block Keywords
 */
namespace AtomTypesBlock
{
	// AtomTypes Block Keyword Enum
	enum AtomTypesKeyword
	{
		AtomTypeKeyword,		/* 'AtomType' - Specifies an AtomType definition  */
		EndAtomTypesKeyword,		/* 'EndAtomTypes' - Signals the end of the AtomTypes block */
		nAtomTypesKeywords		/* Number of keywords defined for this block */
	};
	// Convert text string to AtomTypesKeyword
	AtomTypesKeyword keyword(const char* s);
	// Convert AtomTypesKeyword to text string
	const char* keyword(AtomTypesKeyword id);
	// Return expected number of expected arguments
	int nArguments(AtomTypesKeyword id);
	// Parse AtomTypes block
	bool parse(LineParser& parser, DUQ* duq);
};


/*
 * Configuration Block Keywords
 */
namespace ConfigurationBlock
{
	// Configuration Block Keyword Enum
	enum ConfigurationKeyword
	{
		BoxNormalisationFileKeyword,	/* 'BoxNormalisationFile' - Specifies a file from which to load the RDF normalisation array */
		BraggMaximumQKeyword,		/* 'BraggMaximumQ' - Sets the maximum Q value for Bragg calculation */
		CellAnglesKeyword,		/* 'CellAngles' - Gives the angles of the unit cell */
		CellLengthsKeyword,		/* 'CellLengths' - Gives the relative lengths of the unit cell */
		DensityKeyword,			/* 'Density' - Specifies the density of the simulation, along with its units */
		EndConfigurationKeyword,	/* 'EndConfiguration' - Signals the end of the Configuration block */
		InputCoordinatesKeyword,	/* 'InputCoordinates' - Specifies the file which contains the starting coordinates */
		ModuleKeyword,			/* 'Module' - Starts the setup of a Module for this configuration */
		MultiplierKeyword,		/* 'Multiplier' - Specifies the factor by which relative populations are multiplied when generating the Configuration data */
		NonPeriodicKeyword,		/* 'NonPeriodic' - States that the simulation should be treated as non-periodic */
		OutputCoordinatesKeyword,	/* 'OutputCoordinates' - Specifies the file which should contain output coordinates */
		RDFBinWidthKeyword,		/* 'RDFBinWidth' - Specified bin width for all RDF generation */
		RDFRangeKeyword,		/* 'RDFRange' - Requested extent for RDF calculation */
		SpeciesAddKeyword,		/* 'Species' - Specifies a Species and its relative population to add to this Configuration */
		TemperatureKeyword,		/* 'Temperature' - Defines the temperature of the simulation */
		UseOutputAsInputKeyword,	/* 'UseOutputAsInput' - "Use output coordinates file as input (if it exists) */
		nConfigurationKeywords		/* Number of keywords defined for this block */
	};
	// Convert text string to ConfigurationKeyword
	ConfigurationKeyword keyword(const char* s);
	// Convert ConfigurationKeyword to text string
	const char* keyword(ConfigurationKeyword id);
	// Return expected number of expected arguments
	int nArguments(ConfigurationKeyword id);
	// Parse Configuration block
	bool parse(LineParser& parser, DUQ* duq, Configuration* cfg);
};


/*
 * Module Block Keywords
 */
namespace ModuleBlock
{
	// Module Block Keyword Enum
	enum ModuleKeyword
	{
		ConfigurationKeyword,		/* 'Configuration' - Associates the specified Configuration to this Module */
		DisableKeyword,			/* 'Disable' - Disables the module, preventing it from running */
		EndModuleKeyword,		/* 'EndModule' - Signals the end of the Module block */
		FrequencyKeyword,		/* 'Frequency' - Frequency at which the Module is run */
		IsotopologueKeyword,		/* 'Isotopologue' - Sets the relative popupalation of a Species Isotopologue in a Configuration */
		nModuleKeywords			/* Number of keywords defined for this block */
	};
	// Convert text string to ModuleKeyword
	ModuleKeyword keyword(const char* s);
	// Convert ModuleKeyword to text string
	const char* keyword(ModuleKeyword id);
	// Return expected number of expected arguments
	int nArguments(ModuleKeyword id);
	// Parse Module block
	bool parse(LineParser& parser, DUQ* duq, Module* module, Configuration* cfg, Sample* sam);
};


/*
 * PairPotential Block Keywords
 */
namespace PairPotentialsBlock
{
	// PairPotential Block Keyword Enum
	enum PairPotentialsKeyword
	{
		CoulombKeyword,			/* 'Coulomb' - Specifies a PairPotential with Coulomb contributions only */
		DeltaKeyword,			/* 'Delta' - Gives the spacing between points in the tabulated potentials */
		DispersionKeyword,		/* 'Dispersion' - Specifies a PairPotential with LJ contributions only */
		EndPairPotentialsKeyword,	/* 'EndPairPotentials' - Signals the end of the PairPotentials block */
		FullKeyword,			/* 'Full' - Specifies a full PairPotential with LJ and charge contributions */
		RangeKeyword,			/* 'Range' - Specifies the total range (inc. truncation width) over which to generate potentials */
		TruncationWidthKeyword,		/* 'TruncationWidth' - Width of potential tail over which to reduce to zero */
		nPairPotentialsKeywords		/* Number of keywords defined for this block */
	};
	// Convert text string to PairPotentialKeyword
	PairPotentialsKeyword keyword(const char* s);
	// Convert PairPotentialsKeyword to text string
	const char* keyword(PairPotentialsKeyword id);
	// Return expected number of expected arguments
	int nArguments(PairPotentialsKeyword id);
	// Parse PairPotentials block
	bool parse(LineParser& parser, DUQ* duq);
};


/*
 * Sample Block Keywords
 */
namespace SampleBlock
{
	// Sample Block Keyword Enum
	enum SampleKeyword
	{
		EndSampleKeyword,		/* 'EndSample' - Signals the end of the Sample block */
		ModuleKeyword,			/* 'Module' - Begins a Module associated to this Sample */
		ReferenceDataKeyword,		/* 'ReferenceData' - Specifies a datafile which represents this Sample */
		nSampleKeywords			/* Number of keywords defined for this block */
	};
	// Convert text string to SampleKeyword
	SampleKeyword keyword(const char* s);
	// Convert SampleKeyword to text string
	const char* keyword(SampleKeyword id);
	// Return expected number of expected arguments
	int nArguments(SampleKeyword id);
	// Parse Sample block
	bool parse(LineParser& parser, DUQ* duq, Sample* sample);
};


/*
 * Simulation Block Keywords
 */
namespace SimulationBlock
{
	// Simulation Block Keyword Enum
	enum SimulationKeyword
	{
		BoxNormalisationPointsKeyword,	/* 'BoxNormalisationPoints' - Number of random insertions to use when generating the normalisation array */
		EndSimulationKeyword,		/* 'EndSimulation' - Signals the end of the Simulation block */
		MaxIterationsKeyword,		/* 'MaxIterations' - Maximum number of main loop iterations to perform, or -1 for no limit */
		ParallelStrategyKeyword,	/* 'ParallelStrategy' - Determines the distribution of processes across Configurations */
		SeedKeyword,			/* 'Seed' - Random seed to use */
		WindowFunctionKeyword,		/* 'WindowFunction' - Window function to use in all Fourier transforms */
		nSimulationKeywords		/* '' -  */
	};
	// Convert text string to SimulationKeyword
	SimulationKeyword keyword(const char* s);
	// Convert SimulationKeyword to text string
	const char* keyword(SimulationKeyword id);
	// Return expected number of expected arguments
	int nArguments(SimulationKeyword id);
	// Parse Simulation block
	bool parse(LineParser& parser, DUQ* duq);
};


/*
 * Species Block Keywords
 */
namespace SpeciesBlock
{
	// Species Block Keyword Enum
	enum SpeciesKeyword
	{
		AngleKeyword,			/* 'Angle' - Defines an Angle joining three atoms */
		AtomKeyword,			/* 'Atom' - Specifies an Atom in the Species */
		BondKeyword,			/* 'Bond' - Defines a Bond joining two atoms */
		EndSpeciesKeyword,		/* 'EndSpecies' - Signals the end of the current Species */
		GrainKeyword,			/* 'Grain' - Defines a Grain containing a number of Atoms */
		IsotopologueKeyword,		/* 'Isotopologue' - Defines isotope information for the Atoms */
		nSpeciesKeywords		/* Number of keywords defined for this block */
	};
	// Convert text string to SpeciesKeyword
	SpeciesKeyword keyword(const char* s);
	// Convert SpeciesKeyword to text string
	const char* keyword(SpeciesKeyword id);
	// Return expected number of expected arguments
	int nArguments(SpeciesKeyword id);
	// Parse Species block
	bool parse(LineParser& parser, DUQ* duq, Species* species);
};

#endif
