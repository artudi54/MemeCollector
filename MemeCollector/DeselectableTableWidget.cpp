#include "stdafx.h"
#include "DeselectableTableWidget.hpp"

DeselectableTableWidget::DeselectableTableWidget(QWidget * parent) : QTableWidget(parent) {}

DeselectableTableWidget::~DeselectableTableWidget() {}

void DeselectableTableWidget::mousePressEvent(QMouseEvent * event) {
    QModelIndex item = this->indexAt(event->pos());
    if (item.isValid())
        QTableWidget::mousePressEvent(event);
    else
        this->clearSelection();
}
