#include "NewProjectPage.h"
#include "torque3dfrontloader.h"
#include "moduleListInstance.h"

NewProjectPage::NewProjectPage(QWidget *parent)
   : QWidget(parent)
{
   setupUi(this);
   mFrontloader = NULL;
   mCurrentInstance = NULL;

   TemplatePreviewSizeLabel->setVisible(false);
   TemplatePreviewSizeValue->setVisible(false);

   connect(TemplateList, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_TemplateList_textChanged(const QString &)));
}

NewProjectPage::~NewProjectPage()
{
}

void NewProjectPage::setFrontloader(Torque3DFrontloader *frontloader)
{
   mFrontloader = frontloader;
}

void NewProjectPage::launch()
{
   setDefaults();
   show();
}

void NewProjectPage::setDefaults()
{
   if(mFrontloader != NULL)
   {
      buildTemplateList();
      QString defaultPath = getValidProjectName("New Project");
      QFileInfo defaultFilePath(defaultPath);
   
      // set the default project name
      NameTextEdit->setText(defaultFilePath.fileName());

      // set the default location
      DirectoryTextEdit->setText(defaultFilePath.absolutePath());

      // Default module list instance
      mCurrentInstance = new ModuleListInstance();
      mCurrentInstance->buildInstances(mFrontloader->getModuleList());
   }
}

void NewProjectPage::buildTemplateList()
{
   QMultiMap<QString, ProjectEntry*> *dirList = mFrontloader->getProjectList()->getTemplateDirectoryList();

   QList<ProjectEntry*> entryList = dirList->values("Templates");

   mTemplateNameList.clear();
   TemplateList->clear();

   for(int i=0; i<entryList.size(); i++)
   {
      ProjectEntry *entry = entryList.at(i);

      if(mTemplateNameList.values(entry->mName).size() == 0)
      {
         mTemplateNameList.insert(entry->mName, entry);

         QPixmap *pixmap = NULL;
         if(mFrontloader != NULL)
         {
            pixmap = mFrontloader->getProjectPixmap(entry);
         }

         if(pixmap == NULL)
         {
            TemplateList->addItem(entry->mName);
         }
         else
         {
            QIcon icon(pixmap->scaled(16, 16));
            TemplateList->addItem(icon, entry->mName);
         }
      }
   }
}

void NewProjectPage::on_TemplateList_textChanged(const QString &text)
{
   ProjectEntry *entry = mTemplateNameList.value(text);

   if(entry != NULL)
   {
      // Update the preview image
      QPixmap *pixmap = mFrontloader->getProjectPixmap(entry);
      if(pixmap != NULL)
      {
         PreviewImage->setPixmap(pixmap->scaled(PreviewImage->size()));
      }
      else
      {
         PreviewImage->setText("No Preview Image");
      }

      QFileInfo fileInfo(entry->mRootPath);

      // update the size
      int size = 0;
      QString sizeString;
      sizeString.setNum(size);

      // update the last update value
      QDateTime lastModified = fileInfo.lastModified();

      // update the created date value
      QDateTime createdDate = fileInfo.created();

      TemplatePreviewSizeValue->setText(sizeString);
      TemplatePreviewLastUpdateValue->setText(lastModified.toString(QString("ddd MMM d hh:mm ap")));
      TemplatePreviewCreatedDateValue->setText(createdDate.toString(QString("ddd MMM d hh:mm ap")));
   }
}

QString NewProjectPage::getValidProjectName(QString projectName)
{
   QString basePath = mFrontloader->mUserProjectPath + QDir::separator() + projectName;
   QDir dir(basePath);

   int i = 0;
   while(dir.exists())
   {
      dir.setPath(basePath + QString().setNum(i));
      i++;
   }

   return dir.path();
}

void NewProjectPage::on_newProjectCreateButton_clicked()
{
   if(mFrontloader != NULL)
   {
      ProjectEntry *entry = mTemplateNameList.value(TemplateList->currentText());

      //If a project with no path is specified, the project will delete the "My Projects"
      //Folder and then present the user with an error that the project couldn't be created.
      //Must check if the project name is valid first.
      if (NameTextEdit->text().compare("") == 0)
      {
         QMessageBox::critical(this, tr("Project Name Blank"), tr("Please include a name for your project."), QMessageBox::Ok);
         return;
      }

      if(entry != NULL)
      {
         QString templatePath = QDir::toNativeSeparators(entry->mRootPath);
         QString newProjectPath = QDir::toNativeSeparators(DirectoryTextEdit->text() + "/" + NameTextEdit->text());
         mFrontloader->createNewProject(templatePath, newProjectPath, mCurrentInstance);
      }
   }
}

void NewProjectPage::on_newChooseModulesButton_clicked()
{
   if(mFrontloader && mFrontloader->getProjectModuleListPage())
   {
      mFrontloader->getProjectModuleListPage()->launch(mCurrentInstance, false);
   }
}
