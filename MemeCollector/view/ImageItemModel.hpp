#pragma once

#include "ImageItemWidget.hpp"

class ImageItemModel : public QAbstractTableModel {
	Q_OBJECT
public:
	static constexpr Qt::ItemDataRole RemoveRole = Qt::UserRole;

	explicit ImageItemModel(QObject *parent = nullptr);
	virtual ~ImageItemModel();

	void clear();
	void add_entry(QFileInfo info, QImage image);
	void remove_indices(const std::vector<std::size_t> &indices);
	void rotate_right(const QModelIndex &index);
	void rotate_left(const QModelIndex &index);
	void flip_vertically(const QModelIndex &index);
	void flip_horizontally(const QModelIndex &index);
	QModelIndex closest_match(const QString &search) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	
	static SharedImageItemWidget index_widget(const QModelIndex &index);
public slots:
	void set_size(QSize size, unsigned padding);
private:
	static bool case_insensitive_compare(const QString &lhs, const QString &rhs);
	void move_down(std::size_t index);
	void move_up(std::size_t index);
	void rotate(double deegrees, const QModelIndex &index);
	void flip(bool horizontally, bool vertically, const QModelIndex &index);
	std::vector<SharedImageItemWidget> imagesList;
	unsigned columnNumber;
	unsigned width;
	unsigned padding;
};

