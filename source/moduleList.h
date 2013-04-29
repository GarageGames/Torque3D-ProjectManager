#ifndef _MODULE_LIST_H_
#define _MODULE_LIST_H_

#include <QtGui>
#include <QtCore>
#include <QtXml/QDomDocument>

struct MoveClassEntry
{
   QString  mName;
   QString  mDescription;
   bool     mDoNotWrite;
   bool     mDefaultChoice;

   MoveClassEntry() { mDoNotWrite = false; mDefaultChoice = false; }
};

struct ProjectGenItem
{
   QString  mDescription;
   bool     mDefaultChoice;

   ProjectGenItem() { mDefaultChoice=false; }
   virtual ~ProjectGenItem() {}
};

struct ModuleEntry : public ProjectGenItem
{
   enum Types
   {
      TypeModule,
      TypeModuleGroup,
   };

   Types    mType;

   QString  mName;
   QString  mPath;
   bool     mDoNotWrite;

   // For module group
   QList<ModuleEntry*> mModules;

   ModuleEntry() { mDoNotWrite = false; }

   virtual ~ModuleEntry()
   {
      for(int i=0; i<mModules.size(); ++i)
      {
         ModuleEntry* module = mModules.at(i);
         delete module;
      }
      mModules.clear();
   }
};

struct ProjectDefineEntry : public ProjectGenItem
{
   QString mName;
};

class ModuleList : public QObject
{
   Q_OBJECT

protected:
   QString  mAppName;
   QWidget* mParent;

   QList<ModuleEntry*> mModules;
   QList<ModuleEntry*> mModuleGroups;
   QList<MoveClassEntry*> mMoveClasses;
   QList<ProjectDefineEntry*> mProjectDefines;

   ModuleEntry* createModule(QDomElement& e);
   ModuleEntry* createModuleGroup(QDomElement& e);

public:
   ModuleList(QWidget *parent = NULL);
   virtual ~ModuleList();

   void setAppName(QString name);
   void setParent(QWidget *parent);

   QList<MoveClassEntry*>* getMoveClassList() { return &mMoveClasses; }
   QList<ModuleEntry*>* getModuleClassList() { return &mModules; }
   QList<ModuleEntry*>* getModuleGroupList() { return &mModuleGroups; }
   QList<ProjectDefineEntry*>* getProjectDefineList() { return &mProjectDefines; }

public slots:
   void buildList();
};

typedef QList<MoveClassEntry*> ModuleMoveClassList;
typedef QList<ModuleEntry*> ModuleClassList;
typedef QList<ModuleEntry*> ModuleGroupList;
typedef QList<ProjectDefineEntry*> ProjectDefineList;

#endif   // _MODULE_LIST_H_
