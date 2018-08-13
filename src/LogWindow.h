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
#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <QWidget>

#include "ui_LogWindow.h"

class LogWindow: public QWidget, private Ui::LogWindow
{
	Q_OBJECT
public:
	explicit LogWindow(QWidget *parent = nullptr);

	static LogWindow *instance();
	static void handleMessage(QtMsgType, const QMessageLogContext &, const QString &);

	unsigned int warningCount() const;
	unsigned int errorCount() const;

signals:
	void errorCountChanged(unsigned int errors, unsigned int warnings);

public slots:

private:
	static LogWindow *_window;

	void addMessage(QtMsgType, const QMessageLogContext &, const QString &);

	unsigned int _warning_count;
	unsigned int _error_count;
};

#endif // LOG_WINDOW_H
