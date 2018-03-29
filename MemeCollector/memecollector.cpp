#include "stdafx.h"
#include "MemeCollector.hpp"
#include "autostart.hpp"

MemeCollector::MemeCollector(QWidget *parent)
    : QMainWindow(parent)
    , imageSavePath()
    , showHotkey(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_1), false, this)
    , quicksaveUrlHotkey(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_2), false, this)
    , quicksaveClipboardHotkey(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_3), false, this)
    , cacheMapManager(QDir(QStringLiteral("ImageCache")))
    , itemDialog(nullptr)
    , downloader(new QNetworkAccessManager(this))
    , fileNotifier(new FileNotifier(this))
    , programConfig()
    , configDialog(nullptr)
    , trayIcon(new QSystemTrayIcon(this))
    , quickUrl(nullptr)
    , quickClipboard(nullptr)
    , imageViewer(this) {
    this->setupUi(this);

    this->connect_signals();
    this->connect_menus();

    this->read_database();
    programConfig.load_from(CONFIG_FILE);

    this->apply_settings();

    this->initial_window_show();
    this->validate_cache();
    imageViewer.set_cache_map_manager(cacheMapManager);

    radioURL->click();
    this->setFixedSize(this->sizeHint());
}




MemeCollector::~MemeCollector() {
    programConfig.save_to(CONFIG_FILE);
    QApplication::quit();
}
















void MemeCollector::changeEvent(QEvent * event) {
    if (event->type() == QEvent::WindowStateChange
            && this->windowState() & Qt::WindowMinimized
       )
        QTimer::singleShot(250, this, &QWidget::hide);

    QWidget::changeEvent(event);
}



void MemeCollector::showEvent(QShowEvent * event) {
    actionShow->setEnabled(false);
    actionHide->setEnabled(true);
    trayIcon->hide();
    QMainWindow::showEvent(event);
    //event->accept();
}


void MemeCollector::hideEvent(QHideEvent * event) {
    actionShow->setEnabled(true);
    actionHide->setEnabled(false);
    trayIcon->show();
    QMainWindow::hideEvent(event);
    //event->accept();
}














void MemeCollector::quick_url() {
    if (quickUrl != nullptr) {
        quickUrl->raise();
        quickUrl->activateWindow();
        return;
    }
    quickUrl = new QuickUrlWindow(nullptr, tableWidget);
    quickUrl->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(
        quickUrl, &QObject::destroyed, this,
        [&] { quickUrl = nullptr; }
    );

    QObject::connect(
        quickUrl, &QDialog::accepted, this,
    [&] {
        radioURL->click();

        inputEdit->setText(quickUrl->get_url_str());

        this->table_select_row(quickUrl->get_row_number());

        QString optionalName = quickUrl->get_optional_name();
        if (!optionalName.isEmpty()) {
            changeNameCheckBox->setChecked(true);
            nameInputEdit->setText(optionalName);
        } else
            changeNameCheckBox->setChecked(false);

        collectButton->click();
    }
    );

    quickUrl->show();
}



void MemeCollector::quick_clipboard() {
    if (quickClipboard != nullptr) {
        quickClipboard->raise();
        quickClipboard->activateWindow();
        return;
    }
    quickClipboard = new QuickClipboardWindow(nullptr, tableWidget);
    quickClipboard->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(
        quickClipboard, &QObject::destroyed, this,
        [&] { quickClipboard = nullptr; }
    );

    QObject::connect(
        quickClipboard, &QDialog::accepted, this,
    [&] {
        radioClipboard->click();
        changeNameCheckBox->setChecked(true);
        nameInputEdit->setText(quickClipboard->get_filename());

        this->table_select_row(quickClipboard->get_row_number());

        collectButton->click();
    }
    );

    quickClipboard->show();
}















void MemeCollector::paste_to_input() {
    QClipboard *clipboard = QApplication::clipboard();
    QString toPaste = clipboard->text();
    if (!toPaste.isEmpty())
        inputEdit->setText(toPaste);
}





void MemeCollector::options() {
    if (configDialog != nullptr) {
        configDialog->raise();
        configDialog->activateWindow();
        return;
    }
    configDialog = new ProgramConfigDialog(programConfig, this);
    configDialog->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(
        configDialog, &QObject::destroyed, this,
    [&]() {
        configDialog = nullptr;
    }
    );

    QObject::connect(
        configDialog, &QDialog::accepted, this,
    [&]() {
        programConfig = configDialog->get_config();
        this->apply_settings();
    }
    );
    configDialog->show();
}










