/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */

#include <wx/dynlib.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/file.h>
#include <wx/tokenzr.h>
//#include <wx/txtstrm.h>
//#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <cmath>
#include <stdexcept>
#include <clocale>

#include "springunitsync.h"
#include "utils.h"
#include "settings.h"
#include "springunitsynclib.h"
#include "settings++/custom_dialogs.h"
#include "unitsyncthread.h"


#define LOCK_UNITSYNC wxCriticalSectionLocker lock_criticalsection(m_lock)


IUnitSync& usync()
{
  static SpringUnitSync m_sync;
  return m_sync;
}


SpringUnitSync::SpringUnitSync()
{
}


SpringUnitSync::~SpringUnitSync()
{
  CacheThread().Stop();
  FreeUnitSyncLib();
}


bool SpringUnitSync::LoadUnitSyncLib( const wxString& springdir, const wxString& unitsyncloc )
{
#ifndef HAVE_WX26
  if ( !wxFileName::IsDirWritable( sett().GetSpringDir() ) )
  {
      customMessageBox(SL_MAIN_ICON,_("Unitsync loading was aborted because your spring data directory is not writable. Please check."),_("Unitsync Problem"),wxOK);
      wxLogDebugFunc( _T("sprindatadir not writable") );
      return false;
  }
  else
#endif
  {
     wxLogDebugFunc( _T("") );
     LOCK_UNITSYNC;
     bool ret = _LoadUnitSyncLib( springdir, unitsyncloc );
     if (ret)
     {
        PopulateArchiveList();
        CacheThread().Start();
     }
     return ret;
  }
}


void SpringUnitSync::PopulateArchiveList()
{
  m_maps_list.empty();
  m_mods_list.empty();
  m_mod_array.Empty();
  m_map_array.Empty();

  int numMaps = susynclib()->GetMapCount();
  for ( int i = 0; i < numMaps; i++ )
  {
    wxString name, hash;
    try
    {
     name = susynclib()->GetMapName( i );
     hash = susynclib()->GetMapChecksum( i );
    } catch (...) { continue; }
    try
    {
      m_maps_list[name] = hash;
      m_map_array.Add( name );
    } catch (...)
    {
      wxLogError( _T("Found map with hash collision: ") + name + _T(" hash: ") + hash );
    }
  }
  int numMods = susynclib()->GetPrimaryModCount();
  for ( int i = 0; i < numMods; i++ )
  {
    wxString name, hash;
    try
    {
     name = susynclib()->GetPrimaryModName( i );
     hash = i2s(susynclib()->GetPrimaryModChecksum( i ));
    } catch (...) { continue; }
    try
    {
      m_mods_list[name] = hash;
      m_mod_array.Add( name );
    } catch (...)
    {
      wxLogError( _T("Found mod with hash collision: ") + name + _T(" hash: ") + hash );
    }
  }
}



bool SpringUnitSync::_LoadUnitSyncLib( const wxString& springdir, const wxString& unitsyncloc )
{
  wxSetWorkingDirectory( springdir );
  try {
    susynclib()->Load( unitsyncloc );
  } catch (...) {
    return false;
  }
  CacheThread().Resume();
  return true;
}


void SpringUnitSync::FreeUnitSyncLib()
{
  wxLogDebugFunc( _T("") );
  CacheThread().Pause();
  susynclib()->Unload();
}


bool SpringUnitSync::IsLoaded()
{
  return susynclib()->IsLoaded();
}


wxString SpringUnitSync::GetSpringVersion()
{
  wxLogDebugFunc( _T("") );
  wxString ret;
  try
  {
    ret = susynclib()->GetSpringVersion();
  }
  catch (...){}
  return ret;
}


bool SpringUnitSync::VersionSupports( GameFeature feature )
{
  wxString ver = GetSpringVersion();
  double nver = 0;
  ver = ver.BeforeFirst('b') + ver.AfterFirst('b'); //remove the beta flag

  const char* old_locale = std::setlocale(LC_NUMERIC, "C"); //temp switch to C locale for the decimal separator

  if ( ver.Contains( _T("+") ) ) //remove the + (development) flag, and increase the version
  {
    ver = ver.BeforeFirst('+');
    ver.ToDouble( &nver); // convert to float
    nver = std::floor ( ( nver * 100 ) + 0.9 ); // increments version and rounds up the decimal to 0
  }
  else
  {
    ver.ToDouble( &nver); // convert to float
    nver = nver * 100;
  }
  std::setlocale(LC_NUMERIC, old_locale);
  switch (feature) {
    case GF_XYStartPos: return nver >= 76.0;
    case USYNC_Sett_Handler: return nver >= 76.0;
  }
  return false;
}


