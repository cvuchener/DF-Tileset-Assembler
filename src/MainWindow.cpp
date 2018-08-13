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

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTableWidget>

#include <QtDebug>

#include "ConfigurationWidget.h"
#include "PreviewWidget.h"
#include "Tileset.h"
#include "Palette.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
	setupUi(this);

	auto log_button = new QPushButton(this);
	log_button->setHidden(true);
	status_bar->addPermanentWidget(log_button);

	connect(LogWindow::instance(), &LogWindow::errorCountChanged,
	        [log_button] (unsigned int error_count, unsigned int warning_count) {
		QStringList counts;
		if (error_count > 0)
			counts.append(tr("%1 error(s)", "",
			                 static_cast<int>(error_count))
			              .arg(error_count));
		if (warning_count > 0)
			counts.append(tr("%1 warning(s)", "",
			                 static_cast<int>(warning_count))
			              .arg(warning_count));
		log_button->setText(counts.join(", "));
		if (error_count + warning_count > 0)
			log_button->setHidden(false);
	});
	connect(log_button, &QPushButton::clicked, [] () {
		LogWindow::instance()->show();
		LogWindow::instance()->raise();
	});

	auto layout = new QHBoxLayout(central_widget);

	QSettings settings("./tileset-assembler.ini", QSettings::IniFormat);
	setWindowTitle(settings.value("title", tr("Missing title")).toString());
	_output = settings.value("output", "tileset.png").toString();

	settings.beginGroup("tileset");
	try {
		_tileset = std::make_unique<Tileset>(settings);
	}
	catch (std::exception &e) {
		qCritical().noquote() << tr("Cannot create tileset: %1").arg(e.what());
	}
	settings.endGroup();

	auto conf_widget = new ConfigurationWidget(settings, _tileset.get(), central_widget);
	layout->addWidget(conf_widget);

	settings.beginGroup("colors");
	int palette_count = settings.beginReadArray("palette");
	for (int i = 0; i < palette_count; ++i) {
		settings.setArrayIndex(i);
		auto name = settings.value("name", tr("Unnamed palette")).toString();
		QFile file(settings.value("file").toString());
		if (!file.open(QIODevice::ReadOnly)) {
			qCritical().noquote() << tr("Cannot open palette: %1").arg(file.fileName());
			continue;
		}
		_palettes.emplace_back(name, &file);
	}
	settings.endArray();
	if (_palettes.empty())
		_palettes.emplace_back(tr("Default palette"), Palette());
	int bg_count = settings.beginReadArray("background");
	for (int i = 0; i < bg_count; ++i) {
		settings.setArrayIndex(i);
		auto name = settings.value("name", tr("Unnamed background color")).toString();
		auto color = settings.value("color").toString();
		_backgrounds.emplace_back(name, color);
	}
	settings.endArray();
	if (_backgrounds.empty())
		_backgrounds.emplace_back(tr("Default background"),
		                          QPalette().color(QPalette::Base));
	int outline_count = settings.beginReadArray("outline");
	for (int i = 0; i < outline_count; ++i) {
		settings.setArrayIndex(i);
		auto name = settings.value("name", tr("Unnamed outline color")).toString();
		auto color = settings.value("color").toString();
		_outlines.emplace_back(name, color);
	}
	settings.endArray();
	if (_outlines.empty())
		_outlines.emplace_back(tr("Red"), Qt::red);
	settings.endGroup();

	auto tabs = new QTabWidget(central_widget);
	int preview_count = settings.beginReadArray("previews");
	for (int i = 0; i < preview_count; ++i) {
		settings.setArrayIndex(i);
		auto name = settings.value("name", tr("Unnamed Preview")).toString();
		QFile file(settings.value("file").toString());
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qCritical().noquote() << tr("Failed to open preview file: %1").arg(file.fileName());
			continue;
		}
		try {
			auto preview = new PreviewWidget(&file, _palettes, _backgrounds, _outlines);
			preview->setTileset(_tileset.get());
			preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			connect(conf_widget, &ConfigurationWidget::highlightTiles, preview, &PreviewWidget::setHighlight);
			connect(conf_widget, &ConfigurationWidget::clearHighlightedTiles, preview, &PreviewWidget::clearHighlight);
			tabs->addTab(preview, name);
		}
		catch (std::exception &e) {
			qCritical().noquote() << tr("Cannot create preview %1 from %2: %3")
			               .arg(name)
			               .arg(file.fileName())
			               .arg(e.what());
		}
		file.close();
	}
	settings.endArray();
	layout->addWidget(tabs);
	central_widget->setLayout(layout);
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_save_action_triggered()
{
	if (!_tileset)
		return;
	QFileInfo info(_output);
	if (info.exists() &&
	    QMessageBox::Yes != QMessageBox::question(this,
	                                              tr("Save tileset"),
	                                              tr("%1 already exists. Do you want to replace it?").arg(_output)))
		return;
	if (!info.dir().exists()) {
		QMessageBox::critical(this, tr("Save Error"), tr("Output directory \"%1\" does not exists.").arg(info.dir().path()));
		return;
	}
	if (!_tileset->tileset().save(_output))
		QMessageBox::critical(this, tr("Save Error"), tr("Failed to save tileset."));
	else
		QMessageBox::information(this, tr("Tileset saved"), tr("The tileset was successfully saved to %1.").arg(_output));
}

void MainWindow::on_save_as_action_triggered()
{
	if (!_tileset)
		return;
	QFileDialog dialog(this, tr("Save tileset"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setNameFilters({tr("Portable Network Graphics (*.png)"),
	                       tr("Any files (*)")});
	if (dialog.exec()) {
		auto output = dialog.selectedFiles().front();
		if (!_tileset->tileset().save(output))
			QMessageBox::critical(this, tr("Save Error"), tr("Failed to save tileset."));
	}
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	QMainWindow::closeEvent(e);
	QApplication::instance()->quit();
}
