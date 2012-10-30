#include "ProjectTree.h"
#include <QLabel>
#include "torque3dfrontloader.h"

ProjectTree::ProjectTree(QWidget *parent)
   : QWidget(parent)
{
   mFrontloader = NULL;
   mProjectList = NULL;
   mCurrentItem = NULL;
   mNormalColor.setRgb(255, 255, 255);

   mBaseLayout = new QVBoxLayout(this);
   mBaseLayout->setSpacing(0);
   mBaseLayout->setMargin(0);
   mBaseLayout->setContentsMargins(0, 0, 0, 0);

   mScrollArea = new ProjectScrollArea();
   mScrollArea->setWidgetResizable(true);
   mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

   mBaseLayout->addWidget(mScrollArea);

   mContent = new QWidget(this);  
   mContentLayout = new QVBoxLayout(mContent);
   mContentLayout->setSpacing(0);
   mContentLayout->setMargin(0);
   mContentLayout->setContentsMargins(0, 0, 0, 0);
   mScrollArea->setWidget(mContent);
   mContent->setObjectName("ProjectScrollContent");
}

ProjectTree::~ProjectTree()
{
}

void ProjectTree::setFrontloader(Torque3DFrontloader *frontloader)
{
   mFrontloader = frontloader;
}

void ProjectTree::setProjectList(ProjectList *projectList)
{
   mProjectList = projectList;
   mProjectList->setParent(this);

   connect(mProjectList, SIGNAL(projectEntryRemoved(ProjectEntry *)), this, SLOT(projectEntryRemoved(ProjectEntry *)));
   connect(mProjectList, SIGNAL(projectEntryAdded(ProjectEntry *)), this, SLOT(projectEntryAdded(ProjectEntry *)));
   connect(mProjectList, SIGNAL(projectCategoryAdded(QString)), this, SLOT(projectCategoryAdded(QString)));
   connect(mProjectList, SIGNAL(projectCategoryRemoved(QString)), this, SLOT(projectCategoryRemoved(QString)));  
}

int ProjectTree::addFolder(QString folder)
{
   // create our base folder widget
   QWidget *widget = new QWidget();
   QVBoxLayout *layout = new QVBoxLayout();
   widget->setLayout(layout);

   // create the folder label with the text passed
   QLabel *label = new QLabel(folder);
   
   // add our label
   layout->addWidget(label);
   
   // create the container widget that will hold this folders items
   QWidget *subWidget = new QWidget();
   QVBoxLayout *subLayout = new QVBoxLayout();
   subWidget->setLayout(subLayout);

   layout->addWidget(subWidget);

   int index = mContentLayout->count();
   mContentLayout->addWidget(widget);

   return index;
}

void ProjectTree::addItem(int folderId, ProjectTreeItem *item)
{
   QLayoutItem *layoutItem = mContentLayout->itemAt(folderId);
   
   if(layoutItem != NULL)
   {
      QWidget *widget = layoutItem->widget();
   
      if(widget != NULL)
      {
         QLayout *layout = widget->layout();
         QWidget *subWidget = layout->itemAt(1)->widget();

         if(subWidget != NULL)
         {
            QLayout *subLayout = subWidget->layout();
            subLayout->addWidget(item);
         }
      }
   }
}

