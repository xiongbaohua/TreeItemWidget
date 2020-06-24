#pragma once

#include <qheaderview.h>
#include <qpainter.h>
#include "treeitem.h"
#include <qevent.h>
#include <QApplication>

class HeaderView : public QHeaderView
{
	Q_OBJECT
public:
	MultiColumnOrder orderCols;
	std::map<int, Qt::SortOrder> sortColMap;
	bool ableMultiSort = false;
private:
	enum MouseState
	{
		NoState = 0,
		PressState,
		MoveState
	};
	MouseState mstate = NoState;
	QCursor stageCursor;
	int toHideIndex;
	bool bLeftMousePressed = false;	///用于重写sectoinClicked信号
public:
	HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
	~HeaderView();
	/*override*/
	void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

	///action & emit signal
	/// hide action will emit resize signal
	void hideSectionAndEmitSignal(int logicalIndex);	
	/// show action will emit resize signal
	void showSectionAndEmitSignal(int logicalIndex);
	/// return first visiable section's logical index
	int firstVisiableLogicalIndex();
protected:
	void mousePressEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
signals:
	void hideSectionSignal(int logicalIndex);
	void showSectionSignal(int logicalIndex);
	void sectionRightClicked(int logicalIndex);
	void sectionSingleClicked(int logicalIndex);	////与Qt自带的sectionClicked信号区别
public slots:
	void onSectionClicked(int logicalIndex);

};
