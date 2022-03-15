#include <TQObject.h>
#include <RQ_OBJECT.h>

#include "Pixie16Class.h"
#include "mainSettings.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;

class MainWindow {
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;

   Pixie16 * pixie;
   
   MainSettings * mainSettings;

public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   
   void openPixie();
   
   void getADCTrace();

   void openMainSettings();

   void GoodBye();

};
