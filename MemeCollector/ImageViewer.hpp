#pragma once


#include "ui_ImageViewer.h"

class ImageViewer : public QWidget, private Ui::ImageViewer {
	Q_OBJECT
public:
	ImageViewer(QWidget *parent);
	~ImageViewer();
	void set_directory(const QString &path);
protected:
	virtual bool eventFilter(QObject *watched, QEvent *event) override;
	virtual void showEvent(QShowEvent *event) override;
private:
	QPoint parent_point() const;
	void move_to_parent();
};