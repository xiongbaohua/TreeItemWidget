#ifndef __TREEMODEL_H__
#define __TREEMODEL_H__

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <qapplication.h>
#include "stdafx.h"
#include <qfont.h>
#include "rcnamespace.h"
#include <qheaderview.h>
#include "modelrecord.h"

#define EditEnableRole	64	////自定义Role，当该角色存在且置为0时，屏蔽单元格的可编辑属性，即使单元格所在列为可编辑
template <class T>
class NodeType
{
public:
	NodeType(T val) :value(val) {}
	~NodeType() {
		qDeleteAll(childNodes);
		childNodes.clear();
	}

public:
	T value;
	QVector<NodeType*> childNodes;
};

typedef QList<QPair<int, Qt::SortOrder>> MultiColumnOrder;
class TreeItem;

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeModel(QObject* parent = 0);
	TreeModel(TreeItem* rItem, QObject* parent = 0);
	TreeModel(LPCSTR columns[], QObject* parent = 0);
	TreeModel(QByteArray columns[], int cols, QObject* parent = 0);
	virtual ~TreeModel();

	// override 
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	void sort(int column, Qt::SortOrder order) override;
	bool insertColumns(int position, int columns, const QModelIndex& parent = QModelIndex()) override;
	bool removeColumns(int position, int columns, const QModelIndex& parent = QModelIndex()) override;
	bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole) override;
	bool setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles) override;
	/* sort feature support */
	int sortColumnNum();
	MultiColumnOrder sortMultiColumns();
	bool isMultiColumnSort();
	void setSortAbled(bool enable);
	virtual void resetModelData(TreeItem* rItem); // 重设模型数据
	void addGroupColumn(int column); //增加按字段分组，只能按照添加字段顺序依次分组
	void delGroupColumn(int column); //删除按字段分组，删除顺序不需要按照添加的顺序
	void doGroupByColumns(); /*根据groupColumns执行分组，可能会用到*/
	void doGroupByColumns(const QVector<int>& colVec); /*根据groupColumns执行分组，可能会用到*/
	void doSortByColumns();/*根据orderColumns执行排序，可能会用到*/
	void doSortByColumns(MultiColumnOrder sortOrders);/*根据orderColumns执行排序，可能会用到*/
	///查找record，任一列值与var匹配，返回第一个匹配的index
	QModelIndex findRecordMatchVal(const QVariant& var);
	///重载findRecord，加入列限制
	QModelIndex findRecordMatchValByColumn(const QVariant& var, int column);
	TreeItem* returnRootItem();
	void resetModelDataWithItemList(QVector<TreeItem*>& itemList);
	void appendRows(const QVector<TreeItem*>& itemList);
	QVector<int> getGroupColumns();
	void resetGroupColumns(QVector<int> cols);
	///数据相关操作对外接口
	TreeItem* getItemAt(int i);
	int itemCounts();
	void appendItem(TreeItem* item);///item只添加到itemVector，不会同步到rootItem下
	void appendItemSync(TreeItem* item);///item同步添加到itemVector和rootItem下，适用于view无排序、无分组的场景，当视图新创建时无需手动调用updateIndex
	void truncateItems(int count);
	void updateIndex();	///根据现有的排序、分组更新索引
	void removeAllItems();
	void removeItemAt(int row);////只删除itemVector下的节点
	void removeItemSyncAt(int row);////同步删除itemVector和rootItem下的节点
	////部分显示属性设置
	///列属性设置
	void setTextAlignment(int column, Rcspace::RcTextAlignment aenum);
	void setBackground(int column, unsigned long colorVal);
	void setForeground(int column, unsigned long colorVal);
	void setIcon(int column, Rcspace::RcIconEnum ienum);
	///全局属性设置
	void setFont(QFont font);
	void setTextAlignment(Rcspace::RcTextAlignment aenum);
	void setBackground(unsigned long colorVal);
	void setForeground(unsigned long colorVal);
	void setIcon(Rcspace::RcIconEnum ienum);
	///editable
	void setEditable(bool edit);
	bool isEditable();
	///drag&drop
	void setDragAndDropAble(bool able);
	bool isDragAndDropAble();
	Qt::DropActions supportedDragActions() const override;
	Qt::DropActions	supportedDropActions() const override;
	QStringList mimeTypes() const override;
	QMimeData* mimeData(const QModelIndexList& indexes) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	////checkable
	void setCheckable(bool check);
	bool isCheckable();
	////isItemSelectable，该属性少用，目前只在AutoComboBox中有用到过，仅支持model级别，不支持单元格级别
	void setItemIsSelectable(bool selectable);
	bool isItemIsSelectable();
	///获取item在数据列表中的位置
	int getItemIndex(TreeItem* pitem);
	QModelIndex index(int row, int column, TreeItem* item);
	///设置某列是否可编辑
	void setColumnEditable(int col, bool edit);
	void setAllColumnEditable(bool edit);
	bool getColumnEditable(int col);
	////交换item,不校验position的有效性
	void switchItems(int posLeft, int posRight, TreeItem* parent);
	///设置属性
	void setAttributeGlobal(QVariant var, int role);
	void setAttributeColumn(QVariant var, int column, int role);
	void removeAttributeColumn(int column, int role);
	///
	void setHeaderView(QHeaderView* h);
	QHeaderView* getHeaderView();
	bool reloadData();
	///预分配空间
	void reserveItemSpace(int capacity);
	void setModelRecord(ModelRecord* record);
	///treeNode提供数据树形结构视图，model并不持有它///treeNode相当于model的rootItem，属于隐藏的根节点
	void setModelRecord(ModelRecord* record, NodeType<int>* treeNode);
