#pragma once

#include <QTreeView>
#include <qheaderview.h>
#include <qstyleditemdelegate.h>
#include <qtooltip.h>
#include <qevent.h>
#include "stdafx.h"
#include <qapplication.h>
#include <qpainter.h>
#include "treeitem.h"

class AutoToolTipDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	QTreeView* delegatedView = nullptr;
public:
	AutoToolTipDelegate(QObject* parent);
	~AutoToolTipDelegate();

public:
	bool helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class TreeViewAdapt : public QTreeView
{
	Q_OBJECT

public:
	using QTreeView::QTreeView;
	virtual ~TreeViewAdapt();
	using QTreeView::sizeHintForColumn;
};


class TreeView : public TreeViewAdapt
{
	Q_OBJECT

private:
	TreeViewAdapt* footView = nullptr;

public:
	TreeView(QWidget* parent = nullptr);
	~TreeView();
	void setFootViewHeader();
	void setFootViewModel(QAbstractItemModel* model);
	void setFootViewGeometry();
	TreeViewAdapt* footTreeView();
	//´úÌæsetHeader
	void setHeaderAndReconnectSignals(QHeaderView* header);
	//QRect visualRect(const QModelIndex& index) const override;
protected:
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
signals:
	void mousePressed();
protected slots:
	void onMouseClicked(const QModelIndex& index);
public slots:
	void onDataViewSectionResized(int logicalIndex, int oldSize, int newSize);
	void onDataViewSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
	void onDataViewSectionHidden(int logicalIndex);
	void onDataViewSectionShow(int logicalIndex);
};
