#pragma once

#include "ImagesItemModel.hpp"

class ImagesItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit ImagesItemDelegate(QObject *parent = nullptr);
	virtual ~ImagesItemDelegate();
	void set_padding(unsigned padding);
	unsigned get_padding() const;
signals:
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
