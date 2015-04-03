#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QScrollArea>
#include <QByteArray>
#include <QColor>

class HexWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit HexWidget(QWidget *parent = 0);

    void setData(QByteArray data);
    void setHighlightColor(QColor color);

signals:
    void selectionChanged(quint32 offset);

public slots:
    // ask for highlighting on data region
    void highlight(quint32 offset, quint32 len);
    // select if the view should be in Hex or Binary
    void setRawDataDisplayFormat(QAction *action);

private:
    QByteArray m_data;
    QColor m_highlightColor;
    QWidget* m_offsetArea;
    QWidget* m_rawDataArea;
    QWidget* m_AsciiArea;
};

#endif // HEXWIDGET_H
