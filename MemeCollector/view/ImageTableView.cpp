#include "stdafx.h"
#include "ImageTableView.hpp"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif 


ImageTableView::ImageTableView(QWidget *parent)
	: QTableView(parent)
	, directory()
	, searchText()
	, prevTime(std::chrono::steady_clock::now())
	, curTime(prevTime)
	, imageCacheMap(nullptr)
	, loaderThread(this)
	, imagesLoader(nullptr)
	, imagesModel(new ImageItemModel(this))
	, imagesDelegate(new ImageItemDelegate(this))
	, editMenu(new QMenu(QStringLiteral("Edit"), this)) {

	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->horizontalHeader()->setVisible(false);
	this->verticalHeader()->setVisible(false);
	this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->horizontalHeader()->setMinimumSectionSize(0);
	this->horizontalHeader()->setDefaultSectionSize(120);
	this->verticalHeader()->setMinimumSectionSize(0);
	this->verticalHeader()->setDefaultSectionSize(120);

	this->viewport()->setAttribute(Qt::WA_Hover);

	this->setShowGrid(false);
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	this->setModel(imagesModel);
	this->setItemDelegate(imagesDelegate);
	imagesDelegate->set_padding(7);

	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setEditTriggers(QAbstractItemView::EditKeyPressed);

	this->make_menu();
	this->connect_signals();
}

ImageTableView::ImageTableView(const SharedImageCacheMap & imageCacheMap, const QDir & directory, QWidget * parent)
	: ImageTableView(parent) {
	this->set_directory(imageCacheMap, directory);
}

ImageTableView::~ImageTableView() {
	this->stop_thread();
}






void ImageTableView::set_directory(const SharedImageCacheMap & imageCacheMap, const QDir & directory) {
	this->stop_thread();
	this->directory = directory;
	this->imageCacheMap = imageCacheMap;

	QTimer::singleShot(0, this, [this] {
		imagesModel->clear();

		imagesLoader = new ImageLoader(this->imageCacheMap, this->directory);
		imagesLoader->moveToThread(&loaderThread);
		QObject::connect(&loaderThread, &QThread::started, imagesLoader, &ImageLoader::process_directory);
		QObject::connect(imagesLoader, &ImageLoader::finished, &loaderThread, &QThread::quit, Qt::DirectConnection);
		QObject::connect(imagesLoader, &ImageLoader::finished, this, &ImageTableView::loading_finished);
		QObject::connect(imagesLoader, &ImageLoader::finished, imagesLoader, &ImageLoader::deleteLater);
		QObject::connect(imagesLoader, &ImageLoader::next_info, imagesModel, &ImageItemModel::add_entry);

		loaderThread.start();
		emit loading_started();
	});
}

const QDir & ImageTableView::get_directory() const {
	return directory;
}








void ImageTableView::clear() {
	this->stop_thread();
	QTimer::singleShot(0, this, [this] { imagesModel->clear(); });
}






void ImageTableView::keyboardSearch(const QString & search) {
	if (search.isEmpty())
		return;
	prevTime = curTime;
	curTime = std::chrono::steady_clock::now();
	std::chrono::milliseconds timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
		curTime - prevTime
	);
	std::chrono::milliseconds maxTimeDiff(QApplication::keyboardInputInterval());
	if (timeDiff > maxTimeDiff)
		searchText.clear();

	searchText += search;
	QModelIndex index = imagesModel->closest_match(searchText);
	if (index.isValid()) {
		this->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
		this->scrollTo(index);
	}
}










void ImageTableView::mousePressEvent(QMouseEvent * event) {
	QModelIndex index = this->indexAt(event->pos());
	if (ImageItemModel::index_widget(index) == nullptr)
		this->clearSelection();
	QTableView::mousePressEvent(event);
}


void ImageTableView::resizeEvent(QResizeEvent * event) {
	imagesModel->set_size(event->size(), imagesDelegate->get_padding());
	QTableView::resizeEvent(event);
}


