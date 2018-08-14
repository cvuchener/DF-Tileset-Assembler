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
#include "LayerComboBox.h"

#include "TilemapInfo.h"

#include <QtDebug>

LayerComboBox::LayerComboBox(const Tileset::layer_t &layer, QWidget *parent)
        : QComboBox(parent)
{
	for (unsigned int i = 0; i < layer.alternatives.size(); ++i) {
		const auto &alternative = layer.alternatives[i];
		if (!alternative.sources.empty()) {
			const auto &source = alternative.sources[alternative.icon_source].first;
			auto tile_rect = TilemapInfo(*source).tileRect(alternative.icon_tile);
			addItem(QIcon(source->copy(tile_rect)), alternative.name, i);
		}
		else
			addItem(alternative.name, i);
	}
}

void LayerComboBox::enterEvent(QEvent *event)
{
	QComboBox::enterEvent(event);
	emit mouseEnter();
}

void LayerComboBox::leaveEvent(QEvent *event)
{
	QComboBox::leaveEvent(event);
	emit mouseLeave();
}
