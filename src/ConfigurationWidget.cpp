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
#include "ConfigurationWidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QSettings>

#include "LayerComboBox.h"
#include "Tileset.h"
#include "TileSubset.h"

#include <QtDebug>

ConfigurationWidget::ConfigurationWidget(QSettings &s, const std::vector<Tileset *> &tilesets, QWidget *parent)
        : QWidget(parent)
        , _layout(new QFormLayout(this))
{
	bool ok;
	int config_size = s.beginReadArray("configuration");
	for (int i = 0; i < config_size; ++i) {
		s.setArrayIndex(i);
		auto name = s.value("name", tr("Unnamed setting")).toString();
		auto tileset_index = s.value("tileset", 1).toUInt(&ok) - 1;
		if (!ok || tileset_index >= tilesets.size()) {
			qCritical().noquote() << tr("Invalid tileset index in %1").arg(s.group());
			continue;
		}
		auto tileset = tilesets[tileset_index];
		auto layer_index = s.value("layer").toUInt(&ok) - 1;
		if (!ok || layer_index >= tileset->layers().size()) {
			qCritical().noquote() << tr("Invalid layer index in %1").arg(s.group());
			continue;
		}
		const auto &layer = tileset->layers()[layer_index];
		auto label = new QLabel(name, this);
		auto combobox = new LayerComboBox(layer, this);
		connect(combobox, &LayerComboBox::mouseEnter, [this, tileset_index, &layer] () {
			emit highlightTiles(tileset_index, layer.tiles);
		});
		connect(combobox, &LayerComboBox::mouseLeave, [this] () {
			emit clearHighlightedTiles();
		});
		connect(combobox, qOverload<int>(&QComboBox::currentIndexChanged), [tileset, layer_index] (int index) {
			if (index >= 0)
				tileset->selectAlternative(layer_index, static_cast<unsigned>(index));
		});
		_layout->addRow(label, combobox);
	}
	s.endArray();

	setLayout(_layout);
}
