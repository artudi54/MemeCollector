#pragma once

#include "CachedImage.hpp"

namespace std {
	template<> class hash<QString> {
	public:
		std::size_t operator()(const QString &string) const {
			return qHash(string);
		}
	};
}


class ImageCacheMap {
	friend class CacheMapManager;
public:
	using SharedImageCacheMap = std::shared_ptr<ImageCacheMap>;

	explicit ImageCacheMap(QString cacheFile);
	ImageCacheMap(ImageCacheMap &&other);
	ImageCacheMap& operator=(ImageCacheMap &&other);
	~ImageCacheMap();

	const QString& get_cache_file() const;

	bool sync() const;

	CachedImage get(const QString &path) const;
	void set(const QString &path, CachedImage cachedImage);
	void remove(const QString &path);
	void clear();

	bool operator<(const ImageCacheMap &other) const;
	friend bool operator<(const SharedImageCacheMap &lhs, const SharedImageCacheMap &rhs);
private:
	ImageCacheMap();
	void load() const;

	mutable std::unordered_map<QString, CachedImage> cacheMap;
	QString cacheFile;
	mutable bool loaded;
	bool changed;
	mutable std::mutex mutex;
};
using SharedImageCacheMap = ImageCacheMap::SharedImageCacheMap;