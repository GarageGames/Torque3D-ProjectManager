#ifndef NEWPROJECTPAGE_H
#define NEWPROJECTPAGE_H

#include <QtGui>
#include "ui_NewProjectPage.h"

using namespace Ui;

class Torque3DFrontloader;
class TemplateEntry;
class ModuleListInstance;

class NewProjectPage : public QWidget, public NewProjectPageClass
{
   Q_OBJECT

private:
   Torque3DFrontloader *mFrontloader;
   QMap<QString, TemplateEntry*> mTemplateNameList;
   ModuleListInstance* mCurrentInstance;

public:
   NewProjectPage(QWidget *parent = 0);
   ~NewProjectPage();

   void launch();
   void setFrontloader(Torque3DFrontloader *frontloader);
   void setDefaults();
   QString getValidProjectName(QString projectName);

public slots:
   void buildTemplateList();
   void on_newChooseModulesButton_clicked();

private slots:
   void on_newProjectCreateButton_clicked();
   void on_TemplateList_textChanged(const QString &text);
};

#endif // NEWPROJECTPAGE_H
