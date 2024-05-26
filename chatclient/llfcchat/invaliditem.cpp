#include "invaliditem.h"

InvalidItem::InvalidItem(QWidget *parent) : ListItemBase(parent)
{
    SetItemType(ListItemType::InvalidItem);
}