int ImageTableView::sizeHintForColumn(int column) const {
	QSize size = imagesModel->data(imagesModel->index(0, 0), Qt::SizeHintRole).value<QSize>();
	if (size.isValid())
		return size.width() + 2 * imagesDelegate->get_padding();
	return 0;
}


int ImageTableView::sizeHintForRow(int row) const {
	QSize size = imagesModel->data(imagesModel->index(0, 0), Qt::SizeHintRole).value<QSize>();
	if (size.isValid())
		return size.height() + 2 * imagesDelegate->get_padding();
	return 0;
}








void ImageTableView::configure_actions(const QItemSelection&, const QItemSelection&) {
	bool hasSelection = this->selectionModel()->hasSelection();
	bool hasSingleSelection = this->selectionModel()->selectedIndexes().size() == 1;

	actionOpen->setEnabled(hasSelection);
	actionRename->setEnabled(hasSelection);
	actionPrint->setEnabled(hasSelection);
	actionDelete->setEnabled(hasSelection);
	actionCopyFile->setEnabled(hasSelection);

	actionCopyImageData->setEnabled(hasSingleSelection);
	actionProperties->setEnabled(hasSingleSelection);
#ifdef Q_OS_WIN
	actionSetAsDesktopBackground->setEnabled(hasSelection);
	actionEdit->setEnabled(hasSelection);
	actionProperties->setEnabled(hasSelection);
#endif
}




void ImageTableView::show_menu(const QPoint &pos) {
	editMenu->exec(this->mapToGlobal(pos));
}





void ImageTableView::update_cache(const QModelIndex & index) {
	SharedImageItemWidget widget = ImageItemModel::index_widget(index);
	if (widget != nullptr) {
		QFileInfo info = widget->get_info();
		QImage image = widget->get_pixmap()->toImage();
		QString path = info.absoluteFilePath();
		imageCacheMap->set(path, CachedImage(info.lastModified(), image));
	}
}









void ImageTableView::action_open() {
	for (QModelIndex &index : this->selectionModel()->selectedIndexes())
		this->open_file(index);
}


void ImageTableView::action_rename() {
	QModelIndex index = this->currentIndex();
	if (index.isValid())
		this->edit(index);
}


#ifdef Q_OS_WIN	
void ImageTableView::action_set_as_desktop_wallpaper() {
	SharedImageItemWidget widget = ImageItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		QString path = widget->get_info().absoluteFilePath();
		::SystemParametersInfoW(
			SPI_SETDESKWALLPAPER, 0,
			const_cast<void*>(reinterpret_cast<const void*>(path.toStdWString().c_str())),
			SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE
		);
	}
}
#endif


void ImageTableView::action_print() {
	QModelIndexList indcs = this->selectionModel()->selectedIndexes();
	QPrinter printer;
	printer.newPage();
	printer.setPaperSize(QPrinter::A4);
	printer.setOrientation(QPrinter::Portrait);
	printer.setPageMargins(QMarginsF(0.1, 0.1, 0.1, 0.1), QPageLayout::Inch);


	auto printHandler = [this, indices = std::move(indcs)](QPrinter *printer) {
		QPainter painter(printer);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
		if (indices.empty())
			return;
		for (std::size_t i = 0; i < indices.size() - 1; ++i) {
			SharedImageItemWidget widget = ImageItemModel::index_widget(indices[i]);
			if (widget != nullptr) {
				this->fill_page(printer, &painter, widget);
				printer->newPage();
			}
		}
		SharedImageItemWidget widget = ImageItemModel::index_widget(indices.back());
		if (widget != nullptr)
			this->fill_page(printer, &painter, widget);
	};
	
	QPrintPreviewDialog dialog(&printer, this);
	QObject::connect(&dialog, &QPrintPreviewDialog::paintRequested, this, printHandler);
	dialog.exec();
}


