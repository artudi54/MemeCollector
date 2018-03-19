#pragma once


class DeselectableTableWidget : public QTableWidget {
public:
	explicit DeselectableTableWidget(QWidget * parent = nullptr);
	virtual ~DeselectableTableWidget();
protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
};