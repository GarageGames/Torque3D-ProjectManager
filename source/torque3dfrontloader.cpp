#include "torque3dfrontloader.h"
#include "moduleList.h"
#include "moduleListInstance.h"
#include <QtNetwork>
#include <QStyleFactory>
#include <QDir>
#include <QInputDialog>
#include "PlatformCheck.h"

QWaitCondition pauseThreads;
QMutex mutex;

Torque3DFrontloader::Torque3DFrontloader(QWidget *parent, Qt::WindowFlags flags)
   : QMainWindow(parent, flags)
{
   // this will set this app instance up to listen for any other running in the future,
   // we do this after we send a launch message to make sure we don't communicate with ourselves
   setupTcp();

   // init our UI Form
   ui.setupUi(this);

   // setup all the values needed
   setupValues();

   hideThings();

   // lets create everything that needs to be created
   createActions();
   createMenus();
   setupFileSystemWatcher();
   createProjectList();
   createModuleList();
   createProgressData();
   setupProjectTree();

   // Setup the New Project Page, this needs to be after the ProjectList is set up
   createNewProjectPage();

   // Set up the Project Module List page.  This needs to be after the ModuleList is set up
   createProjectModuleListPage();

   startUp();	

   // --- we do our tcp song and dance to ensure we don't run two instances of the Toolbox
   // lets see if we are already running
   sendLaunchMessage();
}

void Torque3DFrontloader::initProfile()
{
   mOverallProfileTime.start();
}

void Torque3DFrontloader::startProfile(const QString &step)
{
   if(!mCurrentProfileStep.isEmpty())
      stopProfile();

   mCurrentProfileStep = step;
   mCurrentProfileStepTime.start();
}

void Torque3DFrontloader::stopProfile()
{
   QString timeData;
   timeData.setNum(float(mCurrentProfileStepTime.elapsed()) / 1000);
   mProfileDump.append(QString("%1: %2\n").arg(mCurrentProfileStep).arg(timeData));
}

void Torque3DFrontloader::dumpProfile()
{
   if(!mCurrentProfileStep.isEmpty())
      stopProfile();

   QString timeData;
   timeData.setNum(float(mOverallProfileTime.elapsed()) / 1000);
   mProfileDump.append(QString("%1: %2").arg("Overall").arg(timeData));

   QMessageBox::information(NULL, tr("Profile Time"), mProfileDump);
}

void Torque3DFrontloader::hideThings()
{
   // hiding things that aren't working for the moment
   //ui.pushButton_3->setVisible(false);
   ui.QuickStartGroupBox->setVisible(false);
}

void Torque3DFrontloader::disableProjectControls()
{
   // Disable all controls that work with projects
   ui.OpenFolderButton->setDisabled(true);
   ui.GenerateSourceButton->setDisabled(true);
   ui.ChangeModulesButton->setDisabled(true);

   ui.ExistingProjectInfoPreviewImage->clear();
}

void Torque3DFrontloader::enableProjectControls()
{
   // Enable all controls that work with projects
   ui.OpenFolderButton->setDisabled(false);
   ui.GenerateSourceButton->setDisabled(false);
   ui.ChangeModulesButton->setDisabled(false);
}

void Torque3DFrontloader::setupValues()
{
   readSettings();

   mFirstTray = true;

   mBaseAppPath = ProjectList::getAppPath(QApplication::applicationDirPath());
   mPackagingPath = mBaseAppPath + "/Packaging";
   mStagingPath = mPackagingPath + "/Staging";
   mOutputPath = mPackagingPath + "/Output";
   mDataPath = QDir::toNativeSeparators(mBaseAppPath + "/Engine/bin/tools");
   mUserProjectPath = QDir::toNativeSeparators(mBaseAppPath + "/My Projects");
   mCommentsPath = QDir::toNativeSeparators(mDataPath + "/Comments");
	
   // NSIS (Windows installer) path values
   mNSISScriptPath = QDir::toNativeSeparators(mDataPath + "/nsis/scripts");
   mNSISTemplatePath = QDir::toNativeSeparators(mNSISScriptPath + "/templates");
   mNSISDefaultTemplateName = "default_template.nsi";
   mNSISAppPath = QDir::toNativeSeparators(mDataPath + "/nsis/app");
   mNSISAppName = "makensis.exe";
	
   // zip path values
   mZipAppPath = QDir::toNativeSeparators(mDataPath + "/zip");
   mZipAppName = "zip.exe";
	
   // package manager (mac installer) path values
   mPMScriptPath = QDir::toNativeSeparators(mDataPath + "/packagemaker/scripts");
   mPMTemplatePath = QDir::toNativeSeparators(mPMScriptPath + "/templates");
   mPMDefaultTemplateName = "default_template.pmdoc";
   mPMAppPath = QDir::toNativeSeparators(mDataPath + "/packagemaker/app/PackageMaker.app/Contents/MacOs");
   mPMAppName = "PackageMaker";
	
   mTestPath = mBaseAppPath + QDir::separator() +  "Test";
	
   mAppStyleSheetPath.append(mBaseAppPath + "/Engine/bin/tools/style.css");

   // this is defaulted to false and set to true if it finds another version of itself running,
   // this is checked in main.cpp before entering the event loop
   mQuit = false;
   QApplication::setDesktopSettingsAware(false);

   mNewProjectSet = false;
   mSelectedProject = NULL;
   mProjectGenerationProcess = new QProcess(this);
   ui.ProjectInformationGroupBox->setEnabled(false);

   mProcess = NULL;
   mZipProcess = NULL;

   mNewProjectModuleList = NULL;
   mChangeProjectModulesList = NULL;
}

Torque3DFrontloader::~Torque3DFrontloader()
{
   if(mChangeProjectModulesList)
   {
      delete mChangeProjectModulesList;
   }

   writeSettings();
}

bool Torque3DFrontloader::setSelectedProjectByUniqueName(const QString &uniqueName, bool setFirstIfNot)
{
   int size = ui.ProjectTreeList->mProjectAppList.values(uniqueName).size();
   if(size == 0)
      return false;

   ProjectEntry *entry = ui.ProjectTreeList->mProjectAppList.values(uniqueName).at(0);

   if(entry == NULL)
   {
      setFirstSelectedProject();
   }
   else
   {
      setSelectedProject(entry);

      ProjectTreeItem *item = ui.ProjectTreeList->mProjectItemList.value(entry->mRootName + "-" + entry->mName);
      if(item != NULL)
      {
         ui.ProjectTreeList->setSelected(item);
         QScrollBar *bar = ui.ProjectTreeList->mScrollArea->verticalScrollBar();
		 
         bar->setValue(70);
      }
   }

   return true;
}

void Torque3DFrontloader::setupTcp()
{
   connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

   tcpServer.listen(QHostAddress::LocalHost, 53535);
   
   // if we haven't received a message yet then stop looking for one,
   // this is in case we happen to communicate to another app
   // listening on the same port (should be a rare case)
   //tcpClient.close();
}

void Torque3DFrontloader::acceptConnection()
{
   tcpServerConnection = tcpServer.nextPendingConnection();
   connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(updateServerProgress()));
}

void Torque3DFrontloader::updateServerProgress()
{
   QString text = tcpServerConnection->readAll();
   if(text.compare("Launch") == 0)
   {
      tcpClient.close();
      tcpServerConnection->write(QByteArray("Quit"));
      tcpServerConnection->waitForBytesWritten(500);
      activateWindow();
      show();
   }
   tcpServerConnection->close();   
}

void Torque3DFrontloader::updateClientProgress()
{
   QString text = tcpClient.readAll();
   if(text.compare("Quit") == 0)
   {
      QApplication::quit();
      mQuit = true;
   }
}


