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

#include "AboutDialog.h"
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

template<typename T, typename U>
static std::vector<T *> ptr_vec(const std::vector<std::unique_ptr<U>> &vec)
{
	std::vector<T *> out;
	out.reserve(vec.size());
	for (const auto &ptr: vec)
		out.push_back(ptr.get());
	return out;
}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
	setupUi(this);

	// Setup Log status button
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

	// Open settings
	QSettings settings("./tileset-assembler.ini", QSettings::IniFormat);
	setWindowTitle(settings.value("title", tr("Missing title")).toString());

	// Create About dialog
	_about_dialog = std::make_unique<AboutDialog>(settings, this);
	_about_dialog->adjustSize();

	// Create tilesets
	auto tileset_count = settings.beginReadArray("tilesets");
	for (int i = 0; i < tileset_count; ++i) {
		settings.setArrayIndex(i);
		_tilesets.emplace_back(std::make_unique<Tileset>(settings));
	}
	settings.endArray();

	// Create Configuration widgets
	std::vector<ConfigurationWidget *> conf_widgets;
	int conf_tab_count = settings.beginReadArray("configuration");
	if (conf_tab_count == 1) {
		settings.setArrayIndex(0);
		auto conf_widget = new ConfigurationWidget(settings, ptr_vec<Tileset>(_tilesets), central_widget);
		conf_widgets.push_back(conf_widget);
		layout->addWidget(conf_widget);
	}
	else {
		auto tabs = new QTabWidget(central_widget);
		for (int i = 0; i < conf_tab_count; ++i) {
			settings.setArrayIndex(i);
			auto conf_widget = new ConfigurationWidget(settings, ptr_vec<Tileset>(_tilesets), central_widget);
			conf_widgets.push_back(conf_widget);
			tabs->addTab(conf_widget, settings.value("name", tr("Unnamed tab")).toString());
		}
		tabs->setTabPosition(QTabWidget::West);
		layout->addWidget(tabs);
	}
	settings.endArray();

	// Load palettes and colors
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

	// Create previews
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
			auto preview = new PreviewWidget(ptr_vec<const Tileset>(_tilesets), &file, _palettes, _backgrounds, _outlines);
			preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			for (auto conf_widget: conf_widgets) {
				connect(conf_widget, &ConfigurationWidget::highlightTiles,
				        preview, &PreviewWidget::setHighlight);
				connect(conf_widget, &ConfigurationWidget::clearHighlightedTiles,
				        preview, &PreviewWidget::clearHighlight);
			}
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
	QMessageBox ask_overwrite(QMessageBox::Question,
	                          tr("Save tileset"), "",
	                          QMessageBox::Yes | QMessageBox::No |
	                          QMessageBox::YesToAll | QMessageBox::NoToAll,
	                          this);
	bool all_saved = true;
	enum class Overwrite {
		Ask,
		YesToAll,
		NoToAll,
	} overwrite = Overwrite::Ask;
	auto get_overwrite = [&overwrite, &ask_overwrite] (const QString &output) {
		switch (overwrite) {
		case Overwrite::Ask: {
			ask_overwrite.setText(tr("%1 already exists. "
			                         "Do you want to replace it?")
			                      .arg(output));
			auto answer = static_cast<QMessageBox::StandardButton>(ask_overwrite.exec());
			switch (answer) {
			case QMessageBox::YesToAll:
				overwrite = Overwrite::YesToAll;
				Q_FALLTHROUGH();
			case QMessageBox::Yes:
				return true;
			case QMessageBox::NoToAll:
				overwrite = Overwrite::NoToAll;
				Q_FALLTHROUGH();
			case QMessageBox::No:
			default:
				return false;
			}
		}
		case Overwrite::YesToAll:
			return true;
		case Overwrite::NoToAll:
			return false;
		}
		Q_UNREACHABLE();
	};
	std::vector<std::pair<QString, const QPixmap *>> files; // tileset, layer, filename
	for (const auto &tileset: _tilesets) {
		auto outputs = tileset->outputs();
		for (unsigned int i = 0; i < outputs.size(); ++i)
			files.emplace_back(outputs[i], &tileset->pixmap(i));
	}
	std::vector<QString> status(files.size());
	for (unsigned int i = 0; i < files.size(); ++i) {
		const auto &output = files[i].first;
		QFileInfo info(output);
		if (info.exists() && !get_overwrite(output)) {
			status[i] = tr("ignored because file already exists");
			all_saved = false;
			continue;
		}
		if (!info.dir().exists()) {
			status[i] = tr("output directory does not exists");
			all_saved = false;
			continue;
		}
		if (!files[i].second->save(output)) {
			status[i] = tr("failed to save tileset");
			all_saved = false;
		}
		else
			status[i] = tr("success");
	}
	QMessageBox results(this);
	results.setIcon(all_saved ? QMessageBox::Information : QMessageBox::Warning);
	results.setWindowTitle(tr("Save tileset"));
	results.setText(all_saved
	                ? tr("All tilesets were successfully saved.")
	                : tr("One or more tileset could not be saved."));
	QStringList status_strings;
	for (unsigned int i = 0; i < files.size(); ++i) {
		status_strings.push_back(tr("%1: %2.")
		                         .arg(files[i].first)
		                         .arg(status[i]));
	}
	results.setDetailedText(status_strings.join('\n'));
	results.exec();
}

void MainWindow::on_about_action_triggered()
{
	if (_about_dialog)
		_about_dialog->show();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	QMainWindow::closeEvent(e);
	QApplication::instance()->quit();
}
