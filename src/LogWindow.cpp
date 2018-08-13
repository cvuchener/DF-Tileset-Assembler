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
#include "LogWindow.h"

#include <iostream>

LogWindow::LogWindow(QWidget *parent)
        : QWidget(parent)
        , _warning_count(0)
        , _error_count(0)
{
	setupUi(this);

	auto doc = log->document();
	doc->clear();
	QImage error(":img/error");
	QImage warning(":img/warning");
	doc->addResource(QTextDocument::ImageResource, QUrl("img://error"), QVariant(error));
	doc->addResource(QTextDocument::ImageResource, QUrl("img://warning"), QVariant(warning));
}

LogWindow *LogWindow::_window = nullptr;

LogWindow *LogWindow::instance()
{
	if (!_window)
		_window = new LogWindow;
	return _window;
}

void LogWindow::handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	std::cerr << message.toLocal8Bit().data() << std::endl;
	instance()->addMessage(type, context, message);
}

unsigned int LogWindow::errorCount() const
{
	return _error_count;
}

static void insertInlineImage(QTextCursor &cursor, const char *url)
{
	QTextImageFormat image_format;
	QFontMetrics fm(image_format.font());
	image_format.setName(url);
	image_format.setHeight(fm.height());
	cursor.insertImage(image_format);
}

void LogWindow::addMessage(QtMsgType type, const QMessageLogContext &, const QString &message)
{
	QTextCursor cursor(log->document());
	cursor.movePosition(QTextCursor::End);
	if (type == QtWarningMsg) {
		++_warning_count;
		emit errorCountChanged(_error_count, _warning_count);
		insertInlineImage(cursor, "img://warning");
		cursor.insertText(" ");
	}
	else if (type >= QtCriticalMsg) {
		++_error_count;
		emit errorCountChanged(_error_count, _warning_count);
		insertInlineImage(cursor, "img://error");
		cursor.insertText(" ");
	}
	cursor.insertText(message);
	cursor.insertBlock();
	log->setTextCursor(cursor);

	if (type >= QtCriticalMsg)
		show();
}
