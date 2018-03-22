#include "stdafx.h"
#include "ImageCacheMap.hpp"

ImageCacheMap::ImageCacheMap(QString cacheFile)
	: cacheMap()
	, cacheFile(std::move(cacheFile))
	, loaded(false)
	, changed(false)
	, mutex() {
}



ImageCacheMap::ImageCacheMap(ImageCacheMap && other)
	: cacheMap(std::move(other.cacheMap)), cacheFile(std::move(other.cacheFile))
	, loaded(other.loaded)
	, mutex() {}

ImageCacheMap & ImageCacheMap::operator=(ImageCacheMap && other) {
	cacheMap = std::move(other.cacheMap);
	cacheFile = std::move(other.cacheFile);
	loaded = other.loaded;
	return *this;
}

ImageCacheMap::~ImageCacheMap() {
	this->sync();
}



const QString & ImageCacheMap::get_cache_file() const {
	std::lock_guard<std::mutex> lock(mutex);
	return cacheFile;
}

bool ImageCacheMap::sync() const {
	std::lock_guard<std::mutex> lock(mutex);
	if (!loaded || !changed)
		return true;
	if (cacheFile.isEmpty())
		return false;
	qDebug() << "saving";
	QFile file(cacheFile);
	file.open(QIODevice::WriteOnly);
	if (!file.isOpen())
		return false;
	QDataStream stream(&file);

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
	if (!loaded) {
		this->load();
		loaded = true;
	}
	auto it = cacheMap.find(path);
	if (it != cacheMap.end())
		return it->second;
	return CachedImage();
}

void ImageCacheMap::set(const QString & path, CachedImage cachedImage) {
	std::lock_guard<std::mutex> lock(mutex);
	if (!loaded) {
		this->load();
		loaded = true;
	}
	cacheMap[path] = std::move(cachedImage);
	changed = true;
}

void ImageCacheMap::remove(const QString & path) {
	std::lock_guard<std::mutex> lock(mutex);
	if (!loaded) {
		this->load();
		loaded = true;
	}
	if (cacheMap.erase(path) != 0)
		changed = true;
}

void ImageCacheMap::clear() {
	std::lock_guard<std::mutex> lock(mutex);
	loaded = true;
	cacheFile.clear();
	cacheMap.clear();
	changed = true;
}

bool ImageCacheMap::operator<(const ImageCacheMap & other) const {
	std::lock_guard<std::mutex> lock(mutex);
	return cacheFile < other.cacheFile;
}









ImageCacheMap::ImageCacheMap()
	: cacheMap(), cacheFile(), loaded(false), changed(false), mutex() {}






void ImageCacheMap::load() const {
	if (cacheFile.isEmpty())
		return;
	qDebug() << "loading";
	QFile file(cacheFile);
	file.open(QIODevice::ReadOnly);
	if (!file.isOpen())
		return;

	QDataStream stream(&file);
	std::pair<QString, CachedImage> pair;
	std::uint64_t size;
	stream >> size;
	if (stream.status() != QDataStream::Ok) {
		cacheMap.clear();
		return;
	}
	while (size--) {
		stream >> pair.first >> pair.second;
		if (stream.status() != QDataStream::Ok) {
			cacheMap.clear();
			return;
		}
		try {
			cacheMap.insert(std::move(pair));
		}
		catch (std::exception&) {
			cacheMap.clear();
			return;
		}
	}
	return;
}

bool operator<(const SharedImageCacheMap & lhs, const SharedImageCacheMap & rhs) {
	return *lhs < *rhs;
}