void Torque3DFrontloader::sendLaunchMessage(bool sendLaunch)
{
   connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
   connect(&tcpClient, SIGNAL(readyRead()), this, SLOT(updateClientProgress()));
   
   tcpClient.connectToHost(QHostAddress::LocalHost, 53535);
   //tcpClient.waitForConnected(100);

   if(sendLaunch)
      QTimer::singleShot(1000, this, SLOT(sendLaunchMessage(false)));
}

void Torque3DFrontloader::startTransfer()
{
   tcpClient.write(QByteArray("Launch"));
   tcpClient.waitForBytesWritten();
   tcpClient.waitForReadyRead(500);
}

void Torque3DFrontloader::createActions()
{
   //minimizeAction = new QAction(tr("Mi&nimize"), this);
   //connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

   //maximizeAction = new QAction(tr("Ma&ximize"), this);
   //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

   restoreAction = new QAction(tr("&Restore"), this);
   connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

   quitAction = new QAction(tr("&Quit"), this);
   connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));   
   
   clearSettingsAction = new QAction(tr("Clear Settings"), this);
   connect(clearSettingsAction, SIGNAL(triggered()), this, SLOT(clearSettings()));
}

void Torque3DFrontloader::createMenus()
{
   mFileMenu = menuBar()->addMenu(tr("&File"));
   mEditMenu = menuBar()->addMenu(tr("&Edit"));

   mFileMenu->addAction(quitAction);
   mEditMenu->addAction(clearSettingsAction);
}

void Torque3DFrontloader::clearSettings()
{
   QSettings settings;
   settings.remove("");
}

void Torque3DFrontloader::createProjectList()
{
   mProjectList = new ProjectList();
   connect(mProjectList, SIGNAL(projectEntryRemoved(ProjectEntry *)), this, SLOT(projectEntryRemoved(ProjectEntry *)));
   connect(mProjectList, SIGNAL(projectEntryAdded(ProjectEntry *)), this, SLOT(projectEntryAdded(ProjectEntry *)));
   connect(mProjectList, SIGNAL(projectCategoryAdded(QString)), this, SLOT(projectCategoryAdded(QString)));
   connect(mProjectList, SIGNAL(projectCategoryRemoved(QString)), this, SLOT(projectCategoryRemoved(QString))); 
   connect(mProjectList, SIGNAL(minimizeApp()), this, SLOT(showMinimized()));
   connect(mProjectList, SIGNAL(maximizeApp()), this, SLOT(maximizeApp()));
   connect(mProjectList, SIGNAL(hideApp()), this, SLOT(hideApp()));

   connect(ui.ProjectTreeList, SIGNAL(projectRemovalDone()), this, SLOT(projectRemovalDone()));
}

void Torque3DFrontloader::createModuleList()
{
   mModuleList = new ModuleList();
   mModuleList->buildList();
}

void Torque3DFrontloader::createProgressData()
{
   // set up our progress dialog, data, and stages
   mProgressDialog = new ProgressDialog(this);
	
   mPackageStagingStage = new ProgressDialogStage("Staging Project Files");
   //mPackageStagingStage->addSubStage("Compiling scripts", 5);
   mPackageStagingStage->addSubStage("Copying files", 100);

   mPackageZipStage = new ProgressDialogStage("Zip Archive");
   mPackageZipStage->addSubStage("Archiving files", 100);

   mPackageInstallerStage = new ProgressDialogStage("Installer");
   mPackageInstallerStage->addSubStage("Creating script", 10);
   mPackageInstallerStage->addSubStage("Packaging files", 90);

   mPackageWebStage = new ProgressDialogStage("Web Installer");
   mPackageWebStage->addSubStage("Creating Script", 10);
   mPackageWebStage->addSubStage("Packaging files", 90);
    
   mPackageData = new ProgressDialogData("Packaging Progress");

   mCreateTemplateCopyStage = new ProgressDialogStage("Template Files");
   mCreateTemplateCopyStage->addSubStage("Copying files", 100);

   mCreateReformattingFilesStage = new ProgressDialogStage("Reformatting Files");
   mCreateReformattingFilesStage->addSubStage("Reformatting files", 90);
   mCreateReformattingFilesStage->addSubStage("Renaming Files", 10);

   mCreateProjectGenerationStage = new ProgressDialogStage("Source Project");
   mCreateProjectGenerationStage->addSubStage("Generating projects", 100);

   mCreateData = new ProgressDialogData("Project Creation Progress");
   mCreateData->addStage(mCreateTemplateCopyStage, 90);
   mCreateData->addStage(mCreateReformattingFilesStage, 5);
   mCreateData->addStage(mCreateProjectGenerationStage, 5);

   connect(this, SIGNAL(setCopyThreadPause()), &mCopyDir, SLOT(pause()));
   connect(this, SIGNAL(setCopyThreadResume()), &mCopyDir, SLOT(resume()));
   connect(this, SIGNAL(setCopyThreadExit()), &mCopyDir, SLOT(exitNow()));
}

void Torque3DFrontloader::setupProjectTree()
{
   ui.ProjectTreeList->setFrontloader(this);
   ui.ProjectTreeList->setProjectList(mProjectList);
   ui.ProjectTreeList->setupList();

   connect(ui.actionRefresh_Stylesheet, SIGNAL(triggered()), this, SLOT(refreshStylesheet()));
   connect(ui.ProjectTreeList, SIGNAL(projectSelected(ProjectEntry *)), this, SLOT(setSelectedProject(ProjectEntry *)));
}

void Torque3DFrontloader::createNewProjectPage()
{
   mNewProjectPage = new NewProjectPage();
   mNewProjectPage->hide();
   mNewProjectPage->setFrontloader(this);
}

void Torque3DFrontloader::createProjectModuleListPage()
{
   mProjectModuleListPage = new ProjectModuleListPage();
   mProjectModuleListPage->hide();
   mProjectModuleListPage->setFrontloader(this);
}

void Torque3DFrontloader::startUp()
{
   QSettings settings;
   settings.beginGroup("Startup");
   QString startProject = settings.value("lastSelectedProject", "").toString(); 
   settings.endGroup();

   if(startProject.isEmpty())
   {
      setFirstSelectedProject();
   }
   else
   {
      if(!setSelectedProjectByUniqueName(startProject))
      {
         setFirstSelectedProject();
      }
   }
}

void Torque3DFrontloader::setupFileSystemWatcher()
{
   connect(&mFileSystemWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(fileChanged(const QString &)));
   connect(&mFileSystemWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(directoryChanged(const QString &)));

   mFileSystemWatcher.addPath(mBaseAppPath);
   mFileSystemWatcher.addPath(mAppStyleSheetPath);
}

void Torque3DFrontloader::fileChanged(const QString &path)
{
if(path.compare(mAppStyleSheetPath) == 0)
   refreshStylesheet();
}

void Torque3DFrontloader::directoryChanged(const QString &path)
{
}

void Torque3DFrontloader::hideApp()
{
   hide();
}

void Torque3DFrontloader::minimizeApp()
{
   setWindowState(windowState() ^ Qt::WindowMinimized); 
}

void Torque3DFrontloader::maximizeApp()
{
   showNormal();
   activateWindow();
}

void Torque3DFrontloader::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
   switch (reason) 
   {
      case QSystemTrayIcon::Trigger:
         break;

      case QSystemTrayIcon::DoubleClick:
         showNormal();
         activateWindow();
         //setWindowState(windowState() ^ Qt::WindowActive);
         break;

      case QSystemTrayIcon::MiddleClick:
         //showMessage();
         break;

      default:
         ;
   }   
}

void Torque3DFrontloader::resizeEvent(QResizeEvent *event)
{
   //event->ignore();
}

void Torque3DFrontloader::closeEvent(QCloseEvent *event)
{
}

