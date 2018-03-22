#include "stdafx.h"
#include "CachedImage.hpp"

CachedImage::CachedImage(QDateTime modificationTime, QImage image) noexcept
	: modificationTime(std::move(modificationTime))
	, image(std::move(image)) {
}

bool CachedImage::is_empty() const {
	return modificationTime.isNull() && image.isNull();
}

bool CachedImage::is_valid(const QFileInfo & fileInfo) const {
	return fileInfo.lastModified() == modificationTime;
}

bool CachedImage::is_valid(const QDateTime & modificationTime) const {
	return modificationTime == this->modificationTime;
}


QDataStream & operator>>(QDataStream & stream, CachedImage & cachedImage) {
	return stream >> cachedImage.modificationTime >> cachedImage.image;
}

QDataStream & operator<<(QDataStream & stream, const CachedImage & cachedImage) {
	return stream << cachedImage.modificationTime << cachedImage.image;
}

