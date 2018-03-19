#include "stdafx.h"
#include "QuickUrlWindow.hpp"

QuickUrlWindow::QuickUrlWindow(QWidget *parent, QTableWidget *table)
	: QDialog(parent) {
	this->setupUi(this);

	QObject::connect(buttonCancel, &QPushButton::clicked, this, &QWidget::close);
	QObject::connect(buttonOK, &QPushButton::clicked, this, &QuickUrlWindow::accept_input);

	if (table != nullptr && table->rowCount() > 0) {
		for (unsigned i = 0; i < table->rowCount(); ++i)
			typeComboBox->addItem(table->item(i,0)->text());
		QList<QTableWidgetSelectionRange> list = table->selectedRanges();
		if (!list.empty())
			typeComboBox->setCurrentIndex(list.first().topRow());
	}

	QString text = QApplication::clipboard()->text();
	QUrl url = QUrl::fromUserInput(text);
	if (url.isValid() && !url.isLocalFile())
		urlInput->setText(text);
}

QuickUrlWindow::~QuickUrlWindow() {}






unsigned QuickUrlWindow::get_row_number() const {
	return typeComboBox->currentIndex();
}

QString QuickUrlWindow::get_url_str() const {
	return urlInput->text();
}

QString QuickUrlWindow::get_optional_name() const {
	return optionalInput->text();
}





void QuickUrlWindow::accept_input() {
	if (typeComboBox->currentIndex() != -1 && !urlInput->text().isEmpty()) {
		this->accept();
		this->close();
	}
}