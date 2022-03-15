#include <TQObject.h>
#include <RQ_OBJECT.h>

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
//class Pixie16;

class MainWindow {
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;


public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   
   void openPixie();
   
   void getADCTrace();

   void GoodBye();

};