void MemeCollector::table_open_image_viewer() {
    QString name = tableWidget->item(
                       tableWidget->selectedRanges().first().topRow(), 0)->text();
    QString path = tableWidget->item(
                       tableWidget->selectedRanges().first().topRow(), 1)->text();

    imageViewer.set_directory(name, path);
    imageViewer.show();
}


void MemeCollector::table_open_directory() {
    unsigned currentRow = tableWidget->selectedRanges().first().topRow();
    QString text = tableWidget->item(currentRow, 1)->text();
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(text)))
        QMessageBox::critical(
            this, QStringLiteral("Critical Error"),
            QStringLiteral("Could not open directory")
        );
}

#ifdef Q_OS_WIN
void MemeCollector::table_properties() {
    unsigned currentRow = tableWidget->selectedRanges().first().topRow();
    std::wstring path = tableWidget->item(currentRow, 1)->text().toStdWString();
    ::SHELLEXECUTEINFOW info = {};
    info.cbSize = sizeof(info);
    info.lpFile = path.c_str();
    info.nShow = SW_SHOW;
    info.fMask = SEE_MASK_INVOKEIDLIST;
    info.lpVerb = L"properties";
    ::ShellExecuteExW(&info);
}
#endif


void MemeCollector::table_add_new() {
    if (itemDialog != nullptr) {
        if (!itemDialog->isVisible())
            itemDialog->show();
        itemDialog->activateWindow();
        return;
    }
    itemDialog = new ItemDialog(this, tableWidget);
    itemDialog->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(
        itemDialog, &QObject::destroyed, this,
    [&]() {
        itemDialog = nullptr;
    }
    );
    QObject::connect(
        itemDialog, &ItemDialog::path_changed, this,
    [&](QString prevName, QString newName, QString newPath) {
        cacheMapManager.add(newName);
    });

    QObject::connect(
        itemDialog, &QDialog::accepted,
    [&]() {
        this->save_database();
    }
    );

    itemDialog->show();

}


void MemeCollector::table_edit() {
    if (itemDialog != nullptr) {
        if (!itemDialog->isVisible())
            itemDialog->show();
        itemDialog->activateWindow();
        return;
    }
    int row = tableWidget->currentRow();

    itemDialog = new ItemDialog(
        this, tableWidget, tableWidget->item(row, 0), tableWidget->item(row, 1)
    );
    itemDialog->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(
        itemDialog, &QObject::destroyed, this,
    [&]() {
        itemDialog = nullptr;
    }
    );
    QObject::connect(
        itemDialog, &ItemDialog::path_changed, this,
    [&](QString prevName, QString newName, QString newPath) {
        cacheMapManager.remove(prevName);
        cacheMapManager.add(newName);
    });
    QObject::connect(
        itemDialog, &ItemDialog::name_changed, this,
    [&](QString prevName, QString newName) {
        cacheMapManager.rename(prevName, newName);
    });
    QObject::connect(
        itemDialog, &QDialog::accepted,
    [&]() {
        this->save_database();
    }
    );

    itemDialog->show();
}


void MemeCollector::table_remove() {
    QTableWidgetSelectionRange range = tableWidget->selectedRanges().first();
    QString name = tableWidget->item(range.topRow(), range.leftColumn())->text();
    cacheMapManager.remove(name);
    tableWidget->removeRow(range.topRow());
    this->save_database();
}


void MemeCollector::table_remove_all() {
    cacheMapManager.clear();
    tableWidget->setRowCount(0);
    this->save_database();
}








void MemeCollector::read_me() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("ReadMe.txt")));
}








void MemeCollector::switch_to_URL() {
    inputEdit->setEnabled(true);
    selectPictureButton->setEnabled(false);
    changeNameCheckBox->setEnabled(true);
    nameInputEdit->setEnabled(changeNameCheckBox->isChecked());
    pasteButton->setEnabled(true);
}

void MemeCollector::switch_to_clipboard() {
    inputEdit->setEnabled(false);
    selectPictureButton->setEnabled(false);
    changeNameCheckBox->setEnabled(false);
    nameInputEdit->setEnabled(true);
    pasteButton->setEnabled(false);
}

void MemeCollector::switch_to_pc_dir() {
    inputEdit->setEnabled(true);
    selectPictureButton->setEnabled(true);
    changeNameCheckBox->setEnabled(true);
    nameInputEdit->setEnabled(changeNameCheckBox->isChecked());
    pasteButton->setEnabled(true);
}









