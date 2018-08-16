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
}

void TileSubset::set(unsigned int min, unsigned int max, bool value)
{
	if (max >= _tiles.size())
		_tiles.resize(max+1, false);
	std::fill(_tiles.begin() + min, _tiles.begin() + (max+1), value);
}

bool TileSubset::contains(unsigned int tile) const
{
	if (tile < _tiles.size())
		return _tiles[tile];
	return false;
}

unsigned int TileSubset::firstTile() const
{
	auto it = std::find(_tiles.begin(), _tiles.end(), true);
	return static_cast<unsigned int>(std::distance(_tiles.begin(), it));
}

static unsigned int read_tile(QStringRef str)
{
	bool ok;
	auto tile = str.toUInt(&ok, 0);
	if (!ok)
		throw std::runtime_error("invalid tile number");
	return tile;
}

TileSubset TileSubset::fromString(const QString &str)
{
	TileSubset subset;

	for (auto substr: str.splitRef(',')) {
		int sep_index = substr.indexOf('-');
		if (sep_index == -1) {
			auto tile = read_tile(substr);
			subset.set(tile, tile);
		}
		else {
			subset.set(read_tile(substr.left(sep_index)),
			           read_tile(substr.right(substr.size()-sep_index-1)));
		}
	}
	return subset;
}

