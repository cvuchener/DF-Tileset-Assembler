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
#include "ParseError.h"

ParseError::ParseError(const QString &filename, int line_number, const QString &message)
        : _filename(filename)
        , _line_number(line_number)
        , _message(message)
{
	_what_message = QString("%1:%2: %3").arg(_filename).arg(line_number).arg(message).toLocal8Bit();
}

const char *ParseError::what() const noexcept
{
	return _what_message.data();
}

const QString &ParseError::filename() const noexcept
{
	return _filename;
}

int ParseError::lineNumber() const noexcept
{
	return _line_number;
}

const QString &ParseError::message() const noexcept
{
	return _message;
}
