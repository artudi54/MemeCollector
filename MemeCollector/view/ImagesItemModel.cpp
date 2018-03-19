#include "stdafx.h"
#include "ImagesItemModel.hpp"



ImagesItemModel::ImagesItemModel(QObject *parent)
	: QAbstractTableModel(parent)
	, imagesList()
	, columnNumber(0)
	, width(0)
	, padding(0) {}

ImagesItemModel::~ImagesItemModel() {}



void ImagesItemModel::clear() {
	this->beginResetModel();
	imagesList.clear();
	this->endResetModel();
}

void ImagesItemModel::add_entry(QFileInfo info, QImage image) {
	imagesList.push_back(std::make_shared<ImagesItemWidget>(info, QPixmap::fromImage(image)));
	if (width == 0)
		return;	

	int newColumnNumber = width / (imagesList.front()->sizeHint().width() + 2 * padding);

	if (newColumnNumber != columnNumber) {
		this->beginResetModel();
		columnNumber = newColumnNumber;
		this->endResetModel();
	} else if (columnNumber != 0) {
		int column = imagesList.size() % columnNumber,
			row = imagesList.size() / columnNumber;
		if (column == 0) {
			this->beginInsertRows(QModelIndex(), row, row);
			this->endInsertRows();
		} else {
			QModelIndex index = this->index(row, column);
			emit dataChanged(index, index);
		}
	}
}

void ImagesItemModel::remove_indices(const std::vector<std::size_t> &indices) {
	this->beginResetModel();
	auto condition = [&indices, rangeidx = 0, idx = 0](auto&&) mutable {
		if (idx < indices.size() && rangeidx == indices[idx]) {
			++idx;
			++rangeidx;
			return true;
		}
		++rangeidx;
		return false;
	};
	imagesList.erase(
		std::remove_if(
			imagesList.begin(),
			imagesList.end(),
			condition
		),
		imagesList.end()
	);
	this->endResetModel();
}





void ImagesItemModel::rotate_right(const QModelIndex & index) {
	QTransform transformation;
	this->rotate(90, index);
}

void ImagesItemModel::rotate_left(const QModelIndex & index) {
	QTransform transformation;
	this->rotate(-90, index);
}

void ImagesItemModel::flip_vertically(const QModelIndex & index) {
	this->flip(true, false, index);
}

void ImagesItemModel::flip_horizontally(const QModelIndex & index) {
	this->flip(false, true, index);
}






QModelIndex ImagesItemModel::closest_match(const QString & search) const {
	auto match = [](const SharedImagesItemWidget &infoWidget, const QString &search) {
		return infoWidget->get_title().compare(search, Qt::CaseInsensitive) < 0;
	};
	auto it = std::lower_bound(imagesList.begin(), imagesList.end(), search, match);
	if (it != imagesList.end() && (*it)->get_title().startsWith(search, Qt::CaseInsensitive)) {
		std::ptrdiff_t idx = it - imagesList.begin();
		if (columnNumber == 0)
			return QModelIndex();
		return this->index(idx / columnNumber, idx % columnNumber);
	}
	return QModelIndex();
}





