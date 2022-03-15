#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TROOT.h>
#include <TClass.h>
#include <TGClient.h>
#include <TGMenu.h>

#include "Pixie16Class.h"
#include "mainSettings.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TGNumberEntry;

class MainWindow {
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;
   
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSettings;
   
   TGNumberEntry * modIDEntry, *chEntry; 
   
   Pixie16 * pixie;
   
   MainSettings * mainSettings;


public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   
   
   void HandleMenu(Int_t id);
   
   
   void openPixie();
   
   void getADCTrace();

   void openMainSettings();
   
   

   void GoodBye();

};
