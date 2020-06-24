#include "treeitem.h"
#include <qdatetime.h>
#include <QStringList>
#include <queue>

TreeItem::TreeItem()
{
	parentItem = nullptr;
	itemLevel = 0;	///默认为0
}
TreeItem::TreeItem(TreeItem* parent)
	:TreeItem()
{
	Q_ASSERT(parent);
	parentItem = parent;
	itemLevel = parent->itemLevel + 1;
	itemData = QVector<TreeItemData>(parent->columnCount());
}
TreeItem::TreeItem(RecordData* record)
	:TreeItem()
{
	recordData = record;
}
TreeItem::TreeItem(TreeItem* parent, RecordData* record)
	: TreeItem(parent)
{
	recordData = record;
}
TreeItem::TreeItem(const QVector<QVariant>& data, TreeItem* parent, int role/* = Qt::DisplayRole*/)
	: TreeItem()
{
	parentItem = parent;
	if (parent)
	{
		itemLevel = parent->itemLevel + 1;
	}
	itemData = QVector<TreeItemData>(data.size());
	for (int i = 0; i < itemData.size(); i++)
	{
		itemData[i].setData(data[i], role);
	}
}
TreeItem::~TreeItem()
{
	qDeleteAll(childItems);
}
TreeItem* TreeItem::child(int number)
{
	return childItems.value(number);
}
int TreeItem::childCount() const
{
	return childItems.count();
}
int TreeItem::rowCount() const
{
	return childItems.count();
}
int TreeItem::columnCount() const
{
	return itemData.size();
}
QVariant TreeItem::data(int column, int role/* = Qt::DisplayRole*/) const
{
	QVariant var;
	if (recordData)///先到recrodData中查找
	{
		var = recordData->data(column, role);
	}
	if (!var.isValid())
	{
		TreeItemData d = itemData.value(column);
		if (role == Qt::EditRole)	///EditRole 转到DisplayRole
		{
			var = d.data(Qt::DisplayRole);
		}
		else
		{
			var = d.data(role);
		}
	}
	return var;
}
bool TreeItem::insertChildren(int position, int count, int columns)
{
	if (position < 0 || position > childItems.size())
	{
		return false;
	}
	for (int i = 0; i < count; i++)
	{
		QVector<QVariant> data(columns);
		TreeItem* item = new TreeItem(data, this);
		childItems.insert(position, item);
	}
	return true;
}
bool TreeItem::insertColumns(int position, int columns)
{
	if (position < 0 || position > itemData.size())
	{
		return false;
	}
	for (int i = 0; i < columns; i++)
	{
		itemData.insert(position, TreeItemData());
	}
	foreach(TreeItem * child, childItems)
	{
		child->insertColumns(position, columns);
	}
	return true;
}
TreeItem* TreeItem::parent()
{
	return parentItem;
}
bool TreeItem::removeChildren(int position, int count)
{
	if (position < 0 || position >= childItems.size())
	{
		return false;
	}
	for (int i = 0; i < count && position < childItems.size(); i++)
	{
		delete childItems.takeAt(position);
	}
	return true;
}
bool TreeItem::removeColumns(int position, int columns)
{
	if (position < 0 || position + columns > itemData.size())
	{
		return false;
	}
	itemData.remove(position, columns);
	foreach(TreeItem * child, childItems)
		child->removeColumns(position, columns);
	return true;
}
int TreeItem::childNumber() const // index in parent->childItem
{
	if (parentItem)
	{
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
	}
	return 0;
}
bool TreeItem::setData(int column, const QVariant& value, int role)
{
	if (column < 0 || column >= itemData.size())
	{
		return false;
	}
	///不支持recordData，太麻烦
	//if (recordData && recordData->setData(column, value, role))
	//{
	//	return true;
	//}
	TreeItemData& d = itemData[column];
	if (role == Qt::EditRole)	///EditRole 转到DisplayRole
	{
		d.setData(value, Qt::DisplayRole);
		return true;
	}
	d.setData(value, role);
	return true;
}
bool TreeItem::addChild(TreeItem* child)
{
	childItems.append(child);
	child->setParentItem(this);
	return true;
}
bool TreeItem::isVariantLessThen(const QVariant& left, const QVariant& right)
{
	return left < right;
}
bool TreeItem::isVariantEqualTo(const QVariant& left, const QVariant& right)
{
	return left == right;
}
bool TreeItem::isVariantGreaterThan(const QVariant& left, const QVariant& right)
{
	return left > right;
}
/* 获取兄弟节点，包括自己 */
QVector<TreeItem*> TreeItem::fatChildItems()
{
	QVector<TreeItem*> items;
	if (parent())
	{
		return parent()->childrenItems();
	}
	else
	{
		items.append(this);
	}
	return items;

}
/* 获取兄弟节点，不包括自己 */
QVector<TreeItem*> TreeItem::broOtherItems()
{
	QVector<TreeItem*> brothers = parent()->childrenItems();

	for (int i = 0; i < brothers.size(); i++)
	{
		if (brothers.at(i) == this) brothers.removeAt(i);
		break;
	}
	return brothers;
}
/*获取子节点列表*/
QVector<TreeItem*> TreeItem::childrenItems()
{
	return childItems;
}
/* 移除指定位置的子节点 */
TreeItem* TreeItem::takeChildAt(int position)
{
	return childItems.takeAt(position);
}
/*在末尾增加子节点*/
void TreeItem::appendChild(TreeItem* child)
{
	childItems.append(child);
	child->setParentItem(this);
}
/*在头部增加子节点*/
void TreeItem::prependChild(TreeItem* child)
{
	childItems.prepend(child);
	child->setParentItem(this);
}
/*在指定位置插入子节点*/
void TreeItem::insertChildAt(int position, TreeItem* child)
{
	childItems.insert(position, child);
	child->setParentItem(this);
}
/*移除所有子节点*/
QVector<TreeItem*> TreeItem::takeChildAll()
{
	QVector<TreeItem*> all(0);
	childItems.swap(all);
	//childItems.clear();
	return all;
}
/*设置父亲节点*/
void TreeItem::setParentItem(TreeItem* parent)
{
	parentItem = parent;
	if (parent)
	{
		setItemLevel(parent->itemLevel + 1);
		//setModel(parent->model);
	}
	else
	{
		setItemLevel(0);
	}
}
void TreeItem::appendChildList(const QVector<TreeItem*>& childList)
{
	childItems.append(childList);
	QVector<TreeItem*>::const_iterator itr = childList.begin();
	for (; itr != childList.end(); itr++)
	{
		(*itr)->setParentItem(this);
	}
}
void TreeItem::setChildren(const QVector<TreeItem*>& itemVector)
{
	childItems = itemVector;
}
int TreeItem::levelOfItem()
{
	return itemLevel;
}
void TreeItem::setItemLevel(int level)
{
	itemLevel = level;
	for (int i = 0; i < childItems.size(); i++)
	{
		childItems.at(i)->setItemLevel(level + 1);
	}
}
void TreeItem::detachChildren()
{
	childItems.clear();
}
void TreeItem::setForeground(int col, Qt::GlobalColor clr)
{
	QColor color(clr);
	setData(col, color.red() << 24 | color.green() << 16 | color.blue() << 8 | color.alpha(), Qt::ForegroundRole);
}
void TreeItem::setForeground(int col, unsigned long colorVal)
{
	setData(col, (qulonglong)colorVal, Qt::ForegroundRole);
}
void TreeItem::setForeground(int col, int r, int g, int b, int a)
{
	setData(col, r << 24 | g << 16 | b << 8 | a, Qt::ForegroundRole);
}
QVariant TreeItem::foreground(int col)
{
	return data(col, Qt::ForegroundRole);
}
void TreeItem::setBackground(int col, Qt::GlobalColor clr)
{
	QColor color(clr);
	setData(col, color.red() << 24 | color.green() << 16 | color.blue() << 8 | color.alpha(), Qt::BackgroundRole);
}
void TreeItem::setBackground(int col, unsigned long colorVal)
{
	setData(col, (qulonglong)colorVal, Qt::BackgroundRole);
}
void TreeItem::setBackground(int col, int r, int g, int b, int a)
{
	setData(col, r << 24 | g << 16 | b << 8 | a, Qt::BackgroundRole);
}
QVariant TreeItem::background(int col)
{
	return data(col, Qt::BackgroundRole);
}
void TreeItem::setTextAlign(int col, Rcspace::RcTextAlignment aenum)
{
	setData(col, aenum, Qt::TextAlignmentRole);
}
QVariant TreeItem::textAlign(int col)
{
	return data(col, Qt::TextAlignmentRole);
}
void TreeItem::setIcon(int col, Rcspace::RcIconEnum ienum)
{
	setData(col, ienum, Qt::DecorationRole);
}
QVariant TreeItem::cellIcon(int col)
{
	return data(col, Qt::DecorationRole);
}
void TreeItem::setVirtual(bool bVirtual)
{
	isVirtual = bVirtual;
}
bool TreeItem::isVirtualItem()
{
	return isVirtual;
}
void TreeItem::setCheckable(int col, bool check)
{
	if (col < 0 || col >= columnCount())
		return;
	TreeItemData& d = itemData[col];
	if (check)
	{
		d.setData(Qt::Unchecked, Qt::CheckStateRole);	////默认为Unchecked
	}
	else
	{
		d.eraseData(Qt::CheckStateRole);
	}
}
void TreeItem::setChecked(int col, bool check)
{
	setData(col, check ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}
void TreeItem::setCheckState(int col, Qt::CheckState state)
{
	setData(col, state, Qt::CheckStateRole);
}
bool TreeItem::isCheckable(int col)
{
	if (col < 0 || col >= columnCount())
		return false;
	TreeItemData& d = itemData[col];
	QVariant var = d.data(Qt::CheckStateRole);
	if (var.isValid())
		return true;
	return false;
}
bool TreeItem::isChecked(int col)
{
	if (col < 0 || col >= columnCount())
		return false;
	TreeItemData& d = itemData[col];
	QVariant var = d.data(Qt::CheckStateRole);
	if (var.isValid() && var == Qt::Checked)
		return true;
	return false;
}
void TreeItem::eraseData(int col, int role)
{
	if (col < 0 || col >= columnCount())
		return;
	TreeItemData& d = itemData[col];
	d.eraseData(role);
}
void TreeItem::switchChild(int m, int n)
{
	if (m < 0 || n < 0 || m >= childItems.count() || n >= childItems.count())
	{
		return;
	}
	TreeItem* temp = childItems.at(m);
	childItems[m] = childItems.at(n);
	childItems[n] = temp;
}

void TreeItem::setRecordData(RecordData* record)
{
	recordData = record;
}

RecordData* TreeItem::innerRecordData()
{
	return recordData;
}

