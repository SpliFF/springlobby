/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: UserList
#ifndef _USERLIST_H_
#define _USERLIST_H_

#include <map>
#include <string>
#include <assert.h>
#include "user.h"

using namespace std;


//! @brief Used internally by UserList in its std::map<> lists.
struct UserListMapCompare
{
  bool operator()(const string s1, const string s2) const
  {
    return s1.compare(s2) < 0;
  }
};


//! @brief std::map<> list that stores User pointers.
typedef map<string, User*, UserListMapCompare> user_map_t;
//! @brief user_map_t iterator.
typedef user_map_t::iterator user_iter_t;


class UserList
{
  public:
    UserList();
    virtual ~UserList();
  
    // UserList interface
  
    virtual void AddUser( User* user );
    virtual void RemoveUser( const string nick );
  
    virtual User* GetUser( const string nick );
    virtual bool UserExists( const string nick );
  
    virtual int GetNumUsers();
  
  protected:
    // UserList variables
  
    user_map_t m_users;
  
};


#endif  //_USERLIST_H_
