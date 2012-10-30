#ifndef PROJECTTREEWIDGET_H
#define PROJECTTREEWIDGET_H

#include <QTreeWidget>
#include "projectList.h"


class ProjectTreeWidgetItem : public QTreeWidgetItem
{	   
public:
   ProjectTreeWidgetItem() {};
   ProjectTreeWidgetItem(QTreeWidgetItem *parent) : QTreeWidgetItem(parent) {};
   int mIndex;
   bool mIsDir;
};

class ProjectTreeWidget : public QTreeWidget
{
   Q_OBJECT

public:
   ProjectTreeWidget(QWidget *parent);
   ~ProjectTreeWidget();

   void setProjectList(ProjectList *projectList);
   void setupList();
   void resizeProjectTree();

public slots:
   void testEditor();
   void testEditorToggle();


private:
   ProjectList *mProjectList;
};

#endif // PROJECTTREEWIDGET_H
