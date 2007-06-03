//
// Class: Battle
//

#ifndef _BATTLE_H_
#define _BATTLE_H_

#include "userlist.h"


class Ui;
class Server;

/*
battlestatus:  An integer but with limited range: 0..2147483647 (use signed int and consider only positive values and zero). Number is sent as text. Each bit has its meaning:

    * b0 = undefined (reserved for future use)
    * b1 = ready (0=not ready, 1=ready)
    * b2..b5 = team no. (from 0 to 15. b2 is LSB, b5 is MSB)
    * b6..b9 = ally team no. (from 0 to 15. b6 is LSB, b9 is MSB)
    * b10 = mode (0 = spectator, 1 = normal player)
    * b11..b17 = handicap (7-bit number. Must be in range 0..100). Note: Only host can change handicap values of the players in the battle (with HANDICAP command). These 7 bits are always ignored in this command. They can only be changed using HANDICAP command.
    * b18..b21 = reserved for future use (with pre 0.71 versions these bits were used for team color index)
    * b22..b23 = sync status (0 = unknown, 1 = synced, 2 = unsynced)
    * b24..b27 = side (e.g.: arm, core, tll, ... Side index can be between 0 and 15, inclusive)
    * b28..b31 = undefined (reserved for future use)

*/

enum NatType {
  NAT_None = 0,
  NAT_Hole_punching,
  NAT_Fixed_source_ports
};

enum StartType {
  ST_Fixed = 0,
  ST_Random,
  ST_Choose
};


typedef int SyncHash;
#define DEFAULT_SERVER_PORT 8034



struct BattleOptions
{
  BattleOptions() :
    battleid(-1),islocked(false),isreplay(false),ispassworded(false),rankneeded(0),
    nattype(NAT_None),port(DEFAULT_SERVER_PORT),maxplayers(0),spectators(0),
    startmetal(1000),startenergy(1000),maxunits(500),starttype(ST_Fixed),
    comends(true),limitdgun(false),dimmms(false),ghostedbuildings(true),maphash(0),
    hashcode(0) {}

  int battleid;
  bool islocked;
  bool isreplay;
  bool ispassworded;
  int rankneeded;
  std::string founder;
  
  NatType nattype;
  int port;
  std::string ip;
  
  int maxplayers;
  int spectators;

  int startmetal;
  int startenergy;
  int maxunits;
  StartType starttype;

  bool comends;
  bool limitdgun;
  bool dimmms;
  bool ghostedbuildings;
  
  SyncHash maphash;
  SyncHash hashcode;

  std::string description;
  std::string mapname;
  std::string modname;
  
};


class Battle : public UserList
{
  public:
    Battle( Server& serv, Ui& ui, const int& id ) : UserList(),m_serv(serv),m_ui(ui) { m_opts.battleid = id; }
    ~Battle() {
      for (int i = 0; i < GetNumUsers(); i++ ) GetUser(i).SetBattle( NULL );
    }
  
    const BattleOptions& opts() { return m_opts; }
  
    void SetIsReplay( const bool& isreplay ) { m_opts.isreplay = isreplay; }
    void SetIsLocked( const bool& islocked ) { m_opts.islocked = islocked; }
    void SetIsPassworded( const bool& ispassworded ) { m_opts.ispassworded = ispassworded; }
    
    void SetNatType( const NatType nattype ) { m_opts.nattype = nattype; }
    void SetPort( const int& port) { m_opts.port = port; }
    void SetFounder( const std::string& nick ) { m_opts.founder = nick; }
    void SetIp( const std::string& ip ) { m_opts.ip = ip; }
    
    void SetMaxPlayers( const int& maxplayers ) { m_opts.maxplayers = maxplayers; }
    void SetSpectators( const int& spectators ) { m_opts.spectators = spectators; }
    
    void SetStartMetal( const int& smetal ) { m_opts.startmetal = smetal; }
    void SetStartEnergy( const int& senergy ) { m_opts.startenergy = senergy; }
    void SetMaxUnits( const int& maxunits ) { m_opts.maxunits = maxunits; }
    void SetStartType( const StartType& starttype ) { m_opts.starttype = starttype; }
    
    void SetComEndsGame( const bool& comends ) { m_opts.comends = comends; }
    void SetLimitDGun( const bool& limdgun ) { m_opts.limitdgun = limdgun; }
    void SetDimMMs( const bool& dimmm ) { m_opts.dimmms = dimmm; }
    void SetGhostedBuildings( const bool& gbuilds ) { m_opts.ghostedbuildings = gbuilds; }
    
    void SetRankNeeded( const int& rankneeded ) { m_opts.rankneeded = rankneeded; }
        
    void SetMapHash( const SyncHash& maphash ) { m_opts.maphash = maphash; }
    void SetMapname( const std::string& map ) { m_opts.mapname = map; }
    void SetDescription( const std::string& desc ) { m_opts.description = desc; }
    void SetModname( const std::string& mod ) { m_opts.modname = mod; }
    
    User& GetFounder() { return GetUser( m_opts.founder ); }
    
    void AddUser( User& user ) {
      user.SetBattle( this );
      UserList::AddUser( user );
    }
    
    void RemoveUser( User& user ) {
      user.SetBattle( NULL );
      UserList::RemoveUser( user.GetNick() );
    }
    
    void Update();
  /*
    DISABLEUNITS unitname1 unitname2
    ADDBOT BATTLE_ID name owner battlestatus teamcolor {AIDLL}
    ADDSTARTRECT allyno left top right bottom
  */
  protected:
    // Battle variables

    BattleOptions m_opts;
    Server& m_serv;
    Ui& m_ui;
  
    void RemoveUser( std::string const& user ) {}
};


#endif  //_BATTLE_H_
