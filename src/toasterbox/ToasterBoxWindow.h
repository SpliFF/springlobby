#ifndef TOASTERBOXWINDOW_HH_INCLUDED
#define TOASTERBOXWINDOW_HH_INCLUDED

#include <wx/timer.h>
#include <wx/wx.h>
#include <wx/defs.h>
#include "../utils/mixins.hh"

#if wxUSE_POPUPWIN
	#include <wx/popupwin.h>
	typedef wxPopupWindow
		ToasterBase;
#else
	#include <wx/popupwin.h>
	typedef wxWindow
		ToasterBase;
#endif

class ToasterBoxWindow: public ToasterBase, public wxTimer , public SL::NonCopyable
{
  public:
    ToasterBoxWindow(wxWindow* parent, wxTimer *_parent2);
    void SetPopupText(wxString _text, bool _shrink = false);
    void SetPopupSize(int x, int y);
    void SetPopupPosition(int x, int y);
    void SetPopupPauseTime(int milliseconds){pauseTime = milliseconds;}
    void SetPopupBitmap(wxString bitmapFile);
    void SetPopupBitmap(wxBitmap& bitmap);
    void SetPopupBackgroundColor(int r, int g, int b);
    void SetPopupTextColor(int r, int g, int b);
    void SetPopupScrollSpeed(int _sleepTime){sleepTime = _sleepTime;}
    void PrintInfo();
    wxString GetPopupText(){return popupText;}
    bool Play();
    bool DoesTextFit();
    static long count;

	virtual bool AcceptsFocus() const { return false; }
  private:
    void ScrollUp();
    void ScrollDown();
    void DrawText();
    void Notify();

    wxPoint bottomRight, dialogTop;
    int sleepTime, step;
    //how long the box hangs around for
    int pauseTime;
    wxStaticBitmap sbm;
    wxColour textColor;
    wxPanel *pn;
    wxString popupText, bitmapFile;
    wxTimer *parent2;
    long startTime;
    //should we attempt to shrink the text
    //if it's too big for the popup?
    bool shrink;

    //!two dummy functions that do nothing currently
    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& event );

};

#endif //#define TOASTERBOXWINDOW_HH_INCLUDED
