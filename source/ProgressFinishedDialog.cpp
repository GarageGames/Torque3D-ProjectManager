#include "ProgressFinishedDialog.h"
#include <QtGui>

ProgressFinishedDialog::ProgressFinishedDialog(QWidget *parent)
   : QDialog(parent)
{
   setupUi(this);
}

ProgressFinishedDialog::~ProgressFinishedDialog()
{
}

void ProgressFinishedDialog::setFolderLocation(QString location)
{
   LocationText->show();
   mFolderLocation = location;
   LocationText->setText(location);
}

void ProgressFinishedDialog::hideFolderButton()
{
   LocationText->hide();
   OpenFolderButton->setVisible(false);
}

void ProgressFinishedDialog::on_OpenFolderButton_clicked()
{
   QDesktopServices::openUrl(QUrl("file:///" + mFolderLocation));
}