/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */
//
// Class: BattleroomListCtrl
//

#include <wx/intl.h>
#include <wx/menu.h>
#include <wx/textdlg.h>
#include <wx/colordlg.h>
#include <wx/colour.h>
#include <stdexcept>
#include <vector>

#include "battleroomlistctrl.h"
#include "iconimagelist.h"
#include "iunitsync.h"
#include "battle.h"
#include "uiutils.h"
#include "ui.h"
#include "user.h"
#include "server.h"
#include "utils.h"
#include "uiutils.h"
#include "countrycodes.h"

#include "settings++/custom_dialogs.h"

#define TOOLTIP_DELAY 1000

BEGIN_EVENT_TABLE( BattleroomListCtrl,  customListCtrl)

  EVT_LIST_ITEM_RIGHT_CLICK( BRLIST_LIST, BattleroomListCtrl::OnListRightClick )
  EVT_LIST_COL_CLICK       ( BRLIST_LIST, BattleroomListCtrl::OnColClick )
  EVT_MENU                 ( BRLIST_SPEC, BattleroomListCtrl::OnSpecSelect )
  EVT_MENU                 ( BRLIST_KICK, BattleroomListCtrl::OnKickPlayer )
  EVT_MENU                 ( BRLIST_RING, BattleroomListCtrl::OnRingPlayer )
  EVT_MENU                 ( BRLIST_COLOUR, BattleroomListCtrl::OnColourSelect )
  EVT_MENU                 ( BRLIST_HANDICAP, BattleroomListCtrl::OnHandicapSelect )
#if wxUSE_TIPWINDOW
#ifndef __WXMSW__ //disables tooltips on win
  EVT_MOTION(BattleroomListCtrl::OnMouseMotion)
#endif
#endif
END_EVENT_TABLE()

Ui* BattleroomListCtrl::m_ui_for_sort = 0;

