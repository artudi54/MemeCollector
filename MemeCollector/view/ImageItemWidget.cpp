#include "stdafx.h"
#include "ImageItemWidget.hpp"



ImageItemStackedWidget::ImageItemStackedWidget(QWidget * parent)
    : QStackedWidget(parent) {}

ImageItemStackedWidget::~ImageItemStackedWidget() {}

QSize ImageItemStackedWidget::minimumSizeHint() const {
    if (this->currentIndex() != -1)
        return this->currentWidget()->minimumSizeHint();
    return QSize();
}

QSize ImageItemStackedWidget::sizeHint() const {
    if (this->currentIndex() != -1)
        return this->currentWidget()->sizeHint();
    return QSize();
}






ImageItemWidgetEdit::ImageItemWidgetEdit(QWidget * parent)
    : QLineEdit(parent) {}

ImageItemWidgetEdit::~ImageItemWidgetEdit() {}

void ImageItemWidgetEdit::focusInEvent(QFocusEvent * event) {
    QString text = this->text();
    int pos = text.lastIndexOf(QChar('.'));
    if (pos != -1)
        this->setSelection(0, pos);
    else
        this->selectAll();
    this->update();
    event->accept();
}





ImageItemWidget::ImageItemWidget(QWidget *parent)
    : QWidget(parent)
    , layout(this)
    , stackedEditor(this)
    , labelPixmap(this)
    , labelTitle(this)
    , editTitle(this)
    , pathInfo() {
    constexpr QSize PIXMAP_SIZE(120, 120);
    this->setLayout(&layout);
    layout.setMargin(10);
    layout.setSpacing(10);
    labelPixmap.setMinimumSize(PIXMAP_SIZE);
    labelPixmap.setMaximumSize(PIXMAP_SIZE);
    labelPixmap.setAlignment(Qt::AlignCenter);
    labelTitle.setMinimumWidth(PIXMAP_SIZE.width());
    labelTitle.setMaximumWidth(PIXMAP_SIZE.width());
    labelTitle.setAlignment(Qt::AlignHCenter);
    editTitle.setAlignment(Qt::AlignHCenter);
    stackedEditor.addWidget(&labelTitle);
    stackedEditor.addWidget(&editTitle);
    stackedEditor.setCurrentIndex(0);
    layout.addWidget(&labelPixmap, Qt::AlignCenter);
    layout.addWidget(&stackedEditor);
    this->setFocusProxy(&editTitle);
    QObject::connect(&editTitle, &ImageItemWidgetEdit::editingFinished, this, &ImageItemWidget::editing_finished);
}

ImageItemWidget::ImageItemWidget(const QFileInfo &pathInfo, const QPixmap &pixmap, QWidget * parent)
    : ImageItemWidget(parent) {
    this->set_info(pathInfo);
    labelPixmap.setPixmap(pixmap);
}


ImageItemWidget::~ImageItemWidget() {}

void ImageItemWidget::set_info(QFileInfo _pathInfo) {
    pathInfo = std::move(_pathInfo);
    QString title = pathInfo.fileName();
    labelTitle.setText(title);
    editTitle.setText(title);
    this->setToolTip(title);
    QFontMetrics metrics(labelTitle.font());
    labelTitle.setText(metrics.elidedText(title, Qt::ElideRight, labelTitle.width()));
}

const QFileInfo& ImageItemWidget::get_info() const {
    return pathInfo;
}

QString ImageItemWidget::get_title() const {
    return pathInfo.fileName();
}

void ImageItemWidget::set_pixmap(const QPixmap & pixmap) {
    labelPixmap.setPixmap(pixmap);
}

const QPixmap* ImageItemWidget::get_pixmap() const {
    return labelPixmap.pixmap();
}

bool ImageItemWidget::verify_and_update() {
    QString fileName = editTitle.text().trimmed();
    if (fileName.isEmpty() || fileName == pathInfo.fileName())
        return false;
    QFileInfo changedInfo = QFileInfo(pathInfo.dir().filePath(fileName));
    if (changedInfo.suffix() != pathInfo.suffix()) {
        QMessageBox::Button button = QMessageBox::question(
                                         this->parentWidget(),
                                         QStringLiteral("Renaming"),
                                         QStringLiteral("If you change the file's extension, the file may become unusable. Do you want to continue?")
                                     );
        if (button != QMessageBox::Yes)
            return false;
    }
    if (!QFile::rename(pathInfo.absoluteFilePath(), changedInfo.absoluteFilePath())) {
        QMessageBox::warning(
            this->parentWidget(),
            QStringLiteral("Renaming failed"),
            "Could not set filename to: " + fileName
        );
        return false;
    }
    this->set_info(changedInfo);
    return true;
}


void ImageItemWidget::start_editing() {
    stackedEditor.setCurrentIndex(1);

}
