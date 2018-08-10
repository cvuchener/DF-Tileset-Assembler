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
#ifndef TILEMAP_INFO_H
#define TILEMAP_INFO_H

#include <QRect>
#include <QSize>

class QPixmap;

class TilemapInfo
{
public:
	TilemapInfo(const QSize &tile_size = QSize(), const QSize &tilemap_size = QSize(16, 16));
	TilemapInfo(const QPixmap &tileset, const QSize &tilemap_size = QSize(16, 16));

	void setTileSize(const QSize &size);
	void setTileWidth(int width);
	void setTileHeight(int height);
	const QSize &tileSize() const;

	void setTilemapWidth(int width);
	void setTilemapHeight(int height);
	const QSize &tilemapSize() const;
	int tilemapWidth() const;
	int tilemapHeight() const;

	unsigned int tileCount() const;
	QRect tileRect(unsigned int index) const;
	QSize pixmapSize() const;

private:
	QSize tile_size;
	QSize tilemap_size;
};

#endif // TILEMAP_INFO_H
