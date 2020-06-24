#include "headerview.h"

HeaderView::HeaderView(Qt::Orientation orientation, QWidget* parent)
	: QHeaderView(orientation, parent)
{
	connect(this, &QHeaderView::sectionClicked, this, &HeaderView::onSectionClicked);
}

HeaderView::~HeaderView()
{
}

void HeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
	if (!rect.isValid())
		return;
	//使用QHeaderView默认
	QHeaderView::paintSection(painter, rect, logicalIndex);

	TreeModel* tmodel = dynamic_cast<TreeModel*>(model());
	if (tmodel)
	{
		MultiColumnOrder sortOdrs = tmodel->orderColumns;
		for (int i = 0; i < sortOdrs.size(); i++)
		{
			QPair<int, Qt::SortOrder> sortCol = sortOdrs.at(i);
			if (sortCol.first == logicalIndex)
			{
				///暂时只做如下处理
				QStyleOptionHeader opt;
				initStyleOption(&opt);
				switch (sortCol.second)
				{
				case Qt::AscendingOrder:
					opt.sortIndicator = QStyleOptionHeader::SortDown; //
					break;
				case Qt::DescendingOrder:
					opt.sortIndicator = QStyleOptionHeader::SortUp;
					break;
				default:
					break;
				}
				opt.rect = rect;
				opt.section = logicalIndex;
				//获取表头数据
				opt.text = model()->headerData(logicalIndex, Qt::Horizontal).toString();
				opt.textAlignment = defaultAlignment();
				opt.iconAlignment = Qt::AlignVCenter;
				style()->drawControl(QStyle::CE_Header, &opt, painter, this);
				return;
			}
		}
	}
}

///slot to sectionClicked()
void HeaderView::onSectionClicked(int logicalIndex)
{
	//HeadView->flipSortIndicator函数中会调用setSortIndicator，然后emit sortIndicatorChanged 信号
	//TreeView 接收sortIndicatorChanged信号后，会调用sort函数。
	//而在HeadView->flipSortIndicator返回后，才会emit sectionClicked 信号。
	//在headView中，增加了sectionClicked信号的处理函数onSectionClicked，设置表头多列排序样式。
	//这两处需保持效果同步
	Qt::SortOrder sortOdr = QHeaderView::sortIndicatorOrder();
	QPair<int, Qt::SortOrder> colSort(logicalIndex, sortOdr);
	if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
	{
		this->ableMultiSort = true;
		for (int i = 0; i < orderCols.size(); i++)
		{
			if (orderCols.at(i).first == logicalIndex)
			{
				orderCols.removeAt(i);
				break;
			}
		}
	}
	else
	{
		this->ableMultiSort = false;
		orderCols.clear();
	}
	orderCols.append(colSort); //无论如何都把最新的sortIndicator存入orderCols
}

void HeaderView::hideSectionAndEmitSignal(int logicalIndex)
{
	QHeaderView::hideSection(logicalIndex);
	emit hideSectionSignal(logicalIndex);
}

void HeaderView::showSectionAndEmitSignal(int logicalIndex)
{
	QHeaderView::showSection(logicalIndex);
	emit showSectionSignal(logicalIndex);
}

int HeaderView::firstVisiableLogicalIndex()
{
	for (int i = 0; i < count(); i++)
	{
		int idx = logicalIndex(i);
		if (!isSectionHidden(idx))
		{
			return idx;
		}
	}
	return 0;
}

void HeaderView::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::RightButton)
	{
		int logicalIndex = logicalIndexAt(e->pos());
		emit sectionRightClicked(logicalIndex);
	}
	else if (e->button() == Qt::LeftButton)
	{
		bLeftMousePressed = true;
		toHideIndex = logicalIndexAt(e->pos());
		if (toHideIndex > -1)
		{
			mstate = PressState;
		}
	}
	QHeaderView::mousePressEvent(e);
}

void HeaderView::mouseMoveEvent(QMouseEvent* e)
{
	bLeftMousePressed = false;
	int curpos, btmpos;
	if (orientation() == Qt::Orientation::Horizontal)
	{
		curpos = e->y();
		btmpos = QPoint(pos().x(), pos().y() + height()).y();
	}
	else
	{
		curpos = e->x();
		btmpos = QPoint(pos().x() + width(), pos().y()).x();
	}
	if (curpos > btmpos)
	{
		if (mstate == PressState && cursor() == Qt::ArrowCursor)///暂时通过鼠标形状来判断是否
		{
			mstate = MoveState;
			stageCursor = cursor();
			setCursor(QCursor(QPixmap(":Resources/blind.png")));
		}
	}
	else
	{
		if (mstate == MoveState)
		{
			mstate = PressState;
			setCursor(stageCursor);
		}
	}
	QHeaderView::mouseMoveEvent(e);
}

void HeaderView::mouseReleaseEvent(QMouseEvent* e)
{
	if (mstate == MoveState)
	{
		if (toHideIndex > -1 && toHideIndex < count())
		{
			hideSectionAndEmitSignal(toHideIndex);
		}
	}
	mstate = NoState;
	if (bLeftMousePressed)
	{
		bLeftMousePressed = false;
		int pos = orientation() == Qt::Horizontal ? e->x() : e->y();
		int logicalIndex = logicalIndexAt(pos);
		emit sectionSingleClicked(logicalIndex);
	}
	QHeaderView::mouseReleaseEvent(e);
}
