#include "ProjectModuleListPage.h"
#include "torque3dfrontloader.h"
#include "moduleList.h"
#include "moduleListInstance.h"

ProjectModuleListPage::ProjectModuleListPage(QWidget *parent)
   : QWidget(parent)
{
   setupUi(this);
   mFrontloader = NULL;
   mCurrentInstance = NULL;

   //TemplatePreviewSizeLabel->setVisible(false);
   //TemplatePreviewSizeValue->setVisible(false);

   //connect(TemplateList, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_TemplateList_textChanged(const QString &)));

   //mScrollContent = new QWidget(moduleGroupBox);
   mScrollContent = NULL;
   mScrollContentLayout = new QVBoxLayout(moduleGroupBox);
   mScrollContentLayout->setSpacing(0);
   mScrollContentLayout->setMargin(0);
   mScrollContentLayout->setContentsMargins(5, 5, 5, 5);

   mModuleScroll = new QScrollArea();
   mModuleScroll->setWidgetResizable(true);
   mModuleScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   mModuleScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

   mScrollContentLayout->addWidget(mModuleScroll);

   mModuleContent = new QWidget(this);  
   mModuleContentLayout = new QVBoxLayout(mModuleContent);
   mModuleContentLayout->setSpacing(0);
   mModuleContentLayout->setMargin(0);
   mModuleContentLayout->setContentsMargins(5, 5, 5, 5);
   mModuleScroll->setWidget(mModuleContent);
   mModuleContent->setObjectName("ModuleScrollContent");

   mModuleButtons = new QButtonGroup();
   mModuleButtons->setExclusive(false);
   mProjectDefineButtons = new QButtonGroup();
   mProjectDefineButtons->setExclusive(false);
}

ProjectModuleListPage::~ProjectModuleListPage()
{
   delete mModuleButtons;
   delete mProjectDefineButtons;

   // Delete all module group button groups
   while(mModuleGroupButtons.count() > 0)
   {
      QButtonGroup* bg = mModuleGroupButtons.at(0);
      mModuleGroupButtons.removeAt(0);
      delete bg;
   }
}

void ProjectModuleListPage::setFrontloader(Torque3DFrontloader *frontloader)
{
   mFrontloader = frontloader;
}

void ProjectModuleListPage::launch(ModuleListInstance* instance, bool regenerate)
{
   if(regenerate)
   {
      ProjectModuleListOKButton->setVisible(false);
      ProjectModuleListRegenButton->setVisible(true);
   }
   else
   {
      ProjectModuleListOKButton->setVisible(true);
      ProjectModuleListRegenButton->setVisible(false);
   }

   mCurrentInstance = instance;

   //setDefaults();
   populateControls();
   show();
}