BattleroomListCtrl::BattleroomListCtrl( wxWindow* parent, Battle& battle, Ui& ui ) :
	customListCtrl(parent, BRLIST_LIST, wxDefaultPosition, wxDefaultSize,
                wxSUNKEN_BORDER | wxLC_REPORT | wxLC_SINGLE_SEL, _T("BattleroomListCtrl") ),
	m_battle(battle),
  m_sel_user(0), m_sel_bot(0),
  m_ui(ui)
{
  wxListItem col;

  col.SetText( _T("r") );
  col.SetImage(icons().ICON_NONE );
  InsertColumn( 0, col, _T("Player/Bot") );

  col.SetText( _T("s") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 1, col,_T("Faction icon") );

  col.SetText( _T("c") );
  col.SetImage(  icons().ICON_NONE);
  InsertColumn( 2, col, _T("Teamcolour") );

  col.SetText( _T("f") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 3, col, _T("Country") );

  col.SetText( _T("r") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 4, col, _T("Rank") );

  col.SetText( _("Nickname") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 5, col, _T("Ingame name"));

  col.SetText( _("t") );
  col.SetImage(icons().ICON_NONE );
  InsertColumn( 6, col, _T("Team number") );

  col.SetText( _("a") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 7, col, _T("Ally number") );

  col.SetText( _("cpu") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 8, col, _T("CPU speed (might not be accurate)") );

  col.SetText( _("Resource Bonus") );
  col.SetImage( icons().ICON_NONE );
  InsertColumn( 9, col, _T("Resource Bonus") );

  m_sortorder[0].col = 7;
  m_sortorder[0].direction = true;
  m_sortorder[1].col = 6;
  m_sortorder[1].direction = true;
  m_sortorder[2].col = 5;
  m_sortorder[2].direction = true;
  Sort( );

  SetColumnWidth( 0, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 1, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 2, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 3, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 4, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 6, wxLIST_AUTOSIZE_USEHEADER );
  SetColumnWidth( 7, wxLIST_AUTOSIZE_USEHEADER );

  SetColumnWidth( 5, 170 );

  SetColumnWidth( 8, 80 );
  SetColumnWidth( 9, 130 );

  m_popup = new wxMenu();
  wxMenu* m_teams;
  m_teams = new wxMenu();

  for ( int i = 0; i < 16; i++ ) {
    wxMenuItem* team = new wxMenuItem( m_teams, BRLIST_TEAM + i, wxString::Format( _T("%d"), i+1 ) , wxEmptyString, wxITEM_NORMAL );
    m_teams->Append( team );
    Connect( BRLIST_TEAM + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BattleroomListCtrl::OnTeamSelect ) );
  }
  m_popup->Append( -1, _("Team"), m_teams );

  wxMenu* m_allies = new wxMenu();
  for ( int i = 0; i < 16; i++ ) {
    wxMenuItem* ally = new wxMenuItem( m_allies, BRLIST_ALLY + i, wxString::Format( _T("%d"), i+1 ) , wxEmptyString, wxITEM_NORMAL );
    m_allies->Append( ally );
    Connect( BRLIST_ALLY + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BattleroomListCtrl::OnAllySelect ) );
  }
  m_popup->Append( -1, _("Ally"), m_allies );

  m_sides = new wxMenu();
  try {
    for ( int i = 0; i < usync().GetSideCount( m_battle.GetHostModName() ); i++ ) {
      wxMenuItem* side = new wxMenuItem( m_sides, BRLIST_SIDE + i, usync().GetSideName( m_battle.GetHostModName(), i ), wxEmptyString, wxITEM_NORMAL );
      m_sides->Append( side );
      Connect( BRLIST_SIDE + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( BattleroomListCtrl::OnSideSelect ) );
    }
  } catch (...) {}
  m_popup->Append( -1, _("Side"), m_sides );

  m_popup->AppendSeparator();

  wxMenuItem* m_colours = new wxMenuItem( m_popup, BRLIST_COLOUR, _("Set color"), wxEmptyString, wxITEM_NORMAL );
  m_popup->Append( m_colours );

  m_popup->AppendSeparator();

  m_handicap_item = new wxMenuItem( m_popup, BRLIST_HANDICAP, _("Set Resource Bonus"), wxEmptyString, wxITEM_NORMAL );
  m_popup->Append( m_handicap_item );

  m_popup->AppendSeparator();

  m_spec_item = new wxMenuItem( m_popup, BRLIST_SPEC, wxString( _("Spectator") ) , wxEmptyString, wxITEM_CHECK );
  m_popup->Append( m_spec_item );

  m_popup->AppendSeparator();

  wxMenuItem* kick = new wxMenuItem( m_popup, BRLIST_KICK, wxString( _("Kick") ) , wxEmptyString, wxITEM_NORMAL );
  m_popup->Append( kick );
  wxMenuItem* ring = new wxMenuItem( m_popup, BRLIST_RING, wxString( _("Ring") ) , wxEmptyString, wxITEM_NORMAL );
  m_popup->Append( ring );
}


BattleroomListCtrl::~BattleroomListCtrl()
{

}


void BattleroomListCtrl::UpdateList()
{

}


void BattleroomListCtrl::AddUser( User& user )
{
  int index = InsertItem( GetItemCount(),icons().ICON_NREADY );
  try
  {
    ASSERT_LOGIC( index != -1, _T("index = -1") );
  } catch (...) { return; }

  wxLogMessage(_T("BattleroomListCtrl::AddUser index=%d name=%s"),index,user.GetNick().c_str());

  item_content new_content;
  new_content.is_bot = false;
  new_content.data = (void*) &user;
  items.push_back(new_content);

  SetItemData(index, (wxUIntPtr)(items.size()-1) );

  UpdateUser( index );
  SetColumnWidth( 5, wxLIST_AUTOSIZE );
}


void BattleroomListCtrl::RemoveUser( User& user )
{
  if ( &user == m_sel_user ) m_sel_user = 0;
  DeleteItem( GetUserIndex( user ) );
  SetColumnWidth( 5, wxLIST_AUTOSIZE );
}


void BattleroomListCtrl::UpdateUser( User& user )
{
  int index=GetUserIndex( user );
  UpdateUser( index );
  wxLogMessage(_T("BattleroomListCtrl::UpdateUser(User&) index=%d name=%s"),index,user.GetNick().c_str());
}


