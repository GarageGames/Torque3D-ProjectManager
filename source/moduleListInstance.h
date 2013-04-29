#ifndef _MODULE_LIST_INSTANCE_H_
#define _MODULE_LIST_INSTANCE_H_

#include <QtCore>
#include "moduleList.h"

struct ProjectGenInstance
{
   ProjectGenItem* mProjectGenItem;

   // Instance data
   bool     mState;
   int      mGroupIndex;
   QString  mPathData;
   bool     mWritten;
   bool     mPathWritten;

   ProjectGenInstance()
   {
      mProjectGenItem = NULL;
      mState = false;
      mGroupIndex = 0;
      mWritten = false;
      mPathWritten = false;
   }
};

struct ProjectGenMoveInstance
{
   MoveClassEntry* mProjectGenItem;

   // Instance data
   bool mWritten;

   ProjectGenMoveInstance()
   {
      mProjectGenItem = NULL;
      mWritten = false;
   }
};

class ModuleListInstance : public QObject
{
   Q_OBJECT

protected:
   void clearWrittenFlags();

   bool handleMoveClass(QString& text);
   bool handleModulePath(QString& text);
   void handleModule(QStringList& doc, QString& text);
   void handleProjectDefine(QStringList& doc, QString& text);

   void writeUnhandledMoveClasses(QStringList& doc);
   void writeUnhandledModulePaths(QStringList& doc);
   void writeUnhandledModules(QStringList& doc);
   void writeUnhandledProjectDefines(QStringList& doc);

public:
   int mMoveClassIndex;
   QList<ProjectGenMoveInstance*> mMoveClassInstances;
   QList<ProjectGenInstance*> mModuleInstances;
   QList<ProjectGenInstance*> mModuleGroupInstances;
   QList<ProjectGenInstance*> mProjectDefInstances;

public:
   ModuleListInstance(QWidget *parent = NULL);
   virtual ~ModuleListInstance();

   void clear();

   void buildInstances(ModuleList* list);

   bool replaceProjectFileContents(const QString& file);
};

#endif   // _MODULE_LIST_INSTANCE_H_