void ProjectTree::setupList()
{
   if(mProjectList == NULL)
      return;

   bool restoreCurrentItem = false;
   bool currentItemFound = false;
   ProjectTreeItem *newCurrent = NULL;
   QString currentUnique;
   if(mCurrentItem != NULL)
   {
      restoreCurrentItem = true;
      currentUnique.append(mCurrentItem->getUniqueName());
      mCurrentItem = NULL;
   }

   QLayoutItem *child;
   while ((child = mContentLayout->takeAt(0)) != 0)
   {
      delete child->widget();
      delete child;
   }

   mProjectAppList.clear();
   mProjectItemList.clear();

   // build the project list
   mProjectList->buildList();
   
   // grab the appropriate data from the ProjectList
   QList<QString> *nameList = mProjectList->getProjectDirNameList();
   QMultiMap<QString, ProjectEntry*> *dirList = mProjectList->getProjectDirectoryList();

   // loop through all of the directory entries first
   for(int i=0;i < nameList->size(); i++)
   {
      QString name = nameList->at(i);
      QList<ProjectEntry*> list = dirList->values(name);
      
      if(name.compare("Templates") != 0)
      {
         int folderIndex;

         if(list.size() > 0)
            folderIndex = addFolder(name);

         for(int j=0;j < list.size();j++)
         {
            ProjectEntry *entry = list.at(j);
		 
            QString uniqueName = name + "-" + entry->mName;
            mProjectAppList.insert(uniqueName, entry);
		 
            if(mProjectAppList.count(uniqueName) == 1)
            {
               // an entry for this project doesn't already exist so lets add one
               ProjectTreeItem *item = new ProjectTreeItem();

               QFileInfo info(entry->mRootPath);

               item->setTitleName(entry->mName);
               item->setUniqueName(uniqueName);
               item->setSize(QString().setNum(0));
               item->setLastUpdate(info.lastModified().toString(QString("ddd MMM d hh:mm ap")));

               if(restoreCurrentItem && !currentItemFound)
               {
                 
                  if(currentUnique.compare(uniqueName) == 0)
                  {
                     currentItemFound = true;
                     newCurrent = item;    
                  }
               }
			
               if(mFrontloader != NULL)
               {
                  QPixmap *pixmap = mFrontloader->getProjectPixmap(entry);
                  if(pixmap != NULL)
                  {
                     item->setImage(pixmap);
                  }
               }

               mProjectItemList.insert(uniqueName, item);
               addItem(folderIndex, item);
            }
         }
      }
   }

   if(restoreCurrentItem && currentItemFound)
   {
      setSelected(newCurrent);
   } 
   else if(restoreCurrentItem)
   {
      emit projectRemovalDone();
   }

   mContentLayout->addStretch();
}

void ProjectTree::itemClicked(ProjectTreeItem *item)
{
   if(item != NULL && item != mCurrentItem)
   {
      // A project tree item is selected, lets emit a signal for it
      QList<ProjectEntry*> list = mProjectAppList.values(item->getUniqueName());
      QString titleName = item->getTitleName();

      ProjectEntry *entry = NULL;
      for(int i=0; i<list.size(); i++)
      {
         entry = list.at(i);
          
         if(entry->mName.compare(titleName) == 0)
         {
            break;
         }
         else
         {
            entry = NULL;
         }
      }

      if(entry != NULL)
      {
         emit projectSelected(entry);
         setSelected(item);
      }
   }
}

void ProjectTree::setSelected(ProjectTreeItem *item)
{
   if(mCurrentItem != NULL)
   {
      mCurrentItem->setStyleSheet("");
   }
   
   item->setStyleSheet("ProjectTreeItem {border:1px solid rgb(0, 44, 67); background-color: qlineargradient(spread:repeat, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(223, 245, 255, 255), stop:0.49 rgba(252, 254, 255, 255), stop:0.5 rgba(208, 233, 245, 255), stop:1 rgba(239, 250, 255, 255))}");

   mCurrentItem = item;
}

void ProjectScrollArea::mousePressEvent(QMouseEvent *pressEvent)
{
   if (pressEvent->button() == Qt::LeftButton) 
   {
      QWidget *widget = childAt(pressEvent->pos());
      ProjectTree *tree = dynamic_cast<ProjectTree*>(parent());
      if(tree != NULL)
      {
         ProjectTreeItem *item = dynamic_cast<ProjectTreeItem*>(widget);
         if(item == NULL)
            item = dynamic_cast<ProjectTreeItem*>(widget->parent());

         if(item != NULL)
         {
            // A project tree item is clicked
            tree->itemClicked(item);
         }
      }
   }
}

void ProjectTree::projectEntryRemoved(ProjectEntry *entry)
{
}

void ProjectTree::projectEntryAdded(ProjectEntry *entry)
{
}

void ProjectTree::projectCategoryAdded(QString title)
{
}

void ProjectTree::projectCategoryRemoved(QString title)
{
}

ProjectEntry *ProjectTree::getEntryFromAppName(QString uniqueName, QString appName)
{
   QList<ProjectEntry*> entryList = mProjectAppList.values(uniqueName);
   for(int i=0; i<entryList.size(); i++)
   {
      ProjectEntry *entry = entryList.at(i);
      QString fileName = QFileInfo(entry->mPath).fileName();
      if(fileName.compare(appName) == 0)
      {
         return entry;
      }
   }

   return NULL;
}
