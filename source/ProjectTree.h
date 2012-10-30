#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include "ProjectTreeItem.h"
#include "projectList.h"

// forward declarations
class Torque3DFrontloader;

class ProjectScrollArea : public QScrollArea
{
   Q_OBJECT

public:
   void scrollBy(int x, int y) { scrollContentsBy(x, y); };

protected:
   void mousePressEvent(QMouseEvent *pressEvent);
};

class ProjectTree : public QWidget
{
   Q_OBJECT

private:
   Torque3DFrontloader *mFrontloader;
   ProjectList *mProjectList;

public:
   ProjectTree(QWidget *parent);
   ~ProjectTree();

   void setFrontloader(Torque3DFrontloader *frontloader);

   ProjectTreeItem *mCurrentItem;
   QWidget *mContent;
   ProjectScrollArea *mScrollArea;
   QVBoxLayout *mBaseLayout;
   QVBoxLayout *mContentLayout;
   QMultiMap<QString, ProjectEntry*>           mProjectAppList;
   QMultiMap<QString, ProjectTreeItem*>        mProjectItemList;
   QColor mNormalColor;

   void setupList();
   int addFolder(QString folder);
   void addItem(int folderId, ProjectTreeItem *item);
   void itemClicked(ProjectTreeItem *item);
   void setSelected(ProjectTreeItem *item);

   void setProjectList(ProjectList *projectList);

   ProjectEntry *getEntryFromAppName(QString uniqueName, QString appName);

signals:
   void projectSelected(ProjectEntry *entry);
   void projectRemovalDone();

public slots:
   void projectEntryRemoved(ProjectEntry *entry);
   void projectEntryAdded(ProjectEntry *entry);
   void projectCategoryAdded(QString title);
   void projectCategoryRemoved(QString title);
};

#endif // PROJECTTREE_H
