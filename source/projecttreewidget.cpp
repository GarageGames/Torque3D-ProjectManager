#include "projecttreewidget.h"

ProjectTreeWidget::ProjectTreeWidget(QWidget *parent)
   : QTreeWidget(parent)
{
   mProjectList = NULL;
}

ProjectTreeWidget::~ProjectTreeWidget()
{
}

void ProjectTreeWidget::setProjectList(ProjectList *projectList)
{
   mProjectList = projectList;
   mProjectList->setParent(this);
}

void ProjectTreeWidget::setupList()
{
   if(mProjectList == NULL)
      return;

   // Set up the project tree
   clear();
   setColumnCount(2);
   QStringList headers;
   headers.append("Name");
   headers.append("Path");
   setHeaderLabels(headers);
   resizeProjectTree();

   mProjectList->buildList();
   QList<QString> *nameList = mProjectList->getProjectDirNameList();
   QMultiMap<QString, ProjectEntry*> *dirList = mProjectList->getProjectDirectoryList();
   QList<ProjectEntry*> *fileList = mProjectList->getProjectFileList();

   // loop through all of the directory entries first
   for(int i=0;i < nameList->size(); i++)
   {
      QString name = nameList->at(i);
      QList<ProjectEntry*> list = dirList->values(name);
      QTreeWidgetItem* top = new QTreeWidgetItem(QStringList(nameList->at(i)));

      ProjectTreeWidgetItem* project = NULL;
      for(int j=0;j < list.size();j++)
      {
         ProjectEntry *entry = list.at(j);
         QString name = entry->mName;

         QFileInfo file(entry->mPath);
         QString nameString = file.fileName();
         project = new ProjectTreeWidgetItem(top);
         project->setText(0, entry->mName);
         project->setText(1, nameString);
         project->mIsDir = true;
         project->mIndex = j;
         
         if(entry->mArgs.compare("") != 0)
         {
            QVariant data(entry->mArgs);
            project->setData(0, Qt::UserRole, data);
         }
      }

      addTopLevelItem(top);
   }

   // then loop through all of the file specific entries
   for(int i=0;i<fileList->size();i++)
   {
      ProjectEntry *entry = fileList->at(i);

      ProjectTreeWidgetItem* fileItem = new ProjectTreeWidgetItem();
      fileItem->setText(0, entry->mName);
      fileItem->setText(1, QFile(entry->mPath).fileName());
      fileItem->mIsDir = false;
      fileItem->mIndex = i;

      // Check for command line arguments
      if(entry->mArgs.compare("") != 0)
      {
         QVariant data(entry->mArgs);
         fileItem->setData(0, Qt::UserRole, data);
      }

      addTopLevelItem(fileItem);
   }

   // Finish up
   expandAll();
   resizeProjectTree();
}

void ProjectTreeWidget::resizeProjectTree()
{
   resizeColumnToContents(1);
   resizeColumnToContents(0);
}

void ProjectTreeWidget::testEditor()
{
#ifdef Q_WS_WIN
   
   //Q_PID 
   _PROCESS_INFORMATION *pi = mProjectList->mProcess->pid();
   EnumWindows(&ProjectList::EnumWindowsProc, pi->dwThreadId);
   
#endif

   QTimer::singleShot(8000, this, SLOT(testEditorToggle()));
}

void ProjectTreeWidget::testEditorToggle()
{
   mProjectList->toggleEditor(0);
}