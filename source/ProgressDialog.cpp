#include "ProgressDialog.h"
#include <QLabel>
#include <QSpacerItem>

ProgressDialog::ProgressDialog(QWidget *parent)
   : QDialog(parent)
{
   setupUi(this);

   ResumeButton->hide();
   mProgressFinished = new ProgressFinishedDialog(this);
   mProgressFinished->hide();

   mStagesLayout = new QVBoxLayout();
   StagesWidget->setLayout(mStagesLayout);
   mCurrentStage = 0;
   mCurrentSubStage = 0;

   setRunning(false);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setRunning(bool running)
{
   mRunning = false;

   if(running)
   {
      ResumeButton->setShown(false);
      PauseButton->setShown(true);
      CancelButton->setShown(true);
      CloseButton->setShown(false);
   }
   else
   {
      ResumeButton->setShown(false);
      PauseButton->setShown(false);
      CancelButton->setShown(false);
      CloseButton->setShown(true);
   }
}

// setData is the method that launched the progress dialog
void ProgressDialog::setData(ProgressDialogData *data)
{
   reset();
   mData = data;
   setWindowTitle(mData->mTitle);
   setRunning(true);
   refreshStageList();
   show();

   updateStageList();
   updateSubStage();
   updateDetailsTabList();
}

void ProgressDialog::reset()
{
   mCurrentStage = 0;
   mCurrentSubStage = 0;
   mData = NULL;
   setWindowTitle("");
   setOverallProgress(0);
   clearDetailsTabList();
}

void ProgressDialog::updateDetailText(QString text)
{
   mDetailsTabList.at(mCurrentStage)->append(text);
}

void ProgressDialog::updateSubStage()
{
   ProgressDialogStage *currentStage = mData->mStageList.at(mCurrentStage);
   QString name = currentStage->mSubStageText.at(mCurrentSubStage);
   CurrentStageStatusText->setText(name);
   setSubStageProgress(0);
}

void ProgressDialog::updateDetailsTabList()
{
   QTextEdit *widget = new QTextEdit();
   mDetailsTabList.append(widget);
   DetailsTabWidget->addTab(widget, mData->mStageList.at(mCurrentStage)->mStageName);
   DetailsTabWidget->setCurrentIndex(mCurrentStage);
}

void ProgressDialog::incSubStage()
{
   mCurrentSubStage++;

   if(mCurrentSubStage >= mData->mStageList.at(mCurrentStage)->mSubStageText.size())
   {
      //incStage();
   } else
   {
      updateSubStage();
   }
}

void ProgressDialog::incStage()
{
   // update the stage list to bold the current stage
   mCurrentStage++;

   updateStageList();

   // reset the sub stage
   mCurrentSubStage = 0;

   // now lets reset progress
   updateSubStage();
   setSubStageProgress(0);
   updateDetailsTabList();
}

void ProgressDialog::updateStageList()
{
   for(int i=0; i<mStagesLabelList.size(); i++)
   {
      QFont font = mStagesLabelList.at(i)->font();

      if(i == mCurrentStage)
      {
         font.setBold(true);
      }
      else
      {
         font.setBold(false);
      }

      mStagesLabelList.at(i)->setFont(font);
   }
}

void ProgressDialog::setSubStageProgress(int value)
{
   ProgressDialogStage *currentStage = mData->mStageList.at(mCurrentStage);
   
   double totalSubStagePercentage = QVariant(currentStage->mSubStagePercent.at(mCurrentSubStage)).toDouble();
   double floatPercent = totalSubStagePercentage * (QVariant(value).toDouble() / 100);
   int currentPercent = QVariant(floatPercent).toInt();
   
   // to find out how much progress we've traveled, lets iterate through previous
   // sub stages and add up their total percentages
   double mod = 0;
   for(int i=0; i<mCurrentSubStage; i++)
   {
      mod += QVariant(currentStage->mSubStagePercent.at(i)).toDouble();
   }

   currentPercent += QVariant(mod).toInt();

   setStageProgress(currentPercent);
}

void ProgressDialog::setStageProgress(int value)
{
   mCurrentProgress = value;

   int totalStagePercentage = mData->mStagePercentList.at(mCurrentStage);
   
   if(totalStagePercentage == -1)
   totalStagePercentage = 100 / mData->mStageList.size();

   int currentOverallPercent = QVariant(QVariant(totalStagePercentage).toDouble() * (QVariant(value).toDouble() / 100)).toInt();
   
   // lets add up previous stage percentages
   for(int i=0; i<mCurrentStage; i++)
   {
      int addPercent = mData->mStagePercentList.at(i);
      if(addPercent == -1)
      {
         addPercent = 100 / mData->mStageList.size();
      }

      currentOverallPercent += addPercent;
   }
   
   refreshProgress();
   setOverallProgress(currentOverallPercent);
}

void ProgressDialog::setOverallProgress(int value)
{
   OverallProgressBar->setValue(value);
}
	
void ProgressDialog::refreshProgress()
{
   CurrentStageProgressBar->setValue(mCurrentProgress);
}

void ProgressDialog::refreshStageList()
{
   QLayoutItem *child;
   while ((child = mStagesLayout->takeAt(0)) != 0)
   {
      delete child->widget();
      delete child;
   }
   
   mStagesLabelList.clear();
   
   for(int i=0; i<mData->mStageList.size(); i++)
   {
      QLabel *label = new QLabel(mData->mStageList.at(i)->mStageName);

      mStagesLayout->addWidget(label);
      mStagesLabelList.push_back(label);
   }

   QLabel *finished = new QLabel("Finished");
   mStagesLayout->addWidget(finished);
   mStagesLabelList.push_back(finished);
   mStagesLayout->addStretch();
}

void ProgressDialog::clearDetailsTabList()
{
   DetailsTabWidget->clear();

   while(mDetailsTabList.size() > 0)
   {
      delete mDetailsTabList.takeAt(0);
   }
}

void ProgressDialog::completeProgress()
{
   mCurrentProgress = 100;
   refreshProgress();
   setOverallProgress(100);

   mCurrentStage++;
   updateStageList();
   mCurrentSubStage = 0;
   //hide();
   setRunning(false);
}

void ProgressDialog::done()
{
   completeProgress();
   emit on_success();
}

void ProgressDialog::fail()
{
   completeProgress();
   emit on_fail();
   hide();
}

void ProgressDialog::failWithoutHide()
{
   completeProgress();
   emit on_fail();
}

void ProgressDialog::doneWithDialog(QString title, QString folderLocation, QString descriptionText, bool showFolderButton)
{
   done();
   
   mProgressFinished->setWindowTitle(title);

   if(!folderLocation.compare("") == 0)
   {
      mProgressFinished->setFolderLocation(folderLocation);
   }
   else
   {
      mProgressFinished->hideFolderButton();
   }

   if(showFolderButton)
   {
      mProgressFinished->OpenFolderButton->setVisible(true);
   }
   else
   {
      mProgressFinished->OpenFolderButton->setVisible(false);
   }

   mProgressFinished->setDescriptionText(descriptionText);
   
   mProgressFinished->show();
}

ProgressDialogStage::ProgressDialogStage(QString name)
{
   mStageName = name;
}

void ProgressDialogStage::addSubStage(QString subStageText, int percent)
{
   mSubStageText.push_back(subStageText);
   mSubStagePercent.push_back(percent);
}

ProgressDialogData::ProgressDialogData(QString title)
{
   mTitle = title;
}

void ProgressDialogData::addStage(ProgressDialogStage *stage, int percent)
{
   mStageList.push_back(stage);
   mStagePercentList.push_back(percent);
}

void ProgressDialogData::clearData()
{
   mStageList.clear();
}

void ProgressDialog::on_PauseButton_clicked()
{
   emit pause();
   PauseButton->hide();
   ResumeButton->show();
}

void ProgressDialog::on_ResumeButton_clicked()
{
   emit resume();
   ResumeButton->hide();
   PauseButton->show();
}

void ProgressDialog::on_CancelButton_clicked()
{
   emit cancel();
}