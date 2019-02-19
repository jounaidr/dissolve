/*
	*** Version Counter
	*** src/base/version.h
	Copyright T. Youngs 2012-2019

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISSOLVE_VERSIONCOUNTER_H
#define DISSOLVE_VERSIONCOUNTER_H

// Version Counter
class VersionCounter
{
	public:
	// Constructor / Destructor
	VersionCounter();
	~VersionCounter();


	/*
	 * Version Information
	 */
	private:
	// Version counter
	int version_;

	public:
	// Reset version counter to zero
	void zero();


	/*
	 * Operators
	 */
	public:
	// Automatic conversion to integer
	operator const int() const;
	// Prefix increment
	const int operator++();
};

#endif