Qt::ItemFlags ImagesItemModel::flags(const QModelIndex & index) const {
	if (ImagesItemModel::index_widget(index) == nullptr)
		return Qt::NoItemFlags;
	return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

int ImagesItemModel::rowCount(const QModelIndex & parent) const {
	if (parent.isValid() || columnNumber == 0)
		return 0;
	return static_cast<int>(imagesList.size() / columnNumber + (imagesList.size() % columnNumber > 0));
}

int ImagesItemModel::columnCount(const QModelIndex & parent) const {
	if (parent.isValid())
		return 0;
	return columnNumber;
}




QVariant ImagesItemModel::data(const QModelIndex & index, int role) const {
	if (index.isValid() && index.row() * columnNumber + index.column() < imagesList.size()) {
		switch (role) {
		case Qt::DisplayRole:
				return QVariant::fromValue(
					imagesList[index.row() * columnNumber + index.column()]
				);
		case Qt::ToolTipRole:
				return QVariant::fromValue(
					imagesList[index.row() * columnNumber + index.column()]
					->get_title()
				);
		case Qt::SizeHintRole:
				return QVariant::fromValue(
					imagesList[index.row() * columnNumber + index.column()]
					->sizeHint()
				);
		}
	}
	return QVariant();
}

bool ImagesItemModel::setData(const QModelIndex & index, const QVariant & value, int role) {
	if (role == Qt::EditRole && value.canConvert<QFileInfo>()) {
		std::size_t idx = index.row() * columnNumber + index.column();
		QFileInfo newInfo = value.value<QFileInfo>();
		imagesList[idx]->set_info(newInfo);
		QString title = imagesList[idx]->get_title();
		if (idx > 0 && case_insensitive_compare(title, imagesList[idx - 1]->get_title()))
			QTimer::singleShot(0, [this, idx] {this->move_down(idx); });
		else if (idx < imagesList.size() - 1 && case_insensitive_compare(imagesList[idx + 1]->get_title(), title))
			QTimer::singleShot(0, [this, idx] {this->move_up(idx); });
		else
			emit dataChanged(index, index);
		return true;
	}
	return false;
}





QVariant ImagesItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
	return QVariant();
}




//static
SharedImagesItemWidget ImagesItemModel::index_widget(const QModelIndex & index) {
	return index.data(Qt::DisplayRole).value<SharedImagesItemWidget>();
}







void ImagesItemModel::set_size(QSize size, unsigned padding) {
	width = size.width();
	this->padding = padding;
	int newColumnNumber;

	if (imagesList.empty())
		newColumnNumber = 0;
	else
		newColumnNumber = width / (imagesList.front()->sizeHint().width() + 2 * padding);


	if (newColumnNumber != columnNumber) {
		this->beginResetModel();
		columnNumber = newColumnNumber;
		this->endResetModel();
	}
}





bool ImagesItemModel::case_insensitive_compare(const QString & lhs, const QString & rhs) {
	return lhs.compare(rhs, Qt::CaseInsensitive) < 0;
}

void ImagesItemModel::move_down(std::size_t index) { //repair if possible
	this->beginResetModel();
	SharedImagesItemWidget widget = std::move(imagesList[index]);
	QString title = widget->get_title();
	while (index-- && case_insensitive_compare(title, imagesList[index]->get_title()))
		imagesList[index + 1] = std::move(imagesList[index]);
	imagesList[index + 1] = std::move(widget);
	this->endResetModel();
}

void ImagesItemModel::move_up(std::size_t index) {
	this->beginResetModel();
	SharedImagesItemWidget widget = std::move(imagesList[index]);
	QString title = widget->get_title();
	while (index++ < imagesList.size() - 1 && case_insensitive_compare(title, imagesList[index]->get_title()))
		imagesList[index - 1] = std::move(imagesList[index]);
	imagesList[index - 1] = std::move(widget);
	this->endResetModel();
}






void ImagesItemModel::rotate(double deegrees, const QModelIndex & index) {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
	if (widget != nullptr) {
		QString path = widget->get_info().absoluteFilePath();
		QImage image(path);
		QTransform transformation;
		transformation.rotate(deegrees);
		image = image.transformed(transformation, Qt::SmoothTransformation);
		image.save(path);
		image = image.scaled(600, 600, Qt::KeepAspectRatio, Qt::FastTransformation)
			.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		widget->set_pixmap(QPixmap::fromImage(image));
		emit dataChanged(index, index);
	}
}

void ImagesItemModel::flip(bool horizontally, bool vertically, const QModelIndex &index) {
	SharedImagesItemWidget widget = ImagesItemModel::index_widget(index);
	if (widget != nullptr) {
		QString path = widget->get_info().absoluteFilePath();
		QImage image = QImage(path).mirrored(horizontally, vertically);
		image.save(path);
		image = image.scaled(600, 600, Qt::KeepAspectRatio, Qt::FastTransformation)
			.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		widget->set_pixmap(QPixmap::fromImage(image));
		emit dataChanged(index, index);
	}
}