void MemeCollector::select_image() {
    QString path = QFileDialog::getOpenFileName(
                       this, QStringLiteral("Select Image"),
                       QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                       "Standard images (" + ImageMIME::regexp_suffix_list() + ")"
                   );
    if (!path.isEmpty())
        inputEdit->setText(path);
}









void MemeCollector::collect() {
    if (tableWidget->selectedItems().isEmpty()) {
        QMessageBox::information(
            this, QStringLiteral("Input Error"),
            QStringLiteral("No meme folder selected (if list is empty add some directories)")
        );
        return;
    }

    if (radioURL->isChecked())
        this->collect_url();
    else if (radioClipboard->isChecked())
        this->collect_clipboard();
    else if (radioPcDir->isChecked())
        this->collect_pc_directory();
}









void MemeCollector::handle_selection_changed() {
    bool enable = !tableWidget->selectedItems().empty();

    editButton->setEnabled(enable);
    removeButton->setEnabled(enable);

    actionOpenImageViewer->setEnabled(enable);
    actionOpenDirectory->setEnabled(enable);
    actionProperties->setEnabled(enable);
    actionEdit->setEnabled(enable);
    actionRemove->setEnabled(enable);
}







void MemeCollector::collecting_url_finished(QNetworkReply * reply) {
    if (reply->error())
        QMessageBox::critical(this, QStringLiteral("Critical Error"), reply->errorString());
    else {
        QByteArray data = reply->readAll();
        QFile filePath(imageSavePath);
        if (!filePath.open(QFile::WriteOnly) || filePath.write(data) != data.size())
            QMessageBox::critical(
                this, QStringLiteral("Critical Error"),
                QStringLiteral("Saving data to hard drive failed")
            );
        else if (programConfig.showMsgOnComplete)
            this->collecting_complete_message();
    }
    reply->deleteLater();
    progressBar->setValue(0);
}


void MemeCollector::collecting_pc_directory_finished(bool success) {
    if (!success)
        QMessageBox::critical(this, QStringLiteral("Critical Error"), fileNotifier->error_string());
    else if (programConfig.showMsgOnComplete)
        this->collecting_complete_message();
    progressBar->setValue(0);
}









void MemeCollector::update_progress(uint64_t current, uint64_t total) {
    progressBar->setValue(current);
    progressBar->setMaximum(total);
}









