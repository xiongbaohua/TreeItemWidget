#include "treeview.h"
#include <qscrollbar.h>
#include "headerview.h"
#include <qclipboard.h>

////////////////////////////////////////////////////////
AutoToolTipDelegate::AutoToolTipDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{
	delegatedView = dynamic_cast<QTreeView*>(parent);
}

AutoToolTipDelegate::~AutoToolTipDelegate()
{
}

bool AutoToolTipDelegate::helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (!e || !view)
		return false;

	if (e->type() == QEvent::ToolTip) {
		QRect rect = view->visualRect(index);
		QSize size = sizeHint(option, index);
		if (rect.width() < size.width()) {
			QVariant tooltip = index.data(Qt::DisplayRole);
			if (tooltip.canConvert<QString>()) {
				QToolTip::showText(e->globalPos(), STRING("<div>%1</div>").arg(tooltip.toString()), view);
				return true;
			}
		}
		if (!QStyledItemDelegate::helpEvent(e, view, option, index))
			QToolTip::hideText();
		return true;
	}

	return QStyledItemDelegate::helpEvent(e, view, option, index);
}
void AutoToolTipDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_ASSERT(index.isValid());
	QVariant var = index.data(Qt::BackgroundRole);
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (var.isValid())	///qss中对treeview设置border属性时会覆盖index.data(Qt::BackgroundRole)，这里强制刷一遍
	{
		painter->fillRect(option.rect, var.value<QColor>());
	}
	if (item->isVirtualItem())///目录节点跨列显示
	{
		QStyleOptionViewItem myoption(option);
		QRect rect = option.rect;
		QString text = item->data(0).toString();

		if (delegatedView)
		{
			HeaderView* h = dynamic_cast<HeaderView*>(delegatedView->header());
			int poscolumn = delegatedView->header()->logicalIndexAt(0);///默认取可见的第一列（滚动条左右拉动会改变此值）
			if (h)
			{
				poscolumn = h->firstVisiableLogicalIndex();////按照visualIndex顺序取的第一个非隐藏的列（滚动条左右拉动不会改变此值）
			}
			if (poscolumn == index.column())
			{
				rect.setWidth(delegatedView->viewport()->size().width());///暂定设置为这么宽
				QPen oldpen = painter->pen();
				quint32 color = item->foreground(0).toULongLong();
				painter->setPen(QColor((color & 0xff0000) >> 16, (color & 0xff00) >> 8, (color & 0xff), (color & 0xff000000) >> 24));
				painter->setFont(index.data(Qt::FontRole).value<QFont>());
				painter->drawText(rect, Qt::AlignLeft, text);///手动将文字显示，其他的先不管
				painter->setPen(oldpen);///恢复
			}
		}
		else
		{
			///noting to do 
		}
		return QStyledItemDelegate::paint(painter, myoption, index);
	}
	QStyledItemDelegate::paint(painter, option, index);
}

///////////////////////////////////////////////////////
TreeViewAdapt::~TreeViewAdapt()
{
}

TreeView::TreeView(QWidget* parent)
	: TreeViewAdapt(parent)
{
	footView = new TreeViewAdapt(this);
	footView->setFrameShape(QFrame::NoFrame);
	footView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	footView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	footView->setHeaderHidden(true); //汇总视图隐藏表头
	footView->setUniformRowHeights(true);

	///单击时编辑item
	connect(this, &TreeView::clicked, this, &TreeView::onMouseClicked);
	//horizontalScrollBar同步
	connect(horizontalScrollBar(), &QScrollBar::valueChanged, footView->horizontalScrollBar(), &QScrollBar::setValue);
	//两处header 同步
	connect(header(), &QHeaderView::sectionResized, this, &TreeView::onDataViewSectionResized);
	connect(header(), &QHeaderView::sectionMoved, this, &TreeView::onDataViewSectionMoved);
	footView->header()->setDefaultAlignment(Qt::AlignCenter);
	setUniformRowHeights(true);
	///set dynamic ToolTip for truncated items
	if (itemDelegate())
	{
		delete itemDelegate();
	}
	if (footView->itemDelegate())
	{
		delete footView->itemDelegate();
	}
	setItemDelegate(new AutoToolTipDelegate(this));
	footView->setItemDelegate(new AutoToolTipDelegate(footView));
}

TreeView::~TreeView()
{
}

void TreeView::setFootViewHeader()
{
	footView->setHeader(this->header());
}

void TreeView::setFootViewModel(QAbstractItemModel* model)
{
	footView->setModel(model);
}
///
void TreeView::setFootViewGeometry()
{
	///footView has no model or records should be set invisible, or else 
	///it will looks strange
	if (footView->model() == nullptr)
	{
		if (footView->isVisible())
		{
			footView->setVisible(false);
		}
		return;
	}
	int fcount = footView->model()->rowCount();
	if (fcount < 1)
	{
		if (footView->isVisible())
		{
			footView->setVisible(false);
		}
		return;
	}
	if (!footView->isVisible())
	{
		footView->setVisible(true);
	}
	int hHeight = header()->height();
	int rHeight = footView->sizeHintForRow(0);
	setViewportMargins(0, hHeight, 0, fcount * rHeight);	//top margin 留给表头显示, bottom margin 留给汇总行显示
	QRect rc = viewport()->rect();
	footView->setGeometry(QRect(rc.left() + 1, rc.bottom() + hHeight, rc.width() - 1, fcount * rHeight));
}

