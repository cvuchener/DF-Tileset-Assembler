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
#include "Tileset.h"

#include <QFile>
#include <QPainter>

#include "FileLineReader.h"

#include <QtDebug>

static const std::map<QString, QPainter::CompositionMode, std::less<>> Modes = {
        { "SourceOver", QPainter::CompositionMode_SourceOver },
        { "DestinationOver", QPainter::CompositionMode_DestinationOver },
        { "Clear", QPainter::CompositionMode_Clear },
        { "Source", QPainter::CompositionMode_Source },
        { "Destination", QPainter::CompositionMode_Destination },
        { "SourceIn", QPainter::CompositionMode_SourceIn },
        { "DestinationIn", QPainter::CompositionMode_DestinationIn },
        { "SourceOut", QPainter::CompositionMode_SourceOut },
        { "DestinationOut", QPainter::CompositionMode_DestinationOut },
        { "SourceAtop", QPainter::CompositionMode_SourceAtop },
        { "DestinationAtop", QPainter::CompositionMode_DestinationAtop },
        { "Xor", QPainter::CompositionMode_Xor },
        { "Plus", QPainter::CompositionMode_Plus },
        { "Multiply", QPainter::CompositionMode_Multiply },
        { "Screen", QPainter::CompositionMode_Screen },
        { "Overlay", QPainter::CompositionMode_Overlay },
        { "Darken", QPainter::CompositionMode_Darken },
        { "Lighten", QPainter::CompositionMode_Lighten },
        { "ColorDodge", QPainter::CompositionMode_ColorDodge },
        { "ColorBurn", QPainter::CompositionMode_ColorBurn },
        { "HardLight", QPainter::CompositionMode_HardLight },
        { "SoftLight", QPainter::CompositionMode_SoftLight },
        { "Difference", QPainter::CompositionMode_Difference },
        { "Exclusion", QPainter::CompositionMode_Exclusion },
};

Tileset::Tileset(QSettings &s, QObject *parent)
        : QObject(parent)
{
	_output = s.value("output").toString();
	if (_output.isEmpty())
		qCritical().noquote() << tr("Missing output path in %1").arg(s.group());

	auto mode = s.value("mode", "Normal").toString();
	if (mode == "Normal")
		_mode = Mode::Normal;
	else if (mode == "TWBT")
		_mode = Mode::TWBT;
	else if (mode == "Creature")
		_mode = Mode::Creature;
	else
		qCritical().noquote() << tr("Invalid tileset mode in %1").arg(s.group());

	_info.setTileWidth(s.value("tile_width").toInt());
	_info.setTileHeight(s.value("tile_height").toInt());
	_info.setTilemapWidth(s.value("tileset_width", 16).toInt());
	_info.setTilemapHeight(s.value("tileset_height", 16).toInt());

	for (auto &tileset: _tileset)
		tileset = QPixmap(_info.pixmapSize());

	auto layer_count = static_cast<unsigned int>(s.beginReadArray("layers"));
	_layers.resize(layer_count);
	for (unsigned int i = 0; i < layer_count; ++i) {
		s.setArrayIndex(static_cast<int>(i));
		auto &layer = _layers[i];
		QFile layer_file(s.value("file").toString());
		if (!layer_file.open(QIODevice::ReadOnly)) {
			qCritical().noquote() << tr("Failed to open \"%1\".").arg(layer_file.fileName());
			continue;
		}
		FileLineReader reader(&layer_file);
		try {
			layer.tiles = TileSubset::fromString(reader.nextLine());
		}
		catch (std::exception &e) {
			qCritical().noquote() << reader.formatError(tr("Invalid tile list: %1").arg(e.what()));
		}
		unsigned int first_tile = layer.tiles.firstTile();
		layer_t::alternative_t *alternative = nullptr;
		while (reader) {
			auto line = reader.nextLine();
			auto params = line.splitRef(':');
			if (params[0] == "alternative") {
				layer.alternatives.emplace_back();
				alternative = &layer.alternatives.back();
				alternative->name = params.value(1).toString();
				alternative->icon_tile = first_tile;
				alternative->icon_source = 0;
			}
			else if (params[0] == "source") {
				if (!alternative) {
					qCritical().noquote() << reader.formatError(tr("\"source\" must be after an alternative"));
					continue;
				}
				auto filename = params.value(1);
				auto mode = QPainter::CompositionMode_Source;
				if (params.count() >= 3) {
					auto mode_it = Modes.find(params[2]);
					if (mode_it == Modes.end())
						qCritical().noquote() << reader.formatError(tr("Invalid composition mode: %1").arg(params[2]));
					else
						mode = mode_it->second;
				}
				alternative->sources.emplace_back(loadSourceTileset(filename.toString()), mode);
			}
			else if (params[0] == "icon") {
				if (!alternative) {
					qCritical().noquote() << reader.formatError(tr("\"icon\" must be after an alternative"));
					continue;
				}
				bool ok;
				if (params.count() >= 2) {
					alternative->icon_tile = params[1].toUInt(&ok);
					if (!ok || alternative->icon_tile > 255)
						qCritical().noquote() << reader.formatError(tr("Invalid icon tile number"));
				}
				if (params.count() >= 3) {
					alternative->icon_source = params[2].toUInt(&ok);
					if (!ok || alternative->icon_source >= alternative->sources.size())
						qCritical().noquote() << reader.formatError(tr("Invalid icon source index"));
				}
			}
			else {
				qCritical().noquote() << reader.formatError(tr("Invalid tilset option: %1").arg(params[0]));
			}

		}
		if (layer.alternatives.empty()) {
			qCritical().noquote() << tr("Layer %1 has no alternative.").arg(i);
			layer.alternatives.emplace_back(); // add an empty alternative so current can be a valid index
		}
		layer.current = 0;
	}
	s.endArray();

	buildTileset();
}

