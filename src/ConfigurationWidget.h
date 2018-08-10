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
#ifndef CONFIGURATION_WIDGET_H
#define CONFIGURATION_WIDGET_H

#include <QWidget>

class QFormLayout;
class QSettings;
class Tileset;
class TileSubset;

class ConfigurationWidget: public QWidget
{
	Q_OBJECT
public:
	explicit ConfigurationWidget(QSettings &s, Tileset *tileset, QWidget *parent = nullptr);

signals:
	void highlightTiles(const TileSubset &tiles);
	void clearHighlightedTiles();

public slots:

private:
	QFormLayout *_layout;
};

#endif // CONFIGURATION_WIDGET_H
