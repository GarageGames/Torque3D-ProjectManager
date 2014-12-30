#include "projectList.h"
#include <QtXml/QDomDocument>

#ifdef Q_OS_WIN
   HWND ProjectList::appWindow = NULL;
#endif


QString DirEntry::getAppPath() 
{ 
   return ProjectList::getAppPath(QFileInfo(mPath).absolutePath()); 
}

QString ProjectEntry::getLevelPath() 
{ 
   return ProjectList::getAppPath(QFileInfo(mPath).absolutePath()) + "/levels"; 
}

ProjectList::ProjectList(QWidget *parent)
{
   mAppName =  "No app name has been set";
   mParent  =  NULL;
   mProcess =  NULL;
}

void ProjectList::setAppName(QString name)
{
   mAppName = name;
}

void ProjectList::setParent(QWidget *parent)
{
   mParent = parent;
}

QString ProjectList::getAppPath(QString path)
{
   QString basePath = QCoreApplication::applicationDirPath();
   if(!path.isEmpty())
   {
      basePath = path;
   }
#ifdef Q_OS_MAC
   int appIndex = basePath.lastIndexOf(QString(".app"));
   int basePathIndex = basePath.lastIndexOf("/", appIndex);
   QString macPath = basePath.left(basePathIndex);
	
   // since this is getting the app path, lets default it to this
   if(path.isEmpty())
   {
      QDir::setCurrent(macPath);
   }
	
   return macPath;
#else
   return basePath;
#endif
}

void ProjectList::buildList()
{
   // Load in the xml file, parse it, and build out the controls
   QDomDocument doc("projects");

   QString baseAppPath = getAppPath();
   //QString projectsPath = baseAppPath + "/Engine/bin/tools/projects.xml";
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

   bool checkRemovals = false;
   QMap<QString, bool> dirNameRemovalMap;
   QMap<ProjectEntry*, bool> dirEntryRemovalMap;

   if(mProjectDirNameList.size() > 0 || mProjectDirectoryList.size() > 0)
   {
      checkRemovals = true;

      for(int i=0; i<mProjectDirNameList.size(); i++)
      {
         dirNameRemovalMap.insert(mProjectDirNameList.at(i), false);
      }

      QList<ProjectEntry*> entryList = mProjectDirectoryList.values();
      for(int i=0; i<entryList.size(); i++)
      {
         dirEntryRemovalMap.insert(entryList.at(i), false);
      }
   }

   // Process
   QDomElement docElem = doc.documentElement();
   QDomNode n = docElem.firstChild();
   while(!n.isNull())
   {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if(!e.isNull() && e.tagName() == "entry")
      {
         // projectDirectory
         if(e.hasAttribute("type") && e.attribute("type") == "projectDirectory" && e.hasAttribute("path"))
         {
            QString title = e.text();
			
            if(mProjectDirNameList.count(title) == 0)
            {
               mProjectDirNameList.append(title);
               emit projectCategoryAdded(title);
            }

            if(checkRemovals)
            dirNameRemovalMap.insert(title, true);

            // Read through the project directory and build the tree entries
            QString projectEntryPath(e.attribute("path"));
            QDir dir(projectEntryPath);
            // Check if the path contains "templates" and ignore if it does.  Older projects.xml files included
            // the Templates directory as a project directory.
            if(!projectEntryPath.contains("templates", Qt::CaseInsensitive) && dir.exists())
            {
               dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
               QFileInfoList projects = dir.entryInfoList();
               for(int i=0; i<projects.size(); ++i)
               {
                  QFileInfo projectInfo = projects.at(i);
                  
                  // Find an executable within the directory
                  QDir projDir(projectInfo.filePath());
                  if(projDir.cd("game"))
                  {
                     // Add the template path
                     ProjectEntry *newEntry = new ProjectEntry();
                     newEntry->mPath = projDir.path();
                     newEntry->mName = projectInfo.fileName();
                     newEntry->mRootName = title;
                     newEntry->mRootPath = projectInfo.absoluteFilePath();

                     // Check for command line arguments
                     if(e.hasAttribute("args"))
                     {
                        newEntry->mArgs = e.attribute("args", "");
                     }

                     mProjectDirectoryList.insert(title, newEntry);
                     emit projectEntryAdded(newEntry);

                     if(checkRemovals)
                        dirEntryRemovalMap.insert(newEntry, true);
                  }
               }
            }
         }
         else if(e.hasAttribute("type") && e.attribute("type") == "templateDirectory" && e.hasAttribute("path"))
         {
            QString title = e.text();

            QString templateEntryPath(e.attribute("path"));
            QDir dir(templateEntryPath);
            if(dir.exists())
            {
               dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
               QFileInfoList templates = dir.entryInfoList();
               for(int i=0; i<templates.size(); ++i)
               {
                  QFileInfo templateInfo = templates.at(i);
                  
                  // Find the game directory to tell us this is a valid template
                  QDir templateDir(templateInfo.filePath());
                  if(templateDir.cd("game"))
                  {
                     // Add the template path
                     TemplateEntry *newEntry = new TemplateEntry();
                     newEntry->mPath = templateDir.path();
                     newEntry->mName = templateInfo.fileName();
                     newEntry->mRootName = title;
                     newEntry->mRootPath = templateInfo.absoluteFilePath();

                     mTemplateDirectoryList.insert(title, newEntry);

                     newEntry->findPackages();
                  }
               }
            }
         }
      }
      n = n.nextSibling();
   }

   // now lets check for removals and emit any removal signals, we see if there are 
   // any false entires still existing in our originally generated lists
   if(checkRemovals)
   {
      QList<QString> nameList = dirNameRemovalMap.keys();
      for(int i=0; i<nameList.size(); i++)
      {
         bool exists = dirNameRemovalMap.value(nameList.at(i));
		 
         if(!exists)
         {
            emit projectCategoryRemoved(nameList.at(i));
            mProjectDirNameList.removeAt(i);
         }
      }

      QList<ProjectEntry*> entryList = dirEntryRemovalMap.keys();
      for(int i=0; i<entryList.size(); i++)
      {
         bool exists = dirEntryRemovalMap.value(entryList.at(i));

         if(!exists)
         {
            ProjectEntry *entry = entryList.at(i);
            emit projectEntryRemoved(entry);
            mProjectDirectoryList.remove(entry->mRootName, entry);
		    
            delete entry;
         }
      }
   }
}

