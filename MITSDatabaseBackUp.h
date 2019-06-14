#ifndef MITSDATABASEBACKUP_H
#define MITSDATABASEBACKUP_H

#include <QDialog>
#include "ui_MITSDatabaseBackUp.h"
#include <QMouseEvent>
#include <QPointF>
class MITSDatabaseBackUp : public QDialog
{
	Q_OBJECT

public:
	MITSDatabaseBackUp(QWidget *parent = 0);
	~MITSDatabaseBackUp();

	void InitFileListWidget();
	void doAddListWIdgetItem(QString );

public slots:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void doChooseFile();
	void doUploadFile();
	void doBackUp();
	void doRecover();
	void doDealAction();
	
private:
	Ui::MITSDatabaseBackUp ui;
	bool m_pressed;
	QPoint m_pos;
	QString m_uploadFile;
	QAction *downLoad;
	QAction *deleteList;	

public:
	QStringList m_fileNames;
};

#endif // MITSDATABASEBACKUP_H