void MemeCollector::connect_signals() {
    QObject::connect(&showHotkey, &QHotkey::activated, actionShow, [this] {actionShow->trigger();});
    QObject::connect(&quicksaveUrlHotkey, &QHotkey::activated, actionQuicksaveURL, [this] {actionQuicksaveURL->trigger();});
    QObject::connect(&quicksaveClipboardHotkey, &QHotkey::activated, actionQuicksaveImage, [this] {actionQuicksaveImage->trigger();});


    QObject::connect(actionShow, &QAction::triggered, this, [&]() {
        if (this->isVisible()) {
            this->activateWindow();
        } else {
            this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            this->show();
        }
    });
    QObject::connect(actionHide, &QAction::triggered, this, &QWidget::hide);
    QObject::connect(actionOptions, &QAction::triggered, this, &MemeCollector::options);
    QObject::connect(actionQuicksaveURL, &QAction::triggered, this, &MemeCollector::quick_url);
    QObject::connect(actionQuicksaveImage, &QAction::triggered, this, &MemeCollector::quick_clipboard);
    QObject::connect(actionClose, &QAction::triggered, this, &QWidget::close);

    QObject::connect(pasteButton, &QPushButton::clicked, this, &MemeCollector::paste_to_input);
    QObject::connect(clearButton, &QPushButton::clicked, nameInputEdit, &QLineEdit::clear);

    QObject::connect(tableWidget, &QTableWidget::cellDoubleClicked, this, &MemeCollector::table_open_image_viewer);
    QObject::connect(actionOpenImageViewer, &QAction::triggered, this, &MemeCollector::table_open_image_viewer);
    QObject::connect(actionOpenDirectory, &QAction::triggered, this, &MemeCollector::table_open_directory);
#ifdef Q_OS_WIN
    QObject::connect(actionProperties, &QAction::triggered, this, &MemeCollector::table_properties);
#endif
    QObject::connect(actionAddNew, &QAction::triggered, this, &MemeCollector::table_add_new);
    QObject::connect(actionEdit, &QAction::triggered, this, &MemeCollector::table_edit);
    QObject::connect(actionRemove, &QAction::triggered, this, &MemeCollector::table_remove);
    QObject::connect(actionRemoveAll, &QAction::triggered, this, &MemeCollector::table_remove_all);

    QObject::connect(actionReadMeFile, &QAction::triggered, this, &MemeCollector::read_me);

    QObject::connect(radioURL, &QRadioButton::clicked, this, &MemeCollector::switch_to_URL);
    QObject::connect(radioClipboard, &QRadioButton::clicked, this, &MemeCollector::switch_to_clipboard);
    QObject::connect(radioPcDir, &QRadioButton::clicked, this, &MemeCollector::switch_to_pc_dir);

    QObject::connect(changeNameCheckBox, &QCheckBox::toggled, nameInputEdit, &QLineEdit::setEnabled);
    QObject::connect(selectPictureButton, &QCheckBox::clicked, this, &MemeCollector::select_image);

    QObject::connect(addNewButton, &QPushButton::clicked, this, &MemeCollector::table_add_new);
    QObject::connect(editButton, &QPushButton::clicked, this, &MemeCollector::table_edit);
    QObject::connect(removeButton, &QPushButton::clicked, this, &MemeCollector::table_remove);
    QObject::connect(removeAllButton, &QPushButton::clicked, this, &MemeCollector::table_remove_all);

    QObject::connect(collectButton, &QPushButton::clicked, this, &MemeCollector::collect);
    QObject::connect(optionsButton, &QPushButton::clicked, this, &MemeCollector::options);

    QObject::connect(downloader, &QNetworkAccessManager::finished, this, &MemeCollector::collecting_url_finished);

    QObject::connect(fileNotifier, &FileNotifier::progress, this, &MemeCollector::update_progress);
    QObject::connect(fileNotifier, &FileNotifier::finished, this, &MemeCollector::collecting_pc_directory_finished);

    QObject::connect(tableWidget, &QTableWidget::itemSelectionChanged, this, &MemeCollector::handle_selection_changed);
}


void MemeCollector::connect_menus() {
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
        tableWidget, &QTableWidget::customContextMenuRequested,
    [&](const QPoint & point) {
        menuEdit->exec(tableWidget->viewport()->mapToGlobal(point));
    }
    );

    trayIcon->setIcon(this->windowIcon());
    trayIcon->setContextMenu(menuFile);
    QObject::connect(
        trayIcon,
        &QSystemTrayIcon::activated,
    [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            exit(1);
            actionShow->trigger();
        }
    }
    );
}






std::pair<QString, QString> MemeCollector::file_name_and_validate_url(QUrl & url, QString &urlStr) {
    if (urlStr.isEmpty())
        throw "URL input is empty";
    if (!url.isValid())
        throw "Entered URL is invalid";
    if (url.isLocalFile())
        throw "Entered URL is local file - cannot download";

    urlStr = url.url();
    if (urlStr.startsWith("https", Qt::CaseInsensitive)) {
        urlStr.replace(0, 5, "http");
        url = urlStr;
    }

    QNetworkRequest request(url);
    QNetworkAccessManager headerDownloader;
    QNetworkReply *reply = headerDownloader.head(request);
    QEventLoop loop;

    QObject::connect(&headerDownloader, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    tableWidget->setEnabled(false);
    loop.exec();
    tableWidget->setEnabled(true);

    if (reply->error() != QNetworkReply::NoError) {
        throw "Header downloading error";
        reply->deleteLater();
    }
    QVariant headerVariant = reply->header(QNetworkRequest::ContentTypeHeader);
    reply->deleteLater();
    if (headerVariant.isNull())
        throw "Header does not contain content type";
    QString contentType = headerVariant.toString();
    int pos = contentType.indexOf(';');
    if (pos != -1)
        contentType.resize(pos);

    ImageMIME mime = ImageMIME::by_media_type(contentType);
    if (!mime.is_valid())
        throw "URL links to other media type than image";

    return std::make_pair(
               QFileInfo(url.fileName()).completeBaseName(), mime.preffered_suffix()
           );
}


std::pair<QString, QString> MemeCollector::file_name_and_validate_local(QString & localPath) {
    if (localPath.isEmpty())
        throw "Path input field is empty";

    QFileInfo fileInfo(localPath);

    if (!fileInfo.exists())
        throw "Entered path is invalid";
    auto fileSuffixPair = std::make_pair(fileInfo.completeBaseName(), fileInfo.suffix());
    if (!ImageMIME::by_suffix(fileSuffixPair.second).is_valid())
        throw "Path contains media type other than image";

    return fileSuffixPair;
}







QString MemeCollector::save_path_and_overwrite_check(QString & fileName, QString & suffix) {
    QString optionalName = nameInputEdit->text();
    if (changeNameCheckBox->isChecked() && !optionalName.isEmpty())
        fileName = optionalName;

    int selectedRow = tableWidget->selectedRanges().first().topRow();
    QString thumbnailsDirectory = tableWidget->item(selectedRow, 1)->text();
    QString path = QDir(thumbnailsDirectory).filePath(fileName + "." + suffix);
    if (programConfig.askBeforeOverwriting && QFileInfo(path).exists()) {
        int button = QMessageBox::question(
                         this, QStringLiteral("File Overwrite"),
                         QStringLiteral("The file already exists, do you want to overwrite it?"),
                         QMessageBox::Yes | QMessageBox::No
                     );
        if (button == QMessageBox::Yes)
            return path;
        return QString();
    }
    return path;
}
















void MemeCollector::collect_url() {
    QString urlStr = inputEdit->text();
    QUrl url = QUrl::fromUserInput(urlStr);
    std::pair<QString, QString> fileSuffixPair;

    try {
        fileSuffixPair = this->file_name_and_validate_url(url, urlStr);
    } catch (const char *error) {
        QMessageBox::information(this, QStringLiteral("Input Error"), error);
        return;
    }

    imageSavePath = this->save_path_and_overwrite_check(fileSuffixPair.first, fileSuffixPair.second);
    if (imageSavePath.isEmpty())
        return;

    QNetworkReply *reply = downloader->get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &MemeCollector::update_progress);
}



