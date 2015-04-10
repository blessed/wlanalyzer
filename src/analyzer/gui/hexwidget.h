#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QAbstractScrollArea>
#include <QMenu>
#include <QColor>
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
    ~HexWidget();

    void setData(const QByteArray& data);
    //interface is limmited to Random-access QIODevices
    void setData(QIODevice* data);
    inline void setData(QBuffer* data) { setData(static_cast<QIODevice*>(data));}
    inline void setData(QFile* data) { setData(static_cast<QIODevice*>(data));}

    // set smaller or bigger font
    void resizeFont(int sizeIncrement);

signals:
    void selectionChanged(quint32 offset);

public slots:
    // ask for highlighting on data region
    void highlight(quint32 offset, quint32 len);

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
    RegionId regionAtPos(QPoint pos) const;
    // return address pointed by pos in the widget viewport
    // if pos is invalid then ok==false
    qint64 addrAtPos(QPoint pos, bool& ok) const;
    // return address pointed by pos in the widget viewport
    // if pos is invalid ok==false and returns 0x0
    QPoint posAtAddr(qint64 addr, RegionId reg, bool& ok) const;

private:
    // show our context menu with display format options
    void contextMenuEvent(QContextMenuEvent *event);
    void scrollContentsBy(int dx, int dy);
    // draw the data contents of the widget
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);
    // mostly for debuging purposes
    void mouseMoveEvent(QMouseEvent *e);
    void drawDebug(QPainter& painter);

    QIODevice* m_data;
    QMenu m_contextMenu;
    DisplayFormat m_format;

    int m_numBytesInLine;
    int m_numAddressChars;
    int m_textMargin;
    int m_characterWidth;
    int m_addrColumnWidth;
    int m_rawColumnWidth;
    int m_printableColumnWidth;
    int m_lineWidth;
    int m_lineHeight;
    qint64 m_numLines;
    qint64 m_numVisibleLines;
    QPoint m_cursorPos;
};

Q_DECLARE_METATYPE(HexWidget::DisplayFormat);

#endif // HEXWIDGET_H