void Torque3DFrontloader::hideEvent(QHideEvent *event)
{
}

void Torque3DFrontloader::clearEditorSettings()
{
   QSettings settings;
   settings.remove("");
}

void Torque3DFrontloader::setSelectedProject(ProjectEntry *project)
{
   if(project != NULL)
   {
      mSelectedProject = project;
   } else
   {
      mSelectedProject = NULL;
   }

   updateSelectedProjectInfo();

   if(mSelectedProject)
   {
      enableProjectControls();
   }
   else
   {
      disableProjectControls();
   }
}

void Torque3DFrontloader::updateSelectedProjectInfo()
{
   if(mSelectedProject)
   {
      ui.ProjectInformationGroupBox->setEnabled(true);
      ui.ExistingProjectInfoMyGameName->setText(mSelectedProject->mName);
      QDir rootPath(mSelectedProject->mPath);
      rootPath.cdUp();
      rootPath.cdUp();
      QString absPath = rootPath.absolutePath();
      QFileInfo info(absPath);

      // update the preivew image
      QPixmap *pixmap = getProjectPixmap(mSelectedProject);

      if(pixmap != NULL)
      {
         ui.ExistingProjectInfoPreviewImage->setPixmap(pixmap->scaled(ui.ExistingProjectInfoPreviewImage->size()));
      }
      else
      {
         ui.ExistingProjectInfoPreviewImage->setText("No Preview Image");
      }

      QSettings settings;
      settings.beginGroup("ProjectInfo" + mSelectedProject->mName);
      QString appName = settings.value("selectedApp", QFileInfo(mSelectedProject->mName).fileName()).toString();
      settings.endGroup();

      ProjectEntry *appEntry = ui.ProjectTreeList->getEntryFromAppName(mSelectedProject->getUniqueName(), appName);
      if(appEntry != NULL)
      {
         setSelectedApp(appEntry, false);
      }

      if(!QFile::exists(mSelectedProject->mRootPath + "/buildFiles"))
      {
         ui.GenerateSourceButton->setEnabled(false);
         ui.ChangeModulesButton->setEnabled(false);
      }
   }
   else
   {
      ui.ExistingProjectInfoMyGameName->setText("No Project Selected");
      ui.ExistingProjectInfoPreviewImage->setText("No Preview Image");
   }
}

void Torque3DFrontloader::setSelectedApp(ProjectEntry *app, bool skipWidgetUpdate) 
{ 
   if(app != NULL)
   {
      mSelectedApp = app; 
   }
};


void Torque3DFrontloader::setFirstSelectedProject()
{
   ProjectEntry *entry = mProjectList->getFirstProjectEntry();
   if(entry != NULL)
   {
      setSelectedProject(entry);
      ProjectTreeItem *item = ui.ProjectTreeList->mProjectItemList.value(entry->mRootName + "-" + entry->mName);
   
      if(item != NULL)
      {
         ui.ProjectTreeList->setSelected(item);

         enableProjectControls();
         return;
      }
   }

   // There is no first project, so hide the project controls
   disableProjectControls();
}

void Torque3DFrontloader::packageProjectStaging()
{
   // lets compile our game scripts
   //QProcess *process = new QProcess(this);
   mProgressDialog->setSubStageProgress(0);
   mProgressDialog->repaint();

   mStagingFile = 0;

   QDir outputDir(mStagingPath);

   // if the staging dir doesn't exist create it
   if(!outputDir.exists())
   {
      bool createPath = outputDir.mkpath(outputDir.path());
   }

   QDir rootProjectPath(getSelectedProject()->getAppPath());
   // this will move us up from the "game" directory
   //rootProjectPath.cdUp();

   if(rootProjectPath.exists())
   {
      outputDir.setPath(outputDir.path() + QDir::separator() + mSelectedProject->mName + QDir::separator() + "data");
	   
      // create the project staging dir if it doesn't already exist
      if(!outputDir.exists())
      {
         outputDir.mkpath(outputDir.path());
      }
      else
      {
         // this staging path already exists
         int ret = QMessageBox::question(this, tr("Package Project"), tr("This staging path already exists.  Do you want to overwrite?"), 
         QMessageBox::Ok, QMessageBox::Cancel);

         if(ret == QMessageBox::Ok)
         {
            // let's create a basic dialog to let people know this may take a moment
            QDialog *prompt = new QDialog(mProgressDialog);
            QLabel *promptText = new QLabel(QString("Clearing staging location, please wait... "));
            //promptText->setWordWrap(true);
            QVBoxLayout *promptLayout = new QVBoxLayout();
            prompt->setLayout(promptLayout);
            promptLayout->addWidget(promptText);
            prompt->show();
			
            bool removed = deletePath(outputDir.path(), false);
         
            prompt->hide();
            delete prompt;

            if(!removed)
            {
               QMessageBox::critical(this, tr("Could Not Overwrite"), tr("This staging path could not be overwritten (%1), choose a new one or ensure no"
               " other application is accessing any of the files.").arg(outputDir.path()), QMessageBox::Ok);
               mProgressDialog->fail();
               return;
            }
         }
         else if(ret == QMessageBox::Cancel)
         {
            mProgressDialog->fail();
            return;
         }
      }
	   
      if(PlatformCheck::isMac())
      {
         QString webAdd;
         if(!mPackagingWebDone)
         {
            webAdd.append(" Web");
         }

         outputDir.setPath(outputDir.path() + "/" + getSelectedProject()->mName + webAdd);
         
         if(!outputDir.exists())
         {
            outputDir.mkpath(outputDir.path());
         }
      } 

      // our root project path exists, time to copy everything we want over to staging
      rootProjectPath.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden);
      QStringList nameExcludeFilter;
      QStringList nameIncludeFilter;
      QStringList nameSpecificExcludeFilter;

      connect(&mCopyDir, SIGNAL(updateFileCount(int)), this, SLOT(updateStagingFileCount(int)));
      connect(&mCopyDir, SIGNAL(updateFileProgress(int, QString)), this, SLOT(updateStagingFileProgress(int, QString)));
      connect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), this, SLOT(updateStagingFileDone(bool, bool)));
      connect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), &mCopyDir, SLOT(quit()));

      QStringList dirs;
      dirs.push_front(rootProjectPath.path());
      mCopyDir.setValues(dirs, outputDir.path(), nameExcludeFilter, nameIncludeFilter);
      mCopyDir.start();
   }
}

void Torque3DFrontloader::createProjectCheck()
{
   if(!mCreateTemplateCopyDone)
   {
      mCreateTemplateCopyDone = true;
      mCreateTemplateCopy = true;
      createTemplateCopy();
      return;
   }
   mCreateTemplateCopy = false;

   if(!mCreateReformattingFilesDone)
   {
      mCreateReformattingFilesDone = true;
      mCreateReformattingFiles = true;
      createReformattingFiles();
      return;
   }
   mCreateReformattingFiles = false;

   if(!mCreateProjectGenerationDone)
   {
      mCreateProjectGenerationDone = true;
      mCreateProjectGeneration = true;
      createProjectGeneration();
      return;
   }
   mCreateProjectGeneration = false;

   createClear();

   mProgressDialog->doneWithDialog("Project Creation Complete", QDir::toNativeSeparators(mNewProjectDir.path()), 
      "Your new project has been created, you can find it here:");

   QString uniqueName;
   QString projectName = mNewProjectDir.dirName();
   QDir dir(mNewProjectDir);
   dir.cdUp();
   uniqueName.append(dir.dirName() + "-" + projectName);
   setSelectedProjectByUniqueName(uniqueName, false);
}