void TemplateEntry::findPackages()
{
   static const QString templateFileName = "template.xml";

   // Load in the xml file, parse it, and build out the controls
   QDomDocument doc("template");

   QString basePath = mRootPath;
   //QString projectsPath = baseAppPath + "/Engine/bin/tools/projects.xml";
   QString templatePath = basePath + "/" + templateFileName;
   QFile file(templatePath);
   std::string debug = templatePath.toStdString();

   if (!file.open(QIODevice::ReadOnly))
   {
      return;
   }

   if (!doc.setContent(&file))
   {
      file.close();
      return;
   }

   file.close();

   // Process
   QDomElement docElem = doc.documentElement();
   QDomNode n = docElem.firstChild();
   while(!n.isNull())
   {
      QDomElement e = n.toElement();
      if(!e.isNull() && e.tagName() == "package")
      {
         // projectDirectory
         if(e.hasAttribute("path") && e.hasAttribute("inclusion"))
         {
            QString title = e.attribute("path");
            QString inclusion = e.attribute("inclusion");

            if(inclusion == "required")
            {
               mRequiredPackages.append(title);
            }
            else if(inclusion == "recommended")
            {
               mRecommendedPackages.append(title);
            }
         }
      }
      n = n.nextSibling();
   }
}

void ProjectList::appStandardData()
{
}

void ProjectList::appErrorData()
{
}

void ProjectList::appFinished()
{
   emit maximizeApp();
}

void ProjectList::appStateChanged(QProcess::ProcessState newState)
{
   if(newState == QProcess::NotRunning)
      appFinished();
}

#ifdef Q_OS_WIN

BOOL CALLBACK ProjectList::EnumWindowsProc(HWND hwnd, LPARAM param)
{
   DWORD id = GetWindowThreadProcessId(hwnd, NULL);

   if (id == (DWORD)param)
   {
      // store the HWND
      ProjectList::appWindow = hwnd;
      return false;
   }

   return true;
}

#endif

void ProjectList::toggleEditor(int editorMode)
{
#ifdef Q_OS_WIN
   //PostMessage(ProjectList::appWindow, WM_TOGGLE_EDITOR, editorMode, 0);
#endif
}

ProjectEntry *ProjectList::getFirstProjectEntry()
{
   ProjectEntry* entry = NULL;
   QList<QString> keyList = mProjectDirectoryList.keys();
   
   if(keyList.size() > 0)
   {
      if(mProjectDirectoryList.count(keyList.at(0)) > 0)
      {
         entry = mProjectDirectoryList.values(keyList.at(0)).at(0);
      }
   }

   return entry;
}