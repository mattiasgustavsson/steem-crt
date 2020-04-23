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
FILE: patchesbox.cpp
DESCRIPTION: The code for Steem's patches dialog that allows the user to
apply patches to fix ST programs that don't work or are incompatible with
Steem. Patches can also be used to defeat a game protection.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <dirsearch.h>
#include <steemh.h>
#include <patchesbox.h>
#include <gui.h>
#include <mymisc.h>
#include <choosefolder.h>
#include <display.h>
#include <translate.h>
#include <emulator.h>
#include <cpu.h>


TPatchesBox::TPatchesBox() {
  Section="Patches";
#ifdef WIN32
  Left=(GuiSM.cx_screen()-456)/2;
  Top=(GuiSM.cy_screen()-(411+GuiSM.cy_caption()))/2;
  FSLeft=(640-456)/2;
  FSTop=(480-(411+GuiSM.cy_caption()))/2;
#endif
#ifdef UNIX
  PatchList.owner=this;
  ApplyBut.owner=this;
  PatchDirBut.owner=this;
#endif
}


void TPatchesBox::RefreshPatchList() {
  if(Handle==WINDOWTYPE(0)) 
    return;
  EasyStr ThisVerText=(char*)stem_version_text;
  for(INT_PTR n=0;n<ThisVerText.Length();n++) 
  { // Cut off beta number
    if(ThisVerText[n]<'0'||ThisVerText[n]>'9') {
      if(ThisVerText[n]!='.') {
        ThisVerText[n]=0;
        break;
      }
    }
  }
  double ThisVer=atof(ThisVerText);
#ifdef WIN32
  EasyStringList sl;
  SendDlgItemMessage(Handle,100,LB_RESETCONTENT,0,0);
#endif
#ifdef UNIX
  EasyStringList &sl=PatchList.sl;
  sl.DeleteAll();
#endif
  sl.Sort=eslSortByNameI;
  DirSearch ds;
  if(ds.Find(PatchDir+SLASH "*.stp")) 
  {
    do {
      if(ThisVer<atof(GetCSFStr("Text","Obsolete","9999",PatchDir+SLASH+ds.Name))) 
      {
        *strrchr(ds.Name,'.')=0;
        if(ds.Name[0]) 
          sl.Add(ds.Name);
      }
    } while(ds.Next());
  }
  if(sl.NumStrings) 
  {
    int iSel=-1;
    for(int s=0;s<sl.NumStrings;s++) 
    {
      WIN_ONLY(SendDlgItemMessage(Handle,100,LB_ADDSTRING,0,LPARAM(sl[s].String)); )
      if(IsSameStr_I(sl[s].String,SelPatch)) 
        iSel=s;
    }
    if(iSel==-1) 
    {
      iSel=0;
      SelPatch=sl[0].String;
    }
#ifdef WIN32
    EnableWindow(GetDlgItem(Handle,100),TRUE);
    SendDlgItemMessage(Handle,100,LB_SETCURSEL,iSel,0);
#endif
#ifdef UNIX
    PatchList.changesel(iSel);
    PatchList.draw(true,true);
#endif
  }
  else 
  {
    WIN_ONLY(EnableWindow(GetDlgItem(Handle,100),0); )
    SelPatch="";
  }
  ShowPatchFile();
#ifdef WIN32
  WriteCSFStr(Section,"LastKnownVersion",GetPatchVersion(),globalINIFile);
  SetButtonIcon();
#endif
}


void TPatchesBox::PatchPoke(MEM_ADDRESS &ad,int Len,DWORD Data) {
  ad&=0xffffff;
  if(ad<himem) 
  {
    switch(Len) {
    case 1: PEEK(ad)=BYTE(Data);   break;
    case 2: DPEEK(ad)=WORD(Data);  break;
    case 4: LPEEK(ad)=DWORD(Data); break;
    }
  }
  else if(ad>=MEM_IO_BASE) 
  {
    BYTE old_bus_mask=BUS_MASK;
    TRY_M68K_EXCEPTION // thread-safe
      switch(Len) {
      case 1:
        BUS_MASK=(ad&1) ? (BUS_MASK_ACCESS|BUS_MASK_LOBYTE|BUS_MASK_WRITE)
          : (BUS_MASK_ACCESS|BUS_MASK_HIBYTE|BUS_MASK_WRITE);
        io_write_b(ad,BYTE(Data));
        break;
      case 2: 
        BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
        io_write_w(ad,WORD(Data));  
        break;
      case 4:
        BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
        io_write_w(ad,Data>>16);
        io_write_w(ad+2,Data&0xffff);
        break;
      }
    CATCH_M68K_EXCEPTION
     // feedback?
    END_M68K_EXCEPTION
    BUS_MASK=old_bus_mask;
  }
  ad+=Len;
}


