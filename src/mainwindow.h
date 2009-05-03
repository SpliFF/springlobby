#ifndef SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H
#define SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H

#include <wx/frame.h>

class Ui;
class Channel;
class User;
class wxCommandEvent;
class wxListbookEvent;
class wxAuiNotebookEvent;
class MainChatTab;
class MainJoinBattleTab;
class MainSinglePlayerTab;
#ifndef NO_TORRENT_SYSTEM
class MainTorrentTab;
#endif
class wxBoxSizer;
//namespace SL_Extern{
    class wxAuiNotebook;
//}
class wxListbook;
class MainOptionsTab;
class wxBitmap;
class wxImageList;
class ChatPanel;
class settings_frame;
class wxMenuItem;
class wxMenuBar;
class wxMenu;
class ChannelChooserDialog;
class ReplayTab;
class AutojoinChannelDialog;
class WidgetDownloadDialog;

class ReplayTraits;
template < class Traits >
class PlaybackTab;

class SavegameTraits;
template < class Traits >
class SavegameTab;

// FIXME shouldn't copy this here
//typedef wxWindow wxNotebookPage;


// Page indexes
const unsigned int PAGE_CHAT    = 0;
const unsigned int PAGE_JOIN    = 1;
const unsigned int PAGE_SINGLE  = 2;
const unsigned int PAGE_OPTOS   = 3;

static const unsigned int OPT_PAGE_SPRING   = 0;
static const unsigned int OPT_PAGE_CHAT     = 1;
#ifndef NO_TORRENT_SYSTEN
static const unsigned int OPT_PAGE_TORRENT  = 2;
static const unsigned int OPT_PAGE_GENERAL  = 3;
static const unsigned int OPT_PAGE_GROUPS   = 4;
#else
static const unsigned int OPT_PAGE_GENERAL  = 2;
static const unsigned int OPT_PAGE_GROUPS   = 3;
#endif


//! @brief wxFrame that contains the main window of the client.
class MainWindow : public wxFrame
{
  public:
    MainWindow( Ui& ui );
    virtual ~MainWindow();

    typedef PlaybackTab<ReplayTraits>
        ReplayTab;
    typedef PlaybackTab<SavegameTraits>
        SavegameTab;

    // MainWindow interface
    void OpenChannelChat( Channel& channel );
    void OpenPrivateChat( const User& user, bool doFocus = false );

    void ShowConfigure( const unsigned int page = OPT_PAGE_SPRING );
    void ShowSingleplayer();

    /** Show the channel list dialog. */
    void ShowChannelChooser();

    void OnMenuAbout( wxCommandEvent& event );
    void OnMenuJoin( wxCommandEvent& event );
    void OnMenuChat( wxCommandEvent& event );
    void OnMenuConnect( wxCommandEvent& event );
    void OnMenuDisconnect( wxCommandEvent& event );
    void OnMenuSaveOptions( wxCommandEvent& event );
    void OnMenuQuit( wxCommandEvent& event );
    void OnMenuVersion ( wxCommandEvent& event );
    void OnMenuSaveLayout( wxCommandEvent& event );
    void OnMenuLoadLayout( wxCommandEvent& event );
    void OnMenuDefaultLayout( wxCommandEvent& event );
    void OnUnitSyncReload( wxCommandEvent& event );
    void OnMenuStartTorrent( wxCommandEvent& event );
    void OnMenuStopTorrent( wxCommandEvent& event );
    void OnMenuOpen( wxMenuEvent& event );
    void OnMenuAutojoinChannels( wxCommandEvent& event );
    void OnReportBug( wxCommandEvent& event );
    void OnShowDocs( wxCommandEvent& event );
    void OnShowSettingsPP( wxCommandEvent& event );
    void OnMenuSelectLocale( wxCommandEvent& event );
    void OnShowChannelChooser( wxCommandEvent& event );
    void OnShowScreenshots( wxCommandEvent& event );
    void forceSettingsFrameClose();
    void OnUnitSyncReloaded();
    void OnChannelList( const wxString& channel, const int& numusers, const wxString& topic );
    void OnChannelListStart( );


    void OnTabsChanged( wxAuiNotebookEvent& event );
    MainChatTab& GetChatTab();
    MainJoinBattleTab& GetJoinTab();
    MainSinglePlayerTab& GetSPTab();
    ReplayTab& GetReplayTab();
    #ifndef NO_TORRENT_SYSTEM
    MainTorrentTab& GetTorrentTab();
    #endif
    ChatPanel* GetActiveChatPanel();
    ChatPanel* GetChannelChatPanel( const wxString& channel );
    MainOptionsTab& GetOptionsTab();

    void SetTabIcons();

  protected:
    // MainWindow variables
    Ui& m_ui;

    wxMenuItem* m_settings_menu;
    wxMenuBar* m_menubar;
    wxMenu* m_menuTools;

    wxBoxSizer* m_main_sizer;
    wxAuiNotebook* m_func_tabs;

    MainChatTab* m_chat_tab;
    MainJoinBattleTab* m_join_tab;
    MainSinglePlayerTab* m_sp_tab;
    MainOptionsTab* m_opts_tab;
    #ifndef NO_TORRENT_SYSTEM
    MainTorrentTab* m_torrent_tab;
    #endif

    AutojoinChannelDialog* m_autojoin_dialog;
    settings_frame* se_frame;
    bool se_frame_active;
    ChannelChooserDialog* m_channel_chooser;

    ReplayTab* m_replay_tab;
//    SavegameTab* m_savegame_tab;

    wxBitmap GetTabIcon( const unsigned char* data, size_t size  );

    enum {
        MENU_SETTINGSPP,
        MENU_ABOUT = wxID_ABOUT,
        MENU_QUIT = wxID_EXIT,

        MENU_CONNECT = wxID_HIGHEST,
        MENU_DISCONNECT,
        MENU_SAVE_OPTIONS,
        MENU_JOIN,
        MENU_USYNC,
        MENU_TRAC,
        MENU_DOC,
        MENU_CHAT,
        MAIN_TABS,
        MENU_VERSION,
        MENU_START_TORRENT,
        MENU_STOP_TORRENT,
        MENU_AUTOJOIN_CHANNELS,
        MENU_SELECT_LOCALE,
        MENU_CHANNELCHOOSER,
        MENU_SAVE_LAYOUT,
        MENU_LOAD_LAYOUT,
        MENU_DEFAULT_LAYOUT,
        MENU_SCREENSHOTS
    };

    DECLARE_EVENT_TABLE()
};

//ChatPanel& servwin();

// wxWidget IDs


#endif // SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H

/**
    This file is part of SpringLobby,
    Copyright (C) 2007-09

    springsettings is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    springsettings is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SpringLobby.  If not, see <http://www.gnu.org/licenses/>.
**/

