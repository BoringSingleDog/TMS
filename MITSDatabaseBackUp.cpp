#include "stdafx.h"
#include "MITSDatabaseBackUp.h"
#include <QFileDialog>
#include <QDebug>
#include "MITSApp.h"
#include <QMessageBox>
extern MITSApp *g_app;
MITSDatabaseBackUp::MITSDatabaseBackUp(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	m_pressed = false;
	m_pos = QPoint(0,0);
	this->setWindowFlags(Qt::FramelessWindowHint);
	InitFileListWidget();	
	downLoad = new QAction(QString("下载"),NULL);
	deleteList = new QAction(QString("删除"),NULL);

	ui.list_fileColumn->addAction(downLoad);
	ui.list_fileColumn->addAction(deleteList);
	ui.list_fileColumn->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui.lineEdit->setReadOnly(true);			//将LineEdit设置为 只读模式，用户只能通过文件对话框来选取要上传的文件，避免用户输入上传文件路径错误

	connect(ui.pb_close,SIGNAL(clicked()),this,SLOT(close()));
	connect(ui.pb_ChooseFile,SIGNAL(clicked()),this,SLOT(doChooseFile()));
	connect(ui.pb_UploadFile,SIGNAL(clicked()),this,SLOT(doUploadFile()));
	connect(ui.pb_BackUp,SIGNAL(clicked()),this,SLOT(doBackUp()));
	connect(ui.pb_Recover,SIGNAL(clicked()),this,SLOT(doRecover()));
	connect(downLoad,SIGNAL(triggered()),this,SLOT(doDealAction()));
	connect(deleteListButton,SIGNAL(triggered()),this,SLOT(doDealAction()));
	
}

MITSDatabaseBackUp::~MITSDatabaseBackUp()
{
	if(downLoad)
	{
		delete downLoad;
	}
	if(deleteList)
	{
		delete deleteList;
		deleteList = NULL;
	}
}

//向服务器发送获取备份文件列表的请求
void MITSDatabaseBackUp::InitFileListWidget()
{
  int ret =	g_app->requestDBSqlFileList();
  if(ret == ackOK)
  {
	  int size = g_app->m_DBSqlFileDownURL.size();
	  if(g_app->m_DBSqlFileDownURL.size()>0)
	  {
		map<QString,QString>::const_iterator iter = g_app->m_DBSqlFileDownURL.begin();
		while (iter != g_app->m_DBSqlFileDownURL.end())
		{
			QString fileName = iter->first;
			doAddListWIdgetItem(fileName);
			++iter;
		}
	  }
  }
}

void MITSDatabaseBackUp::mousePressEvent(QMouseEvent *event)
{
	 if(event->button() == Qt::LeftButton)
	 {
		 m_pos = event->pos();
		 m_pressed =true;
	 }
}

void MITSDatabaseBackUp::mouseMoveEvent(QMouseEvent *event)
{
	if(m_pressed && (event->buttons() & Qt::LeftButton))
	{
		this->move(event->globalPos() - m_pos);
		event->accept();
	}
}

void MITSDatabaseBackUp::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		m_pressed = false;
	}
}

void MITSDatabaseBackUp::doChooseFile()
{
	m_fileNames = QFileDialog::getOpenFileNames(this,"please choose file",".","");		//获取用户选择的文件名
	foreach(QString fileName,m_fileNames)
	{
		fileName = fileName.section("/",-1);
		this->ui.lineEdit->setText(fileName);
	}
}

void MITSDatabaseBackUp::doUploadFile()
{
	foreach(QString filename,m_fileNames)
	{
		bool ret =	g_app->getHttpCommunicate()->upLoadPic("uploadSql",filename);
		if(ret)
		{
			QMessageBox::information(NULL,"Tips","file  upload succeed ",QMessageBox::Yes);
			ui.list_fileColumn->clear();
			InitFileListWidget();
		}
		else
			QMessageBox::information(NULL,"Tips","file  upload failed ",QMessageBox::Yes);
	}
	
}

void MITSDatabaseBackUp::doBackUp()			//进行数据库备份
{
	//请求备份数据库
	int ret = g_app->requestDBControl(0,"");		//0 表示备份
	if(ret == ackOK)
	{
		ui.list_fileColumn->clear();
		InitFileListWidget();		//重新初始化sql文件列表
		QMessageBox::information(NULL,"Tips","备份数据库成功",QMessageBox::Yes);
	}
	else
		QMessageBox::information(NULL,"Tips","备份数据库失败",QMessageBox::Yes);

}

void MITSDatabaseBackUp::doRecover()		//请求恢复数据库
{
	QListWidgetItem *currentItem = ui.list_fileColumn->currentItem();
	QString operateItemText = currentItem->text();
	map<QString,QString>::const_iterator iter = g_app->m_DBSqlFileDownURL.find(operateItemText);
	int ret = g_app->requestDBControl(1,iter->first);
	if(ret == ackOK)
		QMessageBox::information(NULL,"Tips","数据库恢复成功",QMessageBox::Yes);
	else
		QMessageBox::information(NULL,"Tips","数据库恢复失败",QMessageBox::Yes);
}

void MITSDatabaseBackUp::doDealAction()		//右击下载指定文件
{
	QListWidgetItem *currentItem = ui.list_fileColumn->currentItem();
	QString operateItemText = currentItem->text();

	QAction *senderAction = qobject_cast<QAction*>(sender());
	if(senderAction == deleteList)
	{
		int ret = g_app->requestDBSqlFileModify(operateItemText,0,0);	

		if(ret == ackOK)
		{
			ui.list_fileColumn->clear();
			InitFileListWidget();
			QMessageBox::information(NULL,"Tips","删除文件成功",QMessageBox::Yes);
		}
		else
			QMessageBox::information(NULL,"Tips","删除文件失败",QMessageBox::Yes);

	}
	if(senderAction == downLoad)
	{
		QString m_saveDirPath = QFileDialog::getExistingDirectory(this,"选择保存路径") + "/";
		map<QString,QString>::const_iterator iter =	g_app->m_DBSqlFileDownURL.find(operateItemText);
		bool ret = g_app->getHttpCommunicate()->downLoad(iter->second,m_saveDirPath + operateItemText);
		if(!ret)
			QMessageBox::warning(NULL,"Tips","下载失败",QMessageBox::Yes);
		else
			QMessageBox::information(NULL,"Tips","下载成功",QMessageBox::Yes);
	}
}

void MITSDatabaseBackUp::doAddListWIdgetItem(QString fileName)
{
	QIcon Icon(":/MITSIntoRebotTool/Resources/file.ico");
	QListWidgetItem *item = new QListWidgetItem;
	item->setIcon(Icon);
	item->setText(fileName);
	ui.list_fileColumn->addItem(item);
}

