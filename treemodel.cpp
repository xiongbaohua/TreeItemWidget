#include "treemodel.h"
#include "treeitem.h"
#include <queue>
#include <qmimedata.h>

TreeModel::TreeModel(QObject* parent)
	:QAbstractItemModel(parent)
{
	rootItem = NULL;
	setDefaultAttrOfModel();
}

TreeModel::TreeModel(TreeItem* rItem, QObject* parent)
	: QAbstractItemModel(parent)
{
	rootItem = rItem;
	attrOfColumns = QVector<QMap<int, QVariant>>(rootItem->columnCount(), QMap<int, QVariant>());
	setDefaultAttrOfModel();
}

TreeModel::TreeModel(LPCSTR columns[], QObject* parent)
	:QAbstractItemModel(parent)
{
	QVector<QVariant> itemData;
	for (int i = 0; columns[i] != NULL; i++)
	{
		itemData << STRING(columns[i]);
	}
	rootItem = new TreeItem(itemData);
	attrOfColumns = QVector<QMap<int, QVariant>>(rootItem->columnCount(), QMap<int, QVariant>());
	setDefaultAttrOfModel();
}

TreeModel::TreeModel(QByteArray columns[], int cols, QObject* parent)
{
	QVector<QVariant> itemData;
	for (int i = 0; i < cols; i++)
	{
		itemData << STRING(columns[i]);
	}
	rootItem = new TreeItem(itemData);
	attrOfColumns = QVector<QMap<int, QVariant>>(rootItem->columnCount(), QMap<int, QVariant>());
	setDefaultAttrOfModel();
}

TreeModel::~TreeModel()
{
	if (rootItem)
	{
		delete rootItem;
	}
	itemVector.clear();///itemVector中的元素在delete rootItem处析构
	qDeleteAll(itemToDelVector);
	itemToDelVector.clear();
	qDeleteAll(itemHideVector);
	itemHideVector.clear();
	if (modelRecord)
	{
		delete modelRecord;
	}
	if (oldModelRecord)
	{
		delete oldModelRecord;
	}
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	QVariant d;
	TreeItem* item = getItem(index);
	if (item->isVirtualItem())///目录节点特殊处理，目录节点的值都存在第一列，除非index的列为视图可见的第一列，否则都不显示数据
	{
		//if (head && head->logicalIndexAt(0) == index.column())
		//{
		//	//d = item->data(0, role);
		//}
		//if (!d.isValid() && !(d = attrOfColumns.value(index.column()).value(role)).isValid())	///列级别
		//	d = attrOfModel.value(role);	///model 全局级别
		if (role == Qt::FontRole)
		{
			d = attrOfModel.value(role);///只保留字体信息，其他信息都删除，（某种信息会导致目录节点无法跨列，并且选中时text不显示）
		}
		return d;
	}
	else
	{
		if (!(d = item->data(index.column(), role)).isValid())	///单元格级别
			if (!(d = attrOfColumns.value(index.column()).value(role)).isValid())	///列级别
				d = attrOfModel.value(role);	///model 全局级别
	}

	switch (role)
	{
		//case Qt::DisplayRole:	//显示的文本
		//	return d;
	case Qt::EditRole:
		return item->data(index.column(), Qt::DisplayRole);
	case Qt::ForegroundRole:	//文本颜色
	{
		quint32 color = d.toULongLong();
		return QColor((color & 0xff0000) >> 16, (color & 0xff00) >> 8, (color & 0xff), (color & 0xff000000) >> 24);
	}
	case Qt::TextAlignmentRole:	//文本停靠位置
	{
		int align = d.toInt();
		Rcspace::RcTextAlignment aenum = static_cast<Rcspace::RcTextAlignment>(align);
		return Rcspace::specialTextAlignment(aenum);
	}
	case Qt::DecorationRole:	//icon
	{
		int ienum = d.toInt();
		return Rcspace::specialIcon(static_cast<Rcspace::RcIconEnum>(ienum));
	}
	//case Qt::FontRole:	//字体
	//	return d;
	case Qt::BackgroundRole:	//背景色
	{
		quint32 color = d.toULongLong();
		return QColor((color & 0xff0000) >> 16, (color & 0xff00) >> 8, (color & 0xff), (color & 0xff000000) >> 24);
	}
	//case Qt::CheckStateRole:	//勾选
	//	return d;
	default:
		return d;
	}
	return QVariant();
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TreeItem* parentItem = getItem(parent);

	TreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}


QModelIndex TreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem* childItem = getItem(index);
	TreeItem* parentItem = childItem->parent();

	if (!parentItem || parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}


int TreeModel::rowCount(const QModelIndex& parent) const
{
	TreeItem* parentItem = getItem(parent);

	return parentItem->childCount();
}


int TreeModel::columnCount(const QModelIndex& parent) const
{
	return rootItem->columnCount();
}


Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags;
	if (!index.isValid())
		defaultFlags = 0;
	else
	{
		defaultFlags = QAbstractItemModel::flags(index);
	}
	if (editable && editableColVec.indexOf(index.column()) > -1)
	{
		defaultFlags |= Qt::ItemIsEditable;
	}
	if (dragndropable)
	{
		if (index.isValid())
		{
			defaultFlags |= Qt::ItemIsDragEnabled;
		}
		if (!index.isValid() || getItem(index)->isVirtualItem())
		{
			defaultFlags |= Qt::ItemIsDropEnabled;
		}
	}
	if (checkable)
	{
		defaultFlags |= Qt::ItemIsUserCheckable;
	}
	///如果Item设置了不可编辑,屏蔽EditRole
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (item && item != rootItem && item->data(index.column(), EditEnableRole).isValid() && !item->data(index.column(), EditEnableRole).toBool())
	{
		defaultFlags &= ~Qt::ItemIsEditable;
		defaultFlags &= ~Qt::ItemIsUserCheckable;	///把userCheckable也作为EditEnableRole的一部分
	}
	if (!itemIsSelectable)
	{
		defaultFlags &= ~Qt::ItemIsSelectable;
	}
	return defaultFlags;
}


