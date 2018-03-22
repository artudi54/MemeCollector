#pragma once

#include "cache/ImageCacheMap.hpp"

class ImageLoader : public QObject {
	Q_OBJECT
public:
	explicit ImageLoader(QObject *parent = nullptr);
	ImageLoader(const SharedImageCacheMap &map, QDir directory, QObject *parent = nullptr);
	virtual ~ImageLoader();

	void set_directory(QDir directory);
	const QDir& get_directory() const;

	void process_directory();
	void stop();
signals:
	void finished();
	void next_info(QFileInfo info, QImage image);
private:
	SharedImageCacheMap map;
	std::atomic_bool stopFlag;
	QDir directory;
};
