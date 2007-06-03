//
// Class: MainJoinBattleTab
//

#include "mainjoinbattletab.h"

#include "images/battle_list.xpm"
#include "images/battle.xpm"
#include "images/battle_map.xpm"
#include "images/battle_settings.xpm"

MainJoinBattleTab::MainJoinBattleTab( wxWindow* parent, Ui& ui ) : wxPanel( parent, -1 ),m_ui(ui),m_battle_tab(NULL)
{
  m_main_sizer = new wxBoxSizer( wxVERTICAL );
  m_tabs = new wxNotebook( this, BATTLE_TABS, wxDefaultPosition, wxDefaultSize, wxLB_TOP );
  
  m_imagelist = new wxImageList( 12, 12 );
  m_imagelist->Add( wxBITMAP(battle_list) );
  m_imagelist->Add( wxBITMAP(battle) );
  m_imagelist->Add( wxBITMAP(battle_map) );
  m_imagelist->Add( wxBITMAP(battle_settings) );
  
  m_tabs->AssignImageList( m_imagelist );
  
  m_list_tab = new BattleListTab( m_tabs, m_ui );
  m_tabs->AddPage( m_list_tab, _T("Battle list"), true, 0 );

  m_main_sizer->Add( m_tabs, 1, wxEXPAND );

  SetSizer( m_main_sizer );
  Layout();
}


MainJoinBattleTab::~MainJoinBattleTab()
{
  
}

BattleListTab& MainJoinBattleTab::GetBattleListTab()
{
  assert( m_list_tab != NULL );
  return *m_list_tab;
}

void MainJoinBattleTab::JoinBattle( Battle& battle )
{
  //if ( m_battle_tab != NULL ) delete m_battle_tab;
  m_battle_tab = new BattleRoomTab( m_tabs, battle );
  m_tabs->AddPage( m_battle_tab, _T("Battleroom"), true, 1 );
}


void MainJoinBattleTab::LeaveCurrentBattle()
{
}