void TPatchesBox::ApplyPatch() {
  if(SelPatch.Empty()) 
    return;
  EasyStr pf=PatchDir+SLASH+SelPatch+".stp";
  FILE *f=fopen(pf,"rb");
  if(f)
  {
    EasyStr Text;
    int Len=GetFileLength(f);
    Text.SetLength(Len);
    fread(Text.Text,1,Len,f);
    fclose(f);
    strupr(Text);
    int NumBytesChanged=0;
    DynamicArray<BYTE> Bytes;
    EasyStringList Offsets;
    Offsets.Sort=eslNoSort;
    char *Sect=strstr(Text,"\n[PATCH]");
    bool WordOnly;
    char *OffsetSect=strstr(Text,"\n[OFFSETS]");
    if(OffsetSect) 
    {
      OffsetSect+=2; // skip \n[
      char *OffsetSectEnd=strstr(OffsetSect,"\n[");
      if(OffsetSectEnd==NULL) OffsetSectEnd=Text.Right()+1; // point to NULL
      char *tp=OffsetSect;
      while(tp<OffsetSectEnd) {
        if(*tp==13||*tp==10) *tp=0;
        tp++;
      }
      tp=OffsetSect+strlen(OffsetSect)+1;
      while(tp<OffsetSectEnd) {
        char *next_line=tp+strlen(tp)+1;
        char *eq=strchr(tp,'=');
        if(eq) {
          *eq=0;eq++;
          // Offset name = tp
          WordOnly=0;
          acc_parse_search_string(eq,Bytes,WordOnly);
          MEM_ADDRESS offset_ad=acc_find_bytes(Bytes,WordOnly,0,1);
          if(offset_ad<=0xffffff) {
            while(tp[0]==' ') tp++;
            while(*(tp+strlen(tp)-1)==' ') *(tp+strlen(tp)-1)=0;
            Offsets.Add(tp,(LONG_PTR)offset_ad);
          }
        }
        tp=next_line;
      }
    }
    bool ReturnLengths;
    if(Sect) 
    {
      Sect+=2; // skip \n[
      char *SectEnd=strstr(Sect,"\n[");
      if(SectEnd==NULL) 
        SectEnd=Sect+strlen(Sect); // point to NULL
      char *tp=Sect;
      while(tp<SectEnd) {
        if(*tp==13||*tp==10) *tp=0;
        tp++;
      }
      tp=Sect+strlen(Sect)+1;
      while(tp<SectEnd) {
        char *next_line=tp+strlen(tp)+1;
        char *eq=strchr(tp,'=');
        if(eq) 
        {
          *eq=0;eq++;
          // tp can = Off+$x= or Off= or $x=.
          MEM_ADDRESS offset_ad=0xffffffff;
          int dir=-1;
          char *sym=strchr(tp,'-');
          if(sym==NULL) 
            sym=strchr(tp,'+'),dir=1;
          if(sym) 
            *sym=0;
          // sym points to + or -, dir is 1 for + and -1 for -. If sym==null tp is either Off= or $x=.
          while(tp[0]==' ') tp++;
          while(*(tp+strlen(tp)-1)==' ') *(tp+strlen(tp)-1)=0;
          for(int i=0;i<Offsets.NumStrings;i++) 
          {
            if(IsSameStr(Offsets[i].String,tp)) 
            {
              offset_ad=(MEM_ADDRESS)Offsets[i].Data[0];
              if(sym==NULL) 
                dir=0; // no offset
              break;
            }
          }
          // if offset_ad is 0xffffffff then tp hasn't been found. When sym is set
          // this should cause this part of the patch to be skipped. If sym isn't set then
          // we assume tp is an absolute address.
          if(sym)
            tp=sym+1;
          else if(offset_ad==0xffffffff&&dir)
            offset_ad=0; // not found so treat tp as an absolute address
          if(offset_ad<=0xffffff) 
          {
            MEM_ADDRESS ad=offset_ad+HexToVal(tp)*dir;  // dir=0 if there is no offset
            ReturnLengths=true;
            acc_parse_search_string(eq,Bytes,ReturnLengths);
            // Bytes is now a list of bytes in big endian format, between each byte is a length byte
            int i=0;
            while(i<Bytes.NumItems) {
              NumBytesChanged+=Bytes[i+1];
              if(Bytes[i+1]==1) {
                PatchPoke(ad,1,Bytes[i]);
                i+=2;
              }
              else if(Bytes[i+1]==2) {
                PatchPoke(ad,2,MAKEWORD(Bytes[i+2],Bytes[i]));
                i+=4;
              }
              else if(Bytes[i+1]==4) {
                PatchPoke(ad,4,MAKELONG(MAKEWORD(Bytes[i+6],Bytes[i+4]),
                  MAKEWORD(Bytes[i+2],Bytes[i])));
                i+=8;
              }
            }
          }
        }
        tp=next_line;
      }
    }
    if(NumBytesChanged)
      Alert(T("Patch successfully applied, number of bytes changed: ")+
        NumBytesChanged,T("Successful Patch"),MB_ICONINFORMATION|MB_OK);
    else 
      Alert(T("This patch file doesn't appear to do anything!"),T("Patch Error"),
        MB_ICONEXCLAMATION|MB_OK);
  }
  else
    Alert(T("Couldn't open the patch file!"),T("Patch Error"),
    MB_ICONEXCLAMATION|MB_OK);
}


