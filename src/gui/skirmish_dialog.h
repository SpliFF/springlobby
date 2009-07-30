
#ifndef __skirmish_dialog__
#define __skirmish_dialog__

#include <wx/intl.h>

#include <wx/panel.h>
#include "../mmoptionswrapper.h"
#include "../singleplayerbattle.h"

class wxRadioBox;
class wxChoice;
class wxStaticText;
class wxButton;
class wxCheckBox;
class SimpleFront;


///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class skirmish_dialog
///////////////////////////////////////////////////////////////////////////////
class SkirmishDialog : public wxPanel
{
	private:

	protected:
		wxRadioBox* m_radioBox1;
		wxStaticText* m_map_label;
		wxStaticText* m_sides_label;
		wxChoice* m_map;
		wxChoice* m_sides;
		wxCheckBox* m_map_random;
		wxButton* m_back;
		wxButton* m_advanced;
		wxButton* m_start;

		OptionsWrapper& m_mod_customs;
		const wxString m_modname;
		const wxBitmap& m_bg_img;
		SimpleFront* m_parent;


		NoGuiSinglePlayerBattle m_battle;


		// Virtual event handlers, overide them in your derived class
		virtual void OnBack( wxCommandEvent& event );
		virtual void OnAdvanced( wxCommandEvent& event );
		virtual void OnStart( wxCommandEvent& event );
		virtual void OnRadioBox( wxCommandEvent& event );
		virtual void OnRandom( wxCommandEvent& event );

        typedef std::map< wxString, OptionsWrapper >
            SkirmishMap;

        SkirmishMap m_skirmishes;

	public:
		SkirmishDialog( SimpleFront* parent, const wxIcon& app_icon, const wxBitmap& bg_img,
                        const wxString& modname, OptionsWrapper& mod_customs, wxWindowID id = wxID_ANY );
		~SkirmishDialog();

};

#endif //__skirmish_dialog__
