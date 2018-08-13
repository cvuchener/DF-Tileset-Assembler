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
	explicit PreviewWidget(QIODevice *preview_file,
	                       const std::vector<std::pair<QString, Palette>> &palettes,
	                       const std::vector<std::pair<QString, QColor>> &backgrounds,
	                       const std::vector<std::pair<QString, QColor>> &outlines,
	                       QWidget *parent = nullptr);
	~PreviewWidget() override;

	QSize sizeHint() const override;

	void setTileset(const Tileset *tileset);

signals:

public slots:
	void setHighlight(const TileSubset &subset);
	void clearHighlight();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	void buildPreview();
	void buildHighlight();

	const Tileset *_tileset;
	QColor _background;
	QColor _outline;
	bool _use_colors;
	const Palette *_palette;
	TilemapInfo _info;
	std::vector<uint8_t> _tiles;
	std::vector<uint8_t> _fg_colors;
	std::vector<uint8_t> _bg_colors;
	std::unique_ptr<TileSubset> _highlighted_tiles;
	QPixmap _preview;
	QPixmap _highlight;
};

#endif // PREVIEW_WIDGET_H
