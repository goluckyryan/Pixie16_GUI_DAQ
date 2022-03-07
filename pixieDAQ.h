#include <TQObject.h>
#include <RQ_OBJECT.h>

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class Pixie16;

class MainWindow {
   RQ_OBJECT("MainWindow")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;
   
   Pixie16 * pixie;
   
public:
   MainWindow(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~MainWindow();
   void GoodBye();
   
   void DoDraw();
};
