#include "moduleListInstance.h"

ModuleListInstance::ModuleListInstance(QWidget *parent)
   : QObject((QObject*)parent)
{
   mMoveClassIndex = 0;
}

ModuleListInstance::~ModuleListInstance()
{
   clear();
}

void ModuleListInstance::clear()
{
   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      ProjectGenMoveInstance* inst = mMoveClassInstances.at(i);
      delete inst;
   }
   mMoveClassInstances.clear();

   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      delete inst;
   }
   mModuleInstances.clear();

   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleGroupInstances.at(i);
      delete inst;
   }
   mModuleGroupInstances.clear();

   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mProjectDefInstances.at(i);
      delete inst;
   }
   mProjectDefInstances.clear();
}

void ModuleListInstance::buildInstances(ModuleList* list)
{
   // Do Move class
   ModuleMoveClassList* moveList = list->getMoveClassList();
   for(int i=0; i<moveList->count(); ++i)
   {
      MoveClassEntry* entry = moveList->at(i);
      if(entry->mDefaultChoice)
      {
         mMoveClassIndex = i;
      }

      ProjectGenMoveInstance* inst = new ProjectGenMoveInstance();
      inst->mProjectGenItem = entry;
      mMoveClassInstances.push_back(inst);
   }

   // Do module groups
   ModuleGroupList* groups = list->getModuleGroupList();
   for(int i=0; i<groups->count(); ++i)
   {
      ModuleEntry* entry = groups->at(i);

      ProjectGenInstance* inst = new ProjectGenInstance();
      inst->mProjectGenItem = entry;

      for(int j=0; j<entry->mModules.count(); ++j)
      {
         ModuleEntry* mod = entry->mModules.at(j);
         if(mod->mDefaultChoice)
         {
            inst->mGroupIndex = j;
         }
      }

      mModuleGroupInstances.push_back(inst);
   }

   // Do modules
   ModuleClassList* modules = list->getModuleClassList();
   for(int i=0; i<modules->count(); ++i)
   {
      ModuleEntry* entry = modules->at(i);

      ProjectGenInstance* inst = new ProjectGenInstance();
      inst->mProjectGenItem = entry;
      if(entry->mDefaultChoice)
      {
         inst->mState = true;
      }

      mModuleInstances.push_back(inst);
   }

   // Do project defines
   ProjectDefineList* defines = list->getProjectDefineList();
   for(int i=0; i<defines->count(); ++i)
   {
      ProjectDefineEntry* entry = defines->at(i);

      ProjectGenInstance* inst = new ProjectGenInstance();
      inst->mProjectGenItem = entry;
      if(entry->mDefaultChoice)
      {
         inst->mState = true;
      }

      mProjectDefInstances.push_back(inst);
   }
}

void ModuleListInstance::clearData()
{
   mMoveClassIndex = 0;
   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      MoveClassEntry* move = mMoveClassInstances.at(i)->mProjectGenItem;
      if(move->mDefaultChoice)
      {
         mMoveClassIndex = i;
         break;
      }
   }

   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      ProjectGenItem* item = mModuleGroupInstances.at(i)->mProjectGenItem;
      ModuleEntry* group = static_cast<ModuleEntry*>(item);
      mModuleGroupInstances.at(i)->mGroupIndex = group->mDefaultChoice;
   }

   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      mModuleInstances.at(i)->mState = false;
      mModuleInstances.at(i)->mPathData = "";
   }

   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      mProjectDefInstances.at(i)->mState = false;
      mProjectDefInstances.at(i)->mPathData = "";
   }
}

bool ModuleListInstance::readMoveClass(QString& text)
{
   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      ProjectGenMoveInstance* inst = mMoveClassInstances.at(i);
      MoveClassEntry* move = inst->mProjectGenItem;
      if(!move->mName.isEmpty() && text.contains(move->mName, Qt::CaseInsensitive))
      {
         // Found a move class.  Read in its state.
         if(text.contains("true", Qt::CaseInsensitive))
         {
            mMoveClassIndex = i;
         }

         return true;
      }
   }

   return false;
}