void BattleroomListCtrl::UpdateUser( const int& index )
{
  try
  {
    ASSERT_LOGIC( index != -1, _T("index = -1") );
  } catch (...) { return; }



  wxListItem item;
  item.SetId( index );

  if( !GetItem( item ) ) return;

  item_content user_content = items[(size_t)GetItemData( index )];
  User& user = *((User*) user_content.data);

  wxLogMessage(_T("BattleroomListCtrl::UpdateUser(int) index=%d name=%s"),index,user.GetNick().c_str());



  int statimg;
  if ( &m_battle.GetFounder() == &user ) {
    statimg =icons().GetHostIcon( user.BattleStatus().spectator );
  } else {
      bool spec = user.BattleStatus().spectator;
    statimg = icons().GetReadyIcon( spec, user.BattleStatus().ready, user.BattleStatus().sync );
  }
  SetItemImage( index, statimg );

  SetItemColumnImage( index, 1, -1 );

  if ( !user.BattleStatus().spectator ) {
    icons().SetColourIcon( user.BattleStatus().team, user.BattleStatus().colour );

    try {
      int sideimg = icons().GetSideIcon( m_battle.GetHostModName(), user.BattleStatus().side );
      if ( sideimg >= 0 ) SetItemColumnImage( index, 1, sideimg );
      else SetItem( index, 1, usync().GetSideName( m_battle.GetHostModName(), user.BattleStatus().side ));
    } catch ( ... ) {
      SetItem( index, 1, wxString::Format( _T("s%d"), user.BattleStatus().side + 1 ) );
    }

    SetItemColumnImage( index, 2, icons().GetColourIcon( user.BattleStatus().team ) );

  } else {
    SetItemColumnImage( index, 2, -1 );
  }

  SetItemColumnImage( index, 3,icons().GetFlagIcon( user.GetCountry() ) );
  SetItemColumnImage( index, 4,icons().GetRankIcon( user.GetStatus().rank ) );

  SetItem( index, 5,  user.GetNick() );
  if ( !user.Status().bot ) SetItemColumnImage( index, 5, -1 );
  else SetItemColumnImage( index, 5,icons().ICON_BOT );

  if ( !user.BattleStatus().spectator ) {
    SetItem( index, 6, wxString::Format( _T("%d"), user.BattleStatus().team + 1 ) );
    SetItem( index, 7, wxString::Format( _T("%d"), user.BattleStatus().ally + 1 ) );
    SetItem( index, 9, wxString::Format( _T("%d%%"), user.BattleStatus().handicap ) );
  } else {
    SetItem( index, 6, _T("") );
    SetItem( index, 7, _T("") );
    SetItem( index, 9, _T("") );
  }
  HighlightItemUser( index, user.GetNick() );
  SetItem( index, 8, wxString::Format( _T("%.1f GHz"), user.GetCpu() / 1000.0 ) );
  Sort();
}


int BattleroomListCtrl::GetUserIndex( User& user )
{
  for (int i = 0; i < GetItemCount() ; i++ ) {
    wxListItem item;
    item.SetId( i );
    GetItem( item );

    item_content item_data = items[(size_t)GetItemData( i )];

    if(item_data.is_bot) continue;

    if ( (unsigned long)&user == (unsigned long)item_data.data )
      return i;
  }
  wxLogError( _T("didn't find the battle.") );
  return -1;
}


void BattleroomListCtrl::AddBot( BattleBot& bot )
{
  int index = InsertItem( GetItemCount(),icons().ICON_BOT );
  try
  {
    ASSERT_LOGIC( index != -1, _T("index = -1") );
  } catch (...) { return; }

  item_content new_content;
  new_content.is_bot = true;
  new_content.data = (void*) &bot;
  items.push_back(new_content);

  SetItemData(index, (wxUIntPtr)(items.size()-1) );

  UpdateBot( index );
}


void BattleroomListCtrl::RemoveBot( BattleBot& bot )
{
  if ( &bot == m_sel_bot ) m_sel_bot = 0;
  DeleteItem( GetBotIndex( bot ) );
}


void BattleroomListCtrl::UpdateBot( BattleBot& bot )
{
  UpdateBot( GetBotIndex( bot ) );
}