private:
	TreeItem* buildItemWithModel(TreeItem* pitem, NodeType<int>* node);
protected:
	void setHeader(TreeItem* root);
private:
	TreeItem* getItem(const QModelIndex& index) const;
	void mergeSameGroupedItems(TreeItem* item);
	void sortPrivate(int column, Qt::SortOrder order);
	void setDefaultAttrOfModel();///设置model全局默认属性
	void removeItem(TreeItem* item);
	void setCheckStateAscend(const QModelIndex& index);
	bool setDataUnaffectParentCheckState(const QModelIndex& index, const QVariant& value, int role);
	void sortChildrenItems(TreeItem* parent, int column, Qt::SortOrder order);
	bool itemLessThan(TreeItem* left, TreeItem* right);
	bool itemGreaterThan(TreeItem* left, TreeItem* right);
	bool directItemLessThan(TreeItem* left, TreeItem* right);
	bool directItemGreaterThan(TreeItem* left, TreeItem* right);
	bool multiColEqual(TreeItem* left, TreeItem* right, int cmpSize);
	QModelIndex findRecordByColumn(TreeItem* parent, const QVariant& var, int column);
	QModelIndex findRecordNoColumn(TreeItem* parent, const QVariant& var);
signals:
	void dataAltered(TreeItem* pitem, int column, const QVector<int>& roles = QVector<int>());	///避免与内置的dataChanged信号重名
private:
	QVector<int> editableColVec;	///允许编辑的列
	bool editable = false;		///是否可编辑总开关,单元格是否可编辑，由editable和editableColVec同时决定， editable==true && editableColVec.indexOf(col) > -1 时可编辑
	bool dragndropable = false;	///drag & drop
	bool checkable = false;	///checkable
	bool itemIsSelectable = true;///Qt::ItemIsSelectable
	bool ableMultiColumnSort = false; // bool 类型初始化需要手动给定值
	TreeItem* rootItem = nullptr;	///index root item, for display 
	QVector<TreeItem*> itemVector;	///存储数据节点，重设排序和分组时只重建目录节点，数据节点保留
	QVector<TreeItem*> itemToDelVector;	///待删除的数据节点
	QVector<TreeItem*> itemHideVector;///暂不用
	int sortColumn = -1;
	MultiColumnOrder orderColumns; // support multicolumn sort
	QVector<int> groupColumns;

	///对于单元格属性，依次按照 单元格->列->model全局 的顺序来获取
	///如果获取到当前级别的属性为inValid，则按照上述顺序获取下一属性值
	QMap<int, QVariant> attrOfModel;	///molde全局属性
	QVector<QMap<int, QVariant>> attrOfColumns;	///列级别属性
	///存储表头
	QHeaderView* head = nullptr;
	///快速加载
	ModelRecord* modelRecord = nullptr;
	ModelRecord* oldModelRecord = nullptr;///替换modelRecord作为暂存项，被替换的modelRecord并不立即删除，updateIndex后再删除

	friend class TreeItem;
	friend class HeaderView;
};

#endif