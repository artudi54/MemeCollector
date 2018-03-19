#pragma once

#include "ui_ItemDialog.h"

class ItemDialog : public QDialog, public Ui::ItemDialog {
	Q_OBJECT

public:
	ItemDialog(QWidget *parent, QTableWidget *widget);
	ItemDialog(QWidget *parent, QTableWidget *widget, QTableWidgetItem *name, QTableWidgetItem *path);
	virtual ~ItemDialog();
private slots:
	void accept_input();
	void open_directory();
private:
	void connect_signals();
	void add_new_meme_type();

	QTableWidget *widget;
	std::unique_ptr<QTableWidgetItem, std::function<void(QTableWidgetItem*)>> name;
	std::unique_ptr<QTableWidgetItem, std::function<void(QTableWidgetItem*)>> path;
	bool add;
};