void Torque3DFrontloader::createNewProject(const QString &templatePath, const QStringList &packagePaths, const QString &newProjectPath, ModuleListInstance* moduleInst)
{
   // init values
   mCreateTemplateCopyDone = false;
   mCreateReformattingFilesDone = false;
   mCreateTemplateCopy = false;
   mCreateReformattingFiles = false;
   mCreateProjectGeneration = false;
   mCreateProjectGenerationDone = false;
   mCreateReformattingFilesPause = false;
   mCreateProjectFileCount = 0;
   mProjectGenerationExitNow = false;

   mTemplateDir.setPath(templatePath);
   mNewProjectDir.setPath(newProjectPath);
   mPackagePaths = packagePaths;

   mNewProjectModuleList = moduleInst;

   QDir myProjectsDir(mUserProjectPath);
   if(!myProjectsDir.exists())
      myProjectsDir.mkpath(mUserProjectPath);

   if(mNewProjectDir.exists())
   {
      // a project under this name already exists, need to prompt to replace or cancel
      int ret = QMessageBox::question(this, tr("Create New Project"), tr("A project exists with this name.  Do you want to overwrite?"), 
      QMessageBox::Ok, QMessageBox::Cancel);

      if(ret == QMessageBox::Ok)
      {
         //QDir rmDir(mNewProjectDir.path());
         bool removed = deletePath(mNewProjectDir.path());
         
         if(!removed)
         {
            QMessageBox::critical(this, tr("Could Not Overwrite"), tr("The project could not be overwritten, choose a new name or ensure no"
               " other application is accessing any of the files."), QMessageBox::Ok);
            return;
         }
      }
      else if(ret == QMessageBox::Cancel)
      {
         return;
      }
   }

   mNewProjectPage->hide();

   connect(mProgressDialog, SIGNAL(pause()), this, SLOT(createPause()));
   connect(mProgressDialog, SIGNAL(resume()), this, SLOT(createResume()));
   connect(mProgressDialog, SIGNAL(cancel()), this, SLOT(createCancel()));

   // now lets set the data and show the progress dialog
   mProgressDialog->setData(mCreateData);
   
   createProjectCheck();
}

void Torque3DFrontloader::createPause()
{
   if(mCreateTemplateCopy)
   {
      emit setCopyThreadPause();
   }
   else if(mCreateReformattingFiles)
   {
      emit setReformattingFilesThreadPause();
   }
   else if(mCreateProjectGeneration)
   {
      emit setProjectGenerationThreadPause();
      pauseProcess(mProjectGenerationProcess);
   }
}

void Torque3DFrontloader::createResume()
{
   if(mCreateTemplateCopy)
   {
      emit setCopyThreadResume();
      pauseThreads.wakeAll();
   }
   else if(mCreateReformattingFiles)
   {
      emit setReformattingFilesThreadResume();
   }
   else if(mCreateProjectGeneration)
   {
      emit setProjectGenerationThreadResume();
      resumeProcess(mProjectGenerationProcess);
   }
}

void Torque3DFrontloader::createCancel()
{
   // let's pause our current step
   if(mCreateTemplateCopy)
   {
      emit setCopyThreadPause();
   }
   else if(mCreateReformattingFiles)
   {
      emit setReformattingFilesThreadPause();
   }
   else if(mCreateProjectGeneration)
   {
      emit setProjectGenerationThreadPause();
      pauseProcess(mProjectGenerationProcess);
   }

   // ask the user if they are sure they want to quit
   int rep = QMessageBox::question(this, "Cancel?", "Are you sure you wan't to cancel, "
      "it is recommended you finish?", QMessageBox::Yes, QMessageBox::No);

   // if yes then let's signal an exit
   if(rep == QMessageBox::Yes)
   {
      if(mCreateTemplateCopy)
      {
         emit setCopyThreadExit();
      }
      else if(mCreateReformattingFiles)
      {
         emit setReformattingFilesThreadExit();   
      }
      else if(mCreateProjectGeneration)
      {
         emit setProjectGenerationThreadExit();
      }

   createClear();
   } 

   // now let's resume
   if(mCreateTemplateCopy)
   {
      emit setCopyThreadResume();
      pauseThreads.wakeAll();
   }
   else if(mCreateReformattingFiles)
   {
      emit setReformattingFilesThreadPause();
   }
   else if(mCreateProjectGeneration)
   {
      emit setProjectGenerationThreadResume();
      resumeProcess(mProjectGenerationProcess);
   }
}

void Torque3DFrontloader::createClear()
{
   disconnect(mProgressDialog, SIGNAL(pause()), this, SLOT(createPause()));
   disconnect(mProgressDialog, SIGNAL(resume()), this, SLOT(createResume()));
   disconnect(mProgressDialog, SIGNAL(cancel()), this, SLOT(createCancel()));

   ui.ProjectTreeList->setupList();
}

void Torque3DFrontloader::createTemplateCopy()
{
   // lets make sure our new project path exists
   if(!mNewProjectDir.exists())
      mNewProjectDir.mkdir(mNewProjectDir.path());

   // our new project path exists, time to copy everything we want over to it
   QStringList nameExcludeFilter;
   nameExcludeFilter << "*.obj" << "*.exp" << "*.ilk" << "*.pch" << "*.sbr" << "*.pdb" << "*.sln" << "*.suo";
   nameExcludeFilter << "*/buildFiles/Xcode/*";
   nameExcludeFilter << "*/template.xml";
   QStringList nameIncludeFilter;

   connect(&mCopyDir, SIGNAL(updateFileCount(int)), this, SLOT(updateCreateProjectCount(int)));
   connect(&mCopyDir, SIGNAL(updateFileProgress(int, QString)), this, SLOT(updateCreateProjectProgress(int, QString)));
   connect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), this, SLOT(updateCreateProjectDone(bool, bool)));
   connect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), &mCopyDir, SLOT(quit()));
   QStringList dirs = mPackagePaths;
   dirs.push_front(mTemplateDir.path());
   mCopyDir.setValues(dirs, mNewProjectDir.path(), nameExcludeFilter, nameIncludeFilter);
   mCopyDir.start();
}