int SpringUnitSync::GetNumMods()
{
  wxLogDebugFunc( _T("") );
  return m_mod_array.GetCount();
}


wxArrayString SpringUnitSync::GetModList()
{
  return m_mod_array;
}


int SpringUnitSync::GetModIndex( const wxString& name )
{
  wxLogDebugFunc( _T("name = \"") + name + _T("\"") );
  try
  {
    return susynclib()->GetModIndex( name );
  } catch (...){}
  return -1;
}


bool SpringUnitSync::ModExists( const wxString& modname )
{
  return (m_mods_list.find(modname) != m_mods_list.end());
}


bool SpringUnitSync::ModExists( const wxString& modname, const wxString& hash )
{
  LocalArchivesVector::iterator itor = m_mods_list.find(modname);
  if ( itor == m_mods_list.end() ) return false;
  return itor->second == hash;
}


UnitSyncMod SpringUnitSync::GetMod( const wxString& modname )
{
  wxLogDebugFunc( _T("modname = \"") + modname + _T("\"") );
  UnitSyncMod m;

  m.name = modname;
  m.hash = m_mods_list[modname];

  return m;
}


UnitSyncMod SpringUnitSync::GetMod( int index )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMod m;

  m.name = susynclib()->GetPrimaryModName( index );
  m.hash = i2s( susynclib()->GetPrimaryModChecksum( index ) );

  return m;
 }

int SpringUnitSync::GetNumMaps()
{
  wxLogDebugFunc( _T("") );
  return m_map_array.GetCount();
}


wxArrayString SpringUnitSync::GetMapList()
{
  return m_map_array;
}


bool SpringUnitSync::MapExists( const wxString& mapname )
{
  return (m_maps_list.find(mapname) != m_maps_list.end());
}


bool SpringUnitSync::MapExists( const wxString& mapname, const wxString& hash )
{
  LocalArchivesVector::iterator itor = m_maps_list.find(mapname);
  if ( itor == m_maps_list.end() ) return false;
  return itor->second == hash;
}


UnitSyncMap SpringUnitSync::GetMap( const wxString& mapname )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMap m;

  m.name = mapname;
  m.hash = m_maps_list[mapname];

  return m;
}


UnitSyncMap SpringUnitSync::GetMap( int index )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMap m;

  m.name = m_map_array[index];
  m.hash = m_maps_list[m.name];

   return m;
 }


UnitSyncMap SpringUnitSync::GetMapEx( int index )
{
  UnitSyncMap m;

  m.name = m_map_array[index];
  m.hash = m_maps_list[m.name];

  m.info = _GetMapInfoEx( m.name );

  return m;
}


