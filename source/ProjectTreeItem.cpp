#include "ProjectTreeItem.h"

ProjectTreeItem::ProjectTreeItem(QWidget *parent)
   : QFrame(parent)
{
   setupUi(this);
   setMinimumSize(size());

   //ProjectSize->setVisible(false);
   ProjectSizeValue->setVisible(false);
}

ProjectTreeItem::~ProjectTreeItem()
{
}

void ProjectTreeItem::setTitleName(const QString &name)
{
   ProjectName->setText(name);
}

QString ProjectTreeItem::getTitleName()
{
   return ProjectName->text();
}

void ProjectTreeItem::setUniqueName(const QString &name)
{
   mUniqueName = QString(name);
}

QString ProjectTreeItem::getUniqueName()
{
   return mUniqueName;
}

void ProjectTreeItem::setSize(const QString &size)
{
   ProjectSizeValue->setText(size);
}

void ProjectTreeItem::setLastUpdate(const QString &lastUpdate)
{
   ProjectLastUpdateValue->setText(lastUpdate);
}

void ProjectTreeItem::setImage(QPixmap *pixmap)
{
   QPixmap scaledPixmap = pixmap->scaled(ProjectImage->size());
   ProjectImage->setPixmap(scaledPixmap);
}

QLabel *ProjectTreeItem::getTextObj()
{
   return ProjectName;
}