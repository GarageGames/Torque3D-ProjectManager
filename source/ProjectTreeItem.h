#ifndef PROJECTTREEITEM_H
#define PROJECTTREEITEM_H

#include <QWidget>
#include "ui_ProjectTreeItem.h"

using namespace Ui;

class ProjectTreeItem : public QFrame, public ProjectTreeItemClass
{
   Q_OBJECT

public:
   ProjectTreeItem(QWidget *parent = 0);
   ~ProjectTreeItem();

   QString mUniqueName;

   void setTitleName(const QString &name);
   void setUniqueName(const QString &name);
   void setSize(const QString &size);
   void setLastUpdate(const QString &lastUpdate);
   QString getTitleName();
   QString getUniqueName();
   void setImage(QPixmap *pixmap);
   QLabel *getTextObj();
};

#endif // PROJECTTREEITEM_H