void TPatchesBox::GetPatchText(char *File,Str Text[4]) {
  TConfigStoreFile CSF(File);
  char *Name[3]={"Description","ApplyWhen","Version"};
  Str NewSect=T("Patch Text Section=");
  if(NewSect=="Patch Text Section=") 
    NewSect="";
  char *Sect[2]={NewSect,"Text"};
  for(int s=0;s<2;s++) 
  {
    if(Sect[s][0]==0) s++;
    for(int n=0;n<3;n++)
      if(Text[n].Empty()) 
        Text[n]=CSF.GetStr(Sect[s],Name[n],"");
  }
  Text[3]=CSF.GetStr("Text","PatchAuthor","");
  if(NewSect.NotEmpty()) 
  {
    Str TransBy=CSF.GetStr(NewSect,"PatchAuthor","");
    if(TransBy.NotEmpty()) 
      Text[3]+=Str(WIN_ONLY("\r") "\n")+TransBy;
  }
  CSF.Close();
}


void TPatchesBox::Show() {
  if(Handle!=NULL) 
  {
#ifdef WIN32
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
#endif
    return;
  }
#ifdef WIN32
  if(FullScreen) 
    Top=MAX(Top,MENUHEIGHT);
  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Patches",T("Patches"),
    WS_CAPTION|WS_SYSMENU,Left,Top,456,411+GuiSM.cy_caption(),
    ParentWin,NULL,Inst,NULL);
  if(HandleIsInvalid()) 
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
  MakeParent(HWND(FullScreen?StemWin:NULL));
  CreateWindow("Static",T("Available Patches"),WS_VISIBLE|WS_CHILD,
    10,10,200,20,Handle,(HMENU)99,HInstance,NULL);
  CreateWindowEx(512,"Listbox","",WS_VSCROLL|WS_TABSTOP|WS_VISIBLE|WS_CHILD|
    LBS_NOINTEGRALHEIGHT|LBS_NOTIFY,10,30,180,323,Handle,(HMENU)100,HInstance,
    NULL);
  CreateWindow("Static",T("Description"),WS_VISIBLE|WS_CHILD,
    200,10,240,20,Handle,(HMENU)199,HInstance,NULL);
  HWND Win=CreateWindowEx(512,"Edit","",WS_VISIBLE|WS_CHILD|ES_MULTILINE|
    ES_AUTOVSCROLL|WS_VSCROLL,200,30,240,80,Handle,(HMENU)200,HInstance,NULL);
  MakeEditNoCaret(Win);
  CreateWindow("Static",T("Apply When"),WS_VISIBLE|WS_CHILD,
    200,120,240,20,Handle,(HMENU)209,HInstance,NULL);
  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE|WS_CHILD|ES_MULTILINE|
    ES_AUTOVSCROLL|WS_VSCROLL,200,140,240,40,Handle,(HMENU)210,HInstance,NULL);
  MakeEditNoCaret(Win);
  CreateWindow("Static",T("Version"),WS_VISIBLE|WS_CHILD,
    200,190,240,20,Handle,(HMENU)219,HInstance,NULL);
  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE|WS_CHILD|ES_MULTILINE|
    ES_AUTOVSCROLL|WS_VSCROLL,200,210,240,40,Handle,(HMENU)220,HInstance,NULL);
  MakeEditNoCaret(Win);
  CreateWindow("Static",T("Patch Author(s)"),WS_VISIBLE|WS_CHILD,
    200,260,240,20,Handle,(HMENU)229,HInstance,NULL);
  Win=CreateWindowEx(512,"Edit","",WS_VISIBLE|WS_CHILD|ES_MULTILINE|
    ES_AUTOVSCROLL|WS_VSCROLL,200,280,240,40,Handle,(HMENU)230,HInstance,NULL);
  MakeEditNoCaret(Win);
  CreateWindow("Button",T("Apply Now"),WS_TABSTOP|WS_VISIBLE|WS_CHILD|
    BS_PUSHBUTTON, 200,330,240,23,Handle,(HMENU)300,HInstance,NULL);
  CreateWindow("Static","",WS_CHILD|WS_VISIBLE|SS_ETCHEDHORZ,
    1,360,450,2,Handle,(HMENU)399,HInstance,NULL);
  int Wid=GetTextSize(Font,T("Patch folder")).Width;
  CreateWindow("Static",T("Patch folder"),WS_VISIBLE|WS_CHILD,
    10,375,Wid,23,Handle,(HMENU)400,HInstance,NULL);
  CreateWindowEx(512,"Steem Path Display",PatchDir,WS_CHILD|WS_VISIBLE,
    15+Wid,370,340-(15+Wid),25,Handle,(HMENU)401,HInstance,NULL);
  CreateWindow("Button",T("Choose"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX|
    BS_PUSHLIKE,350,371,90,23,Handle,(HMENU)402,HInstance,NULL);
  SetWindowAndChildrensFont(Handle,Font);
  RefreshPatchList();
  Focus=GetDlgItem(Handle,100);
  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);
  if(StemWin!=NULL) 
    PostMessage(StemWin,WM_USER,1234,0);