GameOptions SpringUnitSync::GetMapOptions( const wxString& name )
{
  wxLogDebugFunc( name );
  GameOptions ret;
 wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( name, _T(""), false ) + _T(".mapoptions") );
    unsigned int count = cache.GetCount();
    for (unsigned int i = 0; i < count; ++i)
    {
      // key  type
      wxArrayString params = wxStringTokenize( cache[i], _T("\t") );
      wxString key = params[0];
      switch ( s2l( params[1] ) )
      {
      case opt_float:
        // name description default_value step_size min_value max_value
        ret.float_map[key] = mmOptionFloat( params[2], key,
            params[3], (float)s2d( params[4] ), (float)s2d( params[5] ),
            (float)s2d( params[6] ), (float)s2d( params[7] ) );
        break;
      case opt_bool:
        // name description default_value
        ret.bool_map[key] = mmOptionBool( params[2], key,
            params[3], (bool)s2l( params[4] ) );
        break;
      case opt_string:
        // name description default_value max_lenght
        ret.string_map[key] = mmOptionString( params[2], key,
            params[3], params[4], (unsigned int) s2l( params[5] ) );
        break;
      case opt_list:
         ret.list_map[key] = mmOptionList( params[2],key,
            params[3], params[4] );
         for (unsigned int j = 6; j < ( (unsigned int)s2l(params[5]) * 3 + 6); j = j + 3)
         {
           ret.list_map[key].addItem( params[j], params[j+1], params[j+2] );
         }
      }
    }
  }
  catch (...)
  {
    int count = susynclib()->GetMapOptionCount(name);
    for (int i = 0; i < count; ++i)
    {
      wxArrayString entry;
      wxString key = susynclib()->GetOptionKey(i);
      entry.Add( key );
      entry.Add( TowxString( susynclib()->GetOptionType(i) ) );
      switch (susynclib()->GetOptionType(i))
      {
      case opt_float:
      {
        ret.float_map[key] = mmOptionFloat(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionNumberDef(i), susynclib()->GetOptionNumberStep(i),
            susynclib()->GetOptionNumberMin(i),susynclib()->GetOptionNumberMax(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( susynclib()->GetOptionNumberDef(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberStep(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberMin(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberMax(i)) );
        break;
      }
      case opt_bool:
      {
        ret.bool_map[key] = mmOptionBool(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionBoolDef(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( susynclib()->GetOptionBoolDef(i) ) );
        break;
      }
      case opt_string:
      {
        ret.string_map[key] = mmOptionString(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionStringDef(i),susynclib()->GetOptionStringMaxLen(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( susynclib()->GetOptionStringDef(i) );
        entry.Add( TowxString( susynclib()->GetOptionStringMaxLen(i) ) );
        break;
      }
      case opt_list:
      {
         ret.list_map[key] = mmOptionList(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionListDef(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( susynclib()->GetOptionListDef(i) );
        entry.Add( TowxString( susynclib()->GetOptionListCount(i) ) );
         for (int j = 0; j < susynclib()->GetOptionListCount(i); ++j)
         {
           wxString descr = susynclib()->GetOptionDesc(i);
           descr.Replace( _T("\n"), _T("") );
           ret.list_map[key].addItem(susynclib()->GetOptionListItemKey(i,j),susynclib()->GetOptionListItemName(i,j), descr);
           entry.Add( susynclib()->GetOptionListItemKey(i,j) );
           entry.Add( susynclib()->GetOptionListItemName(i,j) );
           entry.Add( descr );
         }
      }
      }
    wxString optiontoken;
    unsigned int entrycount = entry.GetCount();
    for ( unsigned int pos = 0; pos < entrycount; pos++ ) optiontoken << entry[pos] << _T('\t');
    cache.Add( optiontoken );
    }
  SetCacheFile( GetFileCachePath( name, _T(""), false ) + _T(".mapoptions"), cache );
  }
  return ret;
}


UnitSyncMap SpringUnitSync::GetMapEx( const wxString& mapname )
{
  wxLogDebugFunc( _T("") );
  int i = GetMapIndex( mapname );
  ASSERT_LOGIC( i >= 0, _T("Map does not exist") );
  return GetMapEx( i );
}


int SpringUnitSync::GetMapIndex( const wxString& name )
{
  int result = m_map_array.Index( name );
  if ( result == wxNOT_FOUND ) result = -1;
  return result;
}


wxString SpringUnitSync::GetModArchive( int index )
{
  wxLogDebugFunc( _T("") );

  LOCK_UNITSYNC;

  return susynclib()->GetPrimaryModArchive( index );
}


wxString SpringUnitSync::GetMapArchive( int index )
{
  wxLogDebugFunc( _T("") );

  LOCK_UNITSYNC;

  int count = susynclib()->GetMapArchiveCount( index );

  if ( count > 0 )
    return susynclib()->GetMapArchiveName( 0 );
  else
    return _T("");
}


GameOptions SpringUnitSync::GetModOptions( const wxString& name )
{
  wxLogDebugFunc( name );
  GameOptions ret;
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( name, _T(""), true ) + _T(".modoptions") );
    unsigned int count = cache.GetCount();
    for (unsigned int i = 0; i < count; ++i)
    {
      // key  type
      wxArrayString params = wxStringTokenize( cache[i], _T('\t') );
      wxString key = params[0];
      switch ( s2l( params[1] ) )
      {
      case opt_float:
        // name description default_value step_size min_value max_value
        ret.float_map[key] = mmOptionFloat( params[2], key,
            params[3], (float)s2d( params[4] ), (float)s2d( params[5] ),
            (float)s2d( params[6] ), (float)s2d( params[7] ) );
        break;
      case opt_bool:
        // name description default_value
        ret.bool_map[key] = mmOptionBool( params[2], key,
            params[3], (bool)s2l( params[4] ) );
        break;
      case opt_string:
        // name description default_value max_lenght
        ret.string_map[key] = mmOptionString( params[2], key,
            params[3], params[4], (unsigned int) s2l( params[5] ) );
        break;
      case opt_list:
         ret.list_map[key] = mmOptionList( params[2],key,
            params[3], params[4] );
         unsigned int maxloop = (unsigned int)s2l(params[5]) * 3 + 5;
         for (unsigned int j = 6; j < maxloop; j = j + 3)
         {
           ret.list_map[key].addItem( params[j], params[j+1], params[j+2] );
         }
      }
    }
  }
  catch (...)
  {
    int count = susynclib()->GetModOptionCount(name);
    for (int i = 0; i < count; ++i)
    {
      wxArrayString entry;
      wxString key = susynclib()->GetOptionKey(i);
      entry.Add( key );
      entry.Add( TowxString( susynclib()->GetOptionType(i) ) );
      switch (susynclib()->GetOptionType(i))
      {
      case opt_float:
      {
        ret.float_map[key] = mmOptionFloat(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionNumberDef(i), susynclib()->GetOptionNumberStep(i),
            susynclib()->GetOptionNumberMin(i),susynclib()->GetOptionNumberMax(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( susynclib()->GetOptionNumberDef(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberStep(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberMin(i) ) );
        entry.Add( TowxString( susynclib()->GetOptionNumberMax(i)) );
        break;
      }
      case opt_bool:
      {
        ret.bool_map[key] = mmOptionBool(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionBoolDef(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( susynclib()->GetOptionBoolDef(i) ) );
        break;
      }
      case opt_string:
      {
        ret.string_map[key] = mmOptionString(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionStringDef(i),susynclib()->GetOptionStringMaxLen(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( susynclib()->GetOptionStringDef(i) );
        entry.Add( TowxString( susynclib()->GetOptionStringMaxLen(i) ) );
        break;
      }
      case opt_list:
      {
         ret.list_map[key] = mmOptionList(susynclib()->GetOptionName(i),key,
            susynclib()->GetOptionDesc(i),susynclib()->GetOptionListDef(i));
        entry.Add( susynclib()->GetOptionName(i) );
        wxString descr = susynclib()->GetOptionDesc(i);
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( susynclib()->GetOptionListDef(i) );
        entry.Add( TowxString( susynclib()->GetOptionListCount(i) ) );
         for (int j = 0; j < susynclib()->GetOptionListCount(i); ++j)
         {
           wxString descr = susynclib()->GetOptionListItemDesc(i,j);
           descr.Replace( _T("\n"), _T("") );
           ret.list_map[key].addItem(susynclib()->GetOptionListItemKey(i,j),susynclib()->GetOptionListItemName(i,j), descr);
           entry.Add( susynclib()->GetOptionListItemKey(i,j) );
           entry.Add( susynclib()->GetOptionListItemName(i,j) );
           entry.Add( descr );
         }
      }
      }
    wxString optiontoken;
    unsigned int entrycount = entry.GetCount();
    for ( unsigned int pos = 0; pos < entrycount; pos++ ) optiontoken << entry[pos] << _T('\t');
    cache.Add( optiontoken );
    }
  SetCacheFile( GetFileCachePath( name, _T(""), true ) + _T(".modoptions"), cache );
  }
  return ret;
}


int SpringUnitSync::GetSideCount( const wxString& modname )
{
  wxLogDebugFunc( _T("") );
  if ( !ModExists( modname ) ) return 0;
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".sidecount") );
  }
  catch (...)
  {
    susynclib()->AddAllArchives( GetModArchive( susynclib()->GetModIndex( modname )  ) );
    cache.Add( TowxString( susynclib()->GetSideCount( modname ) ) );
    SetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".sidecount"), cache );
  }
  return (int)s2l( cache[0] );
}


wxString SpringUnitSync::GetSideName( const wxString& modname, int index )
{
  wxLogDebugFunc( _T("") );
  if ( (index < 0) || (!ModExists( modname )) ) return _T("unknown");
  if ( index >= GetSideCount( modname ) ) return _T("unknown");
  ASSERT_LOGIC( GetSideCount( modname ) > index, _T("Side index too high.") );
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + TowxString( index ) + _T(".sidename") );
  }
  catch (...)
  {
    susynclib()->GetSideCount( modname );
    cache.Add( susynclib()->GetSideName( modname, index ) );
    SetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + TowxString( index ) + _T(".sidename"), cache );
  }
  return cache[0];
}


wxImage SpringUnitSync::GetSidePicture( const wxString& modname, const wxString& SideName )
{
  wxLogDebugFunc( _T("") );

  wxImage cache;

  if ( !cache.LoadFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + SideName + _T(".sidepicture.png") ) )
  {
    susynclib()->SetCurrentMod( modname );
    wxLogDebugFunc( _T("SideName = \"") + SideName + _T("\"") );
    wxString ImgName = _T("SidePics");
    ImgName += _T("/");
    ImgName += SideName.Upper();
    ImgName += _T(".bmp");

    int ini = susynclib()->OpenFileVFS (ImgName );
    ASSERT_RUNTIME( ini, _T("cannot find side image") );

    int FileSize = susynclib()->FileSizeVFS(ini);
    if (FileSize == 0) {
      susynclib()->CloseFileVFS(ini);
      ASSERT_RUNTIME( FileSize, _T("side image has size 0") );
    }

    char* FileContent = new char [FileSize];
    susynclib()->ReadFileVFS(ini, FileContent, FileSize);
    wxMemoryInputStream FileContentStream( FileContent, FileSize );

    cache.LoadFile( FileContentStream, wxBITMAP_TYPE_ANY, -1);
    delete[] FileContent;
    cache.InitAlpha();
    for ( int x = 0; x < cache.GetWidth(); x++ )
      for ( int y = 0; y < cache.GetHeight(); y++ )
        if ( cache.GetBlue( x, y ) == 255 && cache.GetGreen( x, y ) == 255 && cache.GetRed( x, y ) == 255 ) cache.SetAlpha( x, y, 0 ); /// set pixel to be transparent
    cache.SaveFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + SideName + _T(".sidepicture.png"), wxBITMAP_TYPE_PNG );
  }
  return cache;
}


wxArrayString SpringUnitSync::GetAIList( const wxString& modname )
{
  wxLogDebugFunc( _T("") );

  /// list dynamic link libraries
  int dllini = susynclib()->InitFindVFS(  wxDynamicLibrary::CanonicalizeName(_T("AI/Bot-libs/*"), wxDL_MODULE) );

  wxArrayString ret;
  wxString FileName;

  dllini = susynclib()->FindFilesVFS( dllini, FileName );
  while ( dllini ) {
    if ( ret.Index( FileName.BeforeLast( '/') ) == wxNOT_FOUND ) ret.Add ( FileName ); /// don't add duplicates
    dllini = susynclib()->FindFilesVFS( dllini, FileName );
  }
  /// list jar files (java AIs)
  int jarini = susynclib()->InitFindVFS(  _T("AI/Bot-libs/*.jar") );

  jarini = susynclib()->FindFilesVFS( jarini, FileName );
  while ( jarini ) {
    if ( ret.Index( FileName.BeforeLast( '/') ) == wxNOT_FOUND ) ret.Add ( FileName ); /// don't add duplicates
    jarini = susynclib()->FindFilesVFS( jarini, FileName );
  }

  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".luaai") );

    unsigned int aicount = cache.GetCount();
    for ( unsigned int count = 0; count < aicount; count++ ) ret.Add( cache[count] );
  } catch (...)
  {
    /// list mod's LuaAI
    try { /// Older versions of unitsync does not have these functions.
      const int LuaAICount = susynclib()->GetLuaAICount( modname );
      for ( int i = 0; i < LuaAICount; i++ )
      {
         ret.Add( _T( "LuaAI:" ) +  susynclib()->GetLuaAIName( i ) );
         cache.Add( _T( "LuaAI:" ) +  susynclib()->GetLuaAIName( i ) );
      }
    } catch (...) {}
    SetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".luaai"), cache );
  }
  return ret;
}