void BattleroomListCtrl::UpdateBot( const int& index )
{
  try
  {
    ASSERT_LOGIC( index != -1, _T("index = -1") );
  } catch (...) { return; }

  wxListItem item;
  item.SetId( index );

  if( !GetItem( item ) ) return;

  item_content bot_content = items[(size_t)GetItemData( index )];
  BattleBot& bot = *((BattleBot*)bot_content.data);

  icons().SetColourIcon( bot.bs.team,  bot.bs.colour );

  SetItemImage( index,icons().ICON_BOT );

  SetItemColumnImage( index, 1, -1 );

  try {
    int sideimg = icons().GetSideIcon( m_battle.GetHostModName(), bot.bs.side );
    if ( sideimg >= 0 ) SetItemColumnImage( index, 1, sideimg );
    else SetItem( index, 1,  usync().GetSideName( m_battle.GetHostModName(), bot.bs.side) );
  } catch ( ... ) {
    SetItem( index, 1, wxString::Format( _T("s%d"), bot.bs.side + 1 ) );
  }

  SetItemColumnImage( index, 2, icons().GetColourIcon( bot.bs.team ) );

  SetItemColumnImage( index, 3,icons().ICON_NONE );
  SetItemColumnImage( index, 4,icons().ICON_NONE );

  SetItem( index, 5, bot.name + _T(" (") + bot.owner + _T(")") );

  SetItem( index, 6, wxString::Format( _T("%d"), bot.bs.team + 1 ) );
  SetItem( index, 7, wxString::Format( _T("%d"), bot.bs.ally + 1 ) );
  SetItem( index, 9, wxString::Format( _T("%d%%"), bot.bs.handicap ) );

  wxString botdll = bot.aidll;
  if ( botdll.Contains(_T('.')) ) botdll = botdll.BeforeLast(_T('.'));
  if ( botdll.Contains(_T('/')) ) botdll = botdll.AfterLast(_T('/'));
  if ( botdll.Contains(_T('\\')) ) botdll = botdll.AfterLast(_T('\\'));
  if ( botdll.Contains(_T("LuaAI:")) ) botdll = botdll.AfterFirst(_T(':'));

  SetItem( index, 8, botdll );
  Sort();
}


int BattleroomListCtrl::GetBotIndex( BattleBot& bot )
{
  for (int i = 0; i < GetItemCount() ; i++ ) {
    wxListItem item;
    item.SetId( i );
    GetItem( item );

    item_content item_data = items[(size_t)GetItemData( i )];

    if(!item_data.is_bot) continue;

    if ( (unsigned long)&bot == (unsigned long)item_data.data )
      return i;
  }
  wxLogError( _T("didn't find the bot.") );
  return -1;
}


void BattleroomListCtrl::OnListRightClick( wxListEvent& event )
{
  wxLogDebugFunc( _T("") );

  if ( event.GetIndex() == -1 ) return;

  item_content item_content = items[(size_t)GetItemData( event.GetIndex() )];

  if ( item_content.is_bot ) {
    wxLogMessage(_T("Bot"));
    m_sel_user = 0;
    m_sel_bot = (BattleBot*)item_content.data;
    int item = m_popup->FindItem( _("Spectator") );
    m_popup->Enable( item, false );
    m_popup->Check( item, false );
    m_popup->Enable( m_popup->FindItem( _("Ring") ), false );
    m_popup->Enable( m_popup->FindItem( _("Kick") ),true);
  } else {
    wxLogMessage(_T("User"));
    m_sel_bot = 0;
    m_sel_user = (User*)item_content.data;
    int item = m_popup->FindItem( _("Spectator") );
    m_popup->Check( item, m_sel_user->BattleStatus().spectator );
    m_popup->Enable( item, true );
    m_popup->Enable( m_popup->FindItem( _("Ring") ), true );
    bool isSelUserMe =  ( m_ui.IsThisMe(*m_sel_user) );
    m_popup->Enable( m_popup->FindItem( _("Kick") ),!isSelUserMe);
  }

  wxLogMessage(_T("Popup"));
  PopupMenu( m_popup );
  wxLogMessage(_T("Done"));
}


void BattleroomListCtrl::OnTeamSelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  int team = event.GetId() - BRLIST_TEAM;
  if ( m_sel_bot != 0 ) {
    m_battle.SetBotTeam( m_sel_bot->name, team );
  } else if ( m_sel_user != 0 ) {
    m_battle.ForceTeam( *m_sel_user, team );
  }
}


void BattleroomListCtrl::OnAllySelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  int ally = event.GetId() - BRLIST_ALLY;
  if ( m_sel_bot != 0 ) {
    m_battle.SetBotAlly( m_sel_bot->name, ally );
  } else if ( m_sel_user != 0 ) {
    m_battle.ForceAlly( *m_sel_user, ally );
  }
}


