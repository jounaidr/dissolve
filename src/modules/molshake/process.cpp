/*
	*** MolShake Module - Processing
	*** src/modules/molshake/process.cpp
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

#include "modules/molshake/molshake.h"
#include "main/duq.h"
#include "modules/energy/energy.h"
#include "classes/box.h"
#include "classes/cell.h"
#include "classes/changestore.h"
#include "classes/configuration.h"
#include "classes/scaledenergykernel.h"
#include "classes/moleculedistributor.h"
#include "base/processpool.h"
#include "base/timer.h"
#include "templates/genericlisthelper.h"

// Run pre-processing stage
bool MolShakeModule::preProcess(DUQ& duq, ProcessPool& procPool)
{
	return false;
}

// Run main processing
bool MolShakeModule::process(DUQ& duq, ProcessPool& procPool)
{
	/*
	 * Perform a Molecule shake
	 * 
	 * This is a parallel routine, with processes operating as groups.
	 */

	// Check for zero Configuration targets
	if (targetConfigurations_.nItems() == 0)
	{
		Messenger::warn("No Configuration targets for Module.\n");
		return true;
	}

	// Loop over target Configurations
	for (RefListItem<Configuration,bool>* ri = targetConfigurations_.first(); ri != NULL; ri = ri->next)
	{
		// Grab Configuration pointer
		Configuration* cfg = ri->item;

		// Set up process pool - must do this to ensure we are using all available processes
		procPool.assignProcessesToGroups(cfg->processPool());

		// Get reference to relevant module data
		GenericList& moduleData = configurationLocal_ ? cfg->moduleData() : duq.processingModuleData();

		// Retrieve control parameters from Configuration
		double cutoffDistance = keywords_.asDouble("CutoffDistance");
		if (cutoffDistance < 0.0) cutoffDistance = duq.pairPotentialRange();
		double rotationStepSize = GenericListHelper<double>::retrieve(moduleData, "RotationStepSize", uniqueName(), keywords_.asDouble("RotationStepSize"));
		const double rotationStepSizeMax = keywords_.asDouble("RotationStepSizeMax");
		const double rotationStepSizeMin = keywords_.asDouble("RotationStepSizeMin");
		const int nShakesPerMolecule = keywords_.asInt("ShakesPerMolecule");
		double sizeFactor = GenericListHelper<double>::retrieve(moduleData, "SizeFactor", uniqueName(), keywords_.asDouble("SizeFactor"));
		const double targetAcceptanceRate = keywords_.asDouble("TargetAcceptanceRate");
		double translationStepSize = GenericListHelper<double>::retrieve(moduleData, "TranslationStepSize", uniqueName(), keywords_.asDouble("TranslationStepSize"));
		const double translationStepSizeMax = keywords_.asDouble("TranslationStepSizeMax");
		const double translationStepSizeMin = keywords_.asDouble("TranslationStepSizeMin");
		const double rRT = 1.0/(.008314472*cfg->temperature());

		// Print argument/parameter summary
		Messenger::print("MolShake: Cutoff distance is %f.\n", cutoffDistance);
		Messenger::print("MolShake: Performing %i shake(s) per Molecule.\n", nShakesPerMolecule);
		if (sizeFactor < 1.0) Messenger::print("MolShake: Size factor is %f.\n", sizeFactor);
		Messenger::print("MolShake: Step size for translation adjustments is %f Angstroms (allowed range is %f <= delta <= %f).\n", translationStepSize, translationStepSizeMin, translationStepSizeMax);
		Messenger::print("MolShake: Step size for rotation adjustments is %f degrees (allowed range is %f <= delta <= %f).\n", rotationStepSize, rotationStepSizeMin, rotationStepSizeMax);

		ProcessPool::DivisionStrategy strategy = procPool.bestStrategy();

		// Create a Molecule distributor
		DynamicArray<Molecule>& moleculeArray = cfg->molecules();
		MoleculeDistributor distributor(moleculeArray, cfg->cells(), procPool, strategy, false);

		// Create a local ChangeStore and a suitable EnergyKernel
		ChangeStore changeStore(procPool);
		EnergyKernel normalKernel(procPool, cfg, duq.potentialMap(), cutoffDistance);
		ScaledEnergyKernel scaledKernel(sizeFactor, procPool, cfg, duq.potentialMap(), cutoffDistance);
		EnergyKernel& kernel = sizeFactor < 1.0 ? scaledKernel : normalKernel;

		// Initialise the random number buffer
		procPool.initialiseRandomBuffer(ProcessPool::subDivisionStrategy(strategy));

		int shake, nRotationAttempts = 0, nTranslationAttempts = 0, nRotationsAccepted = 0, nTranslationsAccepted = 0, nGeneralAttempts = 0;
		bool accept;
		double currentEnergy, newEnergy, delta, totalDelta = 0.0;
		Matrix3 transform;
		Vec3<double> rDelta;
		const Box* box = cfg->box();

		/*
		 * In order to be able to adjust translation and rotational steps independently, we will perform 80% of moves including both a translation
		 * a rotation, 10% using only translations, and 10% using only rotations.
		 */

		int molId;

		// Set initial random offset for our counter determining whether to perform R+T, R, or T.
		int count = procPool.random()*10;
		bool rotate, translate, changesBroadcastRequired;

		Timer timer;
		procPool.resetAccumulatedTime();
		while (molId = distributor.nextAvailableObject(changesBroadcastRequired), molId != Distributor::AllComplete)
		{
			// Upkeep - Do we need to broadcast changes before we begin the calculation?
			if (changesBroadcastRequired)
			{
				changeStore.distributeAndApply(cfg);
				changeStore.reset();
			}

			// Check for valid molecule
			if (molId == Distributor::NoneAvailable)
			{
				distributor.finishedWithObject();
				continue;
			}

			/*
			 * Calculation Begins
			 */

			// Get Molecule pointer
			Molecule* mol = cfg->molecule(molId);

			Messenger::printVerbose("MolShake: Molecule %i now the target on process %s and contains %i Atoms.\n", molId, procPool.processInfo(), mol->nAtoms());

			// Set current atom targets in ChangeStore (whole molecule)
			changeStore.add(mol);

			// Calculate reference energy for Molecule, including intramolecular terms
			currentEnergy = kernel.energy(mol, ProcessPool::subDivisionStrategy(strategy), true);

			// Loop over number of shakes per atom
			for (shake=0; shake<nShakesPerMolecule; ++shake)
			{
				// Determine what move(s) will we attempt
				if (count == 0)
				{
					rotate = true;
					translate = false;
				}
				else if (count == 1)
				{
					rotate = false;
					translate = true;
				}
				else
				{
					rotate = true;
					translate = true;
				}

				// Create a random translation vector and apply it to the Molecule's centre
				if (translate)
				{
					rDelta.set(procPool.randomPlusMinusOne()*translationStepSize, procPool.randomPlusMinusOne()*translationStepSize, procPool.randomPlusMinusOne()*translationStepSize);
					mol->translate(rDelta);
				}

				// Create a random rotation matrix and apply it to the Molecule
				if (rotate)
				{
					transform.createRotationXY(procPool.randomPlusMinusOne()*rotationStepSize, procPool.randomPlusMinusOne()*rotationStepSize);
					mol->transform(box, transform);
				}

				// Calculate new energy
				newEnergy = kernel.energy(mol, ProcessPool::subDivisionStrategy(strategy), true);
				
				// Trial the transformed atom position
				delta = newEnergy - currentEnergy;
				accept = delta < 0 ? true : (procPool.random() < exp(-delta*rRT));

				if (accept)
				{
					// Accept new (current) position of target Atom
					changeStore.updateAll();
					currentEnergy = newEnergy;
					totalDelta += delta;
					if (rotate) ++nRotationsAccepted;
					if (translate) ++nTranslationsAccepted;
				}
				else changeStore.revertAll();

				// Increase attempt counters
				if (rotate) ++nRotationAttempts;
				if (translate) ++nTranslationAttempts;
				++nGeneralAttempts;

				// Increase and fold move type counter
				++count;
				if (count > 9) count = 0;
			}

			// Store modifications to Atom positions ready for broadcast
			changeStore.storeAndReset();

			/*
			* Calculation End
			*/

			// Tell the distributor we are done
			distributor.finishedWithObject();

		}

		// Make sure any remaining changes are broadcast
		changeStore.distributeAndApply(cfg);
		changeStore.reset();

		// Collect statistics across all processe
		if (!procPool.allSum(&totalDelta, 1, strategy)) return false;
		if (!procPool.allSum(&nGeneralAttempts, 1, strategy)) return false;
		if (!procPool.allSum(&nTranslationAttempts, 1, strategy)) return false;
		if (!procPool.allSum(&nTranslationsAccepted, 1, strategy)) return false;
		if (!procPool.allSum(&nRotationAttempts, 1, strategy)) return false;
		if (!procPool.allSum(&nRotationsAccepted, 1, strategy)) return false;

		timer.stop();

		Messenger::print("MolShake: Total energy delta was %10.4e kJ/mol.\n", totalDelta);

		// Calculate and print acceptance rates
		double transRate = double(nTranslationsAccepted)/nTranslationAttempts;
		double rotRate = double(nRotationsAccepted)/nRotationAttempts;
		Messenger::print("MolShake: Total number of attempted moves was %i (%s work, %s comms, %i nodists, %i broadcasts)\n", nGeneralAttempts, timer.totalTimeString(), procPool.accumulatedTimeString(), distributor.nUnavailableInstances(), distributor.nChangeBroadcastsRequired());
		Messenger::print("MolShake: Overall translation acceptance rate was %4.2f% (%i of %i attempted moves)\n", 100.0*transRate, nTranslationsAccepted, nTranslationAttempts);
		Messenger::print("MolShake: Overall rotation acceptance rate was %4.2f% (%i of %i attempted moves)\n", 100.0*rotRate, nRotationsAccepted, nRotationAttempts);

		// Update and set translation step size
		translationStepSize *= (nTranslationsAccepted == 0) ? 0.8 : transRate/targetAcceptanceRate;
		if (translationStepSize < translationStepSizeMin) translationStepSize = translationStepSizeMin;
		else if (translationStepSize > translationStepSizeMax) translationStepSize = translationStepSizeMax;

		Messenger::print("MolShake: Updated step size for translations is %f Angstroms.\n", translationStepSize); 
		GenericListHelper<double>::realise(moduleData, "TranslationStepSize", uniqueName(), GenericItem::InRestartFileFlag) = translationStepSize;

		// Update and set rotation step size
		rotationStepSize *= (nRotationsAccepted == 0) ? 0.8 : rotRate/targetAcceptanceRate;
		if (rotationStepSize < rotationStepSizeMin) rotationStepSize = rotationStepSizeMin;
		else if (rotationStepSize > rotationStepSizeMax) rotationStepSize = rotationStepSizeMax;

		Messenger::print("MolShake: Updated step size for rotations is %f degrees.\n", rotationStepSize); 
		GenericListHelper<double>::realise(moduleData, "RotationStepSize", uniqueName(), GenericItem::InRestartFileFlag) = rotationStepSize;

		// If sizeFactor is less than 1.0, check the total energy 
		if (sizeFactor < 1.0)
		{
			if (EnergyModule::interatomicEnergy(procPool, cfg, duq.potentialMap()) < 0.0)
			{
				sizeFactor *= 1.2;
				if (sizeFactor > 1.0) sizeFactor = 1.0;
				Messenger::print("MolShake: Total energy now negative, so increasing sizeFactor to %f.\n", sizeFactor);
				GenericListHelper<double>::realise(moduleData, "SizeFactor", uniqueName(), GenericItem::InRestartFileFlag) = sizeFactor;
			}
			else Messenger::print("MolShake: Total energy is positive, so sizeFactor will remain at %f.\n", sizeFactor);
		}

		// Increase coordinate index in Configuration
		if ((nRotationsAccepted > 0) || (nTranslationsAccepted > 0)) cfg->incrementCoordinateIndex();
	}

	return true;
}

// Run post-processing stage
bool MolShakeModule::postProcess(DUQ& duq, ProcessPool& procPool)
{
	return true;
}