#endif//WIN32
#ifdef UNIX
  if (StandardShow(500,420,T("Patches"),
      ICO16_PATCHES,0,(LPWINDOWPROC)WinProc)) return;

  PatchLabel.create(XD,Handle,10,10,200,25,NULL,this,
                    BT_LABEL,T("Available Patches"),0,BkCol);

  PatchList.create(XD,Handle,10,35,200,340,ListviewNotifyHandler,this);

  DescLabel.create(XD,Handle,220,10,270,25,NULL,this,
                    BT_LABEL,T("Description"),0,BkCol);

	DescText.create(XD,Handle,220,35,270,80,WhiteCol);

  ApplyWhenLabel.create(XD,Handle,220,125,270,25,NULL,this,
                    BT_LABEL,T("Apply When"),0,BkCol);

	ApplyWhenText.create(XD,Handle,220,150,270,40,WhiteCol);

  VersionLabel.create(XD,Handle,220,200,270,25,NULL,this,
                    BT_LABEL,T("Version"),0,BkCol);

	VersionText.create(XD,Handle,220,225,270,40,WhiteCol);

  AuthorLabel.create(XD,Handle,220,275,270,25,NULL,this,
                    BT_LABEL,T("Patch Author"),0,BkCol);

	AuthorText.create(XD,Handle,220,300,270,40,WhiteCol);

  ApplyBut.create(XD,Handle,220,350,270,25,ButtonNotifyHandler,this,
                    BT_TEXT,T("Apply Now"),100,BkCol);


  PatchDirLabel.create(XD,Handle,10,385,0,25,NULL,this,
                    BT_LABEL,T("Patch folder"),0,BkCol);

  PatchDirBut.create(XD,Handle,490,385,0,25,ButtonNotifyHandler,this,
                    BT_TEXT,T("Choose"),200,BkCol);
  PatchDirBut.x-=PatchDirBut.w;
  XMoveWindow(XD,PatchDirBut.handle,PatchDirBut.x,PatchDirBut.y);

  PatchDirText.create(XD,Handle,15+PatchDirLabel.w,385,
  									PatchDirBut.x-10-(15+PatchDirLabel.w),25,NULL,this,
                    BT_STATIC | BT_BORDER_INDENT | BT_TEXT_PATH | BT_TEXT,
                    PatchDir,0,WhiteCol);


	RefreshPatchList();

  if (StemWin) PatBut.set_check(true);

  XMapWindow(XD,Handle);
  XFlush(XD);
