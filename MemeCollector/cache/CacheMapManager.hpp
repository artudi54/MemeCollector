#pragma once

#include <set>
#include "ImageCacheMap.hpp"


class CacheMapManager {
public:
	CacheMapManager(QDir directory);
	~CacheMapManager();

	template < class InputIterator >
	void validate(InputIterator beg, InputIterator end);

	SharedImageCacheMap add(const QString &name);
	SharedImageCacheMap get(const QString &name);
	SharedImageCacheMap rename(const QString &oldName, const QString &newName);
	void remove(const QString &name);
	void clear();
private:
	std::set<SharedImageCacheMap>::iterator get_iterator(const QString &name);

	QDir directory;
	std::set<SharedImageCacheMap> cacheMaps;
};




template<class InputIterator>
inline void CacheMapManager::validate(InputIterator it, InputIterator end) {
	auto mapIt = cacheMaps.begin(), mapEnd = cacheMaps.end();
	QString correctCacheFile;
	int compareResult;

	while (mapIt != mapEnd && it != end) {
		correctCacheFile = directory.absoluteFilePath(*it + ".dat");
		compareResult = (*mapIt)->get_cache_file().compare(correctCacheFile);
		if (compareResult < 0) {
			QFile::remove((*mapIt)->get_cache_file());
			cacheMaps.erase(mapIt++);
		} else if (compareResult > 0) {
			this->add(*it);
			++it;
		} else
			++mapIt, it;
	}

	while (mapIt != mapEnd) {
		QFile::remove((*mapIt)->get_cache_file());
		cacheMaps.erase(mapIt++);
	}

	while (it != end) {
		this->add(*it);
		++it;
	}
}