void TreeModel::sort(int column, Qt::SortOrder order)
{
	//HeadView->flipSortIndicator函数中会调用setSortIndicator，然后emit sortIndicatorChanged 信号
	//TreeView 接收sortIndicatorChanged信号后，会调用此处的sort函数。
	//而在HeadView->flipSortIndicator返回后，才会emit sectionClicked 信号。
	//在headView中，增加了sectionClicked信号的处理函数onSectionClicked，设置表头多列排序样式。
	//这两处需保持效果同步

	if (column < 0 || column >= columnCount()) return;
	sortColumn = column;
	QPair<int, Qt::SortOrder> colSort(column, order);
	bool bHasSorted = false;
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier) ///shift 键按下，开启多列排序，例外：参与分组的字段一定会参与排序
	{
		this->ableMultiColumnSort = true;
		for (auto& it : orderColumns)
		{
			if (it.first == column)
			{
				//for (int i = 0; i < orderColumns.size(); i++)
				//{
				//	if (orderColumns.at(i).first == column)
				//	{
				//		orderColumns.removeAt(i);
				//		break;
				//	}
				//}
				bHasSorted = true;
				if (it.second == order)///排序顺序一致，直接跳出
				{
					return;
				}
				else
				{
					it.second = order;
				}
			}
		}
	}
	else
	{
		this->ableMultiColumnSort = false;
		MultiColumnOrder temp;
		temp.swap(orderColumns);
		MultiColumnOrder::iterator it, ite = temp.end();
		for (it = temp.begin(); it != ite; it++)	///保留参与分组的字段，以及当前排序字段
		{
			//if (groupColumns.indexOf((*it).first) > -1 && column != (*it).first)
			//{
			//	orderColumns.append(*it);
			//}
			if (groupColumns.indexOf((*it).first) > -1 || column == (*it).first)
			{
				if (column == it->first)
				{
					bHasSorted = true;
					it->second = order;
				}
				orderColumns.append(*it);
			}
		}
	}
	//orderColumns.append(colSort);///无论如何都添加到末尾
	if (!bHasSorted)
		orderColumns.append(colSort);///未曾参与排序则添加到末尾
	emit layoutAboutToBeChanged();
	sortChildrenItems(rootItem, column, order/*, true*/);
	emit layoutChanged();
}


bool TreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
	bool success;
	beginInsertColumns(parent, position, position + columns - 1);
	success = rootItem->insertColumns(position, columns);
	endInsertColumns();

	///更新排序列、分组列的存储值
	for (int i = 0; i < orderColumns.size(); i++)
	{
		QPair<int, Qt::SortOrder> sord = orderColumns.at(i);
		if (sord.first >= position)
		{
			sord.first += columns;	///position之后的列后移
		}
	}
	for (int i = 0; i < groupColumns.size(); i++)
	{
		int gcol = groupColumns.at(i);
		if (gcol >= position)
		{
			gcol += columns;
		}
	}
	return success;
}


bool TreeModel::removeColumns(int position, int columns, const QModelIndex& parent)
{
	bool success;

	///删除列会影响到原来的排序和分组，先处理被删除的排序和分组，然后再做删除列操作
	///最后更新排序列、分组列的存储值
	for (int i = 0; i < groupColumns.size(); i++)	///删除分组
	{
		int gcol = groupColumns.at(i);
		if (gcol >= position && gcol < (position + columns))
		{
			delGroupColumn(gcol);
		}
	}
	MultiColumnOrder sortOrds;
	for (int i = 0; i < orderColumns.size(); i++)
	{
		QPair<int, Qt::SortOrder> odr = orderColumns.at(i);
		if (odr.first < position || odr.first >= position + columns)
		{
			sortOrds.append(odr);	///保存不被删除的排序列
		}
	}
	sortOrds.swap(orderColumns);
	doSortByColumns();	///重新排序

	/////////////删除列
	beginRemoveColumns(parent, position, position + columns - 1);
	success = rootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (rootItem->columnCount() == 0)
		removeRows(0, rowCount());

	///更新排序列、分组列的存储值
	for (int i = 0; i < groupColumns.size(); i++)	///删除分组
	{
		int gcol = groupColumns.at(i);
		if (gcol > position)
		{
			groupColumns[i] = gcol + columns;
		}
	}
	for (int i = 0; i < orderColumns.size(); i++)
	{
		QPair<int, Qt::SortOrder> odr = orderColumns.at(i);
		if (odr.first > position)
		{
			odr.first += columns;
		}
	}
	return success;
}


bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	TreeItem* parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, rootItem->columnCount());
	endInsertRows();

	return success;
}


bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	TreeItem* parentItem = getItem(parent);
	bool success = true;

	beginRemoveRows(parent, position, position + rows - 1);
	success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	TreeItem* pitem = static_cast<TreeItem*>(index.internalPointer());
	if (pitem == nullptr || pitem == rootItem)
	{
		return true;
	}
	bool success = pitem->setData(index.column(), value, role);
	int childCnt = pitem->childCount();
	emit dataChanged(index, index, QVector<int>(1, role));
	if (childCnt)
	{
		for (int i = 0; i < childCnt; i++)
		{
			setDataUnaffectParentCheckState(createIndex(index.row(), index.column(), pitem->child(i)), value, role);
		}
		emit dataChanged(createIndex(0, index.column(), pitem->child(0)), createIndex(childCnt - 1, index.column(), pitem->child(childCnt - 1)), QVector<int>(1, role));
	}
	if (role == Qt::CheckStateRole)	///checkstate 对父节点的影响
	{
		setCheckStateAscend(parent(index));
	}

	///该信号不用于view显示，待所有相关父节点、子节点数据改变完后再发送，与子节点相关的外部数据是否要处理由外部逻辑确定
	emit dataAltered(pitem, index.column(), QVector<int>(1, role));
	return success;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
	bool success = false;
	if (rootItem)
	{
		success = rootItem->setData(section, value);
	}
	emit headerDataChanged(Qt::Horizontal, section, section);
	return success;
}