void BattleroomListCtrl::OnColourSelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );

  if ( m_sel_bot != 0 ) {
    wxColour CurrentColour = m_sel_bot->bs.colour;
    CurrentColour = GetColourFromUser(this, CurrentColour);
    if ( !CurrentColour.IsColourOk() ) return;
    m_battle.SetBotColour( m_sel_bot->name, CurrentColour );
  } else if ( m_sel_user != 0 ) {
    wxColour CurrentColour = m_sel_user->BattleStatus().colour;
    CurrentColour = GetColourFromUser(this, CurrentColour);
    if ( !CurrentColour.IsColourOk() ) return;
    m_battle.ForceColour( *m_sel_user, CurrentColour );
  }

}


void BattleroomListCtrl::OnSideSelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  int side = event.GetId() - BRLIST_SIDE;
  if ( m_sel_bot != 0 ) {
    m_battle.SetBotSide( m_sel_bot->name, side );
  } else if ( m_sel_user != 0 ) {
    m_battle.ForceSide( *m_sel_user, side );
  }
}


void BattleroomListCtrl::OnHandicapSelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  wxTextEntryDialog dlg( this , _("Please enter a value between 0 and 100"), _("Set Resource Bonus"), _T("0"), wxOK, wxDefaultPosition );
  if ( dlg.ShowModal() == wxID_OK ) {
    long handicap;
    if ( !dlg.GetValue().ToLong( &handicap ) ) {
     wxLogWarning( _T("input is not a number") );
     customMessageBox(SL_MAIN_ICON, _("Not a number"), _("Invalid number") );
     return;
    }
    if ( handicap < 0 || handicap > 100 ) {
      wxLogWarning( _T("input value is out of range") );
      customMessageBox(SL_MAIN_ICON, _("Value out of range.\n Enter an integer between 0 & 100."), _("Invalid number") );
      return;
    }
    if ( m_sel_bot != 0 ) {
      m_battle.SetBotHandicap( m_sel_bot->name, handicap );
    } else if ( m_sel_user != 0 ) {
      m_battle.SetHandicap( *m_sel_user, handicap );
    }
  }
}


void BattleroomListCtrl::OnSpecSelect( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  if ( m_sel_user != 0 ) {
    m_battle.ForceSpectator( *m_sel_user, m_spec_item->IsChecked() );
  }
}


void BattleroomListCtrl::OnKickPlayer( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  if ( m_sel_bot != 0 ) {
    m_battle.RemoveBot( m_sel_bot->name );
  } else if ( m_sel_user != 0 ) {
    m_battle.KickPlayer( *m_sel_user );
  }
}


void BattleroomListCtrl::OnRingPlayer( wxCommandEvent& event )
{
  wxLogDebugFunc( _T("") );
  if ( m_sel_bot != 0 ) {
  } else if ( m_sel_user != 0 ) {
    m_battle.GetServer().Ring( m_sel_user->GetNick() );
  }
}


void BattleroomListCtrl::OnColClick( wxListEvent& event )
{
  if ( event.GetColumn() == -1 ) return;
  wxListItem col;
  GetColumn( m_sortorder[0].col, col );
  col.SetImage( -1 );
  SetColumn( m_sortorder[0].col, col );

  int i;
  for ( i = 0; m_sortorder[i].col != event.GetColumn() && i < 4; ++i ) {}
  if ( i > 3 ) { i = 3; }
  for ( ; i > 0; i--) { m_sortorder[i] = m_sortorder[i-1]; }
  m_sortorder[0].col = event.GetColumn();
  m_sortorder[0].direction = !m_sortorder[0].direction;


  GetColumn( m_sortorder[0].col, col );
  //col.SetImage( ( m_sortorder[0].direction )?ICON_UP:ICON_DOWN );
  SetColumn( m_sortorder[0].col, col );

  Sort();
}


