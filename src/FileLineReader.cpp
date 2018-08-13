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
#include "FileLineReader.h"

#include <QFile>

FileLineReader::FileLineReader(QIODevice *file)
        : _stream(file)
        , _current_line(0)
{
	_stream.setCodec("UTF-8");
	_stream.setAutoDetectUnicode(true);
}

QString FileLineReader::nextLine()
{
	auto line = _stream.readLine();
	++_current_line;
	return line;
}

int FileLineReader::currentLineNumber() const
{
	return _current_line;
}

FileLineReader::operator bool() const
{
	return !_stream.atEnd();
}

QString FileLineReader::formatError(const QString &message) const
{
	auto file = dynamic_cast<const QFile *>(_stream.device());
	return QString("%1:%2: %3")
	                .arg(file ? file->fileName() : QString())
	                .arg(_current_line)
	                .arg(message);
}

ParseError FileLineReader::parseError(const QString &message) const
{
	auto file = dynamic_cast<const QFile *>(_stream.device());
	return ParseError(file ? file->fileName() : QString(), _current_line, message);
}