void Torque3DFrontloader::createReformattingFiles()
{
   mProgressDialog->incStage();

   QString templateName = mTemplateDir.dirName();
   QString newName = mNewProjectDir.dirName();
   QString file = QDir::toNativeSeparators(mNewProjectDir.path() + "/");

   QStringList fileList;
   fileList << "game/" + templateName + ".torsion"
      << "source/torqueConfig.h";

   for(int i=0; i<fileList.size(); i++)
   {
      // loop through all of the files and change the references
      replaceTextInFile(QDir::toNativeSeparators(file + fileList.at(i)), templateName, newName);

      mProgressDialog->setSubStageProgress(11 * (i+16));
      mProgressDialog->updateDetailText("reformat: " + fileList.at(i));
   }

   // replace our entry in the project.conf
   QString projectConfSrc("setGameProjectName(\"" + templateName + "\");");
   QString projectConfDst("setGameProjectName(\"" + newName + "\");");
   replaceTextInFile(QDir::toNativeSeparators(file + "buildFiles/config/project.conf"), templateName, newName);
   replaceTextInFile(QDir::toNativeSeparators(file + "buildFiles/config/project.mac.conf"), templateName, newName);
   if(mNewProjectModuleList)
   {
      mNewProjectModuleList->replaceProjectFileContents( QDir::toNativeSeparators(file + "buildFiles/config/project.conf") );
   }
   mProgressDialog->setSubStageProgress(11 * (2+16));
   mProgressDialog->updateDetailText(QString("reformat: ") + "project.conf");
   replaceTextInFile(QDir::toNativeSeparators(file + "buildFiles/config/project.360.conf"), templateName, newName);
   mProgressDialog->setSubStageProgress(11 * (3+16));
   mProgressDialog->updateDetailText(QString("reformat: ") + "project.360.conf");
   replaceTextInFile(QDir::toNativeSeparators(file + "buildFiles/config/project.mac.conf"), templateName, newName);
   mProgressDialog->setSubStageProgress(11 * (4+16));
   mProgressDialog->updateDetailText(QString("reformat: ") + "project.mac.conf");


   replaceTextInFile(QDir::toNativeSeparators(file + "game/main.cs"), QString("\"" + templateName + "\""), QString("\"" + newName + "\""));

   mProgressDialog->incSubStage();

   // rename the .exe
   renameFile(file + "game/" + templateName + ".exe", newName + ".exe");
   
   // rename the .exe's dll
   renameFile(file + "game/" + templateName + ".dll", newName + ".dll");
   
   // rename the NP (Firefox) web plugin .dll
   renameFile(file + "game/NP " + templateName + " Plugin.dll", "NP " + newName + " Plugin.dll");
   
   // rename the IE (Internet Explorer) web plugin .dll
   renameFile(file + "game/IE " + templateName + " Plugin.dll", "IE " + newName + " Plugin.dll");
   
   // rename the debug .exe
   renameFile(file + "game/" + templateName + "_DEBUG.exe", newName + "_DEBUG.exe");

   // rename the debug .exe's dll
   renameFile(file + "game/" + templateName + "_DEBUG.dll", newName + "_DEBUG.dll");
   
   // check for an "r" .exe, useful for testing
   renameFile(file + "game/r" + templateName + ".exe", "r" + newName + ".exe");
   
   // check for mac apps
   renameMacApp(file + "game/" + templateName + ".app", templateName, templateName + " Bundle", "" + newName + ".app", newName + " Bundle");
   renameMacApp(file + "game/" + templateName + "_DEBUG.app", templateName + "_DEBUG", templateName + " Bundle", "" + newName + "_DEBUG.app", newName + " Bundle");
   
   mProgressDialog->setSubStageProgress(33);

   // rename the .torsion
   renameFile(file + "game/" + templateName + ".torsion", newName + ".torsion");

   mProgressDialog->setSubStageProgress(66);

   // rename the .torsion.opt
   renameFile(file + "game/" + templateName + ".torsion.opt", newName + ".torsion.opt");
   
   mProgressDialog->setSubStageProgress(100);

   if(!mCreateReformattingFilesPause)
      createProjectCheck();
}

void Torque3DFrontloader::createProjectGeneration()
{
   mProgressDialog->incStage();

   QString rootPath = mNewProjectDir.path();
   QString buildFiles(rootPath + "/buildFiles");

   if(!QFile::exists(buildFiles))
   {
      mProgressDialog->setSubStageProgress(100);
      mProgressDialog->updateDetailText("No source to generate");
      createProjectCheck();
      return;
   }

   QStringList argList;
   QString stringFirst(mBaseAppPath + "/Tools/projectGenerator/projectGenerator.php");
   QString stringSecond(rootPath + "/buildFiles/config/project.conf");
   QString stringThird(mBaseAppPath);
   
   argList.append(mBaseAppPath + "/Tools/projectGenerator/projectGenerator.php");
   
   if(PlatformCheck::isWin())
   {
      argList.append(rootPath + "/buildFiles/config/project.conf");
   }
   else
   {
      argList.append(rootPath + "/buildFiles/config/project.mac.conf");
   }
      
   argList.append(mBaseAppPath);

   if(mProjectGenerationProcess != NULL)
      delete mProjectGenerationProcess;

   mProjectGenerationProcess = new QProcess(this);
   mProjectGenerationProcess->setWorkingDirectory(rootPath);

   connect(this, SIGNAL(setProjectGenerationThreadExit()), this, SLOT(projectGenerationExitNow()));
   connect(mProjectGenerationProcess, SIGNAL(started()), this, SLOT(projectGenerationStarted()));
   connect(mProjectGenerationProcess, SIGNAL(error()), this, SLOT(projectGenerationError()));
   connect(mProjectGenerationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(projectGenerationFinished(int, QProcess::ExitStatus)));
   connect(mProjectGenerationProcess, SIGNAL(finished()), mProjectGenerationProcess, SLOT(kill()));
   connect(mProjectGenerationProcess, SIGNAL(readyReadStandardOutput()),this, SLOT(projectGenerationStandardWrite()) );
   connect(mProjectGenerationProcess, SIGNAL(readyReadStandardError()), this, SLOT(projectGenerationErrorWrite()) );

   if(PlatformCheck::isWin())
   {
      QString phpPath(mBaseAppPath + "/Engine/bin/php/php.exe");
      mProjectGenerationProcess->start(phpPath, argList);
   }
   else if(PlatformCheck::isMac())
   {
      QString phpPath("/usr/bin/php");
      mProjectGenerationProcess->start(phpPath, argList);
   }
}

void Torque3DFrontloader::projectGenerationStarted()
{
   mProjectGenerationCount = 0;
}

void Torque3DFrontloader::projectGenerationError()
{
}

void Torque3DFrontloader::projectGenerationFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
   if(!mProjectGenerationExitNow)
   {
      mProgressDialog->setSubStageProgress(100);
      createProjectCheck();
   }
   else
   {
      // forced exit so we close out
      mProgressDialog->fail();
   }

   disconnect(this, SIGNAL(setProjectGenerationThreadExit()), this, SLOT(projectGenerationExitNow()));
   disconnect(mProjectGenerationProcess, SIGNAL(started()), this, SLOT(projectGenerationStarted()));
   disconnect(mProjectGenerationProcess, SIGNAL(error()), this, SLOT(projectGenerationError()));
   disconnect(mProjectGenerationProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(projectGenerationFinished(int, QProcess::ExitStatus)));
   disconnect(mProjectGenerationProcess, SIGNAL(finished()), mProjectGenerationProcess, SLOT(kill()));
   disconnect(mProjectGenerationProcess, SIGNAL(readyReadStandardOutput()),this, SLOT(projectGenerationStandardWrite()) );
   disconnect(mProjectGenerationProcess, SIGNAL(readyReadStandardError()), this, SLOT(projectGenerationErrorWrite()) );
}

void Torque3DFrontloader::projectGenerationExitNow()
{
   mProjectGenerationExitNow = true;
   mProjectGenerationProcess->kill();  
}

void Torque3DFrontloader::projectGenerationErrorWrite()
{
   QString data = mProjectGenerationProcess->readAllStandardError();
}

void Torque3DFrontloader::projectGenerationStandardWrite()
{
   QString data = mProjectGenerationProcess->readAllStandardOutput();

   mProjectGenerationCount++;
   mProgressDialog->setSubStageProgress(mProjectGenerationCount);
   mProgressDialog->updateDetailText(data);
}

void Torque3DFrontloader::replaceTextInFile(QString file, QString srcText, QString dstText)
{
   QFile srcFile(file);

   if(srcFile.open(QIODevice::ReadOnly))
   {
      QString fileString = srcFile.readAll();
      srcFile.close();
	  
      if(srcFile.open(QIODevice::WriteOnly))
      {
         fileString.replace(srcText, dstText);
         srcFile.write(QByteArray(fileString.toUtf8()));
         srcFile.close();
      }
   }

   srcFile.close();
}

void Torque3DFrontloader::updateCreateProjectCount(int count)
{
   mCreateProjectFileCount = count;
}

