#pragma once


class CachedImage {
public:
	CachedImage() noexcept(std::is_nothrow_constructible_v<QDateTime>) {};
	CachedImage(const CachedImage&) = default;
	CachedImage(CachedImage&&) noexcept = default;
	CachedImage& operator=(const CachedImage&) = default;
	CachedImage& operator=(CachedImage&&) noexcept = default;
	~CachedImage() = default;

	CachedImage(QDateTime modificationTime, QImage image) noexcept;

	bool is_empty() const;
	bool is_valid(const QFileInfo &fileInfo) const;
	bool is_valid(const QDateTime &modificationTime) const;

	friend QDataStream& operator>>(QDataStream &stream, CachedImage &cachedImage);
	friend QDataStream& operator<<(QDataStream &stream, const CachedImage &cachedImage);

	QDateTime modificationTime;
	QImage image;

};

