#include "moduleList.h"
#include "projectList.h"

ModuleList::ModuleList(QWidget *parent)
   : QObject(parent)
{
   mAppName =  "No app name has been set";
   mParent  =  NULL;
}

ModuleList::~ModuleList()
{
   for(int i=0; i<mModules.size(); ++i)
   {
      ModuleEntry* module = mModules.at(i);
      delete module;
   }
   mModules.clear();

   for(int i=0; i<mModuleGroups.size(); ++i)
   {
      ModuleEntry* module = mModuleGroups.at(i);
      delete module;
   }
   mModuleGroups.clear();

   for(int i=0; i<mMoveClasses.size(); ++i)
   {
      MoveClassEntry* mc = mMoveClasses.at(i);
      delete mc;
   }
   mMoveClasses.clear();

   for(int i=0; i<mProjectDefines.size(); ++i)
   {
      ProjectDefineEntry* pd = mProjectDefines.at(i);
      delete pd;
   }
   mProjectDefines.clear();
}

void ModuleList::setAppName(QString name)
{
   mAppName = name;
}

void ModuleList::setParent(QWidget *parent)
{
   mParent = parent;
}

ModuleEntry* ModuleList::createModule(QDomElement& e)
{
   if(!e.hasAttribute("name"))
      return NULL;

   ModuleEntry* module = new ModuleEntry();
   module->mType = ModuleEntry::TypeModule;
   module->mName = e.attribute("name");
   module->mDescription = e.text();

   if(e.hasAttribute("path"))
   {
      module->mPath = e.attribute("path");
   }

   if(e.hasAttribute("donotwrite"))
   {
      module->mDoNotWrite = e.attribute("donotwrite").toInt();
   }

   if(e.hasAttribute("default"))
   {
      module->mDefaultChoice = e.attribute("default").toInt();
   }

   return module;
}

ModuleEntry* ModuleList::createModuleGroup(QDomElement& e)
{
   if(!e.hasAttribute("description"))
      return NULL;

   ModuleEntry* moduleGroup = new ModuleEntry();
   moduleGroup->mType = ModuleEntry::TypeModuleGroup;
   moduleGroup->mDescription = e.attribute("description");

   // Process all modules in the group
   QDomNode nModule = e.firstChild();
   while(!nModule.isNull())
   {
      QDomElement eModule = nModule.toElement();
      if(!eModule.isNull() && eModule.tagName() == "module")
      {
         // Work with a module
         ModuleEntry* module = createModule(eModule);
         if(module)
         {
            moduleGroup->mModules.push_back(module);
         }
      }

      nModule = nModule.nextSibling();
   }

   return moduleGroup;
}

void ModuleList::buildList()
{
   // Load in the xml file, parse it, and build out the controls
   QDomDocument doc("projects");

   QString baseAppPath = ProjectList::getAppPath();
   QString projectsPath = baseAppPath + "projects.xml";
   QFile file(projectsPath);
      
   if(!file.exists())
      file.setFileName("projects.xml");

   if (!file.open(QIODevice::ReadOnly))
   {
      QMessageBox::critical(mParent, mAppName,
         "Unable to find projects.xml file.",
         QMessageBox::Ok,
         QMessageBox::Ok);
      return;
   }

   if (!doc.setContent(&file))
   {
      file.close();
      QMessageBox::critical(mParent, mAppName,
         "Unable to find projects.xml file.",
         QMessageBox::Ok,
         QMessageBox::Ok);
      return;
   }

   file.close();

   // Process
   QDomElement docElem = doc.documentElement();
   QDomNode n = docElem.firstChild();
   while(!n.isNull())
   {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if(!e.isNull() && e.tagName() == "entry")
      {
         if(e.hasAttribute("type") && e.attribute("type") == "modules")
         {
            QDomNode nModule = e.firstChild();
            while(!nModule.isNull())
            {
               QDomElement eModule = nModule.toElement();
               if(!eModule.isNull() && eModule.tagName() == "module")
               {
                  // Work with a module
                  ModuleEntry* module = createModule(eModule);
                  if(module)
                  {
                     if(e.hasAttribute("default"))
                     {
                        module->mDefaultChoice = (e.attribute("default").toInt() == 1);
                     }
                     mModules.push_back(module);
                  }
               }
               else if(!eModule.isNull() && eModule.tagName() == "moduleGroup")
               {
                  // Work with a module group
                  ModuleEntry* moduleGroup = createModuleGroup(eModule);
                  if(moduleGroup)
                  {
                     mModuleGroups.push_back(moduleGroup);
                  }
               }

               nModule = nModule.nextSibling();
            }
         }
         else if(e.hasAttribute("type") && e.attribute("type") == "projectDefines")
         {
            QDomNode nPD = e.firstChild();
            while(!nPD.isNull())
            {
               QDomElement ePD = nPD.toElement();
               if(!ePD.isNull() && ePD.tagName() == "projectDefine")
               {
                  // Work with a project define
                  ProjectDefineEntry* pd = new ProjectDefineEntry();
                  pd->mName = ePD.attribute("name");
                  pd->mDescription = ePD.text();

                  mProjectDefines.push_back(pd);
               }

               nPD = nPD.nextSibling();
            }
         }
         else if(e.hasAttribute("type") && e.attribute("type") == "moveClasses")
         {
            QDomNode nMC = e.firstChild();
            while(!nMC.isNull())
            {
               QDomElement eMC = nMC.toElement();
               if(!eMC.isNull() && eMC.tagName() == "moveClass")
               {
                  // Work with a Move class
                  MoveClassEntry* mc = new MoveClassEntry();
                  mc->mName = eMC.attribute("name");
                  mc->mDescription = eMC.text();
                  if(eMC.hasAttribute("default"))
                  {
                     mc->mDefaultChoice = eMC.attribute("default").toInt();
                  }
                  if(eMC.hasAttribute("donotwrite"))
                  {
                     mc->mDoNotWrite = eMC.attribute("donotwrite").toInt();
                  }

                  mMoveClasses.push_back(mc);
               }

               nMC = nMC.nextSibling();
            }
         }
      }

      n = n.nextSibling();
   }
}
