#pragma once

class ImageItemStackedWidget : public QStackedWidget {
public:
	explicit ImageItemStackedWidget(QWidget *parent = nullptr);
	virtual ~ImageItemStackedWidget();
	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
};





class ImageItemWidgetEdit : public QLineEdit {
	Q_OBJECT
public:
	explicit ImageItemWidgetEdit(QWidget * paret = nullptr);
	virtual ~ImageItemWidgetEdit();
signals:
protected:
	virtual void focusInEvent(QFocusEvent *event) override;
};




class ImageItemWidget : public QWidget {
	Q_OBJECT
public:
	explicit ImageItemWidget(QWidget *parent = nullptr);
	ImageItemWidget(const QFileInfo &pathInfo, const QPixmap &pixmap, QWidget *parent = nullptr);
	virtual ~ImageItemWidget();
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
	ImageItemStackedWidget stackedEditor;
	QLabel labelTitle;
	ImageItemWidgetEdit editTitle;
};

using SharedImageItemWidget = std::shared_ptr<ImageItemWidget>;

Q_DECLARE_METATYPE(SharedImageItemWidget)