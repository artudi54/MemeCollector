#pragma once

#include "ImageCacheMap.hpp"

class ImagesLoader : public QObject {
	Q_OBJECT
public:
	explicit ImagesLoader(QObject *parent = nullptr);
	ImagesLoader(ImageCacheMap *map, QDir directory, QObject *parent = nullptr);
	virtual ~ImagesLoader();

	void set_directory(QDir directory);
	const QDir& get_directory() const;

	void process_directory();
	void stop();
signals:
	void finished();
	void next_info(QFileInfo info, QImage image);
private:
	ImageCacheMap *map;
	std::atomic_bool stopFlag;
	QDir directory;
};
