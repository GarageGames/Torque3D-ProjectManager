#ifndef TORQUE3DFRONTLOADER_H
#define TORQUE3DFRONTLOADER_H

#include <QtGui>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include "ui_torque3dfrontloader.h"

#include "ProgressDialog.h"
#include "copyDir.h"
#include "ProjectTree.h"
#include "NewProjectPage.h"
#include "ProjectModuleListPage.h"

#ifdef Q_WS_WIN
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#endif

#ifdef Q_WS_MAC
#include <signal.h>   
#endif


// thread globals
extern QWaitCondition pauseThreads;
extern QMutex mutex;

// forward declaration
class DynamicToolWidget;
class QTcpServer;
class QTcpSocket;
class QHttp;
class QHttpResponseHeader;
class ModuleList;
class ModuleListInstance;

class Torque3DFrontloader : public QMainWindow
{
   Q_OBJECT

public:
   Torque3DFrontloader(QWidget *parent = 0, Qt::WFlags flags = 0);
   ~Torque3DFrontloader();

   bool mQuit;
   bool mFirstTray;
   QString mCommentsUUID;

   // this is to communicate with itself in the instance we run a second instance
   // of this same application, in that situation it should communicate to the existing instance
   // with "Launch" and wait for a "Quit" message to close out, this prevents the rare instance
   // another app my be listening on that port
   QTcpServer tcpServer;
   QTcpSocket tcpClient;
   QTcpSocket *tcpServerConnection;
   void setupTcp();
    
   void hideThings();

   void disableProjectControls();
   void enableProjectControls();

   void setupValues();
   void createActions();
   void createMenus();
   void createProjectList();
   void createProgressData();
   void setupProjectTree();
   void createNewProjectPage();
   void createProjectModuleListPage();
   void startUp();

   void createModuleList();
   ProjectModuleListPage* getProjectModuleListPage() { return mProjectModuleListPage; }

   void setupFileSystemWatcher();

   void readSettings();
   void writeSettings();

#ifdef Q_WS_WIN
   int PauseResumeThreadList(DWORD dwOwnerPID, bool bResumeThread = false);
#endif
   bool getResourceInfo(const QString &filePath, QString &plugin, QString &fileDesc, QString &internalName, 
                        QString &mimeType, QString &originalFileName, QString &productName, QString &companyName, 
                        QString &companyKey, QString &version);


   bool setResourceString(const QString &filePath, const QString &name, QString &value);

   bool deletePath(QString path, bool includeBase = true);
   bool generateProjects(QString rootPath);

   // project packaging steps
   void packageProject(QString projectOutputPath);
   void packageProjectStaging();
   void packageInstaller(bool isWeb = false);
   void packageWeb();
   void packageZip();
   // project creation steps
   void createTemplateCopy();
   void createReformattingFiles();
   void createProjectGeneration();
	
   void replaceTextInFile(QString file, QString srcText, QString dstText);
   void createNewProject(const QString &templatePath, const QString &newProjectPath, ModuleListInstance* moduleInst);

   QPixmap *getProjectPixmap(ProjectEntry *entry);

   void renameFile(const QString &filePath, const QString &newName);
   void renameMacApp(const QString &filePath, const QString &oldName, const QString &bundleName, const QString &newName, const QString &newBundleName, bool processWebPlugin = false);

   QString mDumpData;
   void dumpData();

   ProjectList *mProjectList;
   ProjectList *getProjectList() { return mProjectList; };

   ModuleList* mModuleList;
   ModuleList* getModuleList() { return mModuleList; };

   bool mNewProjectSet;
   ProjectEntry *mSelectedProject;
   ProjectEntry *getSelectedProject() { return mSelectedProject; };
   ProjectEntry *mSelectedApp;
   void setSelectedApp(ProjectEntry *app, bool skipWidgetUpdate = true);
   ProjectEntry *getSelectedApp() { return mSelectedApp; };

   // path variables
   QString mStagingPath;
   QString mBaseAppPath;
   QString mPackagingPath;
   QString mOutputPath;
   QString mDataPath;
   QString mCommentsPath;
   QString mPackagingGameOutputPath;
   // packaging path variables
   QString mNSISTemplatePath;
   QString mNSISDefaultTemplateName;
   QString mNSISScriptPath;
   QString mNSISAppPath;
   QString mNSISAppName;
   QString mUserProjectPath;
   QString mZipAppPath;
   QString mZipAppName;
   QString mPMTemplatePath;
   QString mPMDefaultTemplateName;
   QString mPMScriptPath;
   QString mPMAppPath;
   QString mPMAppName;

   QString mAppStyleSheetPath;

   QFileSystemWatcher mFileSystemWatcher;

protected:
   void resizeEvent(QResizeEvent *event);
   void closeEvent(QCloseEvent *event);
   void hideEvent(QHideEvent *event);

private:
   Ui::Torque3DFrontloaderClass ui;
   NewProjectPage *mNewProjectPage;
   ProjectModuleListPage* mProjectModuleListPage;

   QHttp *mHttp;

   // menus
   QMenu *mFileMenu;
   QMenu *mEditMenu;
	  
   // actions
   QAction *minimizeAction;
   QAction *maximizeAction;
   QAction *restoreAction;
   QAction *quitAction;

   QAction *newProjectAction;
   QAction *deleteProjectAction;

   QAction *clearSettingsAction;

   CopyDir mCopyDir;
	
