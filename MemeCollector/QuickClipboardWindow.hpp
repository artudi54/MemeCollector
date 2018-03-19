#pragma once


#include "ui_QuickClipboardWindow.h"

class QuickClipboardWindow : public QDialog, public Ui::QuickClipboardWindow {
	Q_OBJECT
public:
	QuickClipboardWindow(QWidget *parent = nullptr, QTableWidget *table = nullptr);
	virtual ~QuickClipboardWindow();
	unsigned get_row_number();
	QString get_filename();
private slots:
	void accept_input();
};