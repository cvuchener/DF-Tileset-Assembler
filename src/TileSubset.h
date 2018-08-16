/*
 * Copyright (C) 2018 Clément Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TILESUBSET_H
#define TILESUBSET_H

#include <vector>
#include <QString>

class TileSubset
{
public:
	TileSubset();

	void set(unsigned int min, unsigned int max, bool value = true);

	bool contains(unsigned int tile) const;
	unsigned int firstTile() const;

	static TileSubset fromString(const QString &);

private:
	std::vector<bool> _tiles;
};

#endif // TILESUBSET_H
