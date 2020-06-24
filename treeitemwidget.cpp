#include "treeitemwidget.h"
#include "qheaderview.h"
#include <vector>

TreeItemWidget::TreeItemWidget(QWidget* parent)
	: QWidget(parent)
{
	dataModel = nullptr; //dataModel 不指定
	totalModel = nullptr;
	setupDispUi(); // view对象初始化
}

TreeItemWidget::~TreeItemWidget()
{
	///headView,dataView,vboxLayout不做显示删除，窗口销毁时自动销毁
	///显示delete model
	if (dataModel)
	{
		disconnect(dataModel, &TreeModel::dataAltered, this, &TreeItemWidget::onModelDataAltered);
		delete dataModel;
	}
	if (totalModel && totalModel != dataModel)	//防止指针共用
	{
		delete totalModel;
	}
	//delete menu
	if (headMenu)
	{
		delete headMenu;
	}
	if (dataMenu && dataMenu != headMenu)	//防止指针共用
	{
		delete dataMenu;
	}
}

void TreeItemWidget::hideSections(vector<int>& hidenLogicalIndexes)
{
	std::vector<int> ::iterator itr = hidenLogicalIndexes.begin();
	QHeaderView* headView = dataView->header();
	HeaderView* castHead = dynamic_cast<HeaderView*>(headView);
	if (castHead)
	{
		for (; itr != hidenLogicalIndexes.end(); itr++)
			castHead->hideSectionAndEmitSignal(*itr);
	}
	else
	{
		for (; itr != hidenLogicalIndexes.end(); itr++)
			headView->hideSection(*itr);
	}
}

/// 根据配置调整列的顺序
void TreeItemWidget::adjSectionViewShow(map<int, pair<int, bool>>& visualLogicalIndexPair)	///根据配置设置列数据的显示 map<visualIndex, <logicalIndex, visible>>
{
	///visualLogicalIndexPair根据visualIndex升序排序，且包含了所有的字段
	map<int, pair<int, bool>>::iterator it = visualLogicalIndexPair.begin(), ite = visualLogicalIndexPair.end();
	for (; it != ite; it++)
	{
		headView->moveSection(headView->visualIndex(it->second.first), it->first);
		if (it->second.second)
		{
			headView->showSectionAndEmitSignal(it->second.first);
		}
		else
		{
			headView->hideSectionAndEmitSignal(it->second.first);
		}
	}
}

void TreeItemWidget::addGroupColumn(int column)
{
	//TreeModel 做好了beginResetModel()/endResetModel()
	dataModel->addGroupColumn(column);
}

void TreeItemWidget::delGroupColumn(int column)
{
	///TreeModel 做好了beginResetModel()/endResetModel()
	dataModel->delGroupColumn(column);
}

void TreeItemWidget::setupDispUi(void)
{
	vboxLayout = new QVBoxLayout();
	vboxLayout->setSpacing(0);
	vboxLayout->setContentsMargins(0, 0, 0, 0);
	dataView = new TreeView(this);
	vboxLayout->addWidget(dataView, 1);
	/// dataView 
	dataView->setSelectionMode(QAbstractItemView::ExtendedSelection); // 允许多项目选择
	dataView->setAlternatingRowColors(true);
	dataView->setSelectionBehavior(QAbstractItemView::SelectRows); //默认选择为行
	dataView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	dataView->setAnimated(false);
	dataView->setAllColumnsShowFocus(true);
	dataView->setAlternatingRowColors(false);
	dataView->footTreeView()->setRootIsDecorated(false);///设置top-level节点折叠icon为不显示，一般汇总行不会存在树形结构
	//dataView->setTreePosition(-1);
	//dataView->setTreePosition(headView->firstVisiableLogicalIndex());
	///headView
	headView = new HeaderView(Qt::Horizontal, this);
	///使用自定义排序响应逻辑需绑定sectionSingleClicked信号，如果不需要自定义排序响应逻辑则取消绑定
	connect(headView, &HeaderView::sectionSingleClicked, this, &TreeItemWidget::onHeadViewSectionSingleClicked);
	////信号转发
	connect(headView, &HeaderView::hideSectionSignal, this, &TreeItemWidget::onHeadSectionHide);
	connect(headView, &HeaderView::showSectionSignal, this, &TreeItemWidget::onHeadSectionShow);

	dataView->setHeaderAndReconnectSignals(headView);
	dataView->setSortingEnabled(false);//默认值就是false，可以不用设置，这里只是为了强调将其设置为false，排序功能不用Qt自带的逻辑
	headView->setFirstSectionMovable(true);
	this->setLayout(vboxLayout);

	//headView->setSectionResizeMode(QHeaderView::Interactive);
	//conncet signals & slots
	connect(dataView, &QAbstractItemView::doubleClicked, this, &TreeItemWidget::onRowDoubleClicked);
	connect(dataView, &QTreeView::expanded, this, &TreeItemWidget::onItemExpanded);
	connect(dataView, &QTreeView::collapsed, this, &TreeItemWidget::onItemCollapsed);
	connect(dataView, &QAbstractItemView::clicked, this, &TreeItemWidget::onItemClicked);
	connect(dataView, &TreeView::mousePressed, this, &TreeItemWidget::onDataViewMousePressed);
	connect(headView, &HeaderView::sectionRightClicked, this, &TreeItemWidget::onHeaderSectionRightClicked);
	///menu
	headMenu = nullptr;
	dataMenu = nullptr;
}

