#pragma once

#include "CachedImage.hpp"
//apply this more
namespace std {
	template<> class hash<QString> {
	public:
		std::size_t operator()(const QString &string) const {
			return qHash(string);
		}
	};
}


class ImageCacheMap {
public:

	ImageCacheMap(QString cacheFile);
	~ImageCacheMap();

	bool read(QString cacheFile);
	const QString& get_cache_file() const;

	bool sync() const;

	CachedImage get(const QString &path) const;
	void set(const QString &path, CachedImage cachedImage);
	void remove(const QString &path);

private:

	std::unordered_map<QString, CachedImage> cacheMap;
	QString cacheFile;
	mutable std::mutex mutex;
};