int SpringUnitSync::GetNumUnits( const wxString& modname )
{
  wxLogDebugFunc( _T("") );

  susynclib()->AddAllArchives( susynclib()->GetPrimaryModArchive( susynclib()->GetModIndex( modname ) ) );
  susynclib()->ProcessUnitsNoChecksum();

  return susynclib()->GetUnitCount();
}


wxArrayString SpringUnitSync::GetUnitsList( const wxString& modname )
{
  wxLogDebugFunc( modname );

  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".units") );
  } catch(...)
  {
    susynclib()->SetCurrentMod( modname );
    while ( susynclib()->ProcessUnitsNoChecksum() );
    unsigned int unitcount = susynclib()->GetUnitCount();
    for ( unsigned int i = 0; i < unitcount; i++ )
    {
      cache.Add( susynclib()->GetFullUnitName(i) << _T(" (") << susynclib()->GetUnitName(i) << _T(")") );
    }

    SetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".units"), cache );

  }

  return cache;
}



wxImage SpringUnitSync::GetMinimap( const wxString& mapname, int width, int height )
{
  wxLogDebugFunc( mapname );

  wxString originalsizepath = GetFileCachePath( mapname, _T(""), false ) + _T(".minimap.png");

  wxImage img;

  try
  {
  ASSERT_RUNTIME( wxFileExists( originalsizepath ), _T("File cached image does not exist") );

  img = wxImage( originalsizepath, wxBITMAP_TYPE_PNG );
  ASSERT_RUNTIME( img.Ok(), _T("Failed to load cache image") );

  MapInfo mapinfo = _GetMapInfoEx( mapname );

  float picratio = (float)mapinfo.height / (float)mapinfo.width;
  int resizewidth, resizeheight;
  if ( picratio < 1 )
  {
    resizewidth = width;
    resizeheight = (int)( (float)resizewidth * picratio );
  }
  else
  {
    resizeheight = height;
    resizewidth = (int)( (float)resizeheight / picratio );
  }

  img.Rescale( resizewidth, resizeheight );

  } catch (...)
  {
    try
    {

    img = susynclib()->GetMinimap( mapname );

    img.SaveFile( originalsizepath, wxBITMAP_TYPE_PNG );

    MapInfo mapinfo = _GetMapInfoEx( mapname );

    float picratio = (float)mapinfo.height / (float)mapinfo.width;
    int resizewidth, resizeheight;
    if ( picratio < 1 )
    {
      resizewidth = width;
      resizeheight = (int)( (float)resizewidth * picratio );
    }
    else
    {
      resizeheight = height;
      resizewidth = (int)( (float)resizeheight / picratio );
    }

    img.Rescale( resizewidth, resizeheight );
    }
    catch(...)
    {
      img = wxImage( -1, -1 );
    }
  }

  return img;
}