Tileset::Mode Tileset::mode() const
{
	return _mode;
}

const std::vector<Tileset::layer_t> &Tileset::layers() const
{
	return _layers;
}

void Tileset::selectAlternative(unsigned int layer_index, unsigned int alternative)
{
	if (layer_index >= _layers.size())
		return;
	auto &layer = _layers[layer_index];
	if (alternative >= layer.alternatives.size())
		return;
	layer.current = alternative;
	buildTileset();
}

const QPixmap &Tileset::pixmap(unsigned int layer) const
{
	assert(layer < PixmapCount);
	return _tileset[layer];
}

const TilemapInfo &Tileset::tilesetInfo() const
{
	return _info;
}

std::vector<QString> Tileset::outputs() const
{
	switch (_mode) {
	case Mode::Normal:
	case Mode::Creature:
		return { _output };
	case Mode::TWBT:
		return {
			TWBTFileName(_output, TWBTNormal),
			TWBTFileName(_output, TWBTBackground),
			TWBTFileName(_output, TWBTTop),
		};
	}
	Q_UNREACHABLE();
}

QString Tileset::TWBTFileName(QString name, Tileset::TWBTLayer layer)
{
	if (layer != TWBTNormal) {
		auto index = name.lastIndexOf('.');
		if (layer == TWBTBackground)
			name.insert(index, "-bg");
		else if (layer == TWBTTop)
			name.insert(index, "-top");
	}
	return name;
}

const Tileset::source_t *Tileset::loadSourceTileset(const QString &name)
{
	auto it = _sources.lower_bound(name);
	if (it == _sources.end() || it->first != name) {
		it = _sources.emplace_hint(it, std::piecewise_construct,
		                           std::forward_as_tuple(name),
		                           std::forward_as_tuple());
		std::vector<QString> filenames;
		switch (_mode) {
		case Mode::Normal:
		case Mode::Creature:
			filenames.push_back(name);
			break;
		case Mode::TWBT:
			filenames.push_back(TWBTFileName(name, TWBTNormal));
			filenames.push_back(TWBTFileName(name, TWBTBackground));
			filenames.push_back(TWBTFileName(name, TWBTTop));
			break;
		}
		for (unsigned int i = 0; i < filenames.size(); ++i) {
			const auto &filename = filenames[i];
			qDebug().noquote() << tr("Loading %1").arg(filename);
			if (!it->second[i].load(filename))
				qCritical().noquote() << tr("Failed to load source image from %1.").arg(filename);
		}
	}
	return &it->second;
}

