#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QList>
#include <QVBoxLayout>
#include <QTextEdit>
#include "ui_ProgressDialog.h"
#include "ProgressFinishedDialog.h"

using namespace Ui;

// this is a data class meant to be created to control the display of different data 
// in the progress dialog, this way each type of progress dialog just creates a new
// data type and then can set it when upating progress (avoids re-creating multiple similar dialogs)
class ProgressDialogStage
{

public:
   ProgressDialogStage(QString name);

   QStringList mSubStageText;
   QString mStageName;
   // the amount of percentage each substage will take of the entire stage
   QList<int> mSubStagePercent;

   int getSubStageCount() { return mSubStageText.size(); };
   void addSubStage(QString subStageText, int percent);
};

class ProgressDialogData
{
public:
   ProgressDialogData(QString title);

   QList<ProgressDialogStage*> mStageList;
   QList<int> mStagePercentList;
   QString mTitle;

   void addStage(ProgressDialogStage *stage, int percent = -1);
   void clearData();
};

class ProgressDialog : public QDialog, public ProgressDialogClass
{
   Q_OBJECT

private:
   ProgressFinishedDialog *mProgressFinished;
   ProgressDialogData *mData;
   QVBoxLayout *mStagesLayout;
   int mCurrentStage;
   int mCurrentSubStage;
   QList<QLabel*> mStagesLabelList;
   QList<QTextEdit*> mDetailsTabList;
   int mCurrentProgress;
   void updateSubStage();
   bool mRunning;

public:
   ProgressDialog(QWidget *parent = 0);
   ~ProgressDialog();

   void setData(ProgressDialogData *data);

   // progress updating calls
   void setSubStageProgress(int value);
   void setStageProgress(int value);
   void setOverallProgress(int value);
   void refreshProgress();
   void completeProgress();
   void done();
   void doneWithDialog(QString title, QString folderLocation, QString descriptionText, bool showFolderButton = false);
   void setRunning(bool running);
   void reset();

   void fail();
   void failWithoutHide();

public slots:
   void incSubStage();
   void incStage();
   void updateStageList();
   void updateDetailText(QString text);
   void refreshStageList();	
   void clearDetailsTabList();
   void updateDetailsTabList();

signals:
   void pause();
   void resume();
   void cancel();
   void on_fail();
   void on_success();

private slots:
   void on_CancelButton_clicked();
   void on_ResumeButton_clicked();
   void on_PauseButton_clicked();
};

#endif // PROGRESSDIALOG_H