#endif//UNIX

}


void TPatchesBox::Hide() {
  if(Handle==NULL) 
    return;
#ifdef WIN32
  ShowWindow(Handle,SW_HIDE);
  if(FullScreen) 
    SetFocus(StemWin);
  DestroyWindow(Handle);Handle=NULL;
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
#endif
#ifdef UNIX
  if(XD==NULL)
    return;
  StandardHide();
  if(StemWin)
    PatBut.set_check(0);
#endif
}


void TPatchesBox::ShowPatchFile() {

#ifdef WIN32
  EnableWindow(GetDlgItem(Handle,200),SelPatch.NotEmpty());
  EnableWindow(GetDlgItem(Handle,210),SelPatch.NotEmpty());
  EnableWindow(GetDlgItem(Handle,220),SelPatch.NotEmpty());
  EnableWindow(GetDlgItem(Handle,230),SelPatch.NotEmpty());
  EnableWindow(GetDlgItem(Handle,300),SelPatch.NotEmpty());
#endif

  if(SelPatch.NotEmpty()) 
  {
    Str Text[4];
    GetPatchText(PatchDir+SLASH+SelPatch+".stp",Text);

#ifdef WIN32
    SendDlgItemMessage(Handle,200,WM_SETTEXT,0,LPARAM(Text[0].Text));
    SendDlgItemMessage(Handle,210,WM_SETTEXT,0,LPARAM(Text[1].Text));
    SendDlgItemMessage(Handle,220,WM_SETTEXT,0,LPARAM(Text[2].Text));
    SendDlgItemMessage(Handle,230,WM_SETTEXT,0,LPARAM(Text[3].Text));
#endif

#ifdef UNIX
  DescText.set_text(Text[0]);      DescText.draw();
  ApplyWhenText.set_text(Text[1]); ApplyWhenText.draw();
  VersionText.set_text(Text[2]);   VersionText.draw();
  AuthorText.set_text(Text[3]);    AuthorText.draw();
#endif
  }
}


#ifdef WIN32


void TPatchesBox::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem Patches";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_PATCHES);
}


EasyStr TPatchesBox::GetPatchVersion() {
  DWORD Attrib=GetFileAttributes(PatchDir);
  if(Attrib<0xffffffff&&(Attrib & FILE_ATTRIBUTE_DIRECTORY)) 
  {
    FILE *f=fopen(PatchDir+SLASH+"version","rb");
    if(f) 
    {
      char Text[100];ZeroMemory(Text,100);
      fread(Text,1,100,f);
      fclose(f);
      return EasyStr(Text);
    }
  }
  return "";
}


void TPatchesBox::SetButtonIcon() {
  if(StemWin==NULL) 
    return;
  Str LastVerSeen=GetCSFStr("Patches","LastKnownVersion","",globalINIFile);
  if(LastVerSeen.NotEmpty()) 
  {
    if(NotSameStr_I(GetPatchVersion(),LastVerSeen)) 
    {
      SendDlgItemMessage(StemWin,113,WM_SETTEXT,0,
        LPARAM(Str(RC_ICO_PATCHESNEW).Text));
      return;
    }
  }
  SendDlgItemMessage(StemWin,113,WM_SETTEXT,0,LPARAM(Str(RC_ICO_PATCHES).Text));
}


