#pragma once

#include "ImagesLoader.hpp"
#include "ImagesItemModel.hpp"
#include "ImagesItemDelegate.hpp"
#include "ImageCacheMap.hpp"

class ImagesTableView : public QTableView {
	Q_OBJECT
public:
	explicit ImagesTableView(QWidget *parent = nullptr);
	ImagesTableView(const QDir &directory, QWidget *parent = nullptr);
	~ImagesTableView();

	void set_directory(const QDir &directory);
	const QDir& get_directory() const;

	void clear();

	virtual void keyboardSearch(const QString &search) override;
signals:
	void loading_started();
	void loading_finished();
protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual int sizeHintForColumn(int column) const override;
	virtual int sizeHintForRow(int row) const override;
private slots:
	void configure_actions(const QItemSelection&, const QItemSelection&);
	void show_menu(const QPoint &pos);

	void update_cache(const QModelIndex & index);

	void action_open();
	void action_rename();
#ifdef Q_OS_WIN	
	void action_set_as_desktop_wallpaper();
#endif
	void action_print();
#ifdef Q_OS_WIN	
	void action_edit();
#endif
	void action_rotate_right();
	void action_rotate_left();
	void action_flip_vertically();
	void action_flip_horizontally();
	void action_copy_image_data();
	void action_copy_file();
	void action_delete();
#ifdef Q_OS_WIN	
	void action_properties();
#endif

	void open_file(const QModelIndex &index);
private:
	void make_menu();
	void connect_signals();
	void stop_thread();
	QDir directory;
	QString searchText;
	std::chrono::steady_clock::time_point prevTime, curTime;

	ImageCacheMap cacheMap;

	QThread *loaderThread; //make on stack
	ImagesLoader *imagesLoader;
	ImagesItemModel *imagesModel;
	ImagesItemDelegate *imagesDelegate;
	QMenu *editMenu;
	QAction *actionOpen;
	QAction *actionRename;
#ifdef Q_OS_WIN	
	QAction *actionSetAsDesktopBackground = nullptr;
#endif
	QAction *actionPrint;
#ifdef Q_OS_WIN	
	QAction *actionEdit;
#endif
	QAction *actionRotateRight;
	QAction *actionRotateLeft;
	QAction *actionFlipVertically;
	QAction *actionFlipHorizontally;
	QAction *actionCopyImageData;
	QAction *actionCopyFile;
	QAction *actionDelete;
#ifdef Q_OS_WIN	
	QAction *actionProperties;
#endif

};
