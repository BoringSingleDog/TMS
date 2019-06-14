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
	downLoad = new QAction(QString("����"),NULL);
	deleteList = new QAction(QString("ɾ��"),NULL);

	ui.list_fileColumn->addAction(downLoad);
	ui.list_fileColumn->addAction(deleteList);
	ui.list_fileColumn->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui.lineEdit->setReadOnly(true);			//��LineEdit����Ϊ ֻ��ģʽ���û�ֻ��ͨ���ļ��Ի�����ѡȡҪ�ϴ����ļ��������û������ϴ��ļ�·������

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

//����������ͻ�ȡ�����ļ��б������
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
	m_fileNames = QFileDialog::getOpenFileNames(this,"please choose file",".","");		//��ȡ�û�ѡ����ļ���
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

void MITSDatabaseBackUp::doBackUp()			//�������ݿⱸ��
{
	//���󱸷����ݿ�
	int ret = g_app->requestDBControl(0,"");		//0 ��ʾ����
	if(ret == ackOK)
	{
		ui.list_fileColumn->clear();
		InitFileListWidget();		//���³�ʼ��sql�ļ��б�
		QMessageBox::information(NULL,"Tips","�������ݿ�ɹ�",QMessageBox::Yes);
	}
	else
		QMessageBox::information(NULL,"Tips","�������ݿ�ʧ��",QMessageBox::Yes);

}

void MITSDatabaseBackUp::doRecover()		//����ָ����ݿ�
{
	QListWidgetItem *currentItem = ui.list_fileColumn->currentItem();
	QString operateItemText = currentItem->text();
	map<QString,QString>::const_iterator iter = g_app->m_DBSqlFileDownURL.find(operateItemText);
	int ret = g_app->requestDBControl(1,iter->first);
	if(ret == ackOK)
		QMessageBox::information(NULL,"Tips","���ݿ�ָ��ɹ�",QMessageBox::Yes);
	else
		QMessageBox::information(NULL,"Tips","���ݿ�ָ�ʧ��",QMessageBox::Yes);
}

void MITSDatabaseBackUp::doDealAction()		//�һ�����ָ���ļ�
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
			QMessageBox::information(NULL,"Tips","ɾ���ļ��ɹ�",QMessageBox::Yes);
		}
		else
			QMessageBox::information(NULL,"Tips","ɾ���ļ�ʧ��",QMessageBox::Yes);

	}
	if(senderAction == downLoad)
	{
		QString m_saveDirPath = QFileDialog::getExistingDirectory(this,"ѡ�񱣴�·��") + "/";
		map<QString,QString>::const_iterator iter =	g_app->m_DBSqlFileDownURL.find(operateItemText);
		bool ret = g_app->getHttpCommunicate()->downLoad(iter->second,m_saveDirPath + operateItemText);
		if(!ret)
			QMessageBox::warning(NULL,"Tips","����ʧ��",QMessageBox::Yes);
		else
			QMessageBox::information(NULL,"Tips","���سɹ�",QMessageBox::Yes);
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

