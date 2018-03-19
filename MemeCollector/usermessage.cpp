#include "stdafx.h"
#include "usermessage.hpp"

int input_error(QWidget *widget, const QString &text) {
	return QMessageBox::information(
		widget, "Input Error",
		text, QMessageBox::Ok
	);
}

int critical_error(QWidget *widget, const QString &text) {
	return QMessageBox::critical(
		widget, "Critical Error",
		text, QMessageBox::Ok
	);
}

int operation_successful(QWidget * widget, const QString & text) {
	return QMessageBox::information(
		widget, "Operation Successful",
		text, QMessageBox::Ok
	);
}
