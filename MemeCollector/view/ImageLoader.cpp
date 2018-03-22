#include "stdafx.h"
#include "ImageLoader.hpp"

ImageLoader::ImageLoader(QObject *parent)
	: QObject(parent)
	, map(nullptr)
	, directory()
	, stopFlag(false) {}

ImageLoader::ImageLoader(const SharedImageCacheMap &map, QDir directory, QObject * parent)
	: QObject(parent)
	, map(map)
	, directory(std::move(directory))
	, stopFlag(false) {
}

ImageLoader::~ImageLoader() {}






void ImageLoader::set_directory(QDir directory) {
	this->directory = std::move(directory);
}

const QDir & ImageLoader::get_directory() const {
	return directory;
}




void ImageLoader::process_directory() {
	if (!directory.exists())
		emit finished();
	for (QFileInfo &info : directory.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase)) {
		if (stopFlag)
			break;
		QString path = info.absoluteFilePath();
		QImage image;
		CachedImage cachedImage = map->get(path);
		if (cachedImage.is_empty() || !cachedImage.is_valid(info)) {
			image = QImage(info.absoluteFilePath());
			if (image.isNull())
				continue;
			image = image.scaled(400, 400, Qt::KeepAspectRatio, Qt::FastTransformation)
						  .scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			cachedImage.image = image;
			cachedImage.modificationTime = info.lastModified();
			map->set(path, cachedImage);
		} else
			image = std::move(cachedImage.image);
		emit next_info(info, image);
	}
	emit finished();
}

void ImageLoader::stop() {
	stopFlag = true;
}
