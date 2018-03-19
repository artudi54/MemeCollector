#pragma once

class ImagesItemStackedWidget : public QStackedWidget {
public:
	explicit ImagesItemStackedWidget(QWidget *parent = nullptr);
	virtual ~ImagesItemStackedWidget();
	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
};





class ImagesItemWidgetEdit : public QLineEdit {
	Q_OBJECT
public:
	explicit ImagesItemWidgetEdit(QWidget * paret = nullptr);
	virtual ~ImagesItemWidgetEdit();
signals:
protected:
	virtual void focusInEvent(QFocusEvent *event) override;
};




class ImagesItemWidget : public QWidget {
	Q_OBJECT
public:
	explicit ImagesItemWidget(QWidget *parent = nullptr);
	ImagesItemWidget(const QFileInfo &pathInfo, const QPixmap &pixmap, QWidget *parent = nullptr);
	virtual ~ImagesItemWidget();
	void set_info(QFileInfo pathInfo);
	const QFileInfo& get_info() const;
	QString get_title() const;
	void set_pixmap(const QPixmap &pixmap);
	const QPixmap* get_pixmap() const;
	bool verify_and_update();
public slots:
	void start_editing();
signals:
	void editing_finished();
private:
	QFileInfo pathInfo;
	QVBoxLayout layout;
	QLabel labelPixmap;
	ImagesItemStackedWidget stackedEditor;
	QLabel labelTitle;
	ImagesItemWidgetEdit editTitle;
};

using SharedImagesItemWidget = std::shared_ptr<ImagesItemWidget>;

Q_DECLARE_METATYPE(SharedImagesItemWidget)