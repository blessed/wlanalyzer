#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QAbstractScrollArea>
#include <QMenu>
#include <QByteArray>
#include <QColor>

class HexWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:
    // How should we represent the binary data
    enum class DisplayFormat { BIT_FIELD, HEX};

    explicit HexWidget(QWidget *parent = 0);

    void setData(QByteArray data);
    void setHighlightColor(QColor color);
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

private:
    // show our context menu with display format options
    void contextMenuEvent(QContextMenuEvent *event);
    // draw the data contents of the widget
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);

    // calculate column widths, line heights, etc.
    void recalculateFontMetrics();

    QByteArray m_data;
    QColor m_highlightColor;
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
    int m_numLines;
    int m_numVisibleLines;
};

Q_DECLARE_METATYPE(HexWidget::DisplayFormat);

#endif // HEXWIDGET_H
