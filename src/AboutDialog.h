#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "ui_AboutDialog.h"

class QSettings;

class AboutDialog : public QDialog, private Ui::AboutDialog
{
	Q_OBJECT
public:
	explicit AboutDialog(QSettings &s, QWidget *parent = nullptr);
};

#endif // ABOUT_DIALOG_H
