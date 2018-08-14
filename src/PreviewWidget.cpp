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
#include "PreviewWidget.h"

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPaintEvent>

#include "CP437.h"
#include "FileLineReader.h"
#include "Tileset.h"

#include <QtDebug>

PreviewWidget::PreviewWidget(const std::vector<const Tileset *> &tilesets,
                             QIODevice *preview_file,
                             const std::vector<std::pair<QString, Palette>> &palettes,
                             const std::vector<std::pair<QString, QColor>> &backgrounds,
                             const std::vector<std::pair<QString, QColor>> &outlines,
                             QWidget *parent)
        : QWidget(parent)
        , _tilesets(tilesets)
        , _background(backgrounds.front().second)
        , _outline(outlines.front().second)
        , _use_colors(true)
        , _palette(&palettes.front().second)
{
	// Init tilesets info
	if (_tilesets.empty())
		throw std::runtime_error(tr("Empty tileset list").toLocal8Bit().data());
	_info.setTileSize(_tilesets[0]->tilesetInfo().tileSize());
	for (auto tileset: _tilesets)
		connect(tileset, &Tileset::tilesetUpdated, this, &PreviewWidget::buildPreview);

	// Read preview file
	bool ok;
	FileLineReader reader(preview_file);
	auto first_line = reader.nextLine();
	auto parameters = first_line.split(' ');
	if (parameters.size() != 2)
		throw reader.parseError(tr("Invalid parameter count"));
	_info.setTilemapWidth(parameters[0].toInt(&ok));
	if (!ok)
		throw reader.parseError(tr("Invalid tile width"));
	_info.setTilemapHeight(parameters[1].toInt(&ok));
	if (!ok)
		throw reader.parseError(tr("Invalid tile height"));
	auto tile_count = _info.tileCount();
	while (reader) {
		auto line = reader.nextLine();
		auto params = line.splitRef(':');
		if (params[0] == "tiles") {
			if (params.size() > 1)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			for (int i = 0; i < _info.tilemapHeight(); ++i) {
				auto line = reader.nextLine();
				line.resize(_info.tilemapWidth());
				for (auto c: line)
					_tiles.push_back(CP437::fromUnicode(c.unicode()));
			}
		}
		else if (params[0] == "foreground" || params[0] == "background") {
			if (params.size() > 1)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			auto &colors = line == "foreground" ? _fg_colors : _bg_colors;
			for (int i = 0; i < _info.tilemapHeight(); ++i) {
				auto line = reader.nextLine();
				line.resize(_info.tilemapWidth());
				for (auto c: line) {
					auto code = c.unicode();
					uint8_t color = 0;
					if (code >= '0' && code <= '9')
						color = static_cast<uint8_t>(code - '0');
					else if (code >= 'a' && code <= 'f')
						color = static_cast<uint8_t>(code - 'a' + 10);
					else if (code >= 'A' && code <= 'F')
						color = static_cast<uint8_t>(code - 'A' + 10);
					else {
						qCritical().noquote() << reader.formatError(tr("Invalid color: %1").arg(c));
						continue;
					}
					colors.push_back(color);
				}
			}
		}
		else if (params[0] == "tilesets") {
			if (params.size() > 1)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			for (int i = 0; i < _info.tilemapHeight(); ++i) {
				auto line = reader.nextLine();
				line.resize(_info.tilemapWidth());
				for (auto c: line) {
					auto code = c.unicode();
					unsigned int index = 0;
					if (code >= '0' && code <= '9')
						index = static_cast<unsigned int>(code - '0');
					else if (code >= 'a' && code <= 'z')
						index = static_cast<unsigned int>(code - 'a' + 10);
					else if (code >= 'A' && code <= 'Z')
						index = static_cast<unsigned int>(code - 'A' + 10);
					else {
						qCritical().noquote() << reader.formatError(tr("invalid character: %1").arg(c));
						continue;
					}
					if (index >= _tilesets.size()) {
						qCritical().noquote() << reader.formatError(tr("Tileset index too high"));
						continue;
					}
					_source_tilesets.push_back(index);
				}
			}
		}
		else if (params[0] == "tilesizefrom") {
			if (params.size() > 2)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			if (params.size() < 2) {
				qCritical().noquote() << reader.formatError(tr("Missing tileset index"));
				continue;
			}
			bool ok;
			unsigned int index = params[1].toUInt(&ok);
			if (!ok || index >= _tilesets.size()) {
				qCritical().noquote() << reader.formatError(tr("Invalid tileset index"));
				continue;
			}
			_info.setTileSize(_tilesets[index]->tilesetInfo().tileSize());
		}
		else if (params[0] == "tilesize") {
			if (params.size() > 3)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			if (params.size() < 2) {
				qCritical().noquote() << reader.formatError(tr("Missing tile size"));
				continue;
			}
			bool ok;
			int width = params[1].toInt(&ok);
			if (!ok || width <= 0) {
				qCritical().noquote() << reader.formatError(tr("Invalid tile width"));
				continue;
			}
			_info.setTileWidth(width);
			if (params.size() >= 3) {
				int height = params[2].toInt(&ok);
				if (!ok || height <= 0) {
					qCritical().noquote() << reader.formatError(tr("Invalid tile height"));
					continue;
				}
				_info.setTileHeight(height);
			}
			else
				_info.setTileHeight(width);

		}
		else if (line == "nocoloring") {
			if (params.size() > 1)
				qWarning().noquote() << reader.formatError(tr("Unused extras parameters"));
			_use_colors = false;
		}
		else {
			qCritical().noquote() << reader.formatError(tr("Invalid keyword: %1").arg(line));
			continue;
		}
	}
	_tiles.resize(tile_count, 0);
	_source_tilesets.resize(tile_count, 0);
	_fg_colors.resize(tile_count, 15);
	_bg_colors.resize(tile_count, 0);

	// Setup context menu
	auto context_menu = new QMenu(this);
	if (_use_colors && palettes.size() > 1) {
		auto palette_menu = context_menu->addMenu(tr("Palettes"));
		for (const auto &p: palettes) {
			auto action = palette_menu->addAction(p.second.makePreview(), p.first);
			connect(action, &QAction::triggered, [this, palette = &p.second] () {
				_palette = palette;
				buildPreview();
			});
		}
	}
	if (backgrounds.size() > 1) {
		auto background_menu = context_menu->addMenu(tr("Backgrounds"));
		for (const auto &p: backgrounds) {
			QPixmap icon(16, 16);
			icon.fill(p.second);
			auto action = background_menu->addAction(icon, p.first);
			connect(action, &QAction::triggered, [this, color = p.second] () {
				_background = color;
				update();
			});
		}
	}
	if (outlines.size() > 1) {
		auto outline_menu = context_menu->addMenu(tr("Outlines"));
		for (const auto &p: outlines) {
			QPixmap icon(16, 16);
			icon.fill(p.second);
			auto action = outline_menu->addAction(icon, p.first);
			connect(action, &QAction::triggered, [this, color = p.second] () {
				_outline = color;
				buildHighlight();
			});
		}
	}
	if (!context_menu->isEmpty()) {
		connect(this, &QWidget::customContextMenuRequested,
		        [this, context_menu] (const QPoint &pos) {
			context_menu->popup(mapToGlobal(pos));
		});
		setContextMenuPolicy(Qt::CustomContextMenu);
	}

	buildPreview();
}

