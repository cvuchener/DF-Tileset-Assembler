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

#include <array>
#include <QString>

class TileSubset
{
public:
	using container_t = std::array<bool, 256>;

	TileSubset();

	void set(uint8_t min, uint8_t max, bool value = true);

	const container_t &tiles() const;

	static TileSubset fromString(const QString &);

private:
	container_t _tiles;
};

#endif // TILESUBSET_H
