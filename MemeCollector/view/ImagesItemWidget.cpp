#include "stdafx.h"
#include "ImagesItemWidget.hpp"



ImagesItemStackedWidget::ImagesItemStackedWidget(QWidget * parent)
	: QStackedWidget(parent) {}

ImagesItemStackedWidget::~ImagesItemStackedWidget() {}

QSize ImagesItemStackedWidget::minimumSizeHint() const {
	if (this->currentIndex() != -1)
		return this->currentWidget()->minimumSizeHint();
	return QSize();
}

QSize ImagesItemStackedWidget::sizeHint() const {
	if (this->currentIndex() != -1)
		return this->currentWidget()->sizeHint();
	return QSize();
}






ImagesItemWidgetEdit::ImagesItemWidgetEdit(QWidget * parent)
	: QLineEdit(parent) {}

ImagesItemWidgetEdit::~ImagesItemWidgetEdit() {}

void ImagesItemWidgetEdit::focusInEvent(QFocusEvent * event) {
	QString text = this->text();
	int pos = text.lastIndexOf(QChar('.'));
	if (pos != -1)
		this->setSelection(0, pos);
	else
		this->selectAll();
	this->update();
	event->accept();
}





ImagesItemWidget::ImagesItemWidget(QWidget *parent)
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
	QObject::connect(&editTitle, &ImagesItemWidgetEdit::editingFinished, this, &ImagesItemWidget::editing_finished);
}

ImagesItemWidget::ImagesItemWidget(const QFileInfo &pathInfo, const QPixmap &pixmap, QWidget * parent)
	: ImagesItemWidget(parent) {
	this->set_info(pathInfo);
	labelPixmap.setPixmap(pixmap);
}


ImagesItemWidget::~ImagesItemWidget(){}

void ImagesItemWidget::set_info(QFileInfo _pathInfo) {
	pathInfo = std::move(_pathInfo);
	QString title = pathInfo.fileName();
	labelTitle.setText(title);
	editTitle.setText(title);
	this->setToolTip(title);
	QFontMetrics metrics(labelTitle.font());
	labelTitle.setText(metrics.elidedText(title, Qt::ElideRight, labelTitle.width()));
}

const QFileInfo& ImagesItemWidget::get_info() const {
	return pathInfo;
}

QString ImagesItemWidget::get_title() const {
	return pathInfo.fileName();
}

void ImagesItemWidget::set_pixmap(const QPixmap & pixmap) {
	labelPixmap.setPixmap(pixmap);
}

const QPixmap* ImagesItemWidget::get_pixmap() const {
	return labelPixmap.pixmap();
}

bool ImagesItemWidget::verify_and_update() {
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


void ImagesItemWidget::start_editing() {
	stackedEditor.setCurrentIndex(1);

}
