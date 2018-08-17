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
#ifndef PREVIEW_WIDGET_H
#define PREVIEW_WIDGET_H

#include <QWidget>

#include <memory>

#include "Palette.h"
#include "TilemapInfo.h"

class Tileset;
class TileSubset;

class QIODevice;

class PreviewWidget : public QWidget
{
	Q_OBJECT
public:
	static constexpr int OutlineWidth = 2;

	explicit PreviewWidget(const std::vector<const Tileset *> &tilesets,
	                       QIODevice *preview_file,
	                       const std::vector<std::pair<QString, Palette>> &palettes,
	                       const std::vector<std::pair<QString, QColor>> &backgrounds,
	                       const std::vector<std::pair<QString, QColor>> &outlines,
	                       QWidget *parent = nullptr);
	~PreviewWidget() override;

	QSize sizeHint() const override;

	const TilemapInfo &info() const;

signals:

public slots:
	void setHighlight(unsigned int tileset_index, const TileSubset &subset);
	void clearHighlight();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	void buildPreview();
	void buildHighlight();

	std::vector<const Tileset *> _tilesets;
	QColor _background;
	QColor _outline;
	bool _use_colors;
	const Palette *_palette;
	TilemapInfo _info;
	struct layer_t {
		std::vector<unsigned int> tiles;
		std::vector<unsigned int> source_tilesets;
		std::vector<uint8_t> fg_colors;
		std::vector<uint8_t> bg_colors;
	};
	std::vector<layer_t> _layers;
	unsigned int _highlighted_tileset;
	std::unique_ptr<TileSubset> _highlighted_tiles;
	QPixmap _preview;
	QPixmap _highlight;
};

#endif // PREVIEW_WIDGET_H
