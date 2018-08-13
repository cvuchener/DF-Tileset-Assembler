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

FileLineReader::FileLineReader(QFile &file)
        : _file(file)
        , _current_line(0)
{

}

QString FileLineReader::nextLine()
{
	auto line = _file.readLine();
	++_current_line;
	while (!line.isEmpty() && (line.back() == '\n' || line.back() == '\r'))
		line.remove(line.count()-1, 1);
	return QString::fromUtf8(line);
}

int FileLineReader::currentLineNumber() const
{
	return _current_line;
}

FileLineReader::operator bool() const
{
	return !_file.atEnd();
}

QString FileLineReader::formatError(const QString &message) const
{
	return QString("%1:%2: %3")
	                .arg(_file.fileName())
	                .arg(_current_line)
	                .arg(message);
}

ParseError FileLineReader::parseError(const QString &message) const
{
	return ParseError(_file.fileName(), _current_line, message);
}
