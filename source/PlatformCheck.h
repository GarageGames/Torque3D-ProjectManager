#ifndef PLATFORMCHECK_H
#define PLATFORMCHECK_H

class PlatformCheck
{
public:
   //PlatformCheck() {};
   //NewProjectPage(QWidget *parent = 0);
   //~NewProjectPage();
   static bool mWin;
   static bool mMac;

   static bool isWin() { return PlatformCheck::mWin; };
   static bool isMac() { return PlatformCheck::mMac; };
};

#endif // PLATFORMCHECK_H