void ProjectModuleListPage::addProjectGenItem(QLayout* contentLayout, ProjectGenItem* item, int index, QButtonGroup* buttonGroup, const QString& path, bool checked, bool isRadio)
{
   QWidget *widget = new QWidget();
   QVBoxLayout *layout = new QVBoxLayout();
   layout->setSpacing(0);
   layout->setMargin(0);
   layout->setContentsMargins(5, 1, 5, 1);
   widget->setLayout(layout);

   if(isRadio)
   {
      QRadioButton* button = new QRadioButton(item->mDescription);
      if(checked)
      {
         button->setChecked(true);
      }
      layout->addWidget(button);
      buttonGroup->addButton(button);
      buttonGroup->setId(button, index);
   }
   else
   {
      QCheckBox* button = new QCheckBox(item->mDescription);
      if(checked)
      {
         button->setChecked(true);
      }
      layout->addWidget(button);
      buttonGroup->addButton(button);
      buttonGroup->setId(button, index);

      // Is this a module with a path?
      ModuleEntry* mod = dynamic_cast<ModuleEntry*>(item);
      if(mod && !mod->mPath.isEmpty())
      {
         QWidget *pathwidget = new QWidget();
         QVBoxLayout *pathlayout = new QVBoxLayout();
         pathlayout->setSpacing(0);
         pathlayout->setMargin(0);
         pathlayout->setContentsMargins(20, 0, 0, 0);
         pathwidget->setLayout(pathlayout);

         QLabel* label = new QLabel(mod->mPath + ":");
         pathlayout->addWidget(label);

         QWidget *editwidget = new QWidget();
         QHBoxLayout *editlayout = new QHBoxLayout();
         editlayout->setSpacing(0);
         editlayout->setMargin(0);
         editlayout->setContentsMargins(0, 0, 0, 0);
         editwidget->setLayout(editlayout);

         QLineEdit* edit = new QLineEdit();
         edit->setObjectName("PathEdit");
         edit->setText(path);
         editlayout->addWidget(edit);

         DirectorySelectButton* filebutton = new DirectorySelectButton();
         filebutton->setText("...");
         filebutton->setObjectName("PathButton");
         filebutton->setMaximumWidth(20);
         editlayout->addWidget(filebutton);

         pathlayout->addWidget(editwidget);

         layout->addWidget(pathwidget);
      }
   }

   // Add the item to the list
   contentLayout->addWidget(widget);
}

void ProjectModuleListPage::populateControls()
{
   // Set up the move classes and networking
   MoveClassesList->clear();
   if(mCurrentInstance)
   {
      int defaultItem = 0;

      ModuleList* list = mFrontloader->getModuleList();
      ModuleMoveClassList* moveList = list->getMoveClassList();
      for(int i=0; i<moveList->count(); ++i)
      {
         MoveClassEntry* entry = moveList->at(i);
         MoveClassesList->addItem(entry->mDescription);
         if(mCurrentInstance->mMoveClassIndex == i)
         {
            defaultItem = i;
         }
      }
      MoveClassesList->setCurrentIndex(defaultItem);
   }
   else
   {
      MoveClassesList->addItem("Standard Move Class");
   }

   // Delete all entries in the module list
   QLayoutItem *child;
   while ((child = mModuleContentLayout->takeAt(0)) != 0)
   {
      delete child->widget();
      delete child;
   }

   // Delete all module group button groups
   while(mModuleGroupButtons.count() > 0)
   {
      QButtonGroup* bg = mModuleGroupButtons.at(0);
      mModuleGroupButtons.removeAt(0);
      delete bg;
   }

   // Build out the module list
   if(mCurrentInstance)
   {
      ModuleList* list = mFrontloader->getModuleList();

      // Module Groups
      for(int i=0; i<mCurrentInstance->mModuleGroupInstances.count(); ++i)
      {
         ProjectGenInstance* entryInst = mCurrentInstance->mModuleGroupInstances.at(i);
         ModuleEntry* entry = static_cast<ModuleEntry*>(entryInst->mProjectGenItem);

         QGroupBox* widget = new QGroupBox();
         widget->setTitle(entry->mDescription);

         QVBoxLayout *layout = new QVBoxLayout();
         layout->setSpacing(0);
         layout->setMargin(0);
         layout->setContentsMargins(5, 5, 5, 5);
         widget->setLayout(layout);

         QButtonGroup* bg = new QButtonGroup();
         mModuleGroupButtons.push_back(bg);

         // Add each of the modules for the group
         for(int j=0; j<entry->mModules.count(); ++j)
         {
            ModuleEntry* module = entry->mModules.at(j);
            addProjectGenItem(layout, module, j, bg, "", (j==entryInst->mGroupIndex), true);
         }

         // Add the module to the list
         mModuleContentLayout->addWidget(widget);
      }

      // Modules
      for(int i=0; i<mCurrentInstance->mModuleInstances.count(); ++i)
      {
         ProjectGenInstance* entryInst = mCurrentInstance->mModuleInstances.at(i);
         ModuleEntry* entry = static_cast<ModuleEntry*>(entryInst->mProjectGenItem);

         addProjectGenItem(mModuleContentLayout, entry, i, mModuleButtons, entryInst->mPathData, entryInst->mState);
      }

      // Project defines
      for(int i=0; i<mCurrentInstance->mProjectDefInstances.count(); ++i)
      {
         ProjectGenInstance* entryInst = mCurrentInstance->mProjectDefInstances.at(i);
         ProjectDefineEntry* entry = static_cast<ProjectDefineEntry*>(entryInst->mProjectGenItem);

         addProjectGenItem(mModuleContentLayout, entry, i, mProjectDefineButtons, entryInst->mPathData, entryInst->mState);
      }
   }
}