MapInfo SpringUnitSync::_GetMapInfoEx( const wxString& mapname )
{
  MapInfo info;
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( mapname, _T(""), false ) + _T(".infoex") );

    ASSERT_RUNTIME( cache.GetCount() >= 10, _T("not enought lines found in cache info ex") );
    info.author = cache[0];
    info.tidalStrength =  s2l( cache[1] );
    info.gravity = s2l( cache[2] );
    info.maxMetal = s2l( cache[3] );
    info.extractorRadius = s2d( cache[4] );
    info.minWind = s2l( cache[5] );
    info.maxWind = s2l( cache[6] );
    info.width = s2l( cache[7] );
    info.height = s2l( cache[8] );
    info.posCount = s2l( cache[9] );

    wxArrayString posinfo = wxStringTokenize( cache[10], _T(' '), wxTOKEN_RET_EMPTY );
    for ( int i = 0; i < info.posCount; i++)
    {
       StartPos position;
       position.x = s2l( posinfo[i].BeforeFirst( _T('-') ) );
       position.y = s2l( posinfo[i].AfterFirst( _T('-') ) );
       info.positions[i] = position;
    }

    unsigned int LineCount = cache.GetCount();
    for ( unsigned int i = 11; i < LineCount; i++ ) info.description << cache[i] << _T('\n');

  }
  catch (...)
  {
    info = susynclib()->GetMapInfoEx( mapname, 1 );

    cache.Add ( info.author );
    cache.Add( TowxString( info.tidalStrength ) );
    cache.Add( TowxString( info.gravity ) );
    cache.Add( TowxString( info.maxMetal ) );
    cache.Add( TowxString( info.extractorRadius ) );
    cache.Add( TowxString( info.minWind ) );
    cache.Add( TowxString( info.maxWind )  );
    cache.Add( TowxString( info.width ) );
    cache.Add( TowxString( info.height ) );
    cache.Add( TowxString( info.posCount ) );

    wxString postring;
    for ( int i = 0; i < info.posCount; i++)
    {
       postring << TowxString( info.positions[i].x ) << _T('-') << TowxString( info.positions[i].y ) << _T(' ');
    }
    cache.Add( postring );

    wxArrayString descrtoken = wxStringTokenize( info.description, _T('\n') );
    unsigned int desclinecount = descrtoken.GetCount();
    for ( unsigned int count = 0; count < desclinecount; count++ ) cache.Add( descrtoken[count] );

    SetCacheFile( GetFileCachePath( mapname, _T(""), false ) + _T(".infoex"), cache );
  }

  return info;
}


