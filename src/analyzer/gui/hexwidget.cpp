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
    QAbstractScrollArea(parent),
    m_data(nullptr),
    m_format(DisplayFormat::HEX),
    m_numBytesInLine(16),
    m_numAddressChars(4)
{

#ifdef DEBUG_BUILD
    // trigger mouseMoveEvent also without mouse down
    setMouseTracking(true);
#endif

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

HexWidget::~HexWidget()
{
    delete m_data;
}

void HexWidget::setData(QIODevice* data)
{
    Q_ASSERT(data);
    //only random access QIODevices have semantics reasonable for this widget
    Q_ASSERT(data->isSequential() == false);
    Q_ASSERT(data->isReadable());
    m_data = data;
    recalculateFontMetrics();
}

void HexWidget::setData(const QByteArray& data)
{
    auto buf = new QBuffer();
    buf->setData(data);
    setData(buf);
}

void HexWidget::recalculateFontMetrics()
{
    const QFontMetrics &fm = fontMetrics();
    m_lineHeight = fm.lineSpacing();
    qint64 dsize = dataSize();

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
    m_numVisibleLines = qMin(static_cast<qint64>(viewport()->height() / m_lineHeight), m_numLines);

    // calculate number of characters requred to represent the address
    m_numAddressChars = 4;
    while((dsize >>= 16) > 0)
        m_numAddressChars += 4; // plus 1 char inter word spacing

    const int colMargin = 2 * m_textMargin;
    m_characterWidth = fm.width(QChar('M'));
    m_textMargin = m_characterWidth;

    const int addrColumnWidth = colMargin + m_characterWidth * m_numAddressChars;
    const int rawColumnWidth = colMargin + m_characterWidth * rawColumnChars;
    const int printableColumnWidth = colMargin + m_characterWidth * m_numBytesInLine;
    const int lineWidth = addrColumnWidth + rawColumnWidth + printableColumnWidth;

    // manage correct scroll and scrollbar ranges
    horizontalScrollBar()->setRange(0, lineWidth + m_textMargin - viewport()->width());
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setRange(0, m_numLines - m_numVisibleLines);
    verticalScrollBar()->setPageStep(m_numVisibleLines);

    m_vp.setRect(horizontalScrollBar()->value(), verticalScrollBar()->value(),
                 viewport()->width(), viewport()->height());

    m_addrColumn.setRect( -m_vp.x(), 0, addrColumnWidth, m_vp.height());
    m_rawColumn.setRect( m_addrColumn.right(),0, rawColumnWidth, m_vp.height());
    m_printableColumn.setRect(m_rawColumn.right(),0, printableColumnWidth, m_vp.height());
}

qint64 HexWidget::dataSize() const
{
    //XXX - QAbstractScrollView is not able to handle larger viewport sizes so
    // for now the datasize 0xffffffff seams to be the limit of what we can display.
    if(m_data)
        return m_data->size();

    return 0;
}

QByteArray HexWidget::dataLineAtAddr(qint64 addr) const
{
    Q_ASSERT(m_data);
    Q_ASSERT(m_data->size() > addr);
    Q_ASSERT(addr >= 0);
    Q_ASSERT(addr % m_numBytesInLine == 0);
    if(!m_data)
        return QByteArray();

    if(m_data->pos() != addr)
        m_data->seek(addr);

    return m_data->read(m_numBytesInLine);
}

HexWidget::RegionId HexWidget::regionAtPos(const QPoint& pos) const
{
    if(pos.x() < m_addrColumn.left() || pos.x() > m_printableColumn.right())
        return RegionId::INVALID;
    if(pos.x() > m_printableColumn.left())
        return RegionId::PRINTABLE;
    if(pos.x() > m_rawColumn.left())
        return RegionId::RAW;
    else
        return RegionId::ADDRESS;
}

qint64 HexWidget::addrAtPos(const QPoint& pos, bool& ok) const
{
    //TODO
    Q_UNUSED(pos);
    Q_UNUSED(ok);
    return 0x0;
}

QPoint HexWidget::posAtAddr(qint64 addr, RegionId reg, bool& ok) const
{
    //TODO
    Q_UNUSED(addr);
    Q_UNUSED(reg);
    Q_UNUSED(ok);
    return QPoint();
}

void HexWidget::highlight(qint64 start_addr, qint64 len)
{
    //TODO
    Q_UNUSED(start_addr);
    Q_UNUSED(len);
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

void HexWidget::scrollContentsBy(int dx, int dy)
{
    // scroll vertically only by full line increments
    viewport()->scroll(dx, dy * m_lineHeight);
    recalculateFontMetrics(); // TODO - use lighter version of this call
    viewport()->update();
}

void HexWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(viewport());
    painter.fillRect(QRect(QPoint(0, 0), m_vp.size()), palette().background());
    painter.fillRect(m_addrColumn, palette().dark());
    painter.fillRect(m_printableColumn, palette().dark());

    for(int j = 0; j <= m_numVisibleLines; ++j)
    {
        // y viewport offset is indexed starting from 1 thus y-1
        quint64 address = (m_vp.y() - 1+j) * m_numBytesInLine;
        // paint the address values
        painter.drawText(QPointF(m_addrColumn.left() + m_textMargin, m_lineHeight * j),
                         numToHexStr(address).right(m_numAddressChars));

        // paint the actual values
        for(int i = 0; i < m_numBytesInLine; ++i)
        {
            // TODO paint the actual values - possibly with nice spacing and
            // alternating background
            // TODO implement highlighting logic
        }
    }

#ifdef DEBUG_BUILD
    drawDebug(painter);
#endif

    painter.drawLine(m_addrColumn.left(), m_addrColumn.top(), m_addrColumn.left(), m_rawColumn.bottom());
    painter.drawLine(m_rawColumn.left(), m_rawColumn.top(), m_rawColumn.left(), m_rawColumn.bottom());
    painter.drawLine(m_printableColumn.left(), m_printableColumn.top(), m_printableColumn.left(), m_printableColumn.bottom());
    painter.drawLine(m_printableColumn.right(), m_printableColumn.top(), m_printableColumn.right(), m_printableColumn.bottom());
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

void HexWidget::mouseMoveEvent(QMouseEvent *e)
{
#ifdef DEBUG_BUILD
    m_cursorPos = e->pos();
    viewport()->update();
#endif
}

void HexWidget::drawDebug(QPainter& painter) const
{
    painter.drawText(m_cursorPos, QString("<%1:%2>").arg(m_cursorPos.x()).arg(m_cursorPos.y()));
    const QColor color_g(116,214,0, 50);
    const QColor color_r(239,65,53, 100);
    RegionId reg = regionAtPos(m_cursorPos);
    switch(reg)
    {
        case RegionId::INVALID:
            painter.fillRect(QRect(QPoint(0,0), m_vp.size()), color_r);
            break;
        case RegionId::ADDRESS:
            painter.fillRect(m_addrColumn,  color_g);
            break;
        case RegionId::RAW:
            painter.fillRect(m_rawColumn, color_g);
            break;
        case RegionId::PRINTABLE:
            painter.fillRect(m_printableColumn, color_g);
            break;
        default:
            Q_ASSERT(false);
    }
}