void Torque3DFrontloader::updateCreateProjectDone(bool success, bool exitNow)
{
   // clearing all of the signals so mCopyDir is reusable in the future, this is nearly an exact copy of the
   // connect() calls, just with disconnect() instead
   disconnect(&mCopyDir, SIGNAL(updateFileCount(int)), this, SLOT(updateCreateProjectCount(int)));
   disconnect(&mCopyDir, SIGNAL(updateFileProgress(int, QString)), this, SLOT(updateCreateProjectProgress(int, QString)));
   disconnect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), this, SLOT(updateCreateProjectDone(bool, bool)));
   disconnect(&mCopyDir, SIGNAL(fileCopyDone(bool, bool)), &mCopyDir, SLOT(quit()));

   if(success && !exitNow)
   {
      createProjectCheck();
   }
   else if(exitNow)
   {
      mCopyDir.quit();
      mProgressDialog->fail();
   }
   else if(!success)
   {
      int ret = QMessageBox::information(this, "Failed", "Project Creation has Failed.\n"
         "Unable to copy project files, ensure no other applications are currently accessing these files");
      mCopyDir.quit();
      mProgressDialog->fail();
   }
}

void Torque3DFrontloader::updateCreateProjectProgress(int count, QString detailText)
{
   mCreateProjectFile = count;

   double floatCount = QVariant(count).toDouble();
   double floatCountTotal = QVariant(mCreateProjectFileCount).toDouble();
   
   double floatPercent = (floatCount / floatCountTotal) * 100;
   int percent = QVariant(floatPercent).toInt();

   mProgressDialog->setSubStageProgress(percent);
   mProgressDialog->updateDetailText(detailText);
}

void Torque3DFrontloader::createReformattingFilesPause()
{
   mCreateReformattingFilesPause = true;
}

void Torque3DFrontloader::createReformattingFilesResume()
{
   createProjectCheck();
}

void Torque3DFrontloader::packageProject(QString projectOutputPath)
{
   // if these are set to true then they won't perform the process,
   // if set to false then it will process and trigger to true when done,
   // by default all but staging is set to true, then UI values are checked
   // whether the other processes should happen
   mPackagingStagingDone = false;
   mPackagingZipDone = true;	
   mPackagingInstallerDone = true;
   mPackagingWebDone = true;
   mPackagingStaging = false;
   mPackagingZip = false;	
   mPackagingInstaller = false;
   mPackagingWeb = false;

   // make sure the output path exists
   if(!QDir(mOutputPath).exists())
      QDir().mkpath(mOutputPath);

   // then make sure the project's output path exists
   mPackagingGameOutputPath = projectOutputPath;
   if(!QDir(mPackagingGameOutputPath).exists())
      QDir().mkdir(mPackagingGameOutputPath);

   // lets add all of the stages to the package data for the progress dialog
   mPackageData->clearData();
   mPackageData->mTitle = QString("Packaging Progress - ") + getSelectedProject()->mName;
   mPackageData->addStage(mPackageStagingStage);

   connect(mProgressDialog, SIGNAL(pause()), this, SLOT(packagePause()));
   connect(mProgressDialog, SIGNAL(resume()), this, SLOT(packageResume()));
   connect(mProgressDialog, SIGNAL(cancel()), this, SLOT(packageCancel()));
   connect(mProgressDialog, SIGNAL(on_fail()), this, SLOT(packageClear()));
   connect(mProgressDialog, SIGNAL(on_success()), this, SLOT(packageClear()));

   // now lets set the data and show the progress dialog
   mProgressDialog->setData(mPackageData);
}

void Torque3DFrontloader::packageInstaller(bool isWeb)
{
   mProgressDialog->incStage();
}

void Torque3DFrontloader::dumpData()
{
   QFile file("dumpLog.txt");
   file.open(QIODevice::WriteOnly);
   file.write(mDumpData.toUtf8(), qstrlen(mDumpData.toUtf8()));
   file.close();
}

void Torque3DFrontloader::packageZip()
{
   mProgressDialog->incStage();

   if(mZipProcess != NULL)
      delete mZipProcess;

   mZipExitNow = false;

   // now we call the zip app
   mZipProcess = new QProcess(this);
   connect(this, SIGNAL(setZipThreadExit()), this, SLOT(zipExitNow()));
   connect(mZipProcess, SIGNAL(started()), this, SLOT(zipStarted()));
   connect(mZipProcess, SIGNAL(error()), this, SLOT(zipError()));
   connect(mZipProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(zipFinished(int, QProcess::ExitStatus)));
   connect(mZipProcess, SIGNAL(finished()), mZipProcess, SLOT(kill()));
   connect(mZipProcess, SIGNAL(readyReadStandardOutput()),this, SLOT(zipStandardWrite()) );
   connect(mZipProcess, SIGNAL(readyReadStandardError()), this, SLOT(zipErrorWrite()) );

   QDir projectStagingDir(mStagingPath + QDir::separator() + mSelectedProject->mName +
      QDir::separator() + "data");
   QDir outputDir(mStagingPath);

   if(!outputDir.exists())
   {
      bool createPath = outputDir.mkpath(outputDir.path());
   }

   outputDir.setPath(outputDir.path() + QDir::separator() + mSelectedProject->mName);

   if(!outputDir.exists())
   {
      bool createPath = outputDir.mkpath(outputDir.path());
   }

   QString zipPath;
   QString outputZipFile(mPackagingGameOutputPath + "/" + mSelectedProject->mName + ".zip");;

   if(PlatformCheck::isWin())
   {
      zipPath.append(mZipAppPath + QDir::separator() + mZipAppName);
   }
   else if(PlatformCheck::isMac())
   {
      zipPath.append("zip");
   }

   if(QFile::exists(outputZipFile))
      QFile::remove(outputZipFile);

   // set the current directory of this process
   mZipProcess->setWorkingDirectory(projectStagingDir.path());

   // reset zip file count
   mZipFileCount = 0;
   QStringList args;
   args << outputZipFile;
   args << "-r";
   args << ".";
   mZipProcess->start(zipPath, args);	  
}

void Torque3DFrontloader::packageWeb()
{ 
   // lets copy over our web files
   QString projectGamePath = getSelectedProject()->getAppPath();
   projectGamePath.append("/web");
   QDir dir(projectGamePath);

   dir.mkpath(mPackagingGameOutputPath + "/web");

   QFileInfoList entryList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

   bool deleted = deletePath(mPackagingGameOutputPath + "/web", false);

   for(int i=0; i < entryList.size(); i++)
   {
   QFileInfo entry = entryList.at(i);

      QString dstPath(mPackagingGameOutputPath + "/web/" + entry.fileName());

      if(entry.exists())
      {
         bool copied = QFile::copy(entry.absoluteFilePath(), dstPath);
      }
   }

   packageInstaller(true);
}

void Torque3DFrontloader::refreshStylesheet()
{
   Torque3DFrontloader::loadStylesheet();
   update();
}

void Torque3DFrontloader::loadStylesheet()
{
   // load the application style sheet
   QString styleString;
   QFile styleFile(":/Torque3DFrontloader/resources/style.css");
   if(styleFile.open(QIODevice::ReadOnly))
   {
      styleString.append(QString(styleFile.readAll()));
      styleFile.close();
   }
	
   if(PlatformCheck::isMac())
   {
      styleFile.setFileName(":/Torque3DFrontloader/resources/style.css");
      if(styleFile.open(QIODevice::ReadOnly))
      {
         styleString.append(QString(styleFile.readAll()));
         styleFile.close();
      }
   }
	
   qApp->setStyleSheet(styleString);
}

void Torque3DFrontloader::on_CreateNewProjectButton_clicked()
{
   mNewProjectPage->launch();   
}


void Torque3DFrontloader::projectEntryRemoved(ProjectEntry *entry)
{
   if(mSelectedProject == entry)
   {
      mNewProjectSet = true;
   }
}

void Torque3DFrontloader::projectRemovalDone()
{
   if(mNewProjectSet)
   {
      setFirstSelectedProject();
   }

   mNewProjectSet = false;
}

void Torque3DFrontloader::projectEntryAdded(ProjectEntry *entry)
{
}

