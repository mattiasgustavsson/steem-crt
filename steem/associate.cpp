/*---------------------------------------------------------------------------
PROJECT: Steem SSE
Atari ST emulator
Copyright (C) 2020 by Anthony Hayward and Russel Hayward + SSE

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.

DOMAIN: GUI
FILE: associate.cpp
DESCRIPTION: The code to perform the sometimes confusing task of associating
Steem with required file types for the current user. 
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <conditions.h>
#include <steemh.h>
#include <gui.h>
#include <debug.h>
#include <mymisc.h>


#ifdef WIN32

#define LOGSECTION LOGSECTION_INIT

char key_location[]="Software\\Classes\\"; // where we put the extensions

/*  Function RegDeleteTree() isn't available in older compiler/Windows versions.
    So we use RegDelnode() and RegDelnodeRecurse() from:
    http://msdn.microsoft.com/en-au/windows/desktop/ms724235(v=vs.85).aspx
    In turn, function StringCchCopy() isn't available, so we use strcpy
    instead.
*/

#include <windows.h>
#include <stdio.h>


// we disable the warning because using strncpy or strcpy_s causes a 
// crash on deassociating
//#pragma warning (disable: 4100)

HRESULT StringCchCopy(LPTSTR pszDest,size_t cchDest,LPCTSTR pszSrc) {
  strcpy(pszDest,pszSrc);
  return 0;
}

//#pragma warning (default: 4100)



//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        } 
        else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[MAX_PATH*2];

    StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

#endif//WIN32


bool IsSteemAssociated(EasyStr Exts) {

#ifdef WIN32

  LONG ErrorCode;
  HKEY Key;
  EasyStr KeyName;
  DWORD Size;
  if(Exts[0]!='.')
    Exts.Insert(".",0); //eg "ST" -> ".ST"
  Exts.Insert(key_location,0);
  ErrorCode=RegOpenKeyEx(HKEY_CURRENT_USER,Exts,0,KEY_ALL_ACCESS,&Key);
  //TRACE_LOG("RegOpenKeyEx %s ErrorCode %d\n",Exts.Text,ErrorCode);
  if(!ErrorCode) // extension recorded, but for us?
  {
    // get value and close key
    Size=400;
    KeyName.SetLength(Size);
    RegQueryValueEx(Key,NULL,NULL,NULL,(BYTE*)KeyName.Text,&Size);
    //TRACE_LOG("RegQueryValueEx %s = %s ErrorCode %d\n",Exts.Text,KeyName.Text,ErrorCode);
    RegCloseKey(Key);
    if(KeyName.Empty())
      KeyName=Exts;
    else
      KeyName.Insert("Software\\Classes\\",0);
    // find shell key
    ErrorCode=RegOpenKeyEx(HKEY_CURRENT_USER,KeyName+"\\Shell",0,
      KEY_ALL_ACCESS,&Key);
    //TRACE_LOG("RegOpenKeyEx %s\\Shell ErrorCode %d\n",KeyName.Text,ErrorCode);
    RegCloseKey(Key);
    if(!ErrorCode)
    {
      ErrorCode=RegOpenKeyEx(HKEY_CURRENT_USER,KeyName+"\\Shell\\OpenSteem\\Command",
        //ErrorCode=RegOpenKeyEx(HKEY_CURRENT_USER,KeyName+"\\Shell\\Open\\Command",
        0,KEY_READ,&Key);
      //TRACE_LOG("RegOpenKeyEx %s\\Shell\\OpenSteem\\Command ErrorCode %d\n",KeyName.Text,ErrorCode);
      if(!ErrorCode)
      {
        Size=400;
        EasyStr RegCommand;
        RegCommand.SetLength(Size);
        RegQueryValueEx(Key,NULL,NULL,NULL,(BYTE*)RegCommand.Text,&Size);
        //TRACE_LOG("RegQueryValueEx %s = %s ErrorCode %d\n",Exts.Text,RegCommand.Text,ErrorCode);
        RegCloseKey(Key);
        EasyStr ThisExeName=GetEXEFileName(); // TODO, no PRGID?
        EasyStr Command=char('"');
        Command.SetLength(MAX_PATH+5);
        GetLongPathName(ThisExeName,Command.Text+1,MAX_PATH);
        Command+=EasyStr(char('"'))+" \"%1\"";
        if(IsSameStr_I(Command,RegCommand))
          return true; // yes, our EXE is associated
      }
    }
  }
#endif  
  return 0; // no, our EXE is not associated
}


void AssociateSteem(EasyStr Exts,EasyStr FileClass) {

#ifdef WIN32
  LONG ErrorCode;
  HKEY Key;
  EasyStr KeyName,OriginalKeyName;
  // check before Exts will change
  bool WasAlreadyAssociated=IsSteemAssociated(Exts);
  if(Exts[0]!='.')
    Exts.Insert(".",0);
  Exts.Insert(key_location,0);
/*  If this version of Steem is already associated, we come here to
    remove the association. We didn't create a separate function to spare
    some code.
*/
  if(WasAlreadyAssociated)
    RegDelnode(HKEY_CURRENT_USER,Exts.c_str());
  else
  {
    // create key
    Exts+="\\Shell\\OpenSteem\\Command";
    ErrorCode=RegCreateKeyEx(HKEY_CURRENT_USER,Exts.Text,0,NULL,
      REG_OPTION_NON_VOLATILE,
      KEY_ALL_ACCESS,NULL,&Key,NULL);
    TRACE_LOG("RegCreateKeyEx %s ErrorCode %d\n",Exts.Text,ErrorCode);
    // set value
    EasyStr ThisExeName=GetEXEFileName();
    EasyStr Command=EasyStr("\"")+ThisExeName+"\" \"%1\"";
    ErrorCode=RegSetValueEx(Key,NULL,0,REG_SZ,
      (BYTE*)Command.Text,(DWORD)Command.Length()+1);
    TRACE_LOG("RegSetValueEx %s ErrorCode %d\n",(BYTE*)Command.Text,ErrorCode);
    RegCloseKey(Key);
  }
#endif
}
