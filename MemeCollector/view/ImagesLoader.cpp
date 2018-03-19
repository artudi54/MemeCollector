#include "stdafx.h"
#include "ImagesLoader.hpp"

ImagesLoader::ImagesLoader(QObject *parent)
	: QObject(parent)
	, map(nullptr)
	, directory()
	, stopFlag(false) {}

ImagesLoader::ImagesLoader(ImageCacheMap * map, QDir directory, QObject * parent)
	: QObject(parent)
	, map(map)
	, directory(std::move(directory))
	, stopFlag(false) {
}

ImagesLoader::~ImagesLoader() {}






void ImagesLoader::set_directory(QDir directory) {
	this->directory = std::move(directory);
}

const QDir & ImagesLoader::get_directory() const {
	return directory;
}




void ImagesLoader::process_directory() {
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
			image = image.scaled(600, 600, Qt::KeepAspectRatio, Qt::FastTransformation)
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

void ImagesLoader::stop() {
	stopFlag = true;
}
