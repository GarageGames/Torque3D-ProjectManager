#include <QtGui/QApplication>
#include "torque3dfrontloader.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   // application info settings
   QCoreApplication::setOrganizationName("GarageGames");
   QCoreApplication::setOrganizationDomain("garagegames.com");
   QCoreApplication::setApplicationName("Torque 3D Project Manager 2.1");

   QDir::setCurrent(QApplication::applicationDirPath());
   a.setWindowIcon(QIcon(":/Torque3DFrontloader/resources/toolbox-16x16_32.png"));
	
   Torque3DFrontloader::loadStylesheet();

   Torque3DFrontloader w;

   // This is to allow a triggered quit in the instantiation of Torque3DFrontloader to actually quit
   if(!w.mQuit)
   {
      a.connect(&w, SIGNAL(setAppStylesheet(QString)), &a, SLOT(setStyleSheet(QString)));
      w.show();
      w.refreshStylesheet();
      a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
      
      return a.exec();
   }
   else
   {
      return 0;
   }
}
