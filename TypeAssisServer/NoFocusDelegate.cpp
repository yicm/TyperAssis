#include "NoFocusDelegate.h"

NoFocusDelegate::NoFocusDelegate()
{
}

NoFocusDelegate::~NoFocusDelegate()
{
}

void NoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
   QStyleOptionViewItem itemOption(option);
   if (itemOption.state & QStyle::State_HasFocus)
   {
      itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
   }
   QStyledItemDelegate::paint(painter, itemOption, index);
}
