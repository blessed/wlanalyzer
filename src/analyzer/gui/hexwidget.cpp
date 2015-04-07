#include <QActionGroup>
#include <QMouseEvent>
#include <QPalette>
#include <QPainter>
#include <QVariant>
#include <QFontDatabase>
#include <QScrollBar>
#include "gui/hexwidget.h"
#include "gui/hexwidget.moc"

HexWidget::HexWidget(QWidget *parent) :
    QAbstractScrollArea(parent), m_highlightColor(Qt::blue),
    m_format(DisplayFormat::HEX),
    m_numBytesInLine(16),
    m_numAddressChars(4)
{
    // setup context menu with required options
    auto group = new QActionGroup(this);
    auto action = group->addAction(tr("Display data as Bit Field"));
    action->setCheckable(true);
    action->setData(qVariantFromValue(DisplayFormat::BIT_FIELD));
    action = group->addAction(tr("Display data as HEX"));
    action->setCheckable(true);
    action->setChecked(true);
    action->setData(qVariantFromValue(DisplayFormat::HEX));
    m_contextMenu.addActions(group->actions());

    connect(group, SIGNAL(triggered(QAction*)), this, SLOT(setRawDataDisplayFormat(QAction*)));

    // setup monospaced font and fontmetrics
    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFont(font);
    recalculateFontMetrics();
}

void HexWidget::recalculateFontMetrics()
{
    // TODO precalculate line heights / widths, scrollbar ranges, etc.
    const QFontMetrics &fm = fontMetrics();
    m_lineHeight = fm.lineSpacing();
    int dsize = m_data.size();

    // calculate number of characters in raw view plus spaces every octet
    int rawColumnChars = 0;
    if(m_format == DisplayFormat::HEX)
    {
        m_numBytesInLine = 16;
        rawColumnChars = m_numBytesInLine * 3;
    }else
    {
        m_numBytesInLine = 8;
        rawColumnChars = m_numBytesInLine * 9;
    }
    m_numLines = dsize / m_numBytesInLine;

    // calculate number of characters requred to represent the address
    m_numAddressChars = 4;
    while((dsize >> 16) > 0)
        m_numAddressChars += 4;

    int colMargin = 2 * m_textMargin;
    m_characterWidth = fm.width(QChar('M'));
    m_textMargin = m_characterWidth;

    m_addrColumnWidth = colMargin + m_characterWidth * m_numAddressChars;
    m_rawColumnWidth = colMargin + m_characterWidth * rawColumnChars;
    m_printableColumnWidth = colMargin + m_characterWidth * m_numBytesInLine;

    m_lineWidth = m_addrColumnWidth + m_rawColumnWidth + m_printableColumnWidth;
}

void HexWidget::setData(QByteArray data)
{
    m_data = data;
    // TODO should use less heavyweight function
    recalculateFontMetrics();
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
    bool data_ok = true;
    DisplayFormat format = static_cast<DisplayFormat>(action->data().toInt(&data_ok));

    Q_ASSERT(data_ok);
    if(format != m_format)
    {
        m_format = format;
        recalculateFontMetrics();
        viewport()->update();
    }
}

void HexWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_contextMenu.exec(event->globalPos());
}

void HexWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(viewport());
    const int x = horizontalScrollBar()->value();
    const int y = verticalScrollBar()->value();
    const int addrColumnPos = -x;
    const int rawColumnPos = addrColumnPos + m_addrColumnWidth;
    const int printableColumnPos = rawColumnPos + m_rawColumnWidth;
    const int printableColumnEnd = printableColumnPos + m_printableColumnWidth;

    painter.drawLine(addrColumnPos, 0, addrColumnPos, viewport()->height());
    painter.drawLine(rawColumnPos, 0, rawColumnPos, viewport()->height());
    painter.drawLine(printableColumnPos, 0, printableColumnPos, viewport()->height());
    painter.drawLine(printableColumnEnd, 0, printableColumnEnd, viewport()->height());

    if(m_format == DisplayFormat::HEX)
        painter.drawText(QRectF(0.0, 0.0 -y, 150., 50. -y), QString("HEX: 0f0f0f"));
    else
        painter.drawText(QRectF(0.0, 0.0 -y, 150., 50. -y), QString("Bit Field: 1010101"));
    // TODO
}

void HexWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        resizeFont(event->delta() > 0 ? 1 : -1 );
    }
    QAbstractScrollArea::wheelEvent(event);
}

void HexWidget::resizeEvent(QResizeEvent *event)
{
    // TODO
}

void HexWidget::resizeFont(int sizeIncrement)
{
    QFont font = this->font();
    const int newSize = font.pointSize() + sizeIncrement;
    if (newSize <= 0)
        return;
    font.setPointSize(newSize);
    setFont(font);
    recalculateFontMetrics();
}