void ProjectModuleListPage::copyToModuleListInstance()
{
   if(!mCurrentInstance)
      return;

   // Copy over move class data
   mCurrentInstance->mMoveClassIndex = MoveClassesList->currentIndex();

   // Copy over module group data
   for(int i=0; i<mModuleGroupButtons.count(); ++i)
   {
      if(i < mCurrentInstance->mModuleGroupInstances.count())
      {
         QButtonGroup* bg = mModuleGroupButtons.at(i);
         mCurrentInstance->mModuleGroupInstances.at(i)->mGroupIndex = bg->checkedId();
      }
   }

   // Copy over module data
   for(int i=0; i<mModuleButtons->buttons().count(); ++i)
   {
      QAbstractButton* button = mModuleButtons->buttons().at(i);
      int id = mModuleButtons->id(button);
      if(id < mCurrentInstance->mModuleInstances.count())
      {
         mCurrentInstance->mModuleInstances.at(id)->mState = button->isChecked();

         // Retrieve any path data
         QObject* parentObj = button->parent();
         QLineEdit* edit = parentObj->findChild<QLineEdit*>("PathEdit");
         if(edit)
         {
            mCurrentInstance->mModuleInstances.at(id)->mPathData = edit->text();
         }
      }
   }

   // Copy over project define data
   for(int i=0; i<mProjectDefineButtons->buttons().count(); ++i)
   {
      QAbstractButton* button = mProjectDefineButtons->buttons().at(i);
      int id = mProjectDefineButtons->id(button);
      if(id < mCurrentInstance->mProjectDefInstances.count())
      {
         mCurrentInstance->mProjectDefInstances.at(id)->mState = button->isChecked();

         // Retrieve any path data
         QObject* parentObj = button->parent();
         QLineEdit* edit = parentObj->findChild<QLineEdit*>("PathEdit");
         if(edit)
         {
            mCurrentInstance->mProjectDefInstances.at(id)->mPathData = edit->text();
         }
      }
   }
}

void ProjectModuleListPage::on_ProjectModuleListRegenButton_clicked()
{
   copyToModuleListInstance();
   close();

   // Write out the module instance to the project file
   if(!mCurrentInstance->mFileSource.isEmpty())
   {
      mCurrentInstance->replaceProjectFileContents(mCurrentInstance->mFileSource);
   }

   // Regenerate the projects
   if(mFrontloader)
   {
      mFrontloader->generateSourceProject();
   }
}

void ProjectModuleListPage::on_ProjectModuleListOKButton_clicked()
{
   copyToModuleListInstance();
   close();
}

//-----------------------------------------------------------------------------

DirectorySelectButton::DirectorySelectButton(QWidget *parent)
   : QPushButton(parent)
{
   connect(this, SIGNAL(clicked()), this, SLOT(chooseDirectory()));
}

void DirectorySelectButton::chooseDirectory()
{
   // Get the directory
   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   // Populate the directory edit control if a directory was chosen
   if(!dir.isNull())
   {
      QObject* parentObj = this->parent();
      QLineEdit* edit = parentObj->findChild<QLineEdit*>("PathEdit");
      if(edit)
      {
         edit->setText(dir);
      }
   }
}
