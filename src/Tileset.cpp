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
	_info.setTileWidth(s.value("tile_width").toInt());
	_info.setTileHeight(s.value("tile_height").toInt());
	_tileset = QPixmap(_info.pixmapSize());
	auto layer_count = static_cast<unsigned int>(s.beginReadArray("layers"));
	_layers.resize(layer_count);
	for (unsigned int i = 0; i < layer_count; ++i) {
		s.setArrayIndex(static_cast<int>(i));
		auto &layer = _layers[i];
		QFile layer_file(s.value("file").toString());
		if (!layer_file.open(QIODevice::ReadOnly))
			throw std::runtime_error(tr("Failed to open \"%1\".").arg(layer_file.fileName()).toLocal8Bit().data());
		FileLineReader reader(layer_file);
		try {
			layer.tiles = TileSubset::fromString(reader.nextLine());
		}
		catch (std::exception &e) {
			throw reader.parseError(tr("Invalid tile list: %1").arg(e.what()));
		}
		unsigned int first_tile = 0;
		while (first_tile < 256 && !layer.tiles.tiles()[first_tile])
			++first_tile;
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
				if (!alternative)
					throw reader.parseError(tr("\"source\" must be after an alternative"));
				auto filename = params.value(1);
				auto it = _input_tiles.lower_bound(filename);
				if (it == _input_tiles.end() || it->first != filename) {
					qDebug() << "Loading" << filename;
					it = _input_tiles.emplace_hint(it, filename.toString(), QPixmap());
					if (!it->second.load(filename.toString()))
						throw reader.parseError(tr("Cannot load source image from %1.").arg(filename));
				}
				auto mode = QPainter::CompositionMode_Source;
				if (params.count() >= 3) {
					auto mode_it = Modes.find(params[2]);
					if (mode_it == Modes.end())
						throw reader.parseError(tr("Invalid composition mode: %1").arg(params[2]));
					mode = mode_it->second;
				}
				alternative->sources.emplace_back(&it->second, mode);
			}
			else if (params[0] == "icon") {
				if (!alternative)
					throw reader.parseError(tr("\"icon\" must be after an alternative"));
				bool ok;
				if (params.count() >= 2) {
					alternative->icon_tile = params[1].toUInt(&ok);
					if (!ok || alternative->icon_tile > 255)
						throw reader.parseError(tr("Invalid icon tile number"));
				}
				if (params.count() >= 3) {
					alternative->icon_source = params[2].toUInt(&ok);
					if (!ok || alternative->icon_source >= alternative->sources.size())
						throw reader.parseError(tr("Invalid icon source index"));
				}
			}
			else {
				throw reader.parseError(tr("Invalid tilset option: %1").arg(params[0]));
			}

		}
		if (layer.alternatives.empty())
			throw reader.parseError(tr("no alternative found"));
		layer.current = 0;
	}
	s.endArray();

	buildTileset();
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

const QPixmap &Tileset::tileset() const
{
	return _tileset;
}

const TilemapInfo &Tileset::tilesetInfo() const
{
	return _info;
}

void Tileset::buildTileset()
{
	QPainter painter;
	_tileset.fill(Qt::transparent);
	painter.begin(&_tileset);
	for (const auto &layer: _layers) {
		for (unsigned int i = 0; i < 256; ++i) {
			if (!layer.tiles.tiles()[i])
				continue;
			const auto &current = layer.alternatives[layer.current];
			for (const auto &p: current.sources) {
				TilemapInfo source_info(*p.first);
				painter.setCompositionMode(p.second);
				painter.drawPixmap(_info.tileRect(i), *p.first, source_info.tileRect(i));
			}
		}
	}
	painter.end();
	emit tilesetUpdated();
}