TreeViewAdapt* TreeView::footTreeView()
{
	return footView;
}

void TreeView::setHeaderAndReconnectSignals(QHeaderView* header)
{
	//解绑原来的信号/槽连接
	disconnect(this->header(), &QHeaderView::sectionResized, this, &TreeView::onDataViewSectionResized);
	disconnect(this->header(), &QHeaderView::sectionMoved, this, &TreeView::onDataViewSectionMoved);
	HeaderView* oldh = dynamic_cast<HeaderView*>(this->header());
	HeaderView* newh = dynamic_cast<HeaderView*>(header);
	if (oldh)
	{
		disconnect(oldh, &HeaderView::hideSectionSignal, this, &TreeView::onDataViewSectionHidden);
		disconnect(oldh, &HeaderView::showSectionSignal, this, &TreeView::onDataViewSectionShow);
	}

	//重新绑定新header的信号到dataView槽
	connect(header, &QHeaderView::sectionResized, this, &TreeView::onDataViewSectionResized);
	connect(header, &QHeaderView::sectionMoved, this, &TreeView::onDataViewSectionMoved);
	if (newh)
	{
		connect(newh, &HeaderView::hideSectionSignal, this, &TreeView::onDataViewSectionHidden);
		connect(newh, &HeaderView::showSectionSignal, this, &TreeView::onDataViewSectionShow);
	}

	this->setHeader(header); // setHeader() will call headView->setParent(this)
}

//QRect TreeView::visualRect(const QModelIndex& index) const
//{
//	QRect rect = QTreeView::visualRect(index);
//	if (index.isValid() && !isIndexHidden(index) && index.column() == header()->logicalIndexAt(0))
//	{
//		TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
//		if (item && item->isVirtualItem())///修改目录节点所占列宽，只有位于可见列的第一列才显示
//		{
//			if (index.column() == header()->logicalIndexAt(0))
//				rect.setWidth(rect.width() * item->columnCount());
//			else
//				rect.setWidth(0);
//		}
//	}
//	return rect;
//}

void TreeView::resizeEvent(QResizeEvent* event)
{

	QTreeView::resizeEvent(event);
	setFootViewGeometry();
}

void TreeView::paintEvent(QPaintEvent* event)
{
	QTreeView::paintEvent(event);
	setFootViewGeometry();
}
void TreeView::mousePressEvent(QMouseEvent* event)
{
	QTreeView::mousePressEvent(event);
	emit mousePressed();
}
void TreeView::keyPressEvent(QKeyEvent* event)
{
	if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_C))
	{
		QModelIndex idx = currentIndex();
		if (idx.isValid())
		{
			QString str = idx.data(Qt::DisplayRole).toString();
			QClipboard* clop = QApplication::clipboard();
			clop->setText(str);
		}
	}
	else if ((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_C))
	{
		QModelIndex idx = currentIndex();
		if (idx.isValid())
		{
			TreeItem* item = static_cast<TreeItem*>(idx.internalPointer());
			QString str = "", spaceStr = "    ";
			for (int i = 0; i < header()->count(); i++)
			{
				if (!isColumnHidden(i))
				{
					str += (item->data(i, Qt::DisplayRole).toString() + spaceStr);
				}
			}
			QClipboard* clop = QApplication::clipboard();
			clop->setText(str);
		}
	}
	return TreeViewAdapt::keyPressEvent(event);
}
void TreeView::onMouseClicked(const QModelIndex& index)
{
	edit(index);
}
void TreeView::onDataViewSectionResized(int logicalIndex, int oldSize, int newSize)
{
	footView->header()->resizeSection(logicalIndex, newSize);
}

void TreeView::onDataViewSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
	QHeaderView* head = footView->header();
	head->moveSection(head->visualIndex(logicalIndex), newVisualIndex);
	HeaderView* h = dynamic_cast<HeaderView*>(header());
	if (h)
	{
		setTreePosition(h->firstVisiableLogicalIndex());
	}
	else
	{
		setTreePosition(0);
	}
}

void TreeView::onDataViewSectionHidden(int logicalIndex)
{
	footView->header()->hideSection(logicalIndex);
	HeaderView* h = dynamic_cast<HeaderView*>(header());
	if (h)
	{
		setTreePosition(h->firstVisiableLogicalIndex());
	}
	else
	{
		setTreePosition(0);
	}
}

void TreeView::onDataViewSectionShow(int logicalIndex)
{
	footView->header()->showSection(logicalIndex);
	HeaderView* h = dynamic_cast<HeaderView*>(header());
	if (h)
	{
		setTreePosition(h->firstVisiableLogicalIndex());
	}
	else
	{
		setTreePosition(0);
	}
}