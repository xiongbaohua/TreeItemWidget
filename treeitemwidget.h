#pragma once

#include "treemodel.h"
#include <qtreeview.h>
#include "stdafx.h"
#include "headerview.h"
#include <qboxlayout.h>
#include "treeview.h"
#include <qabstractitemmodel.h>
#include <qmenu.h>

using namespace std;

class TreeItemWidget : public QWidget
{
	Q_OBJECT

public:
	TreeItemWidget(QWidget* parent = nullptr);
	virtual ~TreeItemWidget();
	void hideSections(std::vector<int>& hidenLogicalIndexes); /// 设置隐藏列
	void adjSectionViewShow(map<int, pair<int, bool>>& visualLogicalIndexPair);	///根据配置设置列数据的显示 map<visualIndex, <logicalIndex, visible>>
private:
	void addGroupColumn(int column);
	void delGroupColumn(int column);
	void setupDispUi(void);
public:
	/*model set and get*/
	void setupDataModel(TreeModel* dModel);
	void setupTotalModel(TreeModel* tModel);
	TreeModel* getDataModel();
	TreeModel* getTotalModel();
	QVector<QByteArray> getColumnNames();
	///选定行操作
	QModelIndex findRecord(const QVariant& var);	///根据某个给定值找到第一个包含该给定值的单元格
	QModelIndex findRecord(const QVariant& var, int column);	///重载，限定了匹配值的列
	void setFocusOnIndex(const QModelIndex& idx); ///选中某行并将滚动条拖动直至该行可见
	void ensureVisible(const QModelIndex& idx);
	///右键菜单相关
	//void openHeaderViewCustomContextMenu();			///deprecated
	void openDataViewCustomContextMenu();
	TreeItem* currentItem();
	void updateDisplay();	///更新数据展示界面
	void updateTotalDisplay();	//更新汇总数据展示区
	void expandAll();
	void expand(TreeItem* item);
	void expandRecursively(TreeItem* item, int depth = -1);
	void collapseAll();
	void collapse(TreeItem* item);
	bool isExpanded(TreeItem* item);
	QList<TreeItem*> selectedRows();
	QModelIndexList selectedIndexes();
	void selectItems(const QVector<TreeItem*>& items);
	void selectItems(const QVector<TreeItem*>& items, const QVector<int>& rows);//根据行号和行号对应的item选中，省去item->childNumber()的调用
	///groupable
	void setGroupable(bool group);
	bool isGroupable();
	////drag & drop 
	void setDragAndDropAble(bool able);
	bool isDragAndDropAble();
	////checkable
	void setCheckable(bool check);
	bool isCheckable();
	///editable
	void setEditable(bool edit);
	bool isEditable();
	///sortable
	void setSortable(bool sort);
	bool isSortable();
	///shouldRootDecorate
	void setShouldRootDecorate(bool should);
private:
	void syncAttrToModel();
signals:
	void mousePressed();
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void itemDataChanged(TreeItem* pitem, int column, const QVector<int>& roles = QVector<int>());
	void rowDoubleClicked(const QModelIndex& index);
	void expandedOrCollapsed(const QModelIndex& index, int nType);	//0-折叠，1-展开
	void itemClicked(const QModelIndex& index);
	void sectionShowed(int logicalIndex, bool show);
private slots:
	void onModelDataAltered(TreeItem* pitem, int column, const QVector<int>& roles = QVector<int>());
	void onItemExpanded(const QModelIndex& index);
	void onItemCollapsed(const QModelIndex& index);
	void onItemClicked(const QModelIndex& index);
	void onDataViewMousePressed();
	void onHeadViewSectionSingleClicked(int logicalIndex);
	void onHeadSectionShow(int logicalIndex);
	void onHeadSectionHide(int logicalIndex);
public slots:
	virtual void onHeaderContextMenu(const QPoint& pos);	///deprecated
	virtual void onDataRowContextMenu(const QPoint& pos);	///数据显示区右键菜单
	virtual void onActionOfHeaderSection(bool checked);	///表头右键菜单显示列与否action
	virtual void onRowDoubleClicked(const QModelIndex& index);	///数据条目双击操作
	virtual void onHeaderGroup(bool checked);	///表头字段按组排序
	virtual void onHeaderSectionRightClicked(int logicalIndex);	///表头右键菜单
	void onAutoSetHeaderSectionWidth(bool checked = false);	///自动设置列宽
	void onOptimalSetHeaderSectionWidth(bool checked = false);	///设置最佳列宽
	virtual void onDataMenuAction(bool checked);	///dataMenu对应action的响应
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	virtual void onActionCancelSort();///取消排序
public:
	HeaderView* headView; /// 表头视图
	TreeView* dataView;
	QVBoxLayout* vboxLayout; /// vboxLayout 为 dataView 和 totalView 布局
	QMenu* headMenu;
	QMenu* dataMenu;
private:
	bool editable = false;		///edit
	bool dragndropable = false;	///drag & drop
	bool checkable = false;		///checkable
	bool sortable = false;		///sortable
	TreeModel* dataModel;
	TreeModel* totalModel;
	int rightClickSection;	///标注鼠标右键操作的section
	bool groupAbled = true;
	///dataView->rootIsDecorated属性，默认为false，由创建者手动设置。当model数据节点分层时，需手动设置为true，否则不需要设置
	///dataView根据该值和是否存在列分组来设置rootIsDecorated属性
	bool shouldRootDecorate = false;	
};
