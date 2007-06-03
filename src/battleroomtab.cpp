//
// Class: BattleRoomTab
//

#include "battleroomtab.h"


BattleRoomTab::BattleRoomTab( wxWindow* parent, Battle& battle ) : wxPanel( parent, -1 ), m_battle(battle) 
{
  // Create all widgets
  m_splitter = new wxSplitterWindow( this, -1);

  m_player_panel = new wxPanel( m_splitter , -1 );
  m_team_sel = new wxComboBox( m_player_panel, -1, _T("1") );
  m_ally_sel = new wxComboBox( m_player_panel, -1, _T("1") );
  m_color_sel = new wxComboBox( m_player_panel, -1, _T("White") );
  m_side_sel = new wxComboBox( m_player_panel, -1, _T("Arm") );

  m_team_lbl = new wxStaticText( m_player_panel, -1, _T("Team") );
  m_ally_lbl = new wxStaticText( m_player_panel, -1, _T("Ally") );
  m_color_lbl = new wxStaticText( m_player_panel, -1, _T("Color") );
  m_side_lbl = new wxStaticText( m_player_panel, -1, _T("Side") );
  m_map_lbl = new wxStaticText( this, -1, _("$Mapname") );

  m_players = new BattleroomListCtrl( m_splitter );
  m_chat = new ChatPanel( m_splitter, battle );

  m_command_line = new wxStaticLine( this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );

  m_leave_btn = new wxButton( this, -1, _T("Leave") );
  m_start_btn = new wxButton( this, -1, _T("Start") );

  m_ready_chk = new wxCheckBox( this, -1, _T("Im ready") );
  
  // Create Sizers
  m_players_sizer = new wxBoxSizer( wxVERTICAL );
  m_player_sett_sizer = new wxBoxSizer( wxHORIZONTAL );
  m_info_sizer = new wxBoxSizer( wxVERTICAL );
  m_top_sizer = new wxBoxSizer( wxVERTICAL );
  m_buttons_sizer = new wxBoxSizer( wxHORIZONTAL );
  m_main_sizer = new wxBoxSizer( wxVERTICAL );

  // Put widgets in place
  
  m_player_sett_sizer->Add( m_team_lbl, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_team_sel, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_ally_lbl, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_ally_sel, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_color_lbl, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_color_sel, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_side_lbl, 0, wxEXPAND );
  m_player_sett_sizer->Add( m_side_sel, 0, wxEXPAND );
  
  m_players_sizer->Add( m_players, 1, wxEXPAND );
  m_players_sizer->Add( m_player_sett_sizer, 0, wxEXPAND );
  
  m_player_panel->SetSizer( m_players_sizer );
  
  m_splitter->SplitHorizontally( m_player_panel, m_chat );
  
  m_info_sizer->Add( m_map_lbl, 0, wxEXPAND );
  
  
  m_top_sizer->Add( m_splitter, 1, wxEXPAND );
  m_top_sizer->Add( m_info_sizer, 0, wxEXPAND );
  
  m_buttons_sizer->Add( m_leave_btn, 0, wxEXPAND );
  m_buttons_sizer->AddStretchSpacer();
  m_buttons_sizer->Add( m_ready_chk, 0, wxEXPAND );
  m_buttons_sizer->Add( m_start_btn, 0, wxEXPAND );

  m_main_sizer->Add( m_top_sizer, 1, wxEXPAND );
  m_main_sizer->Add( m_command_line, 0, wxEXPAND );
  m_main_sizer->Add( m_buttons_sizer, 0, wxEXPAND );
  
  SetSizer( m_main_sizer );
  Layout();
}


BattleRoomTab::~BattleRoomTab()
{
  
}

