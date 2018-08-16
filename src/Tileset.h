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
	static constexpr std::size_t PixmapCount = 3;
	using source_t = std::array<QPixmap, PixmapCount>;

	Tileset(QSettings &s, QObject *parent = nullptr);

	enum class Mode
	{
		Normal,
		TWBT, // use -bg and -top TWBT layers
	};
	Mode mode() const;

	struct layer_t {
		TileSubset tiles;
		struct alternative_t {
			QString name;
			std::vector<std::pair<const source_t *, QPainter::CompositionMode>> sources;
			unsigned int icon_tile, icon_source;
		};
		std::vector<alternative_t> alternatives;
		unsigned int current;
	};

	const std::vector<layer_t> &layers() const;

	void selectAlternative(unsigned int layer, unsigned int alternative);

	enum TWBTLayer: unsigned int
	{
		TWBTNormal = 0,
		TWBTBackground,
		TWBTTop,
	};
	static_assert(TWBTTop < PixmapCount, "Not enough pixmaps for TWBT");

	const QPixmap &pixmap(unsigned int layer = 0) const;
	const TilemapInfo &tilesetInfo() const;
	std::vector<QString> outputs() const;

	static QString TWBTFileName(QString name, Tileset::TWBTLayer layer);

	template<typename... Args>
	void render(QPainter &painter, const QRect &dest,
	            unsigned int tile, Args &&... args) const
	{
		render(painter, dest, _tileset, tile, std::forward<Args>(args)...);
	}

	template<typename... Args>
	QPixmap renderAlternativeIcon(const layer_t::alternative_t &alternative, Args &&... args) const
	{
		if (alternative.sources.empty())
			return QPixmap();
		auto source = alternative.sources[alternative.icon_source].first;
		if (!source)
			return QPixmap();
		QPixmap icon(_info.tileSize());
		icon.fill(Qt::transparent);
		{
			QPainter painter(&icon);
			render(painter, icon.rect(), *source, alternative.icon_tile, std::forward<Args>(args)...);
		}
		return icon;
	}

signals:
	void tilesetUpdated();

private:
	const source_t *loadSourceTileset(const QString &name);
	void buildTileset();

	template<typename... Args>
	void render(QPainter &painter, const QRect &dest,
	            const std::array<QPixmap, PixmapCount> &pixmaps,
	            unsigned int tile, Args &&... args) const
	{
		switch (_mode) {
		case Mode::Normal:
			normal_render(painter, dest, pixmaps, tile, std::forward<Args>(args)...);
			return;
		case Mode::TWBT:
			twbt_render(painter, dest, pixmaps, tile, std::forward<Args>(args)...);
			return;
		}
	}

	void normal_render(QPainter &p, const QRect &dest, const std::array<QPixmap, PixmapCount> &pixmaps,
	                   unsigned int tile) const;
	void normal_render(QPainter &p, const QRect &dest, const std::array<QPixmap, PixmapCount> &pixmaps,
	                   unsigned int tile, const QColor &foreground, const QColor &background) const;
	void twbt_render(QPainter &p, const QRect &dest, const std::array<QPixmap, PixmapCount> &pixmaps,
	                 unsigned int tile) const;
	void twbt_render(QPainter &p, const QRect &dest, const std::array<QPixmap, PixmapCount> &pixmaps,
	                 unsigned int tile, const QColor &foreground, const QColor &background) const;

	Mode _mode;
	std::vector<layer_t> _layers;
	std::map<QString, source_t, std::less<>> _sources;
	TilemapInfo _info;
	std::array<QPixmap, PixmapCount> _tileset;
	QString _output;
};

#endif // TILESET_H
