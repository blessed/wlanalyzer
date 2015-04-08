#include <QActionGroup>
#include <QMouseEvent>
#include <QPalette>
#include <QPainter>
#include <QVariant>
#include <QFontDatabase>
#include <QScrollBar>
#include "gui/hexwidget.h"
#include "gui/hexwidget.moc"


const char *HEX_PATTERN = "0123456789abcdef";

template <typename T>
QString numToHexStr(T val)
{
    // buffer is the size of number of nibbles in number + trailing '\0'
    const unsigned NUM_NIBBLES = sizeof(T) * 2;
    char buf[NUM_NIBBLES + 1] = {false,};
    for(int i = NUM_NIBBLES -1; i >= 0; --i)
    {
        buf[i] = HEX_PATTERN[val & 0xf];
        val >>= 4;
    }
    return QString(buf);
}

template <typename T>
QString numToBinStr(T val)
{
    // buffer is the size of number of bits in number + trailing '\0'
    const unsigned NUM_BITS = sizeof(T) * 8;
    char buf[NUM_BITS + 1] = {false,};
    for(int i = NUM_BITS -1; i >= 0; --i)
    {
        buf[i] = '0' + (val & 0x1);
        val >>= 1;
    }
    return QString(buf);
}

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
    m_numLines = dsize / m_numBytesInLine + 1;
    m_numVisibleLines = viewport()->height() / m_lineHeight;

    // calculate number of characters requred to represent the address
    m_numAddressChars = 4;
    while((dsize >> 16) > 0)
        m_numAddressChars += 5; // plus 1 char inter word spacing

    int colMargin = 2 * m_textMargin;
    m_characterWidth = fm.width(QChar('M'));
    m_textMargin = m_characterWidth;

    m_addrColumnWidth = colMargin + m_characterWidth * m_numAddressChars;
    m_rawColumnWidth = colMargin + m_characterWidth * rawColumnChars;
    m_printableColumnWidth = colMargin + m_characterWidth * m_numBytesInLine;

    m_lineWidth = m_addrColumnWidth + m_rawColumnWidth + m_printableColumnWidth;

    // manage correct scroll and scrollbar ranges
    horizontalScrollBar()->setRange(0, m_lineWidth + m_textMargin - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setRange(0, m_numLines - m_numVisibleLines);
    verticalScrollBar()->setPageStep(m_numVisibleLines);
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
    const int viewport_h = viewport()->height();
    const int viewport_w = viewport()->width();

    const int addrColumnPos = -x;
    const int rawColumnPos = addrColumnPos + m_addrColumnWidth;
    const int printableColumnPos = rawColumnPos + m_rawColumnWidth;
    const int printableColumnEnd = printableColumnPos + m_printableColumnWidth;

    painter.fillRect(QRectF(0.0, 0.0, viewport_w, viewport_h), palette().background());
    painter.fillRect(QRectF(addrColumnPos, 0.0, m_addrColumnWidth, viewport_h), palette().dark());
    painter.fillRect(QRectF(printableColumnPos, 0.0, m_printableColumnWidth, viewport_h), palette().dark());

    painter.drawLine(addrColumnPos, 0, addrColumnPos, viewport_h);
    painter.drawLine(rawColumnPos, 0, rawColumnPos, viewport_h);
    painter.drawLine(printableColumnPos, 0, printableColumnPos, viewport_h);
    painter.drawLine(printableColumnEnd, 0, printableColumnEnd, viewport_h);

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
    recalculateFontMetrics();
    QAbstractScrollArea::resizeEvent(event);
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