void BattleroomListCtrl::Sort()
{
  BattleroomListCtrl::m_ui_for_sort = &m_ui;
  if (!m_ui_for_sort || !m_ui_for_sort->GetServerStatus()  ) return;
  for (int i = 2; i >= 0; i--) {
    switch ( m_sortorder[ i ].col ) {
      case 0 : SortItems( ( m_sortorder[ i ].direction )?&CompareStatusUP:&CompareStatusDOWN , (wxUIntPtr)this ); break;
      case 1 : SortItems( ( m_sortorder[ i ].direction )?&CompareSideUP:&CompareSideDOWN , (wxUIntPtr)this ); break;
      case 2 : SortItems( ( m_sortorder[ i ].direction )?&CompareColorUP:&CompareColorDOWN , (wxUIntPtr)this ); break;
      case 3 : SortItems( ( m_sortorder[ i ].direction )?&CompareCountryUP:&CompareCountryDOWN , (wxUIntPtr)this ); break;
      case 4 : SortItems( ( m_sortorder[ i ].direction )?&CompareRankUP:&CompareRankDOWN , (wxUIntPtr)this ); break;
      case 5 : SortItems( ( m_sortorder[ i ].direction )?&CompareNicknameUP:&CompareNicknameDOWN , (wxUIntPtr)this ); break;
      case 6 : SortItems( ( m_sortorder[ i ].direction )?&CompareTeamUP:&CompareTeamDOWN , (wxUIntPtr)this ); break;
      case 7 : SortItems( ( m_sortorder[ i ].direction )?&CompareAllyUP:&CompareAllyDOWN , (wxUIntPtr)this ); break;
      case 8 : SortItems( ( m_sortorder[ i ].direction )?&CompareCpuUP:&CompareCpuDOWN , (wxUIntPtr)this ); break;
      case 9 : SortItems( ( m_sortorder[ i ].direction )?&CompareHandicapUP:&CompareHandicapDOWN , (wxUIntPtr)this ); break;
    }
  }
}


