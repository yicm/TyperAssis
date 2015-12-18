#ifndef NoFocusDelegate_H
#define NoFocusDelegate_H

#include <QPainter>
#include <QStyledItemDelegate>

class NoFocusDelegate :public QStyledItemDelegate
{
public:
    NoFocusDelegate();
    ~NoFocusDelegate();

protected:
     void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const;

};

#endif // SETHIGHLIGHTSECTIONS_H
