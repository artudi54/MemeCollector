#include "stdafx.h"
#include "ImageViewer.hpp"

ImageViewer::ImageViewer(QWidget *parent)
	: QWidget(parent, Qt::Tool)
	, cacheMapManager(nullptr) {
	this->setupUi(this);
	if (parent != nullptr) {
		parent->installEventFilter(this);
		this->move_to_parent();
	}
}

ImageViewer::~ImageViewer(){}

void ImageViewer::set_cache_map_manager(CacheMapManager & cacheMapManager) {
	this->cacheMapManager = &cacheMapManager;
}


void ImageViewer::set_directory(const QString &name, const QString &path) {
	imageView->set_directory(cacheMapManager->get(name), QDir(path));
}






bool ImageViewer::eventFilter(QObject * watched, QEvent * event) {
	if (event->type() == QEvent::Move)
		this->move_to_parent();
	return QObject::eventFilter(watched, event);
}

void ImageViewer::showEvent(QShowEvent * event) {
	if (this->parentWidget() != nullptr)
		this->move_to_parent();
}






QPoint ImageViewer::parent_point() const {
	QWindow *window = this->parentWidget()->windowHandle();
	if (window != nullptr)
		return window->frameGeometry().topRight();
	return QPoint();
}

void ImageViewer::move_to_parent() {
	this->move(this->parent_point());
}