bool ModuleListInstance::readModulePath(QString& text)
{
   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);
      if(!module->mPath.isEmpty() && text.contains(module->mPath, Qt::CaseInsensitive))
      {
         // Found a match so retrieve the path data
         QString path = "";
         int start = text.indexOf("\"");
         if(start != -1)
         {
            int end = text.indexOf("\"", start+1);
            if(end != -1)
            {
               path = text.mid(start+1, end-start-1);
            }
         }

         // Store the path
         inst->mPathData = path;

         return true;
      }
   }

   return false;
}

bool ModuleListInstance::readModule(QString& text)
{
   // Try against module groups
   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      ProjectGenInstance* groupInst = mModuleGroupInstances.at(i);
      ModuleEntry* group = static_cast<ModuleEntry*>(groupInst->mProjectGenItem);
      for(int j=0; j<group->mModules.count(); ++j)
      {
         ModuleEntry* module = group->mModules.at(j);
         if(!module->mName.isEmpty() && text.contains(module->mName, Qt::CaseInsensitive))
         {
            // Found a match
            groupInst->mGroupIndex = j;
            return true;
         }
      }
   }

   // Now try individual modules
   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);
      if(text.contains(module->mName, Qt::CaseInsensitive))
      {
         // Found a match
         inst->mState = true;
         return true;
      }
   }

   return false;
}

bool ModuleListInstance::readProjectDefine(QString& text)
{
   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mProjectDefInstances.at(i);
      ProjectDefineEntry* pd = static_cast<ProjectDefineEntry*>(inst->mProjectGenItem);
      if(text.contains(pd->mName, Qt::CaseInsensitive))
      {
         // Found a match
         inst->mState = true;
         return true;
      }
   }

   return false;
}

bool ModuleListInstance::readProjectFile(const QString& path)
{
   clearData();

   mFileSource = path;

   QFile srcFile(path);
   int stage = 0;

   if(srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      while(!srcFile.atEnd())
      {
         QString text = srcFile.readLine();
         QString strippedText = text.simplified();

         if(!strippedText.startsWith("//"))
         {
            if(text.contains("Torque3D::beginConfig(", Qt::CaseInsensitive))
            {
               // We now check for modules and project defines.  On to the next stage.
               stage = 1;
            }
            else if(text.contains("Torque3D::endConfig(", Qt::CaseInsensitive))
            {
               // Done checking for modules and project defines.  On to the next stage.
               stage = 2;
            }
            else if(stage == 0 && strippedText.startsWith("$"))
            {
               // Could be a move class variable or a module path.  Start with
               // a move class.
               bool handled = readMoveClass(text);
               if(!handled)
               {
                  // Not a move class so try a module path.
                  readModulePath(text);
               }
            }
            else if(stage == 1 && text.contains("includeModule("))
            {
               // Handle the module
               readModule(text);
            }
            else if(stage == 1 && text.contains("addProjectDefine("))
            {
               // Handle the project define
               readProjectDefine(text);
            }
         }
      }

      srcFile.close();
   }
   else
   {
      return false;
   }

   return true;
}

void ModuleListInstance::clearWrittenFlags()
{
   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      mMoveClassInstances.at(i)->mWritten = false;
   }

   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      mModuleGroupInstances.at(i)->mWritten = false;
      mModuleGroupInstances.at(i)->mPathWritten = false;
   }

   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      mModuleInstances.at(i)->mWritten = false;
      mModuleInstances.at(i)->mPathWritten = false;
   }

   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      mProjectDefInstances.at(i)->mWritten = false;
      mProjectDefInstances.at(i)->mPathWritten = false;
   }
}

bool ModuleListInstance::handleMoveClass(QString& text)
{
   bool handled = false;

   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      ProjectGenMoveInstance* move = mMoveClassInstances.at(i);
      if(!move->mProjectGenItem->mName.isEmpty())
      {
         // Check if we have a match
         if(text.contains(move->mProjectGenItem->mName))
         {
            // We have a match.  Change its value to what we need.
            if(mMoveClassIndex == i)
            {
               // Switch a 'false' to a 'true'
               text.replace("false", "true", Qt::CaseInsensitive);
            }
            else
            {
               // Switch a 'true' to a 'false'
               text.replace("true", "false", Qt::CaseInsensitive);
            }

            move->mWritten = true;
            handled = true;
            break;
         }
      }
   }

   return handled;
}

