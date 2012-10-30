#include "copyDir.h"
#include "torque3dfrontloader.h"

void CopyDir::run()
{  
   mPause = false;
   mExitNow = false;
   bool success = copyDirAndFiles(mSrcDir, mDstDir, &mNameExcludeFilter, &mNameIncludeFilter);

   mutex.lock();
   if(mPause)
   {
      pauseThreads.wait(&mutex);
   }
   mutex.unlock();
   
   emit fileCopyDone(success, mExitNow);
}

void CopyDir::setValues(QString srcDir, QString dstDir, QStringList nameExcludeFilter, QStringList nameIncludeFilter)
{
   mRootDir = srcDir;
   mSrcDir = srcDir;
   mDstDir = dstDir;
   mNameExcludeFilter = nameExcludeFilter;
   mNameIncludeFilter = nameIncludeFilter;
}

bool CopyDir::copyDirAndFiles(QString srcDir, QString dstDir, QStringList *nameExcludeFilter, QStringList *nameIncludeFilter, QStringList *srcFileList, QStringList *dstFileList)
{
   if(mExitNow)
     return false;

   mutex.lock();
   if(mPause)
   {
      pauseThreads.wait(&mutex);
   }
   mutex.unlock();

   bool isFirst = false;

   if(srcFileList == NULL)
   {
      isFirst = true;
      srcFileList = new QStringList;
      dstFileList = new QStringList;
   }

   QDir rootPath(QDir::toNativeSeparators(srcDir));
   QDir destPath(QDir::toNativeSeparators(dstDir));

   rootPath.setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
   QFileInfoList entryList = rootPath.entryInfoList();

   QRegExp rx;
   rx.setPatternSyntax(QRegExp::Wildcard);
   bool moveOn;
   QString dirName;

   for(int i=0; i<entryList.size(); i++)
   {
      if(mExitNow)
      {
	      return false;
      }

	   QFileInfo entry = entryList.at(i);

	   // we do exclude list checking, a lot easier to exclude a couple file types than list all of the included ones
      moveOn = false;
      for(int j=0; j<nameExcludeFilter->size(); j++)
      {
	      rx.setPattern(nameExcludeFilter->at(j));
		   QString name = entry.absoluteFilePath();
	      if(rx.exactMatch(name))
	      {
			   moveOn = true;

			   // now let's check to make sure this specific file isn't on the include list, that overrides exluded types
			   for(int k=0; k<nameIncludeFilter->size(); k++)
			   {
			      // compare the file to the include file adding the root directory (the include filter should be relative files)
			      QString filePath = QDir::toNativeSeparators(entry.filePath());
			      QString include = QDir::toNativeSeparators(mRootDir + "/" + nameIncludeFilter->at(k));
               if(include.compare(filePath) == 0)
   			   {
                     moveOn = false;
		      		  break;
			      }
   			}

			   break;
	      }
	   }

	   // if we have been matched against the exclude list then lets skip
      if(moveOn)
         continue;

      // now we copy it over
      if(entry.isDir())
      {
         dirName = entry.fileName();
         bool pathCreated = false;
         QString targetSubPath(QDir::separator() + dirName);

         if(QDir(destPath.path() + targetSubPath).exists())
         {
            pathCreated = true;
         }
         else
         {
            pathCreated = destPath.mkdir(dirName);
         }

         if(pathCreated)
         {
            copyDirAndFiles(srcDir + QDir::separator() + dirName, destPath.path() + QDir::separator() + dirName, nameExcludeFilter, nameIncludeFilter, srcFileList, dstFileList);
         }
      }
      else if(entry.isFile())
      {
         srcFileList->push_back(entry.absoluteFilePath());
         dstFileList->push_back(dstDir + QDir::separator() + entry.fileName());
      }

   }

   if(isFirst)
   {
      emit updateFileCount(srcFileList->size());

      for(int i=0; i<srcFileList->size(); i++)
      {
         if(mExitNow)
            return false;

         QFile fileEntry(srcFileList->at(i));
         fileEntry.copy(dstFileList->at(i));

         mutex.lock();
         if(mPause)
         {
            pauseThreads.wait(&mutex);
         }
         mutex.unlock();

         if(mExitNow)
            return false;

         emit updateFileProgress(i+1, QFileInfo(fileEntry.fileName()).fileName());
      }
   }

   return true;
}

void CopyDir::pause()
{
   mPause = true;
}

void CopyDir::resume()
{
   mPause = false;
}

void CopyDir::exitNow()
{
   mExitNow = true;
}
