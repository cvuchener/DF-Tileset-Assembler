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
#include "TilemapInfo.h"

#include <QPixmap>

TilemapInfo::TilemapInfo(const QSize &tile_size, const QSize &tilemap_size)
        : tile_size(tile_size)
        , tilemap_size(tilemap_size)
{
}

TilemapInfo::TilemapInfo(const QPixmap &tileset, const QSize &tilemap_size)
        : tile_size(tileset.size().width()/tilemap_size.width(),
                    tileset.size().height()/tilemap_size.height())
        , tilemap_size(tilemap_size)
{
}


void TilemapInfo::setTileSize(const QSize &size)
{
	tile_size = size;
}

void TilemapInfo::setTileWidth(int width)
{
	tile_size.setWidth(width);
}

void TilemapInfo::setTileHeight(int height)
{
	tile_size.setHeight(height);
}

const QSize &TilemapInfo::tileSize() const
{
	return tile_size;
}

void TilemapInfo::setTilemapWidth(int width)
{
	tilemap_size.setWidth(width);
}

void TilemapInfo::setTilemapHeight(int height)
{
	tilemap_size.setHeight(height);
}

const QSize &TilemapInfo::tilemapSize() const
{
	return tilemap_size;
}

int TilemapInfo::tilemapWidth() const
{
	return tilemap_size.width();
}

int TilemapInfo::tilemapHeight() const
{
	return tilemap_size.height();
}

unsigned int TilemapInfo::tileCount() const
{
	return static_cast<unsigned int>(tilemap_size.width() * tilemap_size.height());
}

QRect TilemapInfo::tileRect(unsigned int index) const
{
	int x = static_cast<int>(index) % tilemap_size.width();
	int y = static_cast<int>(index) / tilemap_size.width();
	return QRect(QPoint(x*tile_size.width(), y*tile_size.height()), tile_size);
}

QSize TilemapInfo::pixmapSize() const
{
	return QSize(tile_size.width() * tilemap_size.width(),
	             tile_size.height() * tilemap_size.height());
}
