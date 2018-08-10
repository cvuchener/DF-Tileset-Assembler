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
#include "TileSubset.h"

#include <QVector>

#include <QtDebug>

TileSubset::TileSubset()
{
	std::fill(_tiles.begin(), _tiles.end(), false);
}

void TileSubset::set(uint8_t min, uint8_t max, bool value)
{
	for (unsigned int i = min; i <= max; ++i)
		_tiles[i] = value;
}

const TileSubset::container_t &TileSubset::tiles() const
{
	return _tiles;
}

static uint8_t read_tile(QStringRef str)
{
	bool ok;
	int tile = str.toInt(&ok, 0);
	if (!ok || tile < std::numeric_limits<uint8_t>::min() ||
	    tile > std::numeric_limits<uint8_t>::max())
		throw std::runtime_error("invalid tile number");
	return static_cast<uint8_t>(tile);
}

TileSubset TileSubset::fromString(const QString &str)
{
	TileSubset subset;

	for (auto substr: str.splitRef(',')) {
		int sep_index = substr.indexOf('-');
		if (sep_index == -1) {
			uint8_t tile = read_tile(substr);
			subset.set(tile, tile);
		}
		else {
			subset.set(read_tile(substr.left(sep_index)),
			           read_tile(substr.right(substr.size()-sep_index-1)));
		}
	}
	return subset;
}

