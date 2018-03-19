#include "stdafx.h"
#include "ImagesTableView.hpp"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif 


ImagesTableView::ImagesTableView(QWidget *parent)
	: QTableView(parent)
	, directory()
	, searchText()
	, prevTime(std::chrono::steady_clock::now())
	, curTime(prevTime)
	, cacheMap(QStringLiteral("cache.dat"))
	, loaderThread(new QThread(this))
	, imagesLoader(nullptr)
	, imagesModel(new ImagesItemModel(this))
	, imagesDelegate(new ImagesItemDelegate(this))
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

ImagesTableView::ImagesTableView(const QDir & directory, QWidget * parent)
	: ImagesTableView(parent) {
	this->set_directory(directory);
}

ImagesTableView::~ImagesTableView() {
	this->stop_thread();
}






void ImagesTableView::set_directory(const QDir & directory) {
	this->directory = directory;
	
	this->stop_thread();
	QTimer::singleShot(0, this, [this] {
		imagesModel->clear();

		imagesLoader = new ImagesLoader(&cacheMap, this->directory);
		imagesLoader->moveToThread(loaderThread);
		QObject::connect(loaderThread, &QThread::started, imagesLoader, &ImagesLoader::process_directory);
		QObject::connect(imagesLoader, &ImagesLoader::finished, loaderThread, &QThread::quit, Qt::DirectConnection);
		QObject::connect(imagesLoader, &ImagesLoader::finished, this, &ImagesTableView::loading_finished);
		QObject::connect(imagesLoader, &ImagesLoader::finished, imagesLoader, &ImagesLoader::deleteLater);
		QObject::connect(imagesLoader, &ImagesLoader::next_info, imagesModel, &ImagesItemModel::add_entry);

		loaderThread->start();
		emit loading_started();
	});
}

const QDir & ImagesTableView::get_directory() const {
	return directory;
}








void ImagesTableView::clear() {
	this->stop_thread();
	QTimer::singleShot(0, this, [this] { imagesModel->clear(); });
}