   QString mTestPath;

   QHBoxLayout *mDynamicToolBarLayout;
   QProcess *mProcess;
   QProcess *mProjectGenerationProcess;
   QProcess *mZipProcess;

   // ProgressDialog class
   ProgressDialog *mProgressDialog;

   // Packaging progress dialog stages
   ProgressDialogStage *mPackageStagingStage;
   ProgressDialogStage *mPackageZipStage;
   ProgressDialogStage *mPackageInstallerStage;
   ProgressDialogStage *mPackageWebStage;

   // Packaging progress dialog data
   ProgressDialogData *mPackageData;

   // Create new project progress dialog stages
   ProgressDialogStage *mCreateTemplateCopyStage;
   ProgressDialogStage *mCreateReformattingFilesStage;
   ProgressDialogStage *mCreateProjectGenerationStage;

   // Create new project progress dialog data
   ProgressDialogData *mCreateData;

   bool mPackagingStagingDone;
   bool mPackagingZipDone;
   bool mPackagingInstallerDone;
   bool mPackagingWebDone;

   bool mPackagingStaging;
   bool mPackagingZip;
   bool mPackagingInstaller;
   bool mPackagingWeb;

   // staging variables
   int mStagingFile;
   int mStagingFileCount;
	
   // installer variables
   int mInstallerFileCount;
   bool mNSISExitNow;
   bool mPMExitNow;
	
   // zip variables
   int mZipFileCount;
   bool mZipExitNow;
	
   // create project variables
   int mCreateProjectFileCount;
   int mCreateProjectFile;
   QString mNewProjectPath;
    
   bool mCreateTemplateCopyDone;
   bool mCreateReformattingFilesDone;
   bool mCreateProjectGenerationDone;

   bool mCreateTemplateCopy;
   bool mCreateReformattingFiles;
   bool mCreateProjectGeneration;

   bool mCreateReformattingFilesPause;
   bool mProjectGenerationExitNow;
   int mProjectGenerationCount;

   QDir mTemplateDir;
   QDir mNewProjectDir;
   ModuleListInstance* mNewProjectModuleList;
   ModuleListInstance* mChangeProjectModulesList;

   // startup profiling
   QString mCurrentProfileStep;
   QTime mOverallProfileTime;
   QTime mCurrentProfileStepTime;
   QString mProfileDump;

   void initProfile();
   void startProfile(const QString &step);
   void stopProfile();
   void dumpProfile();

public slots:
   void pauseProcess(QProcess *process);
   void resumeProcess(QProcess *process);

   void clearSettings();
   void openSourceCode();
   void changeProjectModules();
   void generateSourceProject();


   // file system watcher slots
   void fileChanged(const QString &path);
   void directoryChanged(const QString &path);

   // tcp connection slots for handling a second app instance trying to run
   void updateServerProgress();
   void updateClientProgress();
   void acceptConnection();
   void startTransfer();
   void sendLaunchMessage(bool sendLaunch = true);

   void minimizeApp();
   void maximizeApp();
   void clearEditorSettings();

   void setSelectedProject(ProjectEntry *project);
   void updateSelectedProjectInfo();
   bool setSelectedProjectByUniqueName(const QString &uniqueName, bool setFirstIfNot = true);
   void setFirstSelectedProject();

   void refreshStylesheet();
   static void loadStylesheet();
    
   // general create slots
   void createPause();
   void createResume();
   void createCancel();
   void createProjectCheck();
   void createClear();

   // create project file copy slots
   void updateCreateProjectProgress(int count, QString detailText);
   void updateCreateProjectCount(int count);
   void updateCreateProjectDone(bool success, bool exitNow);

   // create project reformatting files slots
   void createReformattingFilesPause();
   void createReformattingFilesResume();

   // create project project generation slots
   void projectGenerationStarted();
   void projectGenerationError();
   void projectGenerationFinished(int exitCode, QProcess::ExitStatus exitStatus);
   void projectGenerationExitNow();
   void projectGenerationErrorWrite();
   void projectGenerationStandardWrite();

   // project tree generation slots
   void projectEntryRemoved(ProjectEntry *entry);
   void projectRemovalDone();
   void projectEntryAdded(ProjectEntry *entry);
   void projectCategoryAdded(QString title);
   void projectCategoryRemoved(QString title);

private slots:
   void on_GenerateSourceButton_clicked();
   void on_ChangeModulesButton_clicked();
   void on_OpenFolderButton_clicked();
   void on_CreateNewProjectButton_clicked();
   //void on_ProjectTree_clicked(const QModelIndex &index);

   void iconActivated(QSystemTrayIcon::ActivationReason reason);
   void hideApp();

signals:
   void setAppStylesheet(QString styleSheet);
   void setCopyThreadPause();
   void setCopyThreadResume();
   void setCopyThreadExit();

   void setZipThreadPause();
   void setZipThreadResume();
   void setZipThreadExit();

   void setInstallerThreadPause();
   void setInstallerThreadResume();
   void setInstallerThreadExit();

   void setWebThreadPause();
   void setWebThreadResume();
   void setWebThreadExit();

   void setProjectGenerationThreadPause();
   void setProjectGenerationThreadResume();
   void setProjectGenerationThreadExit();

   void setReformattingFilesThreadPause();
   void setReformattingFilesThreadResume();
   void setReformattingFilesThreadExit();
};

#endif // TORQUE3DFRONTLOADER_H

