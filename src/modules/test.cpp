/*
	*** Test Module
	*** src/modules/test.cpp
	Copyright T. Youngs 2012-2017

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

#include "modules/test.h"
#include "main/duq.h"
#include "base/sysfunc.h"

// Static Members
List<Module> Test::instances_;

/*
 * Constructor / Destructor
 */

// Constructor
Test::Test() : Module()
{
	// Add to instances list and set unique name for this instance
	instances_.own(this);
	uniqueName_.sprintf("%s%02i", name(), instances_.nItems()-1);

	// Setup variables / control parameters
	options_.add("Test", false);
}

// Destructor
Test::~Test()
{
}

/*
 * Instances
 */

// Create instance of this module
List<Module>& Test::instances()
{
	return instances_;
}

// Create instance of this module
Module* Test::createInstance()
{
	return new Test;
}

/*
 * Definition
 */

// Return name of module
const char* Test::name()
{
	return "Test";
}

// Return brief description of module
const char* Test::brief()
{
	return "Test Module";
}

// Return instance type for module
Module::InstanceType Test::instanceType()
{
	return Module::MultipleInstance;
}

// Whether the Module has a pre-processing stage
bool Test::hasPreProcessing()
{
	return false;
}

// Whether the Module has a processing stage
bool Test::hasProcessing()
{
	return true;
}

// Whether the Module has a post-processing stage
bool Test::hasPostProcessing()
{
	return false;
}

// Modules upon which this Module depends to have run first
const char* Test::dependentModules()
{
	return "";
}

// Setup supplied dependent module (only if it has been auto-added)
bool Test::setupDependentModule(Module* depMod)
{
	return true;
}

// Parse keyword line, returning 1 on success, 0 for not recognised, and -1 for failed
int Test::parseKeyword(LineParser& parser, DUQ* duq, GenericList& targetList)
{
	if (DUQSys::sameString("AddSample", parser.argc(0)))
	{
		
	}
	else return 0;

	return 1;
}

/*
 * Targets
 */

// Return the maximum number of Configurations the Module can target (or -1 for any number)
int Test::nTargetableConfigurations()
{
	return 0;
}

/*
 * Method
 */

// Perform setup tasks for module
bool Test::setup(ProcessPool& procPool)
{
	return true;
}

// Execute pre-processing stage
bool Test::preProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}

// Execute Method
bool Test::process(DUQ& duq, ProcessPool& procPool)
{
	// First, check that we can find the total structure factors that have been specified
	sampleData_.clear();
	
	RefListIterator<Module,Data2D> sampleIterator(sampleData_);
	while (Module* module = sampleIterator.iterate())
	{
		// First, need to grab GenericList that will contain the target
		GenericList& moduleData = module->configurationLocal() ? module->targetConfigurations().firstItem()->moduleData() : duq.processingModuleData();
	}

	// Create weights matrix based on our defined 
	return true;
}

// Execute post-processing stage
bool Test::postProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}