PreviewWidget::~PreviewWidget()
{
}

QSize PreviewWidget::sizeHint() const
{
	return _info.pixmapSize() + QSize(2, 2); // reserve space for borders
}

void PreviewWidget::setHighlight(unsigned int tileset_index, const TileSubset &subset)
{
	_highlighted_tileset = tileset_index;
	_highlighted_tiles = std::make_unique<TileSubset>(subset);
	buildHighlight();
}

void PreviewWidget::clearHighlight()
{
	_highlighted_tiles.reset();
	update();
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);


	QPainter painter(this);
	//painter.setClipRegion(event->region());

	QPoint center = painter.viewport().center();
	QRect rect;

	painter.fillRect(event->rect(), _background);

	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	rect.setSize(_preview.size());
	rect.moveCenter(center);
	painter.drawPixmap(rect, _preview);

	if (_highlighted_tiles) {
		rect.setSize(_highlight.size());
		rect.moveCenter(center);
		painter.drawPixmap(rect, _highlight);
	}
}

void PreviewWidget::buildPreview()
{
	_preview = QPixmap(_info.pixmapSize());
	_preview.fill(Qt::transparent);
	QPainter painter(&_preview);
	for (unsigned int i = 0; i < _info.tileCount(); ++i) {
		const auto &tileset = _tilesets[_source_tilesets[i]]->tileset();
		TilemapInfo tileset_info(tileset);
		uint8_t tile = _tiles[i];
		auto src_rect = tileset_info.tileRect(tile);
		auto dest_rect = _info.tileRect(i);
		if (_use_colors) {
			painter.setCompositionMode(QPainter::CompositionMode_Source);
			painter.drawPixmap(dest_rect, tileset, src_rect);
			painter.setCompositionMode(QPainter::CompositionMode_Multiply);
			painter.fillRect(dest_rect, _palette->colors[_fg_colors[i]]);
			painter.setCompositionMode(QPainter::CompositionMode_DestinationIn); // multiply has overwritten alpha channel?
			painter.drawPixmap(dest_rect, tileset, src_rect);
			painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
			painter.fillRect(dest_rect, _palette->colors[_bg_colors[i]]);
		}
		else {
			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			painter.drawPixmap(dest_rect, tileset, src_rect);
		}
	}
	update();
}
void PreviewWidget::buildHighlight()
{
	if (!_highlighted_tiles)
		return;
	_highlight = QPixmap(_info.pixmapSize() + QSize(2, 2));
	_highlight.fill(Qt::transparent);
	{
		const QPoint origin(1, 1);
		QPainter painter(&_highlight);
		for (unsigned int i = 0; i < _info.tileCount(); ++i) {
			uint8_t tile = _tiles[i];
			unsigned int tileset_index = _source_tilesets[i];
			if (_highlighted_tileset != tileset_index || !_highlighted_tiles->tiles()[tile])
				continue;
			painter.fillRect(_info.tileRect(i).translated(origin).marginsAdded(QMargins(1, 1, 1, 1)), _outline);
		}
		painter.setCompositionMode(QPainter::CompositionMode_Clear);
		for (unsigned int i = 0; i < _info.tileCount(); ++i) {
			uint8_t tile = _tiles[i];
			unsigned int tileset_index = _source_tilesets[i];
			if (_highlighted_tileset != tileset_index || !_highlighted_tiles->tiles()[tile])
				continue;
			painter.fillRect(_info.tileRect(i).translated(origin), Qt::transparent);
		}
	}
	update();
}
