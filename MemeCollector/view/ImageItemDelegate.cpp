#include "stdafx.h"
#include "ImageItemDelegate.hpp"

ImageItemDelegate::ImageItemDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
	, padding(0) {}

ImageItemDelegate::~ImageItemDelegate() {}



void ImageItemDelegate::set_padding(unsigned padding) {
	this->padding = padding;
}

unsigned ImageItemDelegate::get_padding() const {
	return padding;
}




void ImageItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
							   const QModelIndex & index) const {
	SharedImageItemWidget infoWidget = ImageItemModel::index_widget(index);
	if (infoWidget != nullptr) {
		QRect rect = option.rect;
		rect.translate(padding, padding);
		rect.setWidth(rect.width() - 2 * padding);
		rect.setHeight(rect.height() - 2 * padding);
		painter->save();
		painter->eraseRect(option.rect);
		painter->setRenderHint(QPainter::Antialiasing);
		if (option.state & QStyle::State_Selected) {
			QPainterPath path;
			path.addRoundedRect(rect, 3, 3);
			painter->setPen(QPen(option.palette.highlight().color(), 2));
			painter->fillPath(path, option.palette.highlight().color());
			painter->drawPath(path);
		} else if (option.state & QStyle::State_MouseOver) {
			painter->setPen(QPen(option.palette.highlight().color(), 2));
			painter->drawRoundedRect(rect, 3, 3);
		}
		if (option.state & QStyle::State_HasFocus) {
			QStyleOptionFocusRect focus;
			focus.rect = rect;
			QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, painter);
		}
		infoWidget->render(painter, rect.topLeft(), painter->clipRegion(), QWidget::DrawChildren);
		painter->restore();
	}
}



QSize ImageItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(size.height() +  2 * padding);
	size.setWidth(size.width() + 2 * padding);
	return size;
}



QWidget * ImageItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	if (ImageItemModel::index_widget(index) != nullptr) {
		ImageItemWidget *widget = new ImageItemWidget(parent);
		QMargins margins = widget->contentsMargins();
		margins.setLeft(margins.left() + padding);
		margins.setRight(margins.right() + padding);
		margins.setTop(margins.top() + padding);
		margins.setBottom(margins.bottom() + padding);
		widget->setContentsMargins(margins);
		widget->setFocusPolicy(Qt::StrongFocus);
		QObject::connect(widget, &ImageItemWidget::editing_finished, this, &ImageItemDelegate::commit_and_close);
		emit editor_created(widget);
		return widget;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);

}

void ImageItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
	SharedImageItemWidget infoWidget = ImageItemModel::index_widget(index);
	if (infoWidget != nullptr) {
		ImageItemWidget *widget = dynamic_cast<ImageItemWidget*>(editor);
		widget->set_info(infoWidget->get_info());
		widget->set_pixmap(*infoWidget->get_pixmap());
		widget->start_editing();
	} else
		QStyledItemDelegate::setEditorData(editor, index);
}



void ImageItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
	ImageItemWidget *widget = dynamic_cast<ImageItemWidget*>(editor);
	if (widget->verify_and_update()) {
		model->setData(index, QVariant::fromValue(widget->get_info()), Qt::EditRole);
		emit index_edited(index);
	}
}






void ImageItemDelegate::commit_and_close() {
	QWidget *widget = dynamic_cast<QWidget*>(this->sender());
	emit commitData(widget);
	emit closeEditor(widget);
}