#ifdef Q_OS_WIN
void ImageTableView::action_edit() {
	SharedImageItemWidget widget = ImageItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		std::wstring path = widget->get_info().absoluteFilePath().toStdWString();
		::SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(info);
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpFile = path.c_str();
		info.lpVerb = L"edit";
		info.nShow = SW_SHOW;
		ShellExecuteExW(&info);
	}
}
#endif


void ImageTableView::action_rotate_right() {
	QModelIndex index = this->currentIndex();
	if (ImageItemModel::index_widget(index) != nullptr) {
		imagesModel->rotate_right(index);
		this->update_cache(index);
	}
}


void ImageTableView::action_rotate_left() {
	QModelIndex index = this->currentIndex();
	if (ImageItemModel::index_widget(index) != nullptr) {
		imagesModel->rotate_left(index);
		this->update_cache(index);
	}
}


void ImageTableView::action_flip_vertically() {
	QModelIndex index = this->currentIndex();
	if (ImageItemModel::index_widget(index) != nullptr) {
		imagesModel->flip_vertically(index);
		this->update_cache(index);
	}
}


void ImageTableView::action_flip_horizontally() {
	QModelIndex index = this->currentIndex();
	if (ImageItemModel::index_widget(index) != nullptr) {
		imagesModel->flip_horizontally(index);
		this->update_cache(index);
	}
}


void ImageTableView::action_copy_image_data() {
	SharedImageItemWidget widget = ImageItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		QString path = widget->get_info().absoluteFilePath();
		QApplication::clipboard()->setImage(QImage(path));
	}
}


void ImageTableView::action_copy_file() {
	QList<QUrl> urlList;
	for (auto &index : this->selectionModel()->selectedIndexes()) {
		SharedImageItemWidget widget = ImageItemModel::index_widget(index);
		if (widget != nullptr) {
			QString path = widget->get_info().absoluteFilePath();
			urlList.push_back(QUrl::fromLocalFile(path));
		}
	}
	if (!urlList.empty()) {
		QMimeData *data = new QMimeData;
		data->setUrls(urlList);
		QApplication::clipboard()->setMimeData(data);
	}
}


void ImageTableView::action_delete() {
	QModelIndexList list = this->selectionModel()->selectedIndexes();
	QMessageBox::Button button = QMessageBox::question(
		this,
		QStringLiteral("Deleting file(s)"),
		"Are you sure you want to delete " + QString::number(list.size()) + " selected file(s)?"
	);
	if (button != QMessageBox::Yes)
		return;

	std::vector<std::size_t> indices;
	indices.reserve(list.size());
	for (auto &index : list) {
		QFileInfo info = ImageItemModel::index_widget(index)->get_info();
		if (!QFile::remove(info.absoluteFilePath())) {
			QMessageBox::warning(
				this,
				QStringLiteral("Deleting error"),
				"Could not delete file: " + info.fileName()
			);
		} else {
			indices.push_back(index.row() * imagesModel->columnCount() + index.column());
			imageCacheMap->remove(info.absoluteFilePath());
		}
	}
	std::sort(indices.begin(), indices.end());
	imagesModel->remove_indices(indices);
}


#ifdef Q_OS_WIN
void ImageTableView::action_properties() {
	SharedImageItemWidget widget = ImageItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		std::wstring path = widget->get_info().absoluteFilePath().toStdWString();
		::SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(info);
		info.lpFile = path.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = L"properties";
		::ShellExecuteExW(&info);
	}
}
#endif







void ImageTableView::open_file(const QModelIndex & index) const{
	SharedImageItemWidget widget = ImageItemModel::index_widget(index);
	if (widget != nullptr)
		QDesktopServices::openUrl(QUrl::fromLocalFile(widget->get_info().absoluteFilePath()));
}








void ImageTableView::fill_page(QPrinter * printer, QPainter *painter, const SharedImageItemWidget & widget) const {
	QString path = widget->get_info().absoluteFilePath();
	QPixmap pixmap(path);
	QRect rect = printer->pageRect();
	QSize size = pixmap.size().scaled(rect.size(), Qt::KeepAspectRatio);
	rect.translate(-rect.topLeft());
	rect = QStyle::alignedRect(Qt::LayoutDirectionAuto, Qt::AlignCenter, size, rect);
	painter->drawPixmap(rect, pixmap);
}








