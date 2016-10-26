/*
	*** dUQ - PairPotentials
	*** src/main/pairpotentials.cpp
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

#include "main/duq.h"
#include "classes/atomtype.h"

// Set maximum distance for tabulated PairPotentials
void DUQ::setPairPotentialRange(double range)
{
	pairPotentialRange_ = range;
	pairPotentialRangeSquared_ = range*range;
}

// Return maximum distance for tabulated PairPotentials
double DUQ::pairPotentialRange() const
{
	return pairPotentialRange_;
}

// Return maximum squared distance for tabulated PairPotentials
double DUQ::pairPotentialRangeSquared() const
{
	return pairPotentialRangeSquared_;
}

// Set width of PairPotential tail over which to truncate
void DUQ::setPairPotentialTruncationWidth(double width)
{
	pairPotentialTruncationWidth_ = width;
}

// Return width of PairPotential tail over which to truncate
double DUQ::pairPotentialTruncationWidth() const
{
	return pairPotentialTruncationWidth_;
}

// Set delta to use in tabulations
void DUQ::setPairPotentialDelta(double delta)
{
	pairPotentialDelta_ = delta;
}

// Return delta to use in tabulations
double DUQ::pairPotentialDelta() const
{
	return pairPotentialDelta_;
}

// Add missing pair potentials to lists
bool DUQ::addMissingPairPotentials()
{
	// Double loop over defined AtomTypes - search for these potentials in the list
	PairPotential* pot;
	for (AtomType* at1 = atomTypes_.first(); at1 != NULL; at1 = at1->next)
	{
		for (AtomType* at2 = at1; at2 != NULL; at2 = at2->next)
		{
			// Search for existing potential
			if (hasPairPotential(at1, at2)) continue;
			
			// Not there, so create it
			pot = pairPotentials_.add();
			pot->setType(PairPotential::FullType);
			pot->setParameters(at1, at2);
			
			pot->generate(pairPotentialRange_, pairPotentialTruncationWidth_,  pairPotentialDelta_);
		}
	}

	// Finally, go through list and prune any potentials with unknown AtomTypes
	for (pot = pairPotentials_.first(); pot != NULL; pot = pot->next)
	{
		if ((!atomTypes_.contains(pot->atomTypeI())) || (!atomTypes_.contains(pot->atomTypeJ())))
		{ 
			Messenger::print("An existing Potential has been removed, since one (or both) of its original AtomTypes no longer exists.\n");
		}
	}
	
	return true;
}

// Remove specified PairPotential from the list
void DUQ::removePairPotential(PairPotential* pot)
{
	pairPotentials_.remove(pot);
}

// Return index of specified PairPotential
int DUQ::indexOf(PairPotential* pp)
{
	return pairPotentials_.indexOf(pp);
}

// Return number of defined PairPotentials
int DUQ::nPairPotentials() const
{
	return pairPotentials_.nItems();
}

// Add new pair potential to list
PairPotential* DUQ::addPairPotential()
{
	return pairPotentials_.add();
}

// Return first PaiPotential in list
PairPotential* DUQ::pairPotentials() const
{
	return pairPotentials_.first();
}

// Return nth PairPotential in list
PairPotential* DUQ::pairPotential(int n)
{
	return pairPotentials_[n];
}

// Return whether specified PairPotential is defined
PairPotential* DUQ::hasPairPotential(AtomType* at1, AtomType* at2) const
{
	for (PairPotential* pot = pairPotentials_.first(); pot != NULL; pot = pot->next)
	{
		if ((pot->atomTypeI() == at1) && (pot->atomTypeJ() == at2)) return pot;
		if ((pot->atomTypeI() == at2) && (pot->atomTypeJ() == at1)) return pot;
	}
	return NULL;
}

// Regenerate all currently-defined PairPotentials
void DUQ::regeneratePairPotentials()
{
	for (PairPotential* pot = pairPotentials_.first(); pot != NULL; pot = pot->next) pot->generate(pairPotentialRange_, pairPotentialTruncationWidth_, pairPotentialDelta_);
}

// Regenerate specific PairPotential
void DUQ::regeneratePairPotential(PairPotential* pp)
{
	pp->generate(pairPotentialRange_, pairPotentialTruncationWidth_, pairPotentialDelta_);
}

// Save all PairPotentials
bool DUQ::savePairPotentials(const char* baseName) const
{
	// I/O Operation so Master only.
	if (Comm.slave()) return true;
	bool result = true;

	for (PairPotential* pp = pairPotentials_.first(); pp != NULL; pp = pp->next)
	{
		// Generate filename
		Dnchar fileName(-1, "%s-%s-%s.pp", baseName, pp->atomTypeNameI(), pp->atomTypeNameJ());
		result = pp->save(fileName);
		if (!result) break;
	}
	
	return result;
}

// Return map for PairPotentials
const PotentialMap& DUQ::potentialMap()
{
	return potentialMap_;
}
