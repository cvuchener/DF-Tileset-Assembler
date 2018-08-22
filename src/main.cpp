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
#include "MainWindow.h"
#include "LogWindow.h"
#include "Version.h"

#include <QApplication>
#include <QCommandLineParser>

#define DEFAULT_CONFIG_PATH "tileset-assembler.ini"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Tileset-Assembler");
	app.setApplicationDisplayName("Tileset Assembler");
	app.setApplicationVersion(VERSION_STRING);

	QCommandLineParser parser;
	parser.addPositionalArgument("config_path",
	                             QApplication::translate("main", "Path to the INI configuration file (default is \"%1\").").arg(DEFAULT_CONFIG_PATH),
	                             "[config_path]");
	parser.addVersionOption();
	parser.addHelpOption();
	parser.process(app);

	qInstallMessageHandler(LogWindow::handleMessage);

	MainWindow window(parser.positionalArguments().value(0, DEFAULT_CONFIG_PATH));
	window.show();

	return app.exec();
}