void ImagesTableView::keyboardSearch(const QString & search) {
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










void ImagesTableView::mousePressEvent(QMouseEvent * event) {
	QModelIndex index = this->indexAt(event->pos());
	if (ImagesItemModel::index_widget(index) == nullptr)
		this->clearSelection();
	QTableView::mousePressEvent(event);
}

void ImagesTableView::resizeEvent(QResizeEvent * event) {
	imagesModel->set_size(event->size(), imagesDelegate->get_padding());
	QTableView::resizeEvent(event);
}

int ImagesTableView::sizeHintForColumn(int column) const {
	QSize size = imagesModel->data(imagesModel->index(0, 0), Qt::SizeHintRole).value<QSize>();
	if (size.isValid())
		return size.width() + 2 * imagesDelegate->get_padding();
	return 0;
}

int ImagesTableView::sizeHintForRow(int row) const {
	QSize size = imagesModel->data(imagesModel->index(0, 0), Qt::SizeHintRole).value<QSize>();
	if (size.isValid())
		return size.height() + 2 * imagesDelegate->get_padding();
	return 0;
}








void ImagesTableView::configure_actions(const QItemSelection&, const QItemSelection&) {
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


void ImagesTableView::show_menu(const QPoint &pos) {
	editMenu->exec(this->mapToGlobal(pos));
}





void ImagesTableView::update_cache(const QModelIndex & index) {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
	if (widget != nullptr) {
		QFileInfo info = widget->get_info();
		QImage image = widget->get_pixmap()->toImage();
		QString path = info.absoluteFilePath();
		cacheMap.set(path, CachedImage(info.lastModified(), image));
	}
}









void ImagesTableView::action_open() {
	for (QModelIndex &index : this->selectionModel()->selectedIndexes())
		this->open_file(index);
}

void ImagesTableView::action_rename() {
	QModelIndex index = this->currentIndex();
	if (index.isValid())
		this->edit(index);
}

#ifdef Q_OS_WIN	
void ImagesTableView::action_set_as_desktop_wallpaper() {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(this->currentIndex());
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

void ImagesTableView::action_print() { //fix
	QModelIndexList indcs = this->selectionModel()->selectedIndexes();
	QPrinter printer;
	printer.newPage();
	printer.setPaperSize(QPrinter::A4);
	printer.setOrientation(QPrinter::Portrait);
	printer.setPageMargins(QMarginsF(0.1, 0.1, 0.1, 0.1), QPageLayout::Inch);


	auto printHandler = [this, indices = std::move(indcs)](QPrinter *printer) {
		QPainter painter(printer);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
		for (const QModelIndex &index : indices) {
			SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
			if (widget != nullptr) {
				QString path = widget->get_info().absoluteFilePath();
				QPixmap pixmap(path);
				QRect rect = printer->pageRect();
				QSize size = pixmap.size().scaled(rect.size(), Qt::KeepAspectRatio);
				rect.translate(-rect.topLeft());
				rect = QStyle::alignedRect(Qt::LayoutDirectionAuto, Qt::AlignCenter, size, rect);
				painter.drawPixmap(rect, pixmap);
				printer->newPage();
			}
		}
	};
	
	QPrintPreviewDialog dialog(&printer, this);
	QObject::connect(&dialog, &QPrintPreviewDialog::paintRequested, this, printHandler);
	dialog.exec();
}

#ifdef Q_OS_WIN	
void ImagesTableView::action_edit() {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		std::wstring path = widget->get_info().absoluteFilePath().toStdWString();
		::SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof(info);
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpFile = path.c_str();
		info.lpVerb = L"print";
		info.nShow = SW_SHOW;
		ShellExecuteExW(&info);
	}
}
#endif

void ImagesTableView::action_rotate_right() {
	QModelIndex index = this->currentIndex();
	if (ImagesItemModel::index_widget(index) != nullptr) {
		imagesModel->rotate_right(index);
		this->update_cache(index);
	}
}
void ImagesTableView::action_rotate_left() {
	QModelIndex index = this->currentIndex();
	if (ImagesItemModel::index_widget(index) != nullptr) {
		imagesModel->rotate_left(index);
		this->update_cache(index);
	}
}

void ImagesTableView::action_flip_vertically() {
	QModelIndex index = this->currentIndex();
	if (ImagesItemModel::index_widget(index) != nullptr) {
		imagesModel->flip_vertically(index);
		this->update_cache(index);
	}
}

void ImagesTableView::action_flip_horizontally() {
	QModelIndex index = this->currentIndex();
	if (ImagesItemModel::index_widget(index) != nullptr) {
		imagesModel->flip_horizontally(index);
		this->update_cache(index);
	}
}

void ImagesTableView::action_copy_image_data() {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(this->currentIndex());
	if (widget != nullptr) {
		QString path = widget->get_info().absoluteFilePath();
		QApplication::clipboard()->setImage(QImage(path));
	}
}

void ImagesTableView::action_copy_file() {
	QList<QUrl> urlList;
	for (auto &index : this->selectionModel()->selectedIndexes()) {
		SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
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

void ImagesTableView::action_delete() {
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
		QFileInfo info = ImagesItemModel::index_widget(index)->get_info();
		if (!QFile::remove(info.absoluteFilePath())) {
			QMessageBox::warning(
				this,
				QStringLiteral("Deleting error"),
				"Could not delete file: " + info.fileName()
			);
		} else {
			indices.push_back(index.row() * imagesModel->columnCount() + index.column());
			cacheMap.remove(info.absoluteFilePath());
		}
	}
	std::sort(indices.begin(), indices.end());
	imagesModel->remove_indices(indices);
}

#ifdef Q_OS_WIN
void ImagesTableView::action_properties() {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(this->currentIndex());
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







void ImagesTableView::open_file(const QModelIndex & index) {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
	if (widget != nullptr)
		QDesktopServices::openUrl(QUrl::fromLocalFile(widget->get_info().absoluteFilePath()));
}





void ImagesTableView::make_menu() {
	actionOpen = editMenu->addAction(QStringLiteral("Open"), this, &ImagesTableView::action_open);
	actionOpen->setShortcuts({ QKeySequence(Qt::Key_Enter), QKeySequence(Qt::Key_Return) });
	this->addAction(actionOpen);

	actionRename = editMenu->addAction(QStringLiteral("Rename"), this, &ImagesTableView::action_rename, QKeySequence(Qt::Key_F2));
	this->addAction(actionRename);

	editMenu->addSeparator();

#ifdef Q_OS_WIN	
	actionSetAsDesktopBackground = editMenu->addAction(QStringLiteral("Set as desktop background"), this, &ImagesTableView::action_set_as_desktop_wallpaper);
	editMenu->addSeparator();
#endif

	actionPrint = editMenu->addAction(QStringLiteral("Print"), this, &ImagesTableView::action_print, QKeySequence(QKeySequence::Print));
	this->addAction(actionPrint);

#ifdef Q_OS_WIN	
	actionEdit = editMenu->addAction(QStringLiteral("Edit"), this, &ImagesTableView::action_edit);
#endif

	actionRotateRight = editMenu->addAction(QStringLiteral("Rotate right"), this, &ImagesTableView::action_rotate_right);
	actionRotateLeft = editMenu->addAction(QStringLiteral("Rotate left"), this, &ImagesTableView::action_rotate_left);
	actionFlipVertically = editMenu->addAction(QStringLiteral("Flip vertically"), this, &ImagesTableView::action_flip_vertically);
	actionFlipHorizontally = editMenu->addAction(QStringLiteral("Flip horizontally"), this, &ImagesTableView::action_flip_horizontally);


	editMenu->addSeparator();

	actionCopyImageData = editMenu->addAction(QStringLiteral("Copy image data"), this, &ImagesTableView::action_copy_image_data, QKeySequence(Qt::ControlModifier | Qt::Key_C));
	this->addAction(actionCopyImageData);

	actionCopyFile = editMenu->addAction(QStringLiteral("Copy file"), this, &ImagesTableView::action_copy_file, QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_C));
	this->addAction(actionCopyFile);

	editMenu->addSeparator();

	actionDelete = editMenu->addAction(QStringLiteral("Delete"), this, &ImagesTableView::action_delete, QKeySequence(QKeySequence::Delete));
	this->addAction(actionDelete);

#ifdef Q_OS_WIN
	actionProperties = editMenu->addAction(QStringLiteral("Properties"), this, &ImagesTableView::action_properties);
	actionProperties->setShortcuts({ QKeySequence(Qt::AltModifier | Qt::Key_Enter), QKeySequence(Qt::AltModifier | Qt::Key_Return) });
	this->addAction(actionProperties);
#endif
}

void ImagesTableView::connect_signals() {
	QObject::connect(this, &ImagesTableView::customContextMenuRequested, this, &ImagesTableView::show_menu);
	QObject::connect(this, &ImagesTableView::doubleClicked, this, &ImagesTableView::open_file);
	QObject::connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ImagesTableView::configure_actions);
	QObject::connect(imagesModel, &ImagesItemModel::dataChanged, this->viewport(), static_cast<void(QWidget::*)()>(&QWidget::repaint));
	QObject::connect(imagesDelegate, &ImagesItemDelegate::index_edited, this, &ImagesTableView::update_cache);
}

void ImagesTableView::stop_thread() {
	if (loaderThread->isRunning()) {
		imagesLoader->stop();
		loaderThread->wait();
	}
}
