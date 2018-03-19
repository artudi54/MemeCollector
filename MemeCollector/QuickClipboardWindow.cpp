#include "stdafx.h"
#include "QuickClipboardWindow.hpp"

QuickClipboardWindow::QuickClipboardWindow(QWidget *parent, QTableWidget *table)
	: QDialog(parent) {
	this->setupUi(this);

	QObject::connect(buttonCancel, &QPushButton::clicked, this, &QWidget::close);
	QObject::connect(buttonOK, &QPushButton::clicked, this, &QuickClipboardWindow::accept_input);

	if (table != nullptr && table->rowCount() > 0) {
		for (unsigned i = 0; i < table->rowCount(); ++i)
			typeComboBox->addItem(table->item(i, 0)->text());
		QList<QTableWidgetSelectionRange> list = table->selectedRanges();
		if (!list.empty())
			typeComboBox->setCurrentIndex(list.first().topRow());
	}
}

QuickClipboardWindow::~QuickClipboardWindow() {}




unsigned QuickClipboardWindow::get_row_number() {
	return typeComboBox->currentIndex();
}

QString QuickClipboardWindow::get_filename() {
	return fileNameInput->text();
}




void QuickClipboardWindow::accept_input() {
	if (typeComboBox->currentIndex() != -1 && !fileNameInput->text().isEmpty()) {
		this->accept();
		this->close();
	}
}