bool SpringUnitSync::ReloadUnitSyncLib()
{
  usync().FreeUnitSyncLib();
  usync().LoadUnitSyncLib( sett().GetSpringDir(), sett().GetUnitSyncUsedLoc() );
  return true;
}


void SpringUnitSync::SetSpringDataPath( const wxString& path )
{
  susynclib()->SetSpringConfigString( _T("SpringData"), path );
}


wxString SpringUnitSync::GetSpringDataPath()
{
  return susynclib()->GetSpringConfigString( _T("SpringData"), sett().GetSpringDir() );
}


wxString SpringUnitSync::GetFileCachePath( const wxString& name, const wxString& hash, bool IsMod )
{
  wxString ret = sett().GetCachePath();
  if ( !name.IsEmpty() ) ret << name;
  else if ( !hash.IsEmpty() )
  {
    if ( IsMod ) ret << m_mods_list[hash];
    else ret << m_maps_list[hash];
  }
  else return wxEmptyString;
  if ( !hash.IsEmpty() ) ret << hash;
  else
  {
    if ( IsMod ) ret <<  _T("-") << susynclib()->GetPrimaryModChecksumFromName( name );
    else
    {
        //very important to call getmapcount before getmapchecksum
       int total = susynclib()->GetMapCount();
       int index = GetMapIndex( name );
       if ( index == -1 || index > total ) return ret;
       ret << _T("-") << susynclib()->GetMapChecksum( index );
    }
  }
  return ret;
}


wxArrayString SpringUnitSync::GetCacheFile( const wxString& path )
{
  wxArrayString ret;
  wxTextFile file( path );
  file.Open();
  ASSERT_RUNTIME( file.IsOpened() , wxString::Format( _T("cache file( %s ) not found"), path.c_str() ) );
  unsigned int linecount = file.GetLineCount();
  for ( unsigned int count = 0; count < linecount; count ++ )
  {
    ret.Add( file[count] );
  }
  return ret;
}


void SpringUnitSync::SetCacheFile( const wxString& path, const wxArrayString& data )
{
  wxTextFile file( path );
  unsigned int arraycount = data.GetCount();
  for ( unsigned int count = 0; count < arraycount; count++ )
  {
    file.AddLine( data[count] );
  }
  file.Write();
  file.Close();
}


bool SpringUnitSync::FileExists( const wxString& name )
{
  int handle = susynclib()->OpenFileVFS(name);
  if ( handle == 0 ) return false;
  susynclib()->CloseFileVFS(handle);
  return true;
}


wxString SpringUnitSync::GetArchivePath( const wxString& name )
{
  wxLogDebugFunc( name );

  return susynclib()->GetArchivePath( name );
}