void TreeItemWidget::setupDataModel(TreeModel* dModel)
{
	if (dModel == this->dataModel)
	{
		return;
	}
	if (dataModel)
	{
		delete dataModel;
	}
	dataModel = dModel;
	dataView->setModel(dataModel); // setModel不会delete原有的model
	dataModel->setHeaderView(headView);
	connect(dataView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeItemWidget::onSelectionChanged);
	connect(dataModel, &TreeModel::dataAltered, this, &TreeItemWidget::onModelDataAltered);
	syncAttrToModel();
}

void TreeItemWidget::setupTotalModel(TreeModel* tModel)
{
	if (tModel == this->totalModel)
	{
		return;
	}
	if (totalModel)
	{
		delete totalModel;
	}
	totalModel = tModel;
	totalModel->setHeaderView(headView);
	dataView->setFootViewModel(tModel);
}

TreeModel* TreeItemWidget::getDataModel()
{
	return dataModel;
}

TreeModel* TreeItemWidget::getTotalModel()
{
	return totalModel;
}
QVector<QByteArray> TreeItemWidget::getColumnNames()
{
	QVector<QByteArray> namesArr;
	if (dataModel)
	{
		TreeItem* ritem = dataModel->returnRootItem();
		if (ritem)
		{
			for (int i = 0; i < ritem->columnCount(); i++)
			{
				namesArr << BYTEARRAY(ritem->data(i).toString());
			}
		}
	}
	return namesArr;
}
///根据某个给定值找到第一个包含该给定值的单元格
QModelIndex TreeItemWidget::findRecord(const QVariant& var)
{
	return dataModel->findRecordMatchVal(var);
}
///重载，限定了匹配值的列
QModelIndex TreeItemWidget::findRecord(const QVariant& var, int column)
{
	return dataModel->findRecordMatchValByColumn(var, column);
}
///选中某行并将滚动条拖动直至该行可见
void TreeItemWidget::setFocusOnIndex(const QModelIndex& idx)
{
	if (!idx.isValid())
	{
		return;
	}
	////默认情况下setCurrentIndex也会选中该index
	dataView->setFocus();
	dataView->scrollTo(idx, QAbstractItemView::EnsureVisible);
	dataView->setCurrentIndex(idx);
}
void TreeItemWidget::ensureVisible(const QModelIndex& idx)
{
	dataView->scrollTo(idx, QAbstractItemView::EnsureVisible);
}
//void TreeItemWidget::openHeaderViewCustomContextMenu()
//{
//	headView->setContextMenuPolicy(Qt::CustomContextMenu);
//	connect(headView, &HeaderView::customContextMenuRequested, this, &TreeItemWidget::onHeaderContextMenu);
//}
void TreeItemWidget::openDataViewCustomContextMenu()
{
	dataView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(dataView, &TreeView::customContextMenuRequested, this, &TreeItemWidget::onDataRowContextMenu);
}
TreeItem* TreeItemWidget::currentItem()
{
	QModelIndex index = dataView->currentIndex();
	if (index.isValid())
	{
		return static_cast<TreeItem*>(index.internalPointer());
	}
	return nullptr;
}
///更新数据展示界面
void TreeItemWidget::updateDisplay()
{
	dataModel->updateIndex();
	if (shouldRootDecorate)
	{
		if (!dataView->rootIsDecorated())
		{
			dataView->setRootIsDecorated(true);
		}
	}
	else
	{
		if (dataModel->getGroupColumns().size())
		{
			if (!dataView->rootIsDecorated())
			{
				dataView->setRootIsDecorated(true);
			}
		}
		else
		{
			if (dataView->rootIsDecorated())
			{
				dataView->setRootIsDecorated(false);
			}
		}
	}
	dataView->updateGeometry();
}
//更新汇总数据展示区
void TreeItemWidget::updateTotalDisplay()
{
	if (totalModel)
	{
		totalModel->updateIndex();
		//dataView->setFootViewGeometry();
	}
}
void TreeItemWidget::expandAll()
{
	return dataView->expandAll();
}
void TreeItemWidget::expand(TreeItem* item)
{
	QModelIndex index = dataModel->index(item->childNumber(), 0, item);
	return dataView->expand(index);
}
void TreeItemWidget::expandRecursively(TreeItem* item, int depth)
{
	QModelIndex index = dataModel->index(item->childNumber(), 0, item);
	dataView->expand(index);
	if (depth != 0)
	{
		for (int i = 0; i < item->childCount(); i++)
		{
			expandRecursively(item->child(i), depth - 1);
		}
	}
}
void TreeItemWidget::collapseAll()
{
	return dataView->collapseAll();
}
void TreeItemWidget::collapse(TreeItem* item)
{
	QModelIndex index = dataModel->index(item->childNumber(), 0, item);
	return dataView->collapse(index);
}
bool TreeItemWidget::isExpanded(TreeItem* item)
{
	QModelIndex index = dataModel->index(item->childNumber(), 0, item);
	return dataView->isExpanded(index);
}
QList<TreeItem*> TreeItemWidget::selectedRows()
{
	QModelIndexList indexes = dataView->selectionModel()->selectedRows();
	QList<TreeItem*> list;
	for (int i = 0; i < indexes.size(); i++)
	{
		list.append((TreeItem*)indexes.at(i).internalPointer());
	}
	return list;
}
QModelIndexList TreeItemWidget::selectedIndexes()
{
	return  dataView->selectionModel()->selectedRows();
}
void TreeItemWidget::selectItems(const QVector<TreeItem*>& items)
{
	if (items.size() == 0)return;
	dataView->clearSelection();
	QItemSelectionModel* selModel = dataView->selectionModel();
	for (int i = 0; i < items.size(); i++)
	{
		selModel->select(dataModel->index(items.at(i)->childNumber(), 0, items.at(i)), QItemSelectionModel::SelectionFlag::Select);
	}
}
void TreeItemWidget::selectItems(const QVector<TreeItem*>& items, const QVector<int>& rows)
{
	if (items.size() == 0)return;
	if (items.size() != rows.size())return;
	dataView->clearSelection();
	QItemSelectionModel* selModel = dataView->selectionModel();
	for (int i = 0; i < items.size(); i++)
	{
		selModel->select(dataModel->index(rows.at(i), 0, items.at(i)), QItemSelectionModel::SelectionFlag::Select);
	}
}
void TreeItemWidget::setGroupable(bool group)
{
	groupAbled = group;
}
bool TreeItemWidget::isGroupable()
{
	return groupAbled;
}
void TreeItemWidget::setDragAndDropAble(bool able)
{
	dragndropable = able;
	if (dataModel && dataView)
	{
		dataModel->setDragAndDropAble(able);
		dataView->setDragEnabled(able);
		dataView->setAcceptDrops(able);
		dataView->setDropIndicatorShown(able);
	}
}
bool TreeItemWidget::isDragAndDropAble()
{
	return dragndropable;
}
void TreeItemWidget::setCheckable(bool check)
{
	checkable = check;
	if (dataModel)
	{
		dataModel->setCheckable(check);
	}
}
bool TreeItemWidget::isCheckable()
{
	return checkable;
}
void TreeItemWidget::setEditable(bool edit)
{
	editable = edit;
	if (dataModel)
	{
		dataModel->setEditable(edit);
	}
}
bool TreeItemWidget::isEditable()
{
	return editable;
}
void TreeItemWidget::setSortable(bool sort)
{
	sortable = sort;
	if (dataModel)
	{
		dataModel->setSortAbled(sort);
	}
	//if (dataView)
	//{
	//	dataView->setSortingEnabled(sort);
	//}
}
bool TreeItemWidget::isSortable()
{
	return sortable;
}
void TreeItemWidget::setShouldRootDecorate(bool should)
{
	shouldRootDecorate = should;
}
void TreeItemWidget::syncAttrToModel()	////用于重设datamodel时同步属性至新的datamodel
{
	if (dataModel)
	{
		dataModel->setCheckable(checkable);		////checkable
		if (dataView)
		{
			//dataView->setSortingEnabled(sortable);	////sortable
			dataModel->setDragAndDropAble(dragndropable);	///drag & dropable
			dataView->setDragEnabled(dragndropable);
			dataView->setAcceptDrops(dragndropable);
			dataView->setDropIndicatorShown(dragndropable);
		}
		dataModel->setSortAbled(sortable);		///sortable
		dataModel->setEditable(editable);		///editable
	}
}
void TreeItemWidget::onModelDataAltered(TreeItem* pitem, int column, const QVector<int>& roles)
{
	emit itemDataChanged(pitem, column, roles);
}
void TreeItemWidget::onItemExpanded(const QModelIndex& index)
{
	emit expandedOrCollapsed(index, 1);
}
void TreeItemWidget::onItemCollapsed(const QModelIndex& index)
{
	emit expandedOrCollapsed(index, 0);
}
void TreeItemWidget::onItemClicked(const QModelIndex& index)
{
	emit itemClicked(index);
}
void TreeItemWidget::onDataViewMousePressed()
{
	emit mousePressed();
}
void TreeItemWidget::onHeadViewSectionSingleClicked(int logicalIndex)
{
	if (!sortable)///禁止排序，直接返回
	{
		return;
	}
	if (dataModel)
	{
		MultiColumnOrder sorts = dataModel->sortMultiColumns();
		Qt::SortOrder order = Qt::SortOrder::AscendingOrder;
		for (auto it : sorts)
		{
			if (it.first == logicalIndex)///之前已经存在对该列的排序，翻转排序顺序
			{
				order = it.second == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
				break;
			}
		}
		dataModel->sort(logicalIndex, order);
	}
}
void TreeItemWidget::onHeadSectionShow(int logicalIndex)
{
	emit sectionShowed(logicalIndex, true);
}
void TreeItemWidget::onHeadSectionHide(int logicalIndex)
{
	emit sectionShowed(logicalIndex, false);
}
///deprecated,don't use this, use right clicked slot
void TreeItemWidget::onHeaderContextMenu(const QPoint& pos)
{
}
void TreeItemWidget::onDataRowContextMenu(const QPoint& pos)
{
	//测试使用
	QModelIndexList indexes = dataView->selectionModel()->selectedRows();
	TreeItem* tmp;
	QByteArray str;
	for (int i = 0; i < indexes.size(); i++)
	{
		//tmp = (TreeItem*)indexes.at(i).internalPointer();
		str = BYTEARRAY(dataModel->data(indexes.at(i), Qt::DisplayRole).toString());
		if (indexes.at(i).child(0, 0).isValid())
		{
			int k = 0;
		}
	}
	int j = 0;
}
void TreeItemWidget::onActionOfHeaderSection(bool checked)
{
	QAction* action = qobject_cast<QAction*>(sender());
	QString secname = action->text();
	TreeItem* rItem = dataModel->returnRootItem();
	int i = 0;
	while (i < rItem->columnCount() && secname != rItem->data(i).toString())
	{
		++i;
	}
	if (i < rItem->columnCount())
	{
		headView->setSectionHidden(i, checked ? false : true);
		dataView->footTreeView()->header()->setSectionHidden(i, checked ? false : true);
		//dataView->setTreePosition(headView->logicalIndexAt(0));
		dataView->setTreePosition(headView->firstVisiableLogicalIndex());
		emit sectionShowed(i, checked);
	}
}
void TreeItemWidget::onRowDoubleClicked(const QModelIndex& index)
{
	emit(rowDoubleClicked(index));
}
void TreeItemWidget::onHeaderGroup(bool checked)
{
	if (checked)
	{
		dataModel->addGroupColumn(rightClickSection);
		if (!dataView->rootIsDecorated())
		{
			dataView->setRootIsDecorated(true);		///增加列分组必须要设置为true
		}
	}
	else
	{
		dataModel->delGroupColumn(rightClickSection);
		if (dataModel->getGroupColumns().size() == 0 && !shouldRootDecorate && dataView->rootIsDecorated())
		{
			dataView->setRootIsDecorated(false);	///根据分组列个数和shouldRootDecorate属性共同决定
		}
	}
	dataView->expandAll();
}
void TreeItemWidget::onHeaderSectionRightClicked(int logicalIndex)
{
	rightClickSection = logicalIndex;	///记录每次鼠标右键的列位置
	if (headMenu == nullptr)headMenu = new QMenu(this);
	headMenu->clear();	///删除之前创建的action，action没有被其他窗口使用时会自动delete
	TreeItem* rItem = dataModel->returnRootItem();
	QAction* action;
	action = headMenu->addAction(QIcon(), STRING("自动设置列宽"));
	connect(action, &QAction::triggered, this, &TreeItemWidget::onAutoSetHeaderSectionWidth);
	action = headMenu->addAction(QIcon(), STRING("设置最佳列宽"));
	connect(action, &QAction::triggered, this, &TreeItemWidget::onOptimalSetHeaderSectionWidth);
	if (dataModel)
	{
		MultiColumnOrder sortcols = dataModel->sortMultiColumns();
		QVector<int> groupCols = dataModel->getGroupColumns();
		for (auto it : sortcols)
		{
			if (it.first == logicalIndex)///普通排序，非分组排序
			{
				if (groupCols.indexOf(logicalIndex) < 0)
				{
					action = headMenu->addAction(QIcon(), STRING("取消排序"));
					connect(action, &QAction::triggered, this, &TreeItemWidget::onActionCancelSort);
				}
				break;
			}
		}
		if (groupAbled)
		{
			action = headMenu->addAction(QIcon(), STRING("按组排序"));
			connect(action, &QAction::triggered, this, &TreeItemWidget::onHeaderGroup);
			action->setCheckable(true);
			if (groupCols.indexOf(logicalIndex) > -1)
			{
				action->setChecked(true);
			}
			else
			{
				action->setChecked(false);
			}
		}
	}

	headMenu->addSeparator();
	map<int, int> visualLogicalmap;
	for (int i = 0; i < rItem->columnCount(); i++)
	{
		visualLogicalmap[headView->visualIndex(i)] = i;
	}
	for (auto it = visualLogicalmap.begin(); it != visualLogicalmap.end(); it++)
	{
		action = headMenu->addAction(QIcon(), rItem->data(it->second).toString());
		connect(action, &QAction::triggered, this, &TreeItemWidget::onActionOfHeaderSection);
		action->setCheckable(true);
		if (!headView->isSectionHidden(it->second))
		{
			action->setChecked(true);
		}
		else
		{
			action->setChecked(false);
		}
	}

	headMenu->exec(QCursor::pos());
}
void TreeItemWidget::onAutoSetHeaderSectionWidth(bool checked)
{
	headView->resizeSections(QHeaderView::Stretch);
}
void TreeItemWidget::onOptimalSetHeaderSectionWidth(bool checked)
{
	///汇总区域自适应
	QVector<int> nTotalSize;
	if (totalModel && dataView->footTreeView()->isVisible())
	{
		TreeViewAdapt* footView = dataView->footTreeView();
		for (int i = 0; i < dataModel->columnCount(); i++)
		{
			nTotalSize.append(footView->sizeHintForColumn(i));
		}
	}
	headView->resizeSections(QHeaderView::ResizeToContents);
	if (nTotalSize.size() > 0)
	{
		for (int i = 0; i < nTotalSize.size(); i++)
		{
			if (headView->isSectionHidden(i))
				continue;
			if (headView->sectionSize(i) < nTotalSize.at(i))
			{
				headView->resizeSection(i, nTotalSize.at(i));
			}
		}
	}
}
///dataMenu对应action的响应
void TreeItemWidget::onDataMenuAction(bool checked)
{
}
void TreeItemWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	emit selectionChanged(selected, deselected);
}

void TreeItemWidget::onActionCancelSort()
{
	MultiColumnOrder sortcols = dataModel->sortMultiColumns();
	for (int i = 0; i < sortcols.size(); i++)
	{
		if (sortcols[i].first == rightClickSection)
		{
			sortcols.removeAt(i);
			break;
		}
	}
	dataModel->reloadData();
	dataModel->doGroupByColumns();
	dataModel->doSortByColumns(sortcols);
}
