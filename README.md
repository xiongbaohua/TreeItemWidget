# TreeItemWidget
qt model/view
a widget extends from Qt TreeView, with sum bar sticked to the table bottom.
records can be editable, checkable, and has a fast mode for massive data quickly show.

Usage:

TreeItemWidget supply two ways to show data in a tree-like grid view: normal mode and fast mode.

assume having data to show as follow struct:

struct ShowData{

	string col1;
	string col2;
	string col3;
	int col4;
}

1. Normal Mode

//create TreeItemWidget object

TreeItemWidget* disp_view = new TreeItemWidget();

char* columns[]={

	Column_1,
	Column_2,
	Column_3,
	Column_4,
	NULL
}

//create model for disp_view

TreeModel* disp_model = new TreeModel(columns);

disp_view->setupDataModel(viewModel);

///add a total model when sum bar needed

//TreeModel* disp_total_model = new TreeModel(columns);

//disp_view->setupDataModel(disp_total_model);

///asume data reserved in a vector

vector<ShowData> showData;
	
....	

TreeItem* rItem = disp_model->returnRootItem();

//setup TreeItem one by one, when showData large, it's not effective enough

for(int i=0;i<showData.size();i++){

	TreeItem* pItem = new TreeItem(rItem);
	int col = 0;
	pItem->setData(col++, STRING(showData[i].col1.c_str()));
	pItem->setData(col++, STRING(showData[i].col2.c_str()));
	pItem->setData(col++, STRING(showData[i].col3.c_str()));
	pItem->setData(col++, QVariant(showData[i].col4));
}

disp_view->updateDisplay();

//if sum bar needed, set total model data as above

//....

//and update sum bar geometry finally

//disp_view->dataView->setFootViewGeometry();

2. Fast Mode

In this mode, need RecordData and ModelRecord interfaces, implement them first

class ShowRecordData : public RecordData{

private:

	ShowData record;
	
public:

	QVariant data(int column, int role = Qt::DisplayRole){
		switch(column){
			case 0:
			return STRING(record.col1.c_str()));
			case 1:
			return STRING(record.col2.c_str()));
			case 2:
			return STRING(record.col3.c_str()));
			case 3:
			return QVariant(record.col4);
			default:
			return QVariant;
		}
	}
	bool setData(int column, const QVariant& value, int role = Qt::EditRole){
		//set record attribute here
		//...
		return true;
	}
}

class ShowModelRecord : public ModelRecord{

public:

	vector<ShowRecordData> vecRecord;
	
public:

	int recordSize(){
		return vecRecord.size();
	}
	RecordData* recordAt(int pos){
		return &vecRecord[pos];
	}
}

//create TreeItemWidget object

TreeItemWidget* disp_view = new TreeItemWidget();

char* columns[]={

	Column_1,
	Column_2,
	Column_3,
	Column_4,
	NULL
}

//create model for disp_view

TreeModel* disp_model = new TreeModel(columns);

disp_view->setupDataModel(viewModel);

///here, data should be prepared as ShowRecordData vector;

vector<ShowRecordData> showData;

ShowModelRecord* model_record = new ShowModelRecord();

model_record->vecRecord.swap(showData);//swap data vector directly

disp_model->setModelRecord(model_record);

disp_view->updateDisplay();

