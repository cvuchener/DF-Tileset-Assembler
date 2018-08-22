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
#include "Palette.h"

#include <QPainter>
#include <QtDebug>

#include "FileLineReader.h"

Palette::Palette()
        : colors({
                Qt::black,
                Qt::darkBlue,
                Qt::darkGreen,
                Qt::darkCyan,
                Qt::darkRed,
                Qt::darkMagenta,
                Qt::darkYellow,
                Qt::lightGray,
                Qt::darkGray,
                Qt::blue,
                Qt::green,
                Qt::cyan,
                Qt::red,
                Qt::magenta,
                Qt::yellow,
                Qt::white,
        })
{
}

Palette::Palette(QIODevice *colors_file)
{
	static const std::map<QString, unsigned int, std::less<>> Colors = {
	        { "BLACK", 0 },
	        { "BLUE", 1 },
	        { "GREEN", 2 },
	        { "CYAN", 3 },
	        { "RED", 4 },
	        { "MAGENTA", 5 },
	        { "BROWN", 6 },
	        { "LGRAY", 7 },
	        { "DGRAY", 8 },
	        { "LBLUE", 9 },
	        { "LGREEN", 10 },
	        { "LCYAN", 11 },
	        { "LRED", 12 },
	        { "LMAGENTA", 13 },
	        { "YELLOW", 14 },
	        { "WHITE", 15 },
	};
	static const std::map<QString, void (QColor::*)(int), std::less<>> Channels = {
	        { "R", &QColor::setRed },
	        { "G", &QColor::setGreen },
	        { "B", &QColor::setBlue },
        };

	FileLineReader reader(colors_file);
	while (reader) {
		auto line = reader.nextLine().trimmed();
		if (line.front() != '[')
			continue; // ignore non token lines
		int end = line.indexOf(']');
		if (end == -1) {
			qCritical().noquote() << reader.formatError(tr("Tocken is not closed"));
			continue;
		}
		auto values = line.midRef(1, end-1).split(':');
		if (values.count() != 2) {
			qCritical().noquote() << reader.formatError(tr("Invalid parameter count"));
			continue;
		}
		int sep = values[0].indexOf('_');
		auto color_it = Colors.find(values[0].mid(0, sep));
		if (color_it == Colors.end()) {
			qCritical().noquote() << reader.formatError(tr("Invalid color name"));
			continue;
		}
		auto channel_it = Channels.find(values[0].mid(sep+1));
		if (channel_it == Channels.end()) {
			qCritical().noquote() << reader.formatError(tr("Invalid channel name"));
			continue;
		}
		bool ok;
		auto value = values[1].toInt(&ok);
		if (!ok || value < 0 || value > 255) {
			qCritical().noquote() << reader.formatError(tr("Invalid channel value"));
			continue;
		}
		(colors[color_it->second].*channel_it->second)(value);
	}
}

QPixmap Palette::makePreview() const
{
	static constexpr int size = 4;
	QPainter painter;
	QPixmap preview(size * 4, size * 4);
	painter.begin(&preview);
	for (unsigned int i = 0; i < colors.size(); ++i)
		painter.fillRect(QRect(i%4 * size, i/4 * size, size, size), colors[i]);
	painter.end();
	return preview;
}