#define GET_THIS This=(TPatchesBox*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK TPatchesBox::WndProc(HWND Win,UINT Mess,WPARAM wPar,
                                      LPARAM lPar) {
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  TPatchesBox *This;
  switch(Mess) {
  case WM_COMMAND:
    GET_THIS;
    switch(LOWORD(wPar)) {
    case 100:
      if(HIWORD(wPar)==LBN_SELCHANGE) 
      {
        EasyStr NewSel;
        NewSel.SetLength(MAX_PATH);
        SendMessage(HWND(lPar),LB_GETTEXT,SendMessage(HWND(lPar),LB_GETCURSEL,
          0,0),LPARAM(NewSel.Text));
        if(NotSameStr_I(NewSel,This->SelPatch)) 
        {
          This->SelPatch=NewSel;
          This->ShowPatchFile();
        }
      }
      break;
    case 300:
      if(This->SelPatch.NotEmpty()) 
        This->ApplyPatch();
      break;
    case 402:
      SendMessage(HWND(lPar),BM_SETCHECK,1,true);
      EnableAllWindows(0,Win);
      EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
        T("Pick a Folder"),This->PatchDir);
      if(NewFol.NotEmpty()) 
      {
        NO_SLASH(NewFol);
        SendDlgItemMessage(Win,401,WM_SETTEXT,0,(LPARAM)(NewFol).Text);
        SendDlgItemMessage(Win,200,WM_SETTEXT,0,LPARAM(""));
        SendDlgItemMessage(Win,210,WM_SETTEXT,0,LPARAM(""));
        SendDlgItemMessage(Win,220,WM_SETTEXT,0,LPARAM(""));
        SendDlgItemMessage(Win,230,WM_SETTEXT,0,LPARAM(""));
        This->PatchDir=NewFol;
        This->RefreshPatchList();
      }
      SetForegroundWindow(Win);
      EnableAllWindows(true,Win);
      SetFocus(HWND(lPar));
      SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      break;
    }
    break;
  case (WM_USER+1011):
  {
    GET_THIS;
    HWND NewParent=(HWND)lPar;
    if(NewParent) 
    {
      This->CheckFSPosition(NewParent);
      SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,0,0,SWP_NOZORDER|SWP_NOSIZE);
    }
    else
      SetWindowPos(Win,NULL,This->Left,This->Top,0,0,SWP_NOZORDER|SWP_NOSIZE);
    This->ChangeParent(NewParent);
    break;
  }
  case WM_CLOSE:
    GET_THIS;
    This->Hide();
    return 0;
  case DM_GETDEFID:
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#undef GET_THIS

#endif//WIN32

#ifdef UNIX

//---------------------------------------------------------------------------
int TPatchesBox::WinProc(TPatchesBox *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int TPatchesBox::ListviewNotifyHandler(hxc_listview* LV,int Mess,int I)
{
  TPatchesBox *This=(TPatchesBox*)(LV->owner);
  if (LV->sel>=0){
    if (NotSameStr_I(LV->sl[LV->sel].String,This->SelPatch)){
      This->SelPatch=LV->sl[LV->sel].String;
      This->ShowPatchFile();
    }
  }		
  return 0;
}
//---------------------------------------------------------------------------
int TPatchesBox::ButtonNotifyHandler(hxc_button* But,int Mess,int I[])
{
  TPatchesBox *This=(TPatchesBox*)(But->owner);
  if (Mess==BN_CLICKED){
  	if (But->id==100){
	    This->ApplyPatch();
	  }else if (But->id==200){
			fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
		  EasyStr Path=fileselect.choose(XD,This->PatchDir,"",T("Pick a Folder"),
		    FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");  	
		  if (Path.NotEmpty()){
        NO_SLASH(Path);
		  	This->PatchDir=Path;
		  	CreateDirectory(This->PatchDir,NULL);
		  	This->PatchDirText.set_text(This->PatchDir);
		  	
			  This->DescText.set_text("");This->DescText.draw();
			  This->ApplyWhenText.set_text("");This->ApplyWhenText.draw();
			  This->VersionText.set_text("");This->VersionText.draw();
			  This->AuthorText.set_text("");This->AuthorText.draw();
		  	This->RefreshPatchList();
		  }
	  }
  }
  return 0;
}

#endif
