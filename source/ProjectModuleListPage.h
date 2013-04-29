#ifndef PROJECTMODULELISTPAGE_H
#define PROJECTMODULELISTPAGE_H

#include <QtGui>
#include "ui_ProjectModuleList.h"

using namespace Ui;

class Torque3DFrontloader;
class ProjectEntry;
struct ProjectGenItem;
struct ModuleEntry;
class ModuleListInstance;

//-----------------------------------------------------------------------------

class DirectorySelectButton : public QPushButton
{
   Q_OBJECT

public:
   DirectorySelectButton(QWidget *parent=0);
   virtual ~DirectorySelectButton() {}

public slots:
   void chooseDirectory();
};

//-----------------------------------------------------------------------------

class ProjectModuleListPage : public QWidget, public ProjectModuleListClass
{
   Q_OBJECT

protected:
   Torque3DFrontloader *mFrontloader;
//   QMap<QString, ProjectEntry*> mTemplateNameList;

   QList<QButtonGroup*> mModuleGroupButtons;
   QButtonGroup*        mModuleButtons;
   QButtonGroup*        mProjectDefineButtons;

   QWidget*       mScrollContent;
   QVBoxLayout*   mScrollContentLayout;
   QScrollArea*   mModuleScroll;
   QWidget*       mModuleContent;
   QVBoxLayout*   mModuleContentLayout;

   ModuleListInstance* mCurrentInstance;

   void addProjectGenItem(QLayout* contentLayout, ProjectGenItem* item, int index, QButtonGroup* buttonGroup, const QString& path, bool checked, bool isRadio=false);

   void copyToModuleListInstance();

public:
   ProjectModuleListPage(QWidget *parent = 0);
   ~ProjectModuleListPage();

   void launch(ModuleListInstance* instance, bool regenerate);
   void setFrontloader(Torque3DFrontloader *frontloader);
   void populateControls();
//   void setDefaults();
//   QString getValidProjectName(QString projectName);

public slots:
//   void buildTemplateList();

private slots:
//   void on_newProjectCreateButton_clicked();
//   void on_TemplateList_textChanged(const QString &text);
   void on_ProjectModuleListRegenButton_clicked();
   void on_ProjectModuleListOKButton_clicked();
};

#endif   // PROJECTMODULELISTPAGE_H
