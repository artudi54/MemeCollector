#pragma once

#include "ImageItemModel.hpp"

class ImageItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit ImageItemDelegate(QObject *parent = nullptr);
	virtual ~ImageItemDelegate();
	void set_padding(unsigned padding);
	unsigned get_padding() const;
signals:
	void editor_created(QWidget *widget) const;
	void index_edited(const QModelIndex &index) const;
protected:
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
									  const QModelIndex &index) const override;
	virtual QSize sizeHint(const QStyleOptionViewItem &option,
						   const QModelIndex &index) const override;
	virtual QWidget* createEditor(QWidget * parent, const QStyleOptionViewItem & option,
								  const QModelIndex & index) const override;

	virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
										   const QModelIndex &index) const override;
private:
	void commit_and_close();
	unsigned padding;
};
