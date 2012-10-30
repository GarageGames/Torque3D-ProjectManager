#include "PlatformCheck.h"
#include <QtGui>

#ifdef Q_WS_WIN
   bool PlatformCheck::mWin = true;
   bool PlatformCheck::mMac = false;
#endif

#ifdef Q_WS_MAC
   bool PlatformCheck::mMac = true;
   bool PlatformCheck::mWin = false;
#endif