bool ModuleListInstance::handleModulePath(QString& text)
{
   bool handled = false;

   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);

      // Does this module even have a path?
      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);
      if(module->mPath.isEmpty())
         continue;

      // Do we have a match
      if(text.contains(module->mPath))
      {
         // Create the new path
         text = module->mPath + " = \"" + inst->mPathData + "\";\n";

         inst->mPathWritten = true;
         handled = true;
         break;
      }
   }

   return handled;
}

void ModuleListInstance::handleModule(QStringList& doc, QString& text)
{
   // Extract the module name starting with single quote format
   QString name = "";
   int start = text.indexOf("'");
   if(start != -1)
   {
      int end = text.indexOf("'", start+1);
      if(end != -1)
      {
         name = text.mid(start+1, end-start-1);
      }
   }
   else
   {
      // Try double quote format
      start = text.indexOf("\"");
      if(start != -1)
      {
         int end = text.indexOf("\"", start+1);
         if(end != -1)
         {
            name = text.mid(start+1, end-start-1);
         }
      }
   }

   if(name.isEmpty())
   {
      // Could not extract the module name??  Just write out what we have
      doc << text;
      return;
   }

   // With the module name, check that this module is even in the list and if so,
   // should it be included.
   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);

      if(module->mName == name)
      {
         // Found the module but should it be written out?
         if(inst->mState)
         {
            // Include the module
            doc << text;
            inst->mWritten = true;
         }

         // We've dealt with this module
         return;
      }
   }

   // If we're here then we couldn't find the module in out stand alone module list.
   // Now on to try the module groups
   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      ProjectGenInstance* groupInst = mModuleGroupInstances.at(i);
      ModuleEntry* moduleGroup = static_cast<ModuleEntry*>(groupInst->mProjectGenItem);

      // Go through all of the group's modules
      for(int j=0; j<moduleGroup->mModules.count(); ++j)
      {
         ModuleEntry* module = static_cast<ModuleEntry*>(moduleGroup->mModules.at(j));
         if(module->mName == name)
         {
            // Found the module but should it be written out?
            if(groupInst->mGroupIndex == j)
            {
               // Include the module
               doc << text;
               groupInst->mWritten = true;
            }

            // We've dealt with this module
            return;
         }
      }
   }

   // If we're here then the module is not part of out list.  It likely is a
   // manually entered module.  Write it out.
   doc << text;
}

void ModuleListInstance::handleProjectDefine(QStringList& doc, QString& text)
{
   // Extract the define name starting with single quote format
   QString name = "";
   int start = text.indexOf("'");
   if(start != -1)
   {
      int end = text.indexOf("'", start+1);
      if(end != -1)
      {
         name = text.mid(start+1, end-start-1);
      }
   }
   else
   {
      // Try double quote format
      start = text.indexOf("\"");
      if(start != -1)
      {
         int end = text.indexOf("\"", start+1);
         if(end != -1)
         {
            name = text.mid(start+1, end-start-1);
         }
      }
   }

   if(name.isEmpty())
   {
      // Could not extract the define name??  Just write out what we have
      doc << text;
      return;
   }

   // With the define name, check that this module is even in the list and if so,
   // should it be included.
   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mProjectDefInstances.at(i);
      ProjectDefineEntry* pd = static_cast<ProjectDefineEntry*>(inst->mProjectGenItem);

      if(pd->mName == name)
      {
         // Found the define but should it be written out?
         if(inst->mState)
         {
            // Include the define
            doc << text;
            inst->mWritten = true;
         }

         // We've dealt with this define
         return;
      }
   }

   // If we're here then the define is not part of our list.  It likely is a
   // manually entered project define.  Write it out.
   doc << text;
}

void ModuleListInstance::writeUnhandledMoveClasses(QStringList& doc)
{
   for(int i=0; i<mMoveClassInstances.count(); ++i)
   {
      ProjectGenMoveInstance* move = mMoveClassInstances.at(i);
      if(move->mWritten || move->mProjectGenItem->mName.isEmpty())
         continue;

      QString text = move->mProjectGenItem->mName + " = " + ((mMoveClassIndex == i) ? "true" : "false") + ";\n";
      doc << text;

      move->mWritten = true;
   }
}

