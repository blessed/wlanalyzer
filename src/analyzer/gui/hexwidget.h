#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QAbstractScrollArea>
#include <QMenu>
#include <QByteArray>
#include <QFile>
#include <QBuffer>
#include <QSharedPointer>

class HexWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:
    // How should we represent the binary data
    enum class DisplayFormat { BIT_FIELD, HEX};

    // Column identifiers for hit tests
    enum class RegionId {INVALID, ADDRESS, RAW, PRINTABLE};

    explicit HexWidget(QWidget *parent = 0);

    void setData(const QByteArray& data);
    //interface is limited to Random-access QIODevices
    void setData(QIODevice* data);
    inline void setData(QBuffer* data) { setData(static_cast<QIODevice*>(data));}
    inline void setData(QFile* data) { setData(static_cast<QIODevice*>(data));}

    // set smaller or bigger font
    void resizeFont(int sizeIncrement);

signals:
    void addressSelected(qint64 offset);

public slots:
    // ask for highlighting on data region
    void highlight(qint64 start_addr, qint64 len);

private slots:
    // select if the view should be in Hex or Binary
    void setRawDataDisplayFormat(QAction *action);

protected:
    // calculate column widths, line heights, etc.
    void recalculateFontMetrics();
    qint64 dataSize() const;
    QByteArray dataLineAtAddr(qint64 addr) const;
    // return column address/raw data/ascii pointed in viewport by pos
    // or INVALID if outside
    RegionId regionAtPos(const QPoint& pos) const;
    // return address pointed by pos in the widget viewport
    // if pos is invalid ok==false and returns 0x0
    qint64 addrAtPos(const QPoint& pos, bool& ok) const;
    // return Top Left boundary position occupied by data at given addr and reg
    // calculated in absolute viewport coordinates
    // if pos is invalid ok==false and returns QPoint()
    QPoint posAtAddr(qint64 addr, RegionId reg, bool& ok) const;
    // check if address is within range of the data contained in the widget
    bool addrValid(qint64 addr) const;

private:
    // show our context menu with display format options
    void contextMenuEvent(QContextMenuEvent *event);
    void scrollContentsBy(int dx, int dy);
    // draw the data contents of the widget
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *e);
    // mostly for debugging purposes
    void mouseMoveEvent(QMouseEvent *e);
    void drawDebug(QPainter& painter) const;

    QSharedPointer<QIODevice> m_data;
    QMenu m_contextMenu;
    DisplayFormat m_format;

    int m_numBytesInLine;
    int m_numAddressChars;
    int m_textMargin;
    int m_characterWidth;
    QRect m_addrColumn;
    QRect m_rawColumn;
    QRect m_printableColumn;
    // viewport geometry: width, height
    // topLeft represents current position of the viewport top left corner
    // relative to absolute viewport coordinates.
    // Note that x is in pixels ans y is in whole lines due to how this widget is drawn.
    QRect m_vp;
    int m_lineHeight;
    qint64 m_numLines;
    qint64 m_numVisibleLines;
    qint64 m_highlight_addr;
    qint64 m_highlight_len;
    // current position of the cursor for Debugging purposes
    QPoint m_cursorPos;
};

Q_DECLARE_METATYPE(HexWidget::DisplayFormat);

#endif // HEXWIDGET_H