void MemeCollector::collect_clipboard() {
    QString fileName = nameInputEdit->text();
    if (fileName.isEmpty()) {
        QMessageBox::information(
            this, QStringLiteral("Input Error"),
            QStringLiteral("File name input field is empty")
        );
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    QImage image = clipboard->image();

    if (image.isNull()) {
        QMessageBox::critical(
            this, QStringLiteral("Critical Error"),
            QStringLiteral("Clipboard does not contain valid image")
        );
        return;
    }

    QString suffix = ProgramConfig::suffix(programConfig.clipboardSaveFormat);
    imageSavePath = this->save_path_and_overwrite_check(fileName, suffix);
    if (imageSavePath.isEmpty())
        return;

    if (!image.save(imageSavePath, suffix.toStdString().c_str(), 100))
        QMessageBox::critical(
            this, QStringLiteral("Critical Error"),
            QStringLiteral("Saving data to hard drive failed (check if file name is valid)")
        );
    else if (programConfig.showMsgOnComplete)
        this->collecting_complete_message();
}



void MemeCollector::collect_pc_directory() {
    QString localPath = inputEdit->text();
    auto fileSuffixPair = file_name_and_validate_local(localPath);



    imageSavePath = this->save_path_and_overwrite_check(fileSuffixPair.first, fileSuffixPair.second);
    if (imageSavePath.isEmpty())
        return;

    fileNotifier->copy(localPath, imageSavePath);
}

void MemeCollector::collecting_complete_message() {
    QMessageBox msg(this);
    QCheckBox checkBox(QStringLiteral("Don't show this again"));

    msg.setWindowTitle("Operation Successful");
    msg.setIcon(QMessageBox::Information);
    msg.setText(QStringLiteral("Saving data to hard drive successfully completed"));
    msg.setCheckBox(&checkBox);
    msg.exec();

    if (checkBox.isChecked())
        programConfig.showMsgOnComplete = false;
}
















void MemeCollector::table_select_row(unsigned rowNumber) {
    if (rowNumber != -1) {
        tableWidget->clearSelection();
        tableWidget->setRangeSelected(
            QTableWidgetSelectionRange(rowNumber, 0, rowNumber, 1), true
        );
    }
}








void MemeCollector::read_database() {
    QFile file(DATABASE_FILE);
    if (!file.open(QFile::ReadOnly))
        return;

    QXmlStreamReader reader(&file);

    try {
        unsigned rowCount;

        if (!reader.readNextStartElement())
            throw "this is not a valid XML document";

        QXmlStreamAttributes attributes = reader.attributes();
        if (attributes.hasAttribute("count")) {
            bool ok;
            rowCount = attributes.value("count").toUInt(&ok);
            if (!ok)
                throw "row count not valid";
        } else
            throw "row count not found";


        for (unsigned i = 0; i < rowCount; ++i)
            this->read_database_item(reader, i);

        while (reader.readNext() == QXmlStreamReader::Characters && reader.isWhitespace());
        if (reader.tokenType() != QXmlStreamReader::EndElement)
            throw "invalid element";

        while (reader.readNext() == QXmlStreamReader::Characters && reader.isWhitespace());
        if (reader.tokenType() != QXmlStreamReader::EndDocument)
            throw "invalid element";
    }


    catch (const char *errorText) {
        QMessageBox::critical(
            this, QStringLiteral("Critical Error"),
            QString("Database reading error - ") + errorText
        );
        this->save_database();
    }
}

void MemeCollector::read_database_item(QXmlStreamReader &reader, unsigned rowNumber) {
    const char *invElem = "invalid element";
    QString name;
    QString path;


    if (!reader.readNextStartElement())
        throw invElem;

    if (!reader.readNextStartElement())
        throw invElem;
    name = reader.readElementText();

    if (!reader.readNextStartElement())
        throw invElem;
    path = reader.readElementText();

    if (name.isEmpty())
        throw "reading list element failed";
    if (path.isEmpty())
        throw "reading list element failed";

    tableWidget->insertRow(rowNumber);
    tableWidget->setItem(rowNumber, 0, new QTableWidgetItem(name));
    tableWidget->setItem(rowNumber, 1, new QTableWidgetItem(path));

    while (reader.readNext() == QXmlStreamReader::Characters && reader.isWhitespace());
    if (reader.tokenType() != QXmlStreamReader::EndElement)
        throw invElem;
}

void MemeCollector::save_database() {
    QFile file(DATABASE_FILE);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(
            this, QStringLiteral("Critical Error"),
            QStringLiteral("Could not save list database")
        );
        return;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);

    unsigned rowCount = tableWidget->rowCount();

    writer.writeStartDocument();
    writer.writeStartElement("MemeTypes");
    writer.writeAttribute("count", QString::number(rowCount));

    QTableWidgetItem *name;
    QTableWidgetItem *path;
    for (unsigned i = 0; i < rowCount; ++i) {
        name = tableWidget->item(i, 0);
        path = tableWidget->item(i, 1);
        writer.writeStartElement("MemeType");
        writer.writeTextElement("Name", name->text());
        writer.writeTextElement("Path", path->text());
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

}









void MemeCollector::register_shortcuts() {
    showHotkey.setRegistered(false);
    quicksaveUrlHotkey.setRegistered(false);
    quicksaveClipboardHotkey.setRegistered(false);

    switch (programConfig.shortcutMode) {
    case ProgramConfig::ShortcutMode::ShowAndSaveShortcuts:
        quicksaveUrlHotkey.setRegistered(true);
        quicksaveClipboardHotkey.setRegistered(true);
    case ProgramConfig::ShortcutMode::ShowShortcut:
        showHotkey.setRegistered(true);
    }

}




void MemeCollector::add_to_autostart() {
    autostart::add_to_autostart(this->windowTitle(), QApplication::applicationFilePath());
}

void MemeCollector::remove_from_autostart() {
    autostart::remove_from_autostart(this->windowTitle());
}






void MemeCollector::initial_window_show() {
    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() == 2 && arguments[1] == QStringLiteral("--autostart")) {
        if (programConfig.startHidden || programConfig.autostartMode == ProgramConfig::AutostartMode::OnHidden)
            this->hide();
        else
            this->show();
    } else if (programConfig.startHidden)
        this->hide();
    else
        this->show();
}

void MemeCollector::apply_settings() {
    this->register_shortcuts();
    if (programConfig.autostartMode == ProgramConfig::AutostartMode::Off)
        this->remove_from_autostart();
    else
        this->add_to_autostart();
}

void MemeCollector::validate_cache() {
    std::vector<QString> vec(tableWidget->rowCount());
    for (std::size_t i = 0; i < vec.size(); ++i)
        vec[i] = tableWidget->item(i, 0)->text();
    std::sort(vec.begin(), vec.end());
    cacheMapManager.validate(vec.begin(), vec.end());
}






const QString MemeCollector::CONFIG_FILE = QStringLiteral("config.ini");
const QString MemeCollector::DATABASE_FILE = QStringLiteral("MemeDatabase.xml");