void ModuleListInstance::writeUnhandledModulePaths(QStringList& doc)
{
   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      if(inst->mPathWritten)
         continue;

      // Does this module even have a path and if so is the module even selected?
      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);
      if(module->mPath.isEmpty() || !inst->mState)
         continue;

      // Create the new path
      QString text = module->mPath + " = \"" + inst->mPathData + "\";\n";
      doc << text;

      inst->mPathWritten = true;
   }
}

void ModuleListInstance::writeUnhandledModules(QStringList& doc)
{
   // Begin with the grouped modules
   for(int i=0; i<mModuleGroupInstances.count(); ++i)
   {
      ProjectGenInstance* groupInst = mModuleGroupInstances.at(i);
      if(groupInst->mWritten)
         continue;

      ModuleEntry* moduleGroup = static_cast<ModuleEntry*>(groupInst->mProjectGenItem);

      // Write out the chosen module, but only if it has a name
      QString name = moduleGroup->mModules.at(groupInst->mGroupIndex)->mName;
      if(!name.isEmpty())
      {
         QString text = "    includeModule( '" + name + "' );\n";
         doc << text;
      }

      // Indicate that the group was written out, name or not
      groupInst->mWritten = true;
   }

   // Now the ungrouped modules
   for(int i=0; i<mModuleInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mModuleInstances.at(i);
      if(inst->mWritten)
         continue;

      ModuleEntry* module = static_cast<ModuleEntry*>(inst->mProjectGenItem);

      if(inst->mState)
      {
         QString text = "    includeModule( '" + module->mName + "' );\n";
         doc << text;
         inst->mWritten = true;
      }
   }
}

void ModuleListInstance::writeUnhandledProjectDefines(QStringList& doc)
{
   for(int i=0; i<mProjectDefInstances.count(); ++i)
   {
      ProjectGenInstance* inst = mProjectDefInstances.at(i);
      if(inst->mWritten)
         continue;

      ProjectDefineEntry* pd = static_cast<ProjectDefineEntry*>(inst->mProjectGenItem);

      if(inst->mState)
      {
         QString text = "    addProjectDefine( '" + pd->mName + "' );\n";
         doc << text;
         inst->mWritten = true;
      }
   }
}

bool ModuleListInstance::replaceProjectFileContents(const QString& file)
{
   clearWrittenFlags();

   QStringList doc;
   QFile srcFile(file);
   int stage = 0;

   if(srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      while(!srcFile.atEnd())
      {
         QString text = srcFile.readLine();
         QString strippedText = text.simplified();

         if(!strippedText.startsWith("//"))
         {
            if(text.contains("Torque3D::beginConfig(", Qt::CaseInsensitive))
            {
               // We need to have written all move classes and module paths by now
               writeUnhandledMoveClasses(doc);
               writeUnhandledModulePaths(doc);

               // Now write out the beginConfig() line
               doc << text;

               // Next stage
               stage = 1;
            }
            else if(text.contains("Torque3D::endConfig(", Qt::CaseInsensitive))
            {
               // We need to have written all modules by now
               writeUnhandledModules(doc);
               writeUnhandledProjectDefines(doc);

               // Now write out the endConfig() line
               doc << text;

               // Next stage
               stage = 2;
            }
            else if(stage == 0 && strippedText.startsWith("$"))
            {
               // Prior to the beginConfig() stage.  Here we may find move classes and module paths.
               bool handled = false;

               // Start by checking for a move class
               handled = handleMoveClass(text);

               // Should we try to handle a module path?
               if(!handled)
               {
                  handled = handleModulePath(text);
               }

               doc << text;
            }
            else if(stage == 1 && text.contains("includeModule("))
            {
               // Deal with the module
               handleModule(doc, text);
            }
            else if(stage == 1 && text.contains("addProjectDefine("))
            {
               // Deal with the project define
               handleProjectDefine(doc, text);
            }
            else
            {
               // Just write out the line
               doc << text;
            }
         }
         else
         {
            doc << text;
         }
      }

      srcFile.close();
   }
   else
   {
      return false;
   }

   // Write out the new file
   if(srcFile.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      for(int i=0; i<doc.count(); ++i)
      {
         srcFile.write(doc.at(i).toLocal8Bit());
      }

      srcFile.close();
   }
   else
   {
      return false;
   }

   return true;
}
