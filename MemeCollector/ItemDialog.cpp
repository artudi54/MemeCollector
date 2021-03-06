#include "stdafx.h"
#include "ItemDialog.hpp"

ItemDialog::ItemDialog(QWidget *parent, QTableWidget *_widget)
    : QDialog(parent)
    , widget(_widget)
    , name(new QTableWidgetItem, std::default_delete<QTableWidgetItem>())
    , path(new QTableWidgetItem, std::default_delete<QTableWidgetItem>())
    , add(true) {
    this->setupUi(this);
    this->connect_signals();
    this->setWindowTitle(this->windowTitle() + " (adding)");
}


ItemDialog::ItemDialog(QWidget *parent, QTableWidget *_widget, QTableWidgetItem *_name, QTableWidgetItem *_path)
    : QDialog(parent)
    , widget(_widget)
    , name(_name, [](QTableWidgetItem*) {})
, path(_path, [](QTableWidgetItem*) {})
, add(false) {
    this->setupUi(this);
    this->connect_signals();
    nameInput->setText(name->text());
    pathInput->setText(path->text());
    this->setWindowTitle(this->windowTitle() + " (editing)");

}

ItemDialog::~ItemDialog() {}






void ItemDialog::accept_input() {
    QString nameStr = nameInput->text().trimmed();
    QString pathStr = pathInput->text().trimmed();
    while (pathStr.endsWith('/') || pathStr.endsWith('\\'))
        pathStr.chop(1);


    if (nameStr.isEmpty() || pathStr.isEmpty()) {
        QMessageBox::information(
            this, QStringLiteral("Input Error"),
            QStringLiteral("Input is empty")
        );
        return;
    }
    if (!QDir(pathStr).exists()) {
        QMessageBox::information(
            this, QStringLiteral("Input Error"),
            QStringLiteral("The path is invalid")
        );
        return;
    }

    for (unsigned i = 0; i < widget->rowCount(); ++i) {
        QTableWidgetItem *item = widget->item(i, 0);
        if (name.get() != item) {
            if (item->text() == nameStr) {
                QMessageBox::information(
                    this, QStringLiteral("Input Error"),
                    QStringLiteral("Entered name is already on the list")
                );
                return;
            } else if (widget->item(i, 1)->text() == pathStr) {
                QMessageBox::information(
                    this, QStringLiteral("Input Error"),
                    QStringLiteral("Entered path is already on the list")
                );
                return;
            }
        }
    }

    if (pathStr != path->text())
        emit path_changed(name->text(), nameStr, pathStr);
    else if (nameStr != name->text())
        emit name_changed(name->text(), nameStr);

    name->setText(nameStr);
    path->setText(pathStr);
    if (add)
        this->add_new_meme_type();
    name.release();
    path.release();

    this->accept();
    this->close();
}

void ItemDialog::open_directory() {
    QString openPath = QFileDialog::getExistingDirectory(
                           this, QStringLiteral("Select directory"),
                           QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                       );
    if (!openPath.isEmpty()) {
        pathInput->setText(openPath);
        if (nameInput->text().isEmpty())
            nameInput->setText(QDir(openPath).dirName());
    }
}






void ItemDialog::connect_signals() {
    QObject::connect(okButton, &QPushButton::clicked, this, &ItemDialog::accept_input);
    QObject::connect(cancelButton, &QPushButton::clicked, this, &ItemDialog::close);
    QObject::connect(openButton, &QPushButton::clicked, this, &ItemDialog::open_directory);
}

void ItemDialog::add_new_meme_type() {
    int row = widget->rowCount();
    widget->insertRow(row);
    widget->setItem(row, 0, name.get());
    widget->setItem(name->row(), 1, path.get());
}
