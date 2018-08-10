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
#ifndef TILESET_H
#define TILESET_H

#include <QObject>

#include <QPainter>
#include <QPixmap>
#include <QSettings>

#include "TilemapInfo.h"
#include "TileSubset.h"

class Tileset: public QObject
{
	Q_OBJECT
public:
	Tileset(QSettings &s, QObject *parent = nullptr);

	struct layer_t {
		TileSubset tiles;
		struct alternative_t {
			QString name;
			std::vector<std::pair<const QPixmap *, QPainter::CompositionMode>> sources;
			unsigned int icon_tile, icon_source;
		};
		std::vector<alternative_t> alternatives;
		unsigned int current;
	};

	const std::vector<layer_t> &layers() const;

	void selectAlternative(unsigned int layer, unsigned int alternative);

	const QPixmap &tileset() const;
	const TilemapInfo &tilesetInfo() const;

signals:
	void tilesetUpdated();

private:
	void buildTileset();

	std::vector<layer_t> _layers;
	std::map<QString, QPixmap, std::less<>> _input_tiles;
	TilemapInfo _info;
	QPixmap _tileset;
};

#endif // TILESET_H