void Tileset::buildTileset()
{
	QPainter painter;
	for (unsigned int i = 0; i < PixmapCount; ++i) {
		_tileset[i].fill(Qt::transparent);
		painter.begin(&_tileset[i]);
		for (const auto &layer: _layers) {
			for (unsigned int tile = 0; tile < _info.tileCount(); ++tile) {
				if (!layer.tiles.contains(tile))
					continue;
				const auto &current = layer.alternatives[layer.current];
				for (const auto &p: current.sources) {
					const auto &source = (*p.first)[i];
					if (source.isNull())
						continue;
					TilemapInfo source_info(source, _info.tilemapSize());
					painter.setCompositionMode(p.second);
					painter.drawPixmap(_info.tileRect(tile), source, source_info.tileRect(tile));
				}
			}
		}
		painter.end();
	}
	emit tilesetUpdated();
}

void Tileset::normal_render(QPainter &p, const QRect &dest,
                            const std::array<QPixmap, PixmapCount> &pixmaps,
                            unsigned int tile) const
{
	auto src_rect = _info.tileRect(tile);
	p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	p.drawPixmap(dest, pixmaps[0], src_rect);
}

void Tileset::normal_render(QPainter &p, const QRect &dest,
                            const std::array<QPixmap, PixmapCount> &pixmaps,
                            unsigned int tile,
                            const QColor &foreground, const QColor &background) const
{
	const auto &pixmap = pixmaps[0];
	auto src_rect = _info.tileRect(tile);
	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.drawPixmap(dest, pixmap, src_rect);
	p.setCompositionMode(QPainter::CompositionMode_Multiply);
	p.fillRect(dest, foreground);
	p.setCompositionMode(QPainter::CompositionMode_DestinationIn); // multiply has overwritten alpha channel?
	p.drawPixmap(dest, pixmap, src_rect);
	p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	p.fillRect(dest, background);
}

void Tileset::twbt_render(QPainter &p, const QRect &dest,
                          const std::array<QPixmap, PixmapCount> &pixmaps,
                          unsigned int tile) const
{
	auto src_rect = _info.tileRect(tile);
	p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	for (auto layer: { TWBTBackground, TWBTNormal, TWBTTop }) {
		p.drawPixmap(dest, pixmaps[layer], src_rect);
	}
}

void Tileset::twbt_render(QPainter &p, const QRect &dest,
                          const std::array<QPixmap, PixmapCount> &pixmaps,
                          unsigned int tile,
                          const QColor &foreground, const QColor &background) const
{
	QPainter temp_painter;
	QPixmap temp_pixmap(dest.size());
	temp_pixmap.fill(Qt::transparent);
	QRect rect = temp_pixmap.rect();
	auto src_rect = _info.tileRect(tile);
	for (const auto &t: { std::make_tuple(TWBTBackground, background),
	                      std::make_tuple(TWBTNormal, foreground)}) {
		const auto &pixmap = pixmaps[std::get<0>(t)];
		temp_painter.begin(&temp_pixmap);
		temp_painter.setCompositionMode(QPainter::CompositionMode_Source);
		temp_painter.drawPixmap(rect, pixmap, src_rect);
		temp_painter.setCompositionMode(QPainter::CompositionMode_Multiply);
		temp_painter.fillRect(rect, std::get<1>(t));
		temp_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn); // multiply has overwritten alpha channel?
		temp_painter.drawPixmap(rect, pixmap, src_rect);
		temp_painter.end();
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		p.drawPixmap(dest, temp_pixmap, rect);
	}
	p.drawPixmap(dest, pixmaps[TWBTTop], src_rect);
}
