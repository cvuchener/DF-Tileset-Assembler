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

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSettings>

#include "Tileset.h"
#include "TileSubset.h"

#include <QtDebug>

ConfigurationWidget::ConfigurationWidget(QSettings &s, const std::vector<Tileset *> &tilesets, QWidget *parent)
        : QScrollArea(parent)
        , _layout(new QFormLayout(this))
        , _current_widget(nullptr)
{
	setWidgetResizable(true);
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	auto widget = new QWidget(this);

	bool ok;
	int config_size = s.beginReadArray("item");
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
		auto combobox = new QComboBox(this);
		for (unsigned int i = 0; i < layer.alternatives.size(); ++i) {
			const auto &alternative = layer.alternatives[i];
			auto icon = tileset->renderAlternativeIcon(alternative);
			if (icon.isNull())
				combobox->addItem(alternative.name, i);
			else
				combobox->addItem(QIcon(icon), alternative.name, i);
		}
		for (auto widget: { static_cast<QWidget *>(label), static_cast<QWidget *>(combobox) }) {
			widget->setMouseTracking(true);
			_highlights.emplace(std::piecewise_construct,
			                    std::forward_as_tuple(widget),
			                    std::forward_as_tuple(tileset_index, layer.tiles));
		}
		connect(combobox, qOverload<int>(&QComboBox::currentIndexChanged), [tileset, layer_index] (int index) {
			if (index >= 0)
				tileset->selectAlternative(layer_index, static_cast<unsigned>(index));
		});
		_layout->addRow(label, combobox);
	}
	s.endArray();

	widget->setLayout(_layout);
	widget->setMouseTracking(true);
	setWidget(widget);
	setMouseTracking(true);
	setMinimumWidth(widget->sizeHint().width() + verticalScrollBar()->width());
}

void ConfigurationWidget::mouseMoveEvent(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);

	auto pos = event->localPos();
	auto widget = childAt(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
	if (widget == _current_widget)
		return;
	else
		_current_widget = widget;
	auto it = _highlights.find(widget);
	if (it != _highlights.end())
		emit highlightTiles(it->second.first, it->second.second);
	else
		emit clearHighlightedTiles();
}
