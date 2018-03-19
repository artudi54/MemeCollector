#include "stdafx.h"
#include "ImageCacheMap.hpp"

ImageCacheMap::ImageCacheMap(QString cacheFile)
	: cacheMap()
	, cacheFile()
	, mutex() {
	this->read(std::move(cacheFile));
}

ImageCacheMap::~ImageCacheMap() {
	this->sync();
}

bool ImageCacheMap::read(QString cacheFile) {
	QFile file(cacheFile);
	file.open(QIODevice::ReadOnly);
	if (!file.isOpen())
		return false;

	QDataStream stream(&file);
	std::pair<QString, CachedImage> pair;
	std::uint64_t size;
	stream >> size;
	while (size--) {
		stream >> pair.first >> pair.second;
		cacheMap.insert(std::move(pair));
	}
	return true;
}

const QString & ImageCacheMap::get_cache_file() const {
	return cacheFile;
}

bool ImageCacheMap::sync() const {
	QFile file(cacheFile);
	file.open(QIODevice::WriteOnly);
	if (!file.isOpen())
		return false;
	QDataStream stream(&file);

	std::lock_guard<std::mutex> lock(mutex);
	std::uint64_t size = cacheMap.size();
	stream << size;
	for (auto &item : cacheMap) {
		QFileInfo info(item.first);
		if (info.exists() && item.second.is_valid(info))
			stream << item.first << item.second;
	}
	return true;
}











CachedImage ImageCacheMap::get(const QString & path) const {
	std::lock_guard<std::mutex> lock(mutex);
	auto it = cacheMap.find(path);
	if (it != cacheMap.end())
		return it->second;
	return CachedImage();
}

void ImageCacheMap::set(const QString & path, CachedImage cachedImage) {
	std::lock_guard<std::mutex> lock(mutex);
	cacheMap[path] = std::move(cachedImage);
}

void ImageCacheMap::remove(const QString & path) {
	std::lock_guard<std::mutex> lock(mutex);
	cacheMap.erase(path);
}









