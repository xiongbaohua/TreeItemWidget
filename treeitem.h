#ifndef __TREEITEM_H__
#define __TREEITEM_H__

#include <QList>
#include <QVariant>
#include <QVector>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qabstractitemmodel.h>
#include "treemodel.h"
#include "formatnumber.h"
#include "rcnamespace.h"
#include "recorddata.h"

/////////////TreeItemData////////////////
class TreeItemData
{
public:
	QMap<int, QVariant> dataMap;	///QMap<Role,Value>
	inline TreeItemData() : dataMap(QMap<int, QVariant>()) {}
	inline TreeItemData(const TreeItemData& other)
	{
		dataMap = other.dataMap;
	}
	inline bool operator==(const TreeItemData& other) const	///排序不用此重载做比较
	{
		return dataMap == other.dataMap;
	}
	inline void setData(QVariant data, int role)
	{
		dataMap[role] = data;
	}
	inline QVariant data(int role) const
	{
		return dataMap.value(role);
	}
	inline void eraseData(int role)
	{
		QMap<int, QVariant>::iterator it = dataMap.find(role);
		if (it != dataMap.end())
		{
			dataMap.erase(it);
		}
	}
};

/////////////TreeItem//////////////////
class TreeItem
{
public:
	TreeItem();
	TreeItem(TreeItem* parent);
	TreeItem(RecordData* record);
private:
	TreeItem(TreeItem* parent, RecordData* record);
public:
	explicit TreeItem(const QVector<QVariant>& data, TreeItem* parent = 0, int role = Qt::DisplayRole);
	virtual ~TreeItem();

	TreeItem* child(int number);
	int childCount() const;
	int rowCount() const;
	int columnCount() const;
	QVariant data(int column, int role = Qt::DisplayRole) const;
	bool insertChildren(int position, int count, int columns);
	bool insertColumns(int position, int columns);
	TreeItem* parent();
	bool removeChildren(int position, int count);
	bool removeColumns(int position, int columns);
	int childNumber() const; // index in parent->childItems
	bool setData(int column, const QVariant& value, int role = Qt::DisplayRole);
	bool addChild(TreeItem* child);

	static bool isVariantLessThen(const QVariant& left, const QVariant& right);
	static bool isVariantEqualTo(const QVariant& left, const QVariant& right);
	static bool isVariantGreaterThan(const QVariant& left, const QVariant& right);
	QVector<TreeItem*> fatChildItems(); // 获取兄弟节点，包括自己
	QVector<TreeItem*> broOtherItems(); // 获取兄弟节点，不包括自己
	QVector<TreeItem*> childrenItems(); //获取子节点列表引用
	TreeItem* takeChildAt(int position); //移除指定位置的子节点
	void appendChild(TreeItem* child); // 在末尾增加子节点
	void prependChild(TreeItem* child); //在头部增加子节点
	void insertChildAt(int position, TreeItem* child); // 在指定位置插入子节点
	QVector<TreeItem*> takeChildAll(); /*移除所有子节点*/
	void setParentItem(TreeItem* parent);/*设置父亲节点*/
	void appendChildList(const QVector<TreeItem*>& childList); /*在末尾批量插入子节点*/
	void setChildren(const QVector<TreeItem*>& itemVector);
	int levelOfItem();
	void setItemLevel(int level);
	void detachChildren();	///解绑（不销毁）子节点，但子节点的父节点并没有重设
	///设置部分样式
	void setForeground(int col, Qt::GlobalColor clr);
	void setForeground(int col, unsigned long colorVal);
	void setForeground(int col, int r, int g, int b, int a);
	QVariant foreground(int col);
	void setBackground(int col, Qt::GlobalColor clr);
	void setBackground(int col, unsigned long colorVal);
	void setBackground(int col, int r, int g, int b, int a);
	QVariant background(int col);
	void setTextAlign(int col, Rcspace::RcTextAlignment aenum);
	QVariant textAlign(int col);
	void setIcon(int col, Rcspace::RcIconEnum ienum);
	QVariant cellIcon(int col);
	///虚节点属性
	void setVirtual(bool bVirtual);
	bool isVirtualItem();
	///check state settings
	void setCheckable(int col, bool check);
	void setChecked(int col, bool check);
	void setCheckState(int col, Qt::CheckState state);
	bool isCheckable(int col);
	bool isChecked(int col);
	///erase data roles
	void eraseData(int col, int role);
	void switchChild(int m, int n);
private:
	///recordData相关，只对友元开放
	void setRecordData(RecordData* record);
	RecordData* innerRecordData();
private:
	QVector<TreeItem*> childItems;	///使用QVector
	QVector<TreeItemData> itemData;
	RecordData* recordData = nullptr;///用于显示displayRole
	TreeItem* parentItem;
	int itemLevel;	///只在目录节点使用，用于排序
	bool isVirtual = false;	///是否虚节点（分组时生成的目录节点），分组时创建目录节点后手动设置为true

	friend class TreeModel;
};

#endif // __TREEITEM_H__