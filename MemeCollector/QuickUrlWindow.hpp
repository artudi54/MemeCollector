#pragma once

#include "ui_QuickUrlWindow.h"

class QuickUrlWindow : public QDialog, private Ui::QuickUrlWindow {
	Q_OBJECT
public:
	QuickUrlWindow(QWidget *parent = nullptr, QTableWidget *table = nullptr);
	virtual ~QuickUrlWindow();
	unsigned get_row_number() const;
	QString get_url_str() const;
	QString get_optional_name() const;
private slots:
	void accept_input();
};