void ImageTableView::make_menu() {
	actionOpen = editMenu->addAction(QStringLiteral("Open"), this, &ImageTableView::action_open);
	actionOpen->setShortcuts({ QKeySequence(Qt::Key_Enter), QKeySequence(Qt::Key_Return) });
	this->addAction(actionOpen);

	actionRename = editMenu->addAction(QStringLiteral("Rename"), this, &ImageTableView::action_rename, QKeySequence(Qt::Key_F2));
	this->addAction(actionRename);

	editMenu->addSeparator();

#ifdef Q_OS_WIN	
	actionSetAsDesktopBackground = editMenu->addAction(QStringLiteral("Set as desktop background"), this, &ImageTableView::action_set_as_desktop_wallpaper);
	editMenu->addSeparator();
#endif

	actionPrint = editMenu->addAction(QStringLiteral("Print"), this, &ImageTableView::action_print, QKeySequence(QKeySequence::Print));
	this->addAction(actionPrint);

#ifdef Q_OS_WIN	
	actionEdit = editMenu->addAction(QStringLiteral("Edit"), this, &ImageTableView::action_edit);
#endif

	actionRotateRight = editMenu->addAction(QStringLiteral("Rotate right"), this, &ImageTableView::action_rotate_right);
	actionRotateLeft = editMenu->addAction(QStringLiteral("Rotate left"), this, &ImageTableView::action_rotate_left);
	actionFlipVertically = editMenu->addAction(QStringLiteral("Flip vertically"), this, &ImageTableView::action_flip_vertically);
	actionFlipHorizontally = editMenu->addAction(QStringLiteral("Flip horizontally"), this, &ImageTableView::action_flip_horizontally);


	editMenu->addSeparator();

	actionCopyImageData = editMenu->addAction(QStringLiteral("Copy image data"), this, &ImageTableView::action_copy_image_data, QKeySequence(Qt::ControlModifier | Qt::Key_C));
	this->addAction(actionCopyImageData);

	actionCopyFile = editMenu->addAction(QStringLiteral("Copy file"), this, &ImageTableView::action_copy_file, QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_C));
	this->addAction(actionCopyFile);

	editMenu->addSeparator();

	actionDelete = editMenu->addAction(QStringLiteral("Delete"), this, &ImageTableView::action_delete, QKeySequence(QKeySequence::Delete));
	this->addAction(actionDelete);

#ifdef Q_OS_WIN
	actionProperties = editMenu->addAction(QStringLiteral("Properties"), this, &ImageTableView::action_properties);
	actionProperties->setShortcuts({ QKeySequence(Qt::AltModifier | Qt::Key_Enter), QKeySequence(Qt::AltModifier | Qt::Key_Return) });
	this->addAction(actionProperties);
#endif
}





void ImageTableView::connect_signals() {
	QObject::connect(this, &ImageTableView::customContextMenuRequested, this, &ImageTableView::show_menu);
	QObject::connect(this, &ImageTableView::doubleClicked, this, &ImageTableView::open_file);
	QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ImageTableView::configure_actions);
	QObject::connect(imagesModel, &ImageItemModel::dataChanged, this->viewport(), static_cast<void(QWidget::*)()>(&QWidget::repaint));
	QObject::connect(imagesDelegate, &ImageItemDelegate::index_edited, this, &ImageTableView::update_cache);

	QObject::connect(imagesDelegate, &ImageItemDelegate::editor_created, this, [this] {actionOpen->setEnabled(false); });
	QObject::connect(imagesDelegate, &ImageItemDelegate::closeEditor, this, [this] {actionOpen->setEnabled(true); });

}







void ImageTableView::stop_thread() {
	if (loaderThread.isRunning()) {
		imagesLoader->stop();
		loaderThread.wait();
	}
}