void Torque3DFrontloader::projectCategoryAdded(QString title)
{
}

void Torque3DFrontloader::projectCategoryRemoved(QString title)
{
}


QPixmap *Torque3DFrontloader::getProjectPixmap(DirEntry *entry)
{
   QPixmap *pixmap = NULL;
   QFile thumb(entry->mRootPath + QDir::separator() + "thumb.png");

   if(!thumb.exists())
      thumb.setFileName(entry->mRootPath + QDir::separator() + "thumb.jpg");

   if(thumb.exists())
      pixmap = new QPixmap(thumb.fileName());

   return pixmap;
}

bool Torque3DFrontloader::deletePath(QString path, bool includeBase)
{
   QDir rootPath(path);
   rootPath.setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
   QFileInfoList fileList = rootPath.entryInfoList();

   for(int i=0; i<fileList.size(); i++)
   {
      QFileInfo fileInfo = fileList.at(i);

      if(fileInfo.isFile())
      {
         bool remove = QFile(fileInfo.absoluteFilePath()).remove();
         if(!remove)
            return false;
      }
      else
      {
         bool remove = deletePath(fileInfo.absoluteFilePath());
         if(!remove)
            return false;
      }
   }

   bool returnVal = true;

   if(includeBase)
   {
      QDir dir(path);
      dir.cdUp();
      returnVal = dir.rmdir(QDir(path).dirName());
   }

   return returnVal;
}

