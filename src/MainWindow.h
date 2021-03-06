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
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_MainWindow.h"

#include <memory>

#include "Palette.h"

class AboutDialog;
class Tileset;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(const QString &config_path, QWidget *parent = nullptr);
	~MainWindow() override;

public slots:
	void on_save_action_triggered();
	void on_about_action_triggered();

protected:
	void closeEvent(QCloseEvent *) override;

private:
	std::vector<std::unique_ptr<Tileset>> _tilesets;
	std::vector<std::pair<QString, Palette>> _palettes;
	std::vector<std::pair<QString, QColor>> _backgrounds;
	std::vector<std::pair<QString, QColor>> _outlines;
	std::unique_ptr<AboutDialog> _about_dialog;

};

#endif // MAIN_WINDOW_H
