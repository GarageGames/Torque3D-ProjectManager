#ifndef _COPY_DIR_H_
#define _COPY_DIR_H_

#include <QThread>
#include <QtGui>
#include <QtCore>

class Torque3DFrontloader;

class CopyDir : public QThread
{
   Q_OBJECT 

public:
   void run();	  
   void setValues(QString srcDir, QString dstDir, QStringList nameExcludeFilter = QStringList(), QStringList nameIncludeFilter = QStringList());
   bool copyDirAndFiles(QString srcDir, QString dstDir, QStringList *nameExcludeFilter, QStringList *nameIncludeFilter, QStringList *srcFileList = NULL, QStringList *dstFileList = NULL);

signals:
   void updateFileCount(int count);
   void updateFileProgress(int count, QString text);
   void fileCopyDone(bool success, bool exitNow);

public slots:
   void pause();
   void resume();
   void exitNow();

private:
   QString mRootDir;
   QString mSrcDir;
   QString mDstDir;
   QStringList mNameExcludeFilter;
   QStringList mNameIncludeFilter;
   bool mPause;
   bool mExitNow;
};

#endif
