#ifndef _PROJECT_LIST_H_
#define _PROJECT_LIST_H_

#include <QtGui>
#include <QtCore>

#ifdef Q_WS_WIN
#include <comdef.h>
#endif

class ProjectEntry
{
public:
   QString    mPath;
   QString    mName;
   QString    mRootName;
   QString    mRootPath;
   QString    mArgs;

   QString getUniqueName() { return QString(mRootName + "-" + mName); };
   QString getAppName() { return QFileInfo(mPath).fileName(); };
   QString getAppPath();
   QString getLevelPath();
};

/* ProjectList

   This will get a list of projects and demos based on the specified .xml file...
   Here is an example of how to go through the valid data

   ProjectList projectList;
   projectList.buildList();
   QList<QString> *nameList = projectList.getProjectDirNameList();
   QMultiMap<QString, ProjectEntry*> *dirList = projectList.getProjectDirectoryList();
   QList<ProjectEntry*> *fileList = projectList.getProjectFileList();

   // loop through all of the directory entries first
   for(int i=0;i < nameList->size(); i++)
   {
	  QString name = nameList->at(i);
	  QList<ProjectEntry*> list = dirList->values(name);
      
      ...

	  for(int j=0;j < list.size();j++)
	  {
		 ProjectEntry *entry = list.at(j);
	     ...
         
		 if(entry->mArgs.compare("") != 0)
         {
            QVariant data(entry->mArgs);
            project->setData(0, Qt::UserRole, data);
         }
	  }

	  ProjectTree->addTopLevelItem(top);
   }

   // then loop through all of the file specific entries
   for(int i=0;i<fileList->size();i++)
   {
      ProjectEntry *entry = fileList->at(i);

	  ...

	  // Check for command line arguments
	  if(entry->mArgs.compare("") != 0)
      {
		 QVariant data(entry->mArgs);
		 fileItem->setData(0, Qt::UserRole, data);
	  }

	  ProjectTree->addTopLevelItem(fileItem);
   }

*/

class ProjectList : public QObject
{
   Q_OBJECT

public:
#ifdef Q_WS_WIN
   static HWND appWindow;
#endif

   ProjectList(QWidget *parent = NULL);

   void setAppName(QString name);
   void setParent(QWidget *parent);

   QProcess                               *mProcess;

   QList<QString>                         mProjectDirNameList;
   QMultiMap<QString, ProjectEntry*>      mProjectDirectoryList;
   QList<ProjectEntry*>                   mProjectFileList;

   QList<QString>                         *getProjectDirNameList() { return &mProjectDirNameList; };
   QMultiMap<QString, ProjectEntry*>      *getProjectDirectoryList() { return &mProjectDirectoryList; };
   QList<ProjectEntry*>                   *getProjectFileList() { return &mProjectFileList; };

   QMultiMap<QString, ProjectEntry*>      mTemplateDirectoryList;
   QMultiMap<QString, ProjectEntry*>      *getTemplateDirectoryList() { return &mTemplateDirectoryList; };

   void toggleEditor(int editorMode);
   ProjectEntry *getFirstProjectEntry();

   static QString getAppPath(QString path = "");
	  
#ifdef Q_WS_WIN
   static BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam );
#endif
private:
   QString                    mAppName;
   QWidget                    *mParent;

public slots:
   void buildList();
   void appStandardData();
   void appErrorData();
   void appFinished();
   void appStateChanged(QProcess::ProcessState newState);

signals:
   void projectEntryRemoved(ProjectEntry *entry);
   void projectRemovalDone();
   void projectEntryAdded(ProjectEntry *entry);
   void projectCategoryAdded(QString title);
   void projectCategoryRemoved(QString title);
   void minimizeApp();
   void maximizeApp();
   void hideApp();
};

#endif