bool TreeModel::setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles)
{
	bool success = false;
	TreeItem* item = getItem(index);
	if (item && item != rootItem)
	{
		for (auto it = roles.cbegin(); it != roles.cend(); it++)
		{
			success = item->setData(index.column(), it.value(), it.key());
		}
	}
	return success;
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const
{
	if (index.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

int TreeModel::sortColumnNum()
{
	return sortColumn;
}

MultiColumnOrder TreeModel::sortMultiColumns()
{
	return orderColumns;
}

bool TreeModel::isMultiColumnSort()
{
	return ableMultiColumnSort;
}

void TreeModel::setSortAbled(bool enable)
{
	if (!enable)
		orderColumns.clear();
	return;
}

void TreeModel::resetModelData(TreeItem* rItem) // 重设模型数据
{
	beginResetModel();
	if (rootItem && rootItem != rItem)
	{
		delete rootItem;
	}
	rootItem = rItem;
	//rootItem->setModel(this);
	endResetModel();
}

////增加按字段分组
void TreeModel::addGroupColumn(int column) //增加分组
{
	if (groupColumns.indexOf(column) > -1)
	{
		return;
	}
	///
	MultiColumnOrder::iterator it, ite = orderColumns.end();
	for (it = orderColumns.begin(); it != ite; it++)
	{
		if ((*it).first == column)
			break;
	}
	if (it == ite)	///没有参与排序，默认增加为升序排序
	{
		sort(column, Qt::AscendingOrder);
	}

	TreeItem* p = rootItem;
	beginResetModel();	///use resetModel is safer than layoutChanged & RemoveRows
	std::queue<TreeItem*> itemQueue;
	itemQueue.push(p);
	while (!itemQueue.empty())
	{
		TreeItem* item = itemQueue.front();
		itemQueue.pop();
		if (item->childCount() > 0 && item->child(0)->isVirtualItem()) //目录节点入队列
		{
			for (int i = 0; i < item->childCount(); i++)
			{
				itemQueue.push(item->child(i));
			}
		}
		if (item->childCount() > 0 && !item->child(0)->isVirtualItem()) //最后一层目录节点做处理
		{
			QMap<QVariant, TreeItem*> varItemMap;
			QVector<TreeItem*> allChild = item->takeChildAll();
			for (int i = 0; i < allChild.size(); i++)
			{
				TreeItem* sunItem = allChild.at(i);
				QVariant colVar = sunItem->data(column);
				if (varItemMap.contains(colVar))
				{
					varItemMap.value(colVar)->appendChild(sunItem);
				}
				else
				{
					QVector<QVariant> data(item->columnCount());
					data[0] = rootItem->data(column).toString() + ":" + sunItem->data(column).toString();//目录显示格式（列名: 列值）
					//while (data.count() < item->columnCount())
					//{
					//	data << QVariant(); // 目录节点其他列都设为空
					//}

					//TreeItem* nItem = new TreeItem(data, this, item);
					TreeItem* nItem = new TreeItem(data, item);
					nItem->setData(0, sunItem->data(column), Qt::UserRole);	////item[0] UserRole存储sunItem的displayRole，用于删除分组merge时比较
					nItem->setForeground(0, RGB(120, 120, 120));
					if (sunItem->isCheckable(0))
					{
						nItem->setCheckable(0, true);
						sunItem->setChecked(0, false);
					}
					nItem->setVirtual(true);	////分组时创建的目录节点，必须设置为true
					item->appendChild(nItem);
					nItem->appendChild(sunItem);
					varItemMap.insert(colVar, nItem);
				}
			}
		}
	}
	endResetModel();
	groupColumns.append(column);
	return;
}

/////删除按字段分组，删除顺序不需要按照添加的顺序
void TreeModel::delGroupColumn(int column) //删除level级节点下的所有子节点，同时将孙节点变成子节点
{
	int level = groupColumns.indexOf(column);
	if (level < 0)
	{
		return;
	}

	TreeItem* p = rootItem;
	beginResetModel();	///use resetModel is safer than layoutChanged & RemoveRows
	std::queue<TreeItem*> itemQueue;
	itemQueue.push(p);
	while (!itemQueue.empty())
	{
		TreeItem* item = itemQueue.front();
		itemQueue.pop();
		if (item->levelOfItem() == level) // 找到操作节点，对应分组列节点的父亲节点，要删除的节点是该节点的子节点
		{
			QVector<TreeItem*> allChild = item->takeChildAll();
			for (int i = 0; i < allChild.size(); i++)
			{
				item->appendChildList(allChild.at(i)->takeChildAll());
			}
			qDeleteAll(allChild);
			mergeSameGroupedItems(item);
		}
		else if (item->levelOfItem() < level) // 还没到达level层，继续遍历
		{
			for (int i = 0; i < item->childCount(); ++i)
			{
				itemQueue.push(item->child(i));
			}
		}
	}
	endResetModel();
	groupColumns.removeAt(level);
	for (int i = 0; i < orderColumns.size(); i++)
	{
		if (orderColumns.at(i).first == column)
			orderColumns.removeAt(i);
	}
	doSortByColumns();
	return;
}

////根据groupColumns执行分组，可能会用到
//一定要是在没有分组的情况下做
void TreeModel::doGroupByColumns()
{
	QVector<int> gcols;
	gcols.swap(groupColumns);
	QVector<int>::iterator itr = gcols.begin();
	for (; itr != gcols.end(); itr++)
	{
		addGroupColumn(*itr);
	}
}

////根据groupColumns执行分组，可能会用到
//一定要是在没有分组的情况下做
void TreeModel::doGroupByColumns(const QVector<int>& colVec)
{
	groupColumns.clear();
	QVector<int>::const_iterator itr = colVec.begin();
	for (; itr != colVec.end(); itr++)
	{
		addGroupColumn(*itr);
	}
}

////根据orderColumns执行排序，可能会用到
//在数据重设且没有排序时调用，恢复原排序状态
void TreeModel::doSortByColumns()
{
	MultiColumnOrder sorders = MultiColumnOrder();
	orderColumns.swap(sorders);
	for (int i = 0; i < sorders.size(); i++)
	{
		QPair<int, Qt::SortOrder> odr = sorders.at(i);
		sortPrivate(odr.first, odr.second);
	}
}

void TreeModel::doSortByColumns(MultiColumnOrder sortOrders)
{
	orderColumns.swap(sortOrders);
	doSortByColumns();
}

///查找record，任一列值与var匹配，返回第一个匹配的index,否则返回
QModelIndex TreeModel::findRecordMatchVal(const QVariant& var)
{
	return findRecordNoColumn(rootItem, var);
}
///重载findRecord，加入列限制
QModelIndex TreeModel::findRecordMatchValByColumn(const QVariant& var, int column)
{
	return findRecordByColumn(rootItem, var, column);
}

TreeItem* TreeModel::returnRootItem()
{
	return rootItem;
}

void TreeModel::resetModelDataWithItemList(QVector<TreeItem*>& itemList)
{
	//if (rootItem)
	//{
	//	beginResetModel();

	//	rootItem->removeChildren(0, rootItem->childCount());
	//	rootItem->setChildren(itemList);
	//	itemVector.clear();
	//	itemVector = itemList;
	//	endResetModel();
	//}
	qDeleteAll(itemToDelVector);
	itemToDelVector.clear();
	itemVector.swap(itemToDelVector);
	itemVector.swap(itemList);
	updateIndex();
}
void TreeModel::appendRows(const QVector<TreeItem*>& itemList)
{
	if (rootItem)
	{
		beginResetModel();
		rootItem->appendChildList(itemList);
		itemVector.append(itemList);
		endResetModel();
	}
}

QVector<int> TreeModel::getGroupColumns()
{
	return groupColumns;
}
void TreeModel::resetGroupColumns(QVector<int> cols)
{
	groupColumns = cols;
}

void TreeModel::setHeader(TreeItem* root)
{
	if (rootItem)
	{
		delete rootItem;
	}
	rootItem = root;
}

///merge item 节点下相同的分类目录
void TreeModel::mergeSameGroupedItems(TreeItem* item)
{
	std::queue<TreeItem*> itemQueue;
	std::map<QVariant, TreeItem*> itemMap;
	TreeItem* pitem, * citem;
	itemQueue.push(item);
	while (!itemQueue.empty())
	{
		pitem = itemQueue.front();
		itemQueue.pop();
		//pitem的子节点为非分组生成的目录节点则无需处理
		if (pitem->childCount() > 0 && pitem->child(0)->isVirtualItem())	//分组生成的目录节点不会和原始数据节点位于同一高度，因此判断child(0)是否为虚即可
		{
			std::map<QVariant, TreeItem*>::iterator it;
			for (int i = 0; i < pitem->childCount();)
			{
				citem = pitem->child(i);
				QVariant value = citem->data(0, Qt::UserRole);
				if ((it = itemMap.find(value)) == itemMap.end())
				{
					itemMap[value] = citem;
					i++;	//没有发生merge，子节点不会被删除，i++
				}
				else
				{
					itemMap[value]->appendChildList(citem->takeChildAll());
					delete pitem->takeChildAt(i);	//该子节点的子节点被merge到之前相同group的兄弟节点下，变成了一个空分组，需要删除
				}
			}
			for (int i = 0; i < pitem->childCount(); i++)
			{
				itemQueue.push(pitem->child(i));	//将merge之后的子节点加入队列，对每个子节点做merge
			}
			itemMap.clear();	//清空，开启下一轮
		}
	}
}

void TreeModel::sortPrivate(int column, Qt::SortOrder order)	///
{
	if (column < 0 || column >= columnCount()) return;
	sortColumn = column;
	QPair<int, Qt::SortOrder> colSort(column, order);
	orderColumns.append(colSort);
	//layoutAboutToBeChanged and layoutChanged should be call before and after sort
	emit layoutAboutToBeChanged();
	sortChildrenItems(rootItem, column, order);
	emit layoutChanged();
}

TreeItem* TreeModel::getItemAt(int i)
{
	if (i < 0 || i >= itemVector.size())
	{
		return nullptr;
	}
	return itemVector[i];
}

int TreeModel::itemCounts()
{
	return itemVector.size();
}

///设置model全局默认属性
void TreeModel::setDefaultAttrOfModel()
{
	attrOfModel[Qt::TextAlignmentRole] = 0;
	attrOfModel[Qt::DecorationRole] = Rcspace::noIcon;
	attrOfModel[Qt::BackgroundRole] = 0x00ffffff;	///white, rbg=0xffffff,a=0x00
	attrOfModel[Qt::ForegroundRole] = 0xff000000;	///black, rgb=0x000000,a=0xff
	attrOfModel[Qt::FontRole] = QFont();
}
void TreeModel::removeItem(TreeItem* item)
{
	if (!item || item == rootItem)
	{
		return;
	}
	const int row = item->childNumber();
	QModelIndex index = createIndex(row, 0, item);
	beginRemoveRows(index.parent(), row, row);
	item->parent()->takeChildAt(row);
	endRemoveRows();
}
void TreeModel::setCheckStateAscend(const QModelIndex& index)
{
	if (!index.isValid())
	{
		return;
	}
	TreeItem* pItem = static_cast<TreeItem*>(index.internalPointer());
	if (!pItem || pItem == rootItem)
	{
		return;
	}
	int nChild = pItem->childCount();
	int nChecked = 0, nUnchecked = 0;
	Qt::CheckState ostate, nstate;
	ostate = pItem->data(0, Qt::CheckStateRole).value<Qt::CheckState>();
	for (int i = 0; i < nChild; i++)
	{
		Qt::CheckState check = pItem->child(i)->data(0, Qt::CheckStateRole).value<Qt::CheckState>();
		if (check == Qt::Checked)
		{
			nChecked++;
		}
		else if (check == Qt::Unchecked)
		{
			nUnchecked++;
		}
	}
	nstate = nChecked == nChild ? Qt::Checked : nUnchecked == nChild ? Qt::Unchecked : Qt::PartiallyChecked;
	pItem->setCheckState(index.column(), nstate);
	if (nstate != ostate)
	{
		emit dataChanged(index, index, QVector<int>(1, Qt::CheckStateRole));
		setCheckStateAscend(parent(index));
	}
}
bool TreeModel::setDataUnaffectParentCheckState(const QModelIndex& index, const QVariant& value, int role)
{
	TreeItem* pitem = static_cast<TreeItem*>(index.internalPointer());
	if (pitem == nullptr || pitem == rootItem)
	{
		return true;
	}
	bool success = pitem->setData(index.column(), value, role);
	int childCnt = pitem->childCount();
	if (childCnt)
	{
		for (int i = 0; i < childCnt; i++)
		{
			setDataUnaffectParentCheckState(createIndex(index.row(), index.column(), pitem->child(i)), value, role);
		}
		emit dataChanged(createIndex(0, index.column(), pitem->child(0)), createIndex(childCnt - 1, index.column(), pitem->child(childCnt - 1)), QVector<int>(1, role));
	}
	return success;
}
void TreeModel::sortChildrenItems(TreeItem* parent, int column, Qt::SortOrder order)
{
	if (!parent->childCount())
	{
		return;
	}
	auto compare = std::bind(&TreeModel::itemLessThan, this, std::placeholders::_1, std::placeholders::_2);
	MultiColumnOrder& sortOdr = orderColumns;
	int indexOfColumn = sortOdr.size() - 1;///当前排序列在所有排序列中的位置
	for (int i = 0; i < sortOdr.size(); i++)
	{
		if (sortOdr.at(i).first == column)
		{
			indexOfColumn = i;
			break;
		}
	}
	std::queue<TreeItem*> itemQueue;
	TreeItem* pitem;
	QVector<int>& groupCols = groupColumns;
	int level;
	itemQueue.push(parent);
	if ((level = groupCols.indexOf(column)) < 0)	///叶子节点排序
	{
		compare = std::bind(order == Qt::AscendingOrder ? &TreeModel::itemLessThan : &TreeModel::itemGreaterThan, this, std::placeholders::_1, std::placeholders::_2);
		while (!itemQueue.empty())
		{
			pitem = itemQueue.front();
			itemQueue.pop();
			if (pitem->childCount() > 0 && pitem->child(0)->isVirtualItem()/*->childCount() > 0*/)	///
			{
				for (int i = 0; i < pitem->childCount(); i++)
				{
					itemQueue.push(pitem->child(i));
				}
			}
			else ///对pitem的子节点排序
			{
				QVector<TreeItem*>& itemVec = pitem->childItems;
				std::map<TreeItem*, int> bfIndexMap, afIndexMap;
				if (sortOdr.size() > 1)	///多列排序
				{
					int lastSortCol = sortOdr.at(sortOdr.size() - 2).first;	///上一排序字段
					QVector<TreeItem*>::iterator it1 = itemVec.begin(), it2 = it1, ite = itemVec.end(), it3;
					while (it2 != ite)
					{
						while (it2 != ite && multiColEqual(*it1, *it2, indexOfColumn))	///在该列之前字段排序的基础上对Qvector局部排序  
							it2++;
						for (it3 = it1; it3 != it2; it3++)
						{
							bfIndexMap[*it3] = it3 - itemVec.begin();	///保存排序之前的下标
						}
						qStableSort(it1, it2, compare);	///排序
						for (it3 = it1; it3 != it2; it3++)
						{
							afIndexMap[*it3] = it3 - itemVec.begin();	///保存排序之后的下标
						}
						it1 = it2;
					}
					QModelIndexList fromList, toList;
					std::map<TreeItem*, int>::iterator itm, itme;
					itme = bfIndexMap.end();
					for (itm = bfIndexMap.begin(); itm != itme; itm++)	///map 保证了遍历的有序性
					{
						toList.append(createIndex(itm->second, 0, itm->first));
					}
					itme = afIndexMap.end();
					for (itm = afIndexMap.begin(); itm != itme; itm++)
					{
						toList.append(createIndex(itm->second, 0, itm->first));
					}
					changePersistentIndexList(fromList, toList);	////
				}
				else ///单列排序
				{
					QVector<TreeItem*>::iterator it = itemVec.begin(), ite = itemVec.end();
					int index = 0;
					for (; it != ite; it++, index++)
					{
						bfIndexMap[*it] = index;
					}
					qStableSort(itemVec.begin(), itemVec.end(), compare);	///对Qvector做全局排序
					for (it = itemVec.begin(), index = 0; it != ite; it++, index++)
					{
						afIndexMap[*it] = index;
					}
					QModelIndexList fromList, toList;
					std::map<TreeItem*, int>::iterator itm, itme;
					itme = bfIndexMap.end();
					for (itm = bfIndexMap.begin(); itm != itme; itm++)	///map 保证了遍历的有序性
					{
						fromList.append(createIndex(itm->second, 0, itm->first));
					}
					itme = afIndexMap.end();
					for (itm = afIndexMap.begin(); itm != itme; itm++)
					{
						toList.append(createIndex(itm->second, 0, itm->first));
					}
					changePersistentIndexList(fromList, toList);	////
				}
			}
		}
	}
	else	///目录节点排序
	{
		compare = std::bind(order == Qt::AscendingOrder ? &TreeModel::directItemLessThan : &TreeModel::directItemGreaterThan, this, std::placeholders::_1, std::placeholders::_2);
		while (!itemQueue.empty())
		{
			pitem = itemQueue.front();
			itemQueue.pop();
			if (pitem->itemLevel < level)
			{
				for (int i = 0; i < pitem->childCount(); i++)
				{
					itemQueue.push(pitem->child(i));
				}
			}
			else if (pitem->itemLevel == level)///对子节点序列(即要排序的目录节点序列)排序
			{
				QVector<TreeItem*>& itemVec = pitem->childItems;
				std::map<TreeItem*, int> bfIndexMap, afIndexMap;
				QVector<TreeItem*>::iterator it = itemVec.begin(), ite = itemVec.end();
				int i = 0;
				for (; it != ite; it++, i++)
				{
					bfIndexMap[*it] = i;
				}
				qStableSort(itemVec.begin(), itemVec.end(), compare);
				for (it = itemVec.begin(), i = 0; it != ite; it++, i++)
				{
					afIndexMap[*it] = i;
				}
				QModelIndexList fromList, toList;
				std::map<TreeItem*, int>::iterator itm, itme;
				itme = bfIndexMap.end();
				for (itm = bfIndexMap.begin(); itm != itme; itm++)	///map 保证了遍历的有序性
				{
					fromList.append(createIndex(itm->second, 0, itm->first));
				}
				itme = afIndexMap.end();
				for (itm = afIndexMap.begin(); itm != itme; itm++)
				{
					toList.append(createIndex(itm->second, 0, itm->first));
				}
				changePersistentIndexList(fromList, toList);	////
			}
			else
			{
				///nothing todo 
			}
		}
	}
}
bool TreeModel::itemLessThan(TreeItem* left, TreeItem* right)
{
	int sortCol = sortColumn < 0 ? 0 : sortColumn;
	return TreeItem::isVariantLessThen(left->data(sortCol), right->data(sortCol));
}
bool TreeModel::itemGreaterThan(TreeItem* left, TreeItem* right)
{
	int sortCol = sortColumn < 0 ? 0 : sortColumn;
	return TreeItem::isVariantLessThen(right->data(sortCol), left->data(sortCol));
}
bool TreeModel::directItemLessThan(TreeItem* left, TreeItem* right)
{
	return TreeItem::isVariantLessThen(left->data(0), right->data(0));

}
bool TreeModel::directItemGreaterThan(TreeItem* left, TreeItem* right)
{
	return TreeItem::isVariantLessThen(right->data(0), left->data(0));
}
bool TreeModel::multiColEqual(TreeItem* left, TreeItem* right, int cmpSize)
{
	for (int i = 0; i < cmpSize; i++)
	{
		if (!TreeItem::isVariantEqualTo(left->data(orderColumns[i].first), right->data(orderColumns[i].first)))
			return false;
	}
	return true;
}
QModelIndex TreeModel::findRecordByColumn(TreeItem* parent, const QVariant& var, int column)
{
	QModelIndex index;
	if (parent)
	{
		TreeItem* item;
		for (int i = 0; i < parent->childCount(); i++)
		{
			item = parent->child(i);
			if (item->isVirtualItem())///目录节点不在查找范围内，直接跳过
				index = findRecordByColumn(item, var, column);
			else if (item->data(column) == var)
				index = createIndex(i, column, item);
			else
				index = findRecordByColumn(item, var, column);
			if (index.isValid())
				break;
		}
	}
	return index;
}
QModelIndex TreeModel::findRecordNoColumn(TreeItem* parent, const QVariant& var)
{
	QModelIndex index;
	if (parent)
	{
		TreeItem* item;
		for (int i = 0; i < parent->childCount(); i++)
		{
			item = parent->child(i);
			if (item->isVirtualItem())
				index = findRecordNoColumn(item, var);
			else
			{
				for (int j = 0; j < columnCount(); j++)
				{
					if (item->data(j) == var)
					{
						index = createIndex(i, j, item);
						break;
					}
				}
			}
			if (index.isValid())
				break;
		}
	}
	return index;
}
void TreeModel::appendItem(TreeItem* item)
{
	itemVector.append(item);
}

void TreeModel::appendItemSync(TreeItem* item)
{
	itemVector.append(item);
	rootItem->appendChild(item);
}

void TreeModel::truncateItems(int count)
{
	while (--count >= 0 && itemVector.size() > 0)
	{
		itemToDelVector << itemVector.takeLast();
	}
}
void TreeModel::removeAllItems()
{
	//itemToDelVector << itemVector;
	//itemVector.clear();
	itemVector.swap(itemToDelVector);
}
void TreeModel::removeItemAt(int row)
{
	itemToDelVector << itemVector.takeAt(row);
}
void TreeModel::removeItemSyncAt(int row)
{
	itemToDelVector << itemVector.takeAt(row);
	rootItem->setChildren(itemVector);
}
///根据现有的排序、分组更新索引
void TreeModel::updateIndex()
{
	beginResetModel();
	TreeItem* pitem;
	std::queue<TreeItem*> itemQueue;
	itemQueue.push(rootItem);
	while (!itemQueue.empty())
	{
		pitem = itemQueue.front();
		itemQueue.pop();
		if (pitem->childCount() > 0 && pitem->child(0)->isVirtualItem())	///子节点为目录节点则入列
		{
			QVector<TreeItem*> sunList = pitem->takeChildAll();
			for (int i = 0; i < sunList.size(); i++)
			{
				itemQueue.push(sunList.at(i));	///
			}
		}
		else
		{
			pitem->detachChildren();	///字节点为非目录节点，直接解绑父子关系
		}
		if (pitem != rootItem)
		{
			delete pitem;			///删除目录节点
		}
	}
	rootItem->setChildren(itemVector);///写时赋值&批量赋值
	//rootItem->appendChildList(itemVector);////慢。。。
	doGroupByColumns();
	doSortByColumns();
	endResetModel();
	qDeleteAll(itemToDelVector);
	itemToDelVector.clear();
	if (oldModelRecord)
	{
		delete oldModelRecord;
		oldModelRecord = nullptr;
	}
}

////部分显示属性设置
///列属性设置
void TreeModel::setTextAlignment(int column, Rcspace::RcTextAlignment aenum)
{
	if (column < 0 || column >= attrOfColumns.size())
	{
		return;
	}
	attrOfColumns[column][Qt::TextAlignmentRole] = aenum;
}
void TreeModel::setBackground(int column, unsigned long colorVal)
{
	if (column < 0 || column >= attrOfColumns.size())
	{
		return;
	}
	attrOfColumns[column][Qt::BackgroundRole] = (qulonglong)colorVal;
}
void TreeModel::setForeground(int column, unsigned long colorVal)
{
	if (column < 0 || column >= attrOfColumns.size())
	{
		return;
	}
	attrOfColumns[column][Qt::ForegroundRole] = (qulonglong)colorVal;
}
void TreeModel::setIcon(int column, Rcspace::RcIconEnum ienum)
{
	if (column < 0 || column >= attrOfColumns.size())
	{
		return;
	}
	attrOfColumns[column][Qt::DecorationRole] = ienum;
}
///全局属性设置
void TreeModel::setFont(QFont font)
{
	attrOfModel[Qt::FontRole] = font;
}
void TreeModel::setTextAlignment(Rcspace::RcTextAlignment aenum)
{
	attrOfModel[Qt::TextAlignmentRole] = aenum;
}
void TreeModel::setBackground(unsigned long colorVal)
{
	attrOfModel[Qt::BackgroundRole] = (qulonglong)colorVal;
}
void TreeModel::setForeground(unsigned long colorVal)
{
	attrOfModel[Qt::ForegroundRole] = (qulonglong)colorVal;
}
void TreeModel::setIcon(Rcspace::RcIconEnum ienum)
{
	attrOfModel[Qt::DecorationRole] = ienum;
}
void TreeModel::setEditable(bool edit)
{
	editable = edit;
}
bool TreeModel::isEditable()
{
	return editable;
}
void TreeModel::setDragAndDropAble(bool able)
{
	dragndropable = able;
}
bool TreeModel::isDragAndDropAble()
{
	return dragndropable;
}
Qt::DropActions TreeModel::supportedDragActions() const
{
	return Qt::MoveAction;
}
Qt::DropActions	TreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

static const char sTreeItemMimeType[] = "application/x-treeitem";
QStringList TreeModel::mimeTypes() const
{
	return QStringList() << sTreeItemMimeType;
}
QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
	QMimeData* mimeData = new QMimeData;
	QByteArray data; //a kind of RAW format for datas

	//QDataStream is independant on the OS or proc architecture
	//serialization of C++'s basic data types, like char, short, int, char *, etc.
	//Serialization of more complex data is accomplished
	//by breaking up the data into primitive units.
	QDataStream stream(&data, QIODevice::WriteOnly);
	QList<TreeItem*> nodes;

	//
	foreach(const QModelIndex & index, indexes) {
		TreeItem* node = getItem(index);
		if (!nodes.contains(node))
			nodes << node;
	}
	stream << QCoreApplication::applicationPid();
	stream << nodes.count();
	foreach(TreeItem * node, nodes) {
		stream << reinterpret_cast<qlonglong>(node);
	}
	mimeData->setData(sTreeItemMimeType, data);
	return mimeData;
}
bool TreeModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	Q_ASSERT(action == Qt::MoveAction);
	Q_UNUSED(column);
	//test if the data type is the good one
	if (!mimeData->hasFormat(sTreeItemMimeType)) {
		return false;
	}
	QByteArray data = mimeData->data(sTreeItemMimeType);
	QDataStream stream(&data, QIODevice::ReadOnly);
	qint64 senderPid;
	stream >> senderPid;
	if (senderPid != QCoreApplication::applicationPid()) {
		// Let's not cast pointers that come from another process...
		return false;
	}
	TreeItem* parentNode = getItem(parent);
	Q_ASSERT(parentNode);
	int count;
	stream >> count;
	if (row == -1) {	///禁止drop为新的子节点
		return false;
	}
	for (int i = 0; i < count; ++i) {
		// Decode data from the QMimeData
		qlonglong nodePtr;
		stream >> nodePtr;
		TreeItem* node = reinterpret_cast<TreeItem*>(nodePtr);
		if (parentNode != node->parent())	///不允许跨目录drop
		{
			return false;
		}
		if (node->childNumber() < row)	///后续操作会先删除旧节点，目标位置>node当前位置时，row--
			--row;
		// Remove from old position
		removeItem(node);
		beginInsertRows(parent, row, row);
		parentNode->insertChildAt(row, node);
		endInsertRows();
		++row;
	}
	return true;
}
void TreeModel::setCheckable(bool check)
{
	checkable = check;
}
bool TreeModel::isCheckable()
{
	return checkable;
}
void TreeModel::setItemIsSelectable(bool selectable)
{
	itemIsSelectable = selectable;
}
bool TreeModel::isItemIsSelectable()
{
	return itemIsSelectable;
}
int TreeModel::getItemIndex(TreeItem* pitem)
{
	return itemVector.indexOf(pitem);
}
QModelIndex TreeModel::index(int row, int column, TreeItem* item)
{
	if (item == nullptr || item == rootItem)
	{
		return QModelIndex();
	}
	return createIndex(row, column, item);
}
void TreeModel::setColumnEditable(int col, bool edit)
{
	if (edit && editableColVec.indexOf(col) < 0)
	{
		editableColVec.append(col);
	}
	else if (!edit && editableColVec.indexOf(col) > -1)
	{
		editableColVec.remove(editableColVec.indexOf(col));
	}
}
void TreeModel::setAllColumnEditable(bool edit)
{
	editableColVec.clear();
	if (edit)
	{
		for (int i = 0; i < columnCount(); i++)
		{
			editableColVec.append(i);
		}
	}
}
bool TreeModel::getColumnEditable(int col)
{
	if (editableColVec.indexOf(col) > -1)
	{
		return true;
	}
	return false;
}
void TreeModel::switchItems(int posLeft, int posRight, TreeItem* parent)
{
	if (!parent || parent == rootItem)
	{
		TreeItem* tmp;
		tmp = itemVector.at(posLeft);
		itemVector[posLeft] = itemVector.at(posRight);
		itemVector[posRight] = tmp;
	}
	else
	{
		parent->switchChild(posLeft, posRight);
	}
}

void TreeModel::setAttributeGlobal(QVariant var, int role)
{
	attrOfModel[role] = var;
}

void TreeModel::setAttributeColumn(QVariant var, int column, int role)
{
	attrOfColumns[column][role] = var;
}

void TreeModel::removeAttributeColumn(int column, int role)
{
	auto it = attrOfColumns[column].find(role);
	if (it != attrOfColumns[column].end())
	{
		attrOfColumns[column].erase(it);
	}
}

void TreeModel::setHeaderView(QHeaderView* h)
{
	head = h;
}

QHeaderView* TreeModel::getHeaderView()
{
	return head;
}
bool TreeModel::reloadData()
{
	beginResetModel();
	TreeItem* pitem;
	std::queue<TreeItem*> itemQueue;
	itemQueue.push(rootItem);
	while (!itemQueue.empty())
	{
		pitem = itemQueue.front();
		itemQueue.pop();
		if (pitem->childCount() > 0 && pitem->child(0)->isVirtualItem())	///子节点为目录节点则入列
		{
			QVector<TreeItem*> sunList = pitem->takeChildAll();
			for (int i = 0; i < sunList.size(); i++)
			{
				itemQueue.push(sunList.at(i));	///
			}
		}
		else
		{
			pitem->detachChildren();	///字节点为非目录节点，直接解绑父子关系
		}
		if (pitem != rootItem)
		{
			delete pitem;			///删除目录节点
		}
	}
	rootItem->setChildren(itemVector);
	//rootItem->appendChildList(itemVector);///慢。。。
	endResetModel();
	qDeleteAll(itemToDelVector);
	itemToDelVector.clear();
	return true;
}

void TreeModel::reserveItemSpace(int capacity)
{
	if (capacity > itemVector.capacity())
	{
		itemVector.reserve(capacity);
	}
}

void TreeModel::setModelRecord(ModelRecord* record)
{
	if (record && record != modelRecord)
	{
		if (oldModelRecord)
		{
			delete oldModelRecord;
		}
		oldModelRecord = modelRecord;
		modelRecord = record;
		int nSize = modelRecord->recordSize(), itemSize = itemVector.size();
		reserveItemSpace(nSize);
		int i = 0, nMin = min(nSize, itemSize);
		for (; i < nMin; i++)
		{
			itemVector[i]->setRecordData(modelRecord->recordAt(i));
		}
		while (i < nSize)
		{
			itemVector.push_back(new TreeItem(rootItem, modelRecord->recordAt(i)));
			i++;
		}
		if (i < itemSize)
		{
			truncateItems(itemSize - i);
		}
	}
}

void TreeModel::setModelRecord(ModelRecord* record, NodeType<int>* treeNode)
{
	if (record && record != modelRecord)
	{
		if (oldModelRecord)
		{
			delete oldModelRecord;
		}
		oldModelRecord = modelRecord;
		modelRecord = record;
		int nSize = treeNode->childNodes.size(), itemSize = itemVector.size();
		reserveItemSpace(nSize);
		int i = 0, nMin = min(nSize, itemSize);

		for (; i < nMin; i++)
		{
			buildItemWithModel(itemVector[i], treeNode->childNodes[i]);
		}
		while (i < nSize)
		{
			itemVector.push_back(buildItemWithModel(new TreeItem(rootItem), treeNode->childNodes[i]));
			i++;
		}
		if (i < itemSize)
		{
			truncateItems(itemSize - i);
		}
	}
}

TreeItem* TreeModel::buildItemWithModel(TreeItem* pitem, NodeType<int>* node)
{
	pitem->setRecordData(modelRecord->recordAt(node->value));
	int i = 0;
	for (; i < node->childNodes.size(); i++)
	{
		if (i < pitem->childCount())
		{
			buildItemWithModel(pitem->child(i), node->childNodes[i]);
		}
		else
		{
			pitem->appendChild(buildItemWithModel(new TreeItem(pitem), node->childNodes[i]));
		}

	}
	if (i < pitem->childCount())
	{
		pitem->removeChildren(i, pitem->childCount() - i);
	}
	return pitem;
}