void Torque3DFrontloader::openSourceCode()
{
   ProjectEntry *entry = getSelectedProject();
   if(entry == NULL)
      return;
   
   // lets start our path out with the buildFiles directory, after this it begins to differ
   // based on platform and source editor version (on Windows VS 2008 or 2010)
   QString sourceProject(entry->mRootPath + "/buildFiles");

   if(PlatformCheck::isWin())
   {
      QSettings settings;
      settings.beginGroup("Source");
      QString version = settings.value("VSVersion", "2008").toString();
      bool versionCheck = settings.value("VSCheck", true).toBool();
      settings.endGroup();

      // if we haven't already said not to ask again then lets ask what version of VS
      if(versionCheck)
      {
         QStringList items;
         items << "2008" << "2010";
         bool ok;

         QString item = QInputDialog::getItem(this, tr("Visual Studios Version"), 
                           tr("Select your Visual Studios version"), items, 0, false, &ok);

         if(ok && !item.isEmpty())
         {
            version = item;
         }
         else
         {
            // the user has canceled out of the version select so lets exit
            return;
         }
      }

      sourceProject.append("/VisualStudio " + version + "/" + entry->mName + ".sln");
   }
   else if(PlatformCheck::isMac())
   {
      sourceProject.append(QString("/Xcode/%1.xcodeproj").arg(entry->mName));  
   }

   QFile sourceProjectFile(sourceProject);
   if(sourceProjectFile.exists())
   {
      QDesktopServices::openUrl(QUrl("file:///" + sourceProject));
	  
      if(PlatformCheck::isWin())
      {
         QMessageBox::information(this, tr("Opening Source Code"), tr("The solution file has been called and the specified") +
            tr(" Visual Studios version should be opening.  This may take a moment.  If the solution file") +
            tr(" does not open then ensure you have Visual Studios installed and the .sln extension is properly") + 
            tr(" mapped to open in Visual Studios."));
      }
      else if(PlatformCheck::isMac())
      {
         QMessageBox::information(this, tr("Opening Source Code"), tr("The Xcode project file has been called and Xcode should") + 
            tr(" be opening.  This may take a moment.  If it does not properly open ensure you have Xcode installed"));
      }
   }
   else
   {
      int ret = QMessageBox::question(this, tr("Couldn't Find"), tr("Couldn't find a source project file at: ") + sourceProject + tr("\nWould you like to generate a project?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

      if(ret == QMessageBox::Yes)
      {
         generateSourceProject();
      }
      else if(ret == QMessageBox::No)
      {

      }
   }
}

void Torque3DFrontloader::changeProjectModules()
{
   // Create the default module list
   if(mChangeProjectModulesList)
   {
      delete mChangeProjectModulesList;
      mChangeProjectModulesList = NULL;
   }
   mChangeProjectModulesList = new ModuleListInstance();
   mChangeProjectModulesList->buildInstances(getModuleList());

   // Get the modules for the project
   QString file = QDir::toNativeSeparators(getSelectedProject()->mRootPath + "/buildFiles/config/project.conf");
   mChangeProjectModulesList->readProjectFile(file);

   // Open the module window
   getProjectModuleListPage()->launch(mChangeProjectModulesList, true);
}

void Torque3DFrontloader::generateSourceProject()
{ 
   QDialog *prompt = new QDialog();
   prompt->setModal(true);
   QLabel *promptText = new QLabel(QString("Generating source projects, please wait... "));
   //promptText->setWordWrap(true);
   QVBoxLayout *promptLayout = new QVBoxLayout();
   prompt->setLayout(promptLayout);
   promptLayout->addWidget(promptText);
   prompt->show();
   prompt->repaint();

   bool generate = generateProjects(getSelectedProject()->mRootPath);

   prompt->hide();
   delete prompt;

   if(generate)
   {
      QMessageBox::information(this, tr("Project Generation"), tr("Project generation was successful!"));
   } else
   {
      QMessageBox::information(this, tr("Project Generation"), tr("Project generation has failed!"));
   }
}

bool Torque3DFrontloader::generateProjects(QString rootPath)
{
   QString buildFiles(rootPath + "/buildFiles");

   QStringList argList;
   QString stringFirst(mBaseAppPath + "/Tools/projectGenerator/projectGenerator.php");
   QString stringSecond(rootPath + "/buildFiles/config/project.conf");
   QString stringThird(mBaseAppPath);
   
   argList.append(mBaseAppPath + "/Tools/projectGenerator/projectGenerator.php");
   
   if(PlatformCheck::isWin())
   {
      argList.append(rootPath + "/buildFiles/config/project.conf");
   }
   else
   {
      argList.append(rootPath + "/buildFiles/config/project.mac.conf");
   }
   
   argList.append(mBaseAppPath);

   QString data;

   bool success = false;
   if(PlatformCheck::isWin())
   {
      QString phpPath(mBaseAppPath + "/Engine/bin/php/php.exe");
      QProcess *process = new QProcess(this);	  
      process->setWorkingDirectory(rootPath);
	  
      process->start(phpPath, argList);
      success = process->waitForStarted();
	  
      if(success)
         success = process->waitForFinished(300000);  // Wait for up to 5 minutes

      data = process->readAll();

   }
   else if(PlatformCheck::isMac())
   {
      QString phpPath("/usr/bin/php");
      QProcess *process = new QProcess(this);
      process->setWorkingDirectory(rootPath);
       
      process->start(phpPath, argList);
      success = process->waitForStarted();
	  
      if(success)
         success = process->waitForFinished();
   }

   return success;
}

void Torque3DFrontloader::pauseProcess(QProcess *process)
{
#ifdef Q_OS_WIN
   _PROCESS_INFORMATION *pi = process->pid();
   PauseResumeThreadList(pi->dwProcessId);
#endif

#ifdef Q_OS_MAC
   ::kill(process->pid(), SIGSTOP);   
#endif
}

void Torque3DFrontloader::resumeProcess(QProcess *process)
{
#ifdef Q_OS_WIN
   _PROCESS_INFORMATION *pi = process->pid();
   PauseResumeThreadList(pi->dwProcessId, true);
#endif

#ifdef Q_OS_MAC
   ::kill(process->pid(), SIGCONT);  
#endif
}

#ifdef Q_OS_WIN
using namespace std;
   
int Torque3DFrontloader::PauseResumeThreadList(DWORD dwOwnerPID, bool bResumeThread) 
{ 
   HANDLE        hThreadSnap = NULL; 
   BOOL          bRet        = FALSE; 
   THREADENTRY32 te32        = {0}; 
 
   // Take a snapshot of all threads currently in the system. 

   hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
   if (hThreadSnap == INVALID_HANDLE_VALUE) 
      return (FALSE); 
 
   // Fill in the size of the structure before using it. 

   te32.dwSize = sizeof(THREADENTRY32); 
 
   // Walk the thread snapshot to find all threads of the process. 
   // If the thread belongs to the process, add its information 
   // to the display list.
 
   if (Thread32First(hThreadSnap, &te32)) 
   { 
      do 
      { 
         if (te32.th32OwnerProcessID == dwOwnerPID) 
         {
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            if (bResumeThread)
            {
               cout << _T("Resuming Thread 0x") << cout.setf( ios_base::hex ) << te32.th32ThreadID << '\n';
               ResumeThread(hThread);
            }
            else
            {
               cout << _T("Suspending Thread 0x") << cout.setf( ios_base::hex ) << te32.th32ThreadID << '\n';
               SuspendThread(hThread);
            }
            CloseHandle(hThread);
         } 
      }
      while (Thread32Next(hThreadSnap, &te32)); 
      bRet = TRUE; 
   } 
   else
   {
      bRet = FALSE;          // could not walk the list of threads 
   }
 
   // Do not forget to clean up the snapshot object. 
   CloseHandle (hThreadSnap); 
 
   return (bRet); 
} 
#endif

bool Torque3DFrontloader::getResourceInfo(const QString &filePath, QString &plugin, QString &fileDesc, 
                                          QString &internalName, QString &mimeType, QString &originalFileName, 
                                          QString &productName, QString &companyName, QString &companyKey, 
                                          QString &version)
{
#ifdef Q_OS_WIN
   LPTSTR lpszFilePath = (TCHAR *)filePath.utf16();

   DWORD dwHandle, dwLen;
   UINT BufLen;
   LPTSTR lpData;
   LPVOID lpBuffer;

   dwLen = GetFileVersionInfoSize( lpszFilePath, &dwHandle );
   if (!dwLen)
   {
      DWORD err = GetLastError();
      QString errStr;
      errStr.setNum(err);
      QMessageBox::warning(this, "Firefox Plug-in", QString("File contains no version information (") + errStr + QString(") for ") + filePath);
      return false;
   }

   lpData = (LPTSTR) malloc (dwLen);
   if (!lpData)
   {
      QMessageBox::warning(this, "Firefox Plug-in", QString("Unable to create file version buffer for ") + filePath);
      return false;
   }

   if( !GetFileVersionInfo( lpszFilePath, dwHandle, dwLen, lpData ) )
   {
      DWORD err = GetLastError();
      QString errStr;
      errStr.setNum(err);
      QMessageBox::warning(this, "Firefox Plug-in", QString("Unable to retrieve version information (") + errStr + QString(") for ") + filePath);

      free (lpData);
      return false;
   }
   
   // generate the values and store the destination strings to store the results in
   QStringList entryList;
   QList<QString*> destValues;

   entryList << "Plugin";
   destValues.append(&plugin);

   entryList << "FileDescription";
   destValues.append(&fileDesc);

   entryList << "FileVersion";
   destValues.append(&internalName);

   entryList << "MIMEType";
   destValues.append(&mimeType);

   entryList << "OriginalFilename";
   destValues.append(&originalFileName);

   entryList << "ProductName";
   destValues.append(&productName);

   entryList << "CompanyName";
   destValues.append(&companyName);

   entryList << "CompanyKey";
   destValues.append(&companyKey);

   entryList << "ProductVersion";
   destValues.append(&version);

   wchar_t widearray[100];
   for(int i=0; i<entryList.size(); i++)
   {
      QString entryString = entryList.at(i);
      QString entry = "\\StringFileInfo\\040904e4\\" + entryString;
	  
      entry.toWCharArray(widearray);
      widearray[entry.size()] = '\0';
    
      if( VerQueryValue( lpData, widearray, &lpBuffer, (PUINT)&BufLen ) ) 
      {
         destValues.at(i)->clear();
         destValues.at(i)->append(QString::fromWCharArray((wchar_t *)lpBuffer));
      }
   }

   free (lpData);
#endif
   return true;
}


void Torque3DFrontloader::on_OpenFolderButton_clicked()
{
   ProjectEntry *entry = getSelectedProject();

   if(entry != NULL)
      QDesktopServices::openUrl(QUrl("file:///" + entry->mRootPath));
}

bool Torque3DFrontloader::setResourceString(const QString &filePath, const QString &name, QString &value)
{
   value.clear();

#ifdef Q_OS_WIN
   LPTSTR lpszFilePath = (TCHAR *)filePath.utf16();

   DWORD dwHandle, dwLen;
   UINT BufLen;
   LPTSTR lpData;
   LPVOID lpBuffer;

   dwLen = GetFileVersionInfoSize( lpszFilePath, &dwHandle );
   if (!dwLen) 
      return false;

   lpData = (LPTSTR) malloc (dwLen);
   if (!lpData) 
      return false;

   if( !GetFileVersionInfo( lpszFilePath, dwHandle, dwLen, lpData ) )
   {
      free (lpData);
      return false;
   }

   wchar_t widearray[100];
   QString entry = "\\StringFileInfo\\040904e4\\" + name;
   entry.toWCharArray(widearray);
   widearray[entry.size()] = '\0';
    
   if( VerQueryValue( lpData, widearray, &lpBuffer, (PUINT)&BufLen ) ) 
   {
      value.clear();
      value.append(QString::fromWCharArray((wchar_t *)lpBuffer));
   }

   free (lpData);
#endif
	
   return true;
}

void Torque3DFrontloader::on_GenerateSourceButton_clicked()
{
   generateSourceProject();
}

void Torque3DFrontloader::on_ChangeModulesButton_clicked()
{
   changeProjectModules();
}

void Torque3DFrontloader::readSettings()
{
}

void Torque3DFrontloader::writeSettings()
{
}

void Torque3DFrontloader::renameFile(const QString &filePath, const QString &newName)
{
   // newname is without the extension
   QFile file(QDir::toNativeSeparators(filePath));
   if(file.exists())
   {
      QFileInfo fileInfo(file);
      QString newPath = fileInfo.absolutePath() + "/" + newName;
      bool rename = file.rename(newPath);
      mProgressDialog->updateDetailText("renamed from " + file.fileName() + " ...");
      mProgressDialog->updateDetailText("to " + newPath);
   }
}

void Torque3DFrontloader::renameMacApp(const QString &filePath, const QString &oldName, const QString &bundleName, const QString &newName, const QString &newBundleName, bool processWebPlugin)
{  
   QFile file(QDir::toNativeSeparators(filePath));
   if(file.exists())
   {
      // get the name without .app
      QString baseName = newName;
      baseName = baseName.replace(".app", "");
      // change the executable
      renameFile(filePath + "/Contents/MacOS/" + oldName, baseName);
      // update the plist
      replaceTextInFile(filePath + "/Contents/Info.plist", oldName, baseName);
      
      // now we need to do the same for the bundle as well as change the app's reference to the bundle
      QString bundlePath(filePath + "/Contents/Frameworks/" + bundleName + ".bundle");
      renameFile(bundlePath + "/Contents/MacOS/" + bundleName, newBundleName);
      replaceTextInFile(bundlePath + "/Contents/Info.plist", bundleName, newBundleName);
      renameFile(bundlePath, newBundleName + ".bundle");
      renameFile(filePath, newName);
      
      // lets process the web plugin, which involves creating a new
      if(processWebPlugin)
      {
         
      }
   }
}
