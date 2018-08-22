#include "AboutDialog.h"

#include <QApplication>
#include <QSettings>
#include <QUrl>

AboutDialog::AboutDialog(QSettings &s, QWidget *parent)
        : QDialog(parent)
{
	setupUi(this);

	tileset_title->setText(s.value("title", tr("Unnamed Tileset")).toString());
	if (s.contains("author"))
		tileset_form_layout->addRow(new QLabel(tr("Author: ")),
		                            new QLabel(s.value("author").toString()));
	if (s.contains("version"))
		tileset_form_layout->addRow(new QLabel(tr("Version: ")),
		                            new QLabel(s.value("version").toString()));
	if (s.contains("licence"))
		tileset_form_layout->addRow(new QLabel(tr("Licence: ")),
		                            new QLabel(s.value("licence").toString()));
	if (s.contains("url")) {
		auto url = QUrl(s.value("url").toString());
		auto url_label = new QLabel;
		url_label->setOpenExternalLinks(true);
		url_label->setText(QString("<a href=\"%1\">%1</a>").arg(url.toString()));
		tileset_form_layout->addRow(new QLabel(tr("URL: ")), url_label);
	}

	version_value->setText(QApplication::instance()->applicationVersion());
	qt_version_value->setText(tr("%1 (build), %2 (runtime)")
	                          .arg(QT_VERSION_STR)
	                          .arg(qVersion()));
}
