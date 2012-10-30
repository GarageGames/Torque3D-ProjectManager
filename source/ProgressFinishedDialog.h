#ifndef PROGRESSFINISHEDDIALOG_H
#define PROGRESSFINISHEDDIALOG_H

#include <QDialog>
#include "ui_ProgressFinishedDialog.h"

using namespace Ui;

class ProgressFinishedDialog : public QDialog, public ProgressFinishedDialogClass
{
   Q_OBJECT

public:
   ProgressFinishedDialog(QWidget *parent = 0);
   ~ProgressFinishedDialog();

   void setFolderLocation(QString location);
   void hideFolderButton();
   void setDescriptionText(QString text) { DescriptionText->setText(text); };

private slots:
   void on_OpenFolderButton_clicked();

private:
   QString mFolderLocation;
};

#endif // PROGRESSFINISHEDDIALOG_H
