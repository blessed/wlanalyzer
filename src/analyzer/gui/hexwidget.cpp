#include <QPalette>
#include "gui/hexwidget.h"
#include "gui/hexwidget.moc"

HexWidget::HexWidget(QWidget *parent) :
    QScrollArea(parent), m_highlightColor(Qt::blue)
{
}

void HexWidget::setData(QByteArray data)
{
    m_data = data;
}

void HexWidget::setHighlightColor(QColor color)
{
    m_highlightColor = color;
}

void HexWidget::highlight(quint32 offset, quint32 len)
{
    //TODO
}

void HexWidget::setRawDataDisplayFormat(QAction *action)
{
    //TODO
}