int wxCALLBACK BattleroomListCtrl::CompareStatusUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int status1 = 0;
  if ( content1.is_bot ) {
    status1 = 9;
  } else {
    if ( &bl.m_battle.GetFounder() != ((User*)content1.data) )
      status1 = 1;
    if ( ((User*)content1.data)->BattleStatus().ready )
      status1 += 5;
    if ( !((User*)content1.data)->BattleStatus().sync )
      status1 += 7;
    if ( ((User*)content1.data)->BattleStatus().spectator )
      status1 += 10;
  }

  int status2 = 0;
  if ( content2.is_bot ) {
    status2 = 9;
  } else {
    if ( &bl.m_battle.GetFounder() != ((User*)content2.data) )
      status2 = 1;
    if ( ((User*)content2.data)->BattleStatus().ready )
      status2 += 5;
    if ( !((User*)content2.data)->BattleStatus().sync )
      status2 += 7;
    if ( ((User*)content2.data)->BattleStatus().spectator )
      status2 += 10;
  }

  if ( status1 < status2 )
      return -1;
  if ( status1 > status2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareStatusDOWN(long item1, long item2, long sortData)
{
  return CompareStatusUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareSideUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int side1;
  if ( content1.is_bot )
    side1 = ((BattleBot*)content1.data)->bs.side;
  else if ( ((User*)content1.data)->BattleStatus().spectator )
    side1 = 1000;
  else
    side1 = ((User*)content1.data)->BattleStatus().side;

  int side2;
  if ( content2.is_bot )
    side2 = ((BattleBot*)content2.data)->bs.side;
  else if ( ((User*)content2.data)->BattleStatus().spectator )
    side2 = 1000;
  else
    side2 = ((User*)content2.data)->BattleStatus().side;

  if ( side1 < side2 )
      return -1;
  if ( side1 > side2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareSideDOWN(long item1, long item2, long sortData)
{
  return CompareSideUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareColorUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int color1_r, color1_g, color1_b;
  if ( content1.is_bot )
    {
      if ( ((BattleBot*)content1.data)->bs.spectator ) return -1;
      color1_r = ((BattleBot*)content1.data)->bs.colour.Red();
      color1_g = ((BattleBot*)content1.data)->bs.colour.Green();
      color1_b = ((BattleBot*)content1.data)->bs.colour.Blue();
    }
  else
    {
      if ( ((User*)content1.data)->BattleStatus().spectator ) return -1;
      color1_r = ((User*)content1.data)->BattleStatus().colour.Red();
      color1_g = ((User*)content1.data)->BattleStatus().colour.Green();
      color1_b = ((User*)content1.data)->BattleStatus().colour.Blue();
    }

  int color2_r, color2_g, color2_b;
  if ( content2.is_bot )
    {
      if ( ((BattleBot*)content2.data)->bs.spectator ) return 1;
      color2_r = ((BattleBot*)content2.data)->bs.colour.Red();
      color2_g = ((BattleBot*)content2.data)->bs.colour.Green();
      color2_b = ((BattleBot*)content2.data)->bs.colour.Blue();
    }
  else
    {
      if ( ((User*)content2.data)->BattleStatus().spectator ) return 1;
      color2_r = ((User*)content2.data)->BattleStatus().colour.Red();
      color2_g = ((User*)content2.data)->BattleStatus().colour.Green();
      color2_b = ((User*)content2.data)->BattleStatus().colour.Blue();
    }

  if ( (color1_r + color1_g + color1_b)/3 < (color2_r + color2_g + color2_b)/3 )
      return -1;
  if ( (color1_r + color1_g + color1_b)/3 > (color2_r + color2_g + color2_b)/3 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareColorDOWN(long item1, long item2, long sortData)
{
  return CompareColorUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareCountryUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  wxString country1;
  if ( content1.is_bot )
    country1 = _T("");
  else
    country1 = ((User*)content1.data)->GetCountry().Upper();

  wxString country2;
  if ( content2.is_bot )
    country2 = _T("");
  else
    country2 = ((User*)content2.data)->GetCountry().Upper();

  if ( country1 < country2 )
      return -1;
  if ( country1 > country2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareCountryDOWN(long item1, long item2, long sortData)
{
  return CompareCountryUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareRankUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int rank1;
  if ( content1.is_bot )
    rank1 = 1000;
  else
    rank1 = ((User*)content1.data)->GetStatus().rank;

  int rank2;
  if ( content2.is_bot )
    rank2 = 1000;
  else
    rank2 = ((User*)content2.data)->GetStatus().rank;

  if ( rank1 < rank2 )
      return -1;
  if ( rank1 > rank2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareRankDOWN(long item1, long item2, long sortData)
{
  return CompareRankUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareNicknameUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  wxString name1;
  if ( content1.is_bot )
    name1 = ((BattleBot*)content1.data)->name.Upper();
  else
    name1 = ((User*)content1.data)->GetNick().Upper();

  wxString name2;
  if ( content2.is_bot )
    name2 = ((BattleBot*)content2.data)->name.Upper();
  else
    name2 = ((User*)content2.data)->GetNick().Upper();

  if ( name1 < name2 )
      return -1;
  if ( name1 > name2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareNicknameDOWN(long item1, long item2, long sortData)
{
  return CompareNicknameUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareTeamUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int team1;
  if ( content1.is_bot )
    team1 = ((BattleBot*)content1.data)->bs.team;
  else if ( ((User*)content1.data)->BattleStatus().spectator )
    team1 = 1000;
  else
    team1 = ((User*)content1.data)->BattleStatus().team;

  int team2;
  if ( content2.is_bot )
    team2 = ((BattleBot*)content2.data)->bs.team;
  else if ( ((User*)content2.data)->BattleStatus().spectator )
    team2 = 1000;
  else
    team2 = ((User*)content2.data)->BattleStatus().team;

  if ( team1 < team2 )
      return -1;
  if ( team1 > team2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareTeamDOWN(long item1, long item2, long sortData)
{
  return CompareTeamUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareAllyUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int ally1;
  if ( content1.is_bot )
    ally1 = ((BattleBot*)content1.data)->bs.ally;
  else if ( ((User*)content1.data)->BattleStatus().spectator )
    ally1 = 1000;
  else
    ally1 = ((User*)content1.data)->BattleStatus().ally;

  int ally2;
  if ( content2.is_bot )
    ally2 = ((BattleBot*)content2.data)->bs.ally;
  else if ( ((User*)content2.data)->BattleStatus().spectator )
    ally2 = 1000;
  else
    ally2 = ((User*)content2.data)->BattleStatus().ally;

  if ( ally1 < ally2 )
      return -1;
  if ( ally1 > ally2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareAllyDOWN(long item1, long item2, long sortData)
{
  return CompareAllyUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareCpuUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  if ( content1.is_bot ) {
    wxString aidll1 = ((BattleBot*)content1.data)->aidll.Upper();
    if ( content2.is_bot ) {
      wxString aidll2 = ((BattleBot*)content2.data)->aidll.Upper();
      if ( aidll1 < aidll2 )
        return -1;
      if ( aidll1 > aidll2 )
        return 1;
      return 0;
    } else {
      return 1;
    }
  } else {
    int cpu1 = ((User*)content1.data)->GetCpu();
    if ( content2.is_bot ) {
      return -1;
    } else {
      int cpu2 = ((User*)content2.data)->GetCpu();
      if ( cpu1 < cpu2 )
        return -1;
      if ( cpu1 > cpu2 )
        return 1;
      return 0;
    }
  }
}


int wxCALLBACK BattleroomListCtrl::CompareCpuDOWN(long item1, long item2, long sortData)
{
  return CompareCpuUP(item1, item2, sortData)*-1;
}


int wxCALLBACK BattleroomListCtrl::CompareHandicapUP(long item1, long item2, long sortData)
{
  BattleroomListCtrl& bl = *(BattleroomListCtrl*)sortData;
  item_content content1 = bl.items[(size_t)item1];
  item_content content2 = bl.items[(size_t)item2];

  int handicap1;
  if ( content1.is_bot )
    handicap1 = ((BattleBot*)content1.data)->bs.handicap;
  else if ( ((User*)content1.data)->BattleStatus().spectator )
    handicap1 = 1000;
  else
    handicap1 = ((User*)content1.data)->BattleStatus().handicap;

  int handicap2;
  if ( content2.is_bot )
    handicap2 = ((BattleBot*)content2.data)->bs.handicap;
  else if ( ((User*)content2.data)->BattleStatus().spectator )
    handicap2 = 1000;
  else
    handicap2 = ((User*)content2.data)->BattleStatus().handicap;

  if ( handicap1 < handicap2 )
      return -1;
  if ( handicap1 > handicap2 )
      return 1;

  return 0;
}


int wxCALLBACK BattleroomListCtrl::CompareHandicapDOWN(long item1, long item2, long sortData)
{
  return CompareHandicapUP(item1, item2, sortData)*-1;
}

void BattleroomListCtrl::SetTipWindowText( const long item_hit, const wxPoint position)
{
    long item = GetItemData(item_hit);
    item_content content = this->items[(size_t)item];
    int coloumn = getColoumnFromPosition( position );
    if (coloumn > (int)m_colinfovec.size() || coloumn < 0)
    {
        m_tiptext = _T("");
    }
    else
    {
        switch (coloumn)
        {
        case 0: // is bot?
            if ( content.is_bot )
                m_tiptext = _T("This is an AI controlled Player (bot)");
            else if ( ((User*)content.data)->BattleStatus().spectator )
                m_tiptext = _T("Spectator");
            else
                m_tiptext =  _T("Human Player");
            break;
        case 1: // icon
            if ( content.is_bot )
                m_tiptext = usync().GetSideName( m_battle.GetHostModName(),
                        ((BattleBot*)content.data)->bs.side );
            else if ( ((User*)content.data)->BattleStatus().spectator )
                m_tiptext = _T("Spectators have no side");
            else
                m_tiptext =  usync().GetSideName( m_battle.GetHostModName(),
                        ((User*)content.data)->BattleStatus().side );
            break;

        case 3: // country
            m_tiptext = (content.is_bot ? _T("This bot is from nowhere particluar")
                    : GetFlagNameFromCountryCode(((User*)content.data)->GetCountry().Upper()));
            break;
        case 4: // rank
            m_tiptext = (content.is_bot ? _T("This bot has no rank")
                    : ((User*)content.data)->GetRankName(((User*)content.data)->GetStatus().rank));
            break;

        case 5: //name
            m_tiptext = (content.is_bot ?((BattleBot*)content.data)->name
                    : ((User*)content.data)->GetNick() );
            break;

        case 8: // cpu
            m_tiptext = (content.is_bot ? ((BattleBot*)content.data)->aidll
                    : m_colinfovec[coloumn].first);
            break;

        default:
            m_tiptext =m_colinfovec[coloumn].first;
            break;
        }
    }
}

void BattleroomListCtrl::HighlightItem( long item )
{
    item_content user_content = items[(size_t)GetItemData( item )];
    if ( !user_content.is_bot )
    {
        User& user = *((User*) user_content.data);
        HighlightItemUser( item, user.GetNick() );
    }
}
