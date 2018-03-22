#include "stdafx.h"
#include "CacheMapManager.hpp"

CacheMapManager::CacheMapManager(QDir directory)
	: directory(std::move(directory))
	, cacheMaps() {
	for (auto &fileInfo : directory.entryInfoList(QDir::Files | QDir::Hidden))
		cacheMaps.insert(std::make_shared<ImageCacheMap>(fileInfo.absoluteFilePath()));
}

CacheMapManager::~CacheMapManager() {}





SharedImageCacheMap CacheMapManager::add(const QString & name) {
	return * cacheMaps.insert(
		std::make_shared<ImageCacheMap>(directory.absoluteFilePath(name + ".dat"))
	).first;
}

SharedImageCacheMap CacheMapManager::get(const QString & name) {
	auto it = this->get_iterator(name);


	if (it == cacheMaps.end())
		return this->add(name);

	return *it;
}

SharedImageCacheMap CacheMapManager::rename(const QString & oldName, const QString & newName) {
	QString newCacheFile = directory.absoluteFilePath(newName + ".dat");
	auto it = this->get_iterator(oldName);


	if (it == cacheMaps.end())
		return this->add(newName);

	SharedImageCacheMap toMove = *it;
	cacheMaps.erase(it);

	std::unique_lock<std::mutex>(toMove->mutex);
	QFile::rename(toMove->get_cache_file(), newCacheFile);
	toMove->cacheFile = newCacheFile;
	return *cacheMaps.insert(toMove).first;
}

void CacheMapManager::remove(const QString & name) {
	auto it = this->get_iterator(name);

	if (it != cacheMaps.end()) {
		QFile::remove((*it)->get_cache_file());
		(*it)->clear();
		cacheMaps.erase(it);
	}
}

void CacheMapManager::clear() {
	for (auto it = cacheMaps.begin(); it != cacheMaps.end();) {
		(*it)->clear();
		cacheMaps.erase(it++);
	}
}






std::set<SharedImageCacheMap>::iterator CacheMapManager::get_iterator(const QString & name) {
	ImageCacheMap m(directory.absoluteFilePath(name + ".dat"));
	std::shared_ptr<ImageCacheMap> map(&m, [](auto) {});
	auto it = cacheMaps.find(map);
	return it;
}
