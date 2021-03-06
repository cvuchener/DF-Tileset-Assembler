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
#ifndef FILE_LINE_READER_H
#define FILE_LINE_READER_H

#include <QTextStream>

#include "ParseError.h"

class QIODevice;

class FileLineReader
{
public:
	FileLineReader(QIODevice *file);

	QString nextLine();
	int currentLineNumber() const;
	operator bool() const;
	QString formatError(const QString &message) const;
	ParseError parseError(const QString &message) const;

private:
	QTextStream _stream;
	int _current_line;
};

#endif // FILE_LINE_READER_H
