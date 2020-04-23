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
FILE: infobox.cpp
DESCRIPTION: Steem's general information dialog.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <steemh.h>
#ifdef WIN32
#include <richedit.h>
#endif
#include <infobox.h>
#include <translate.h>
#include <resnum.h>
#include <debug.h>
#include <display.h>
#include <gui.h>
#include <mymisc.h>
#include <emulator.h>
#include <acc.h>
#include <draw.h>
#include <computer.h>
#include <notifyinit.h>


const char *Credits[]={
  "Steem Copyright (C) 2020 by Anthony Hayward and Russel Hayward + SSE",
  "This program comes with ABSOLUTELY NO WARRANTY.",
  "This is free software, and you are welcome to redistribute it under certain conditions.\
 See the GPL3 Licence for details.",
  " ",
  "F1: this should display the page you're seeing",
  "F11 (or Break): grab mouse on/off (when emulation is running)",
  "F12: emulation start/stop ",
#ifdef WIN32  
  "Alt: alternative menu (when emulation is stopped)",
  "Page Up: ST key Help",
  "Page Down: ST key Undo",
  "Alt-Enter: toggle fullscreen",
  " ",
#endif  
  "Thanks to all the people who have helped, knowingly or not, make Steem better.",
  " ",
#ifdef WIN32  
  "There are many options in Steem, please take the time to read the manual once.",
  "It is an RTF file, you can read it here or in your favourite editor.",
#endif
  NULL};


#ifdef WIN32
WNDPROC TGeneralInfo::OldEditWndProc;
#define INFOBOX_HEIGHT 420
#endif


TGeneralInfo::TGeneralInfo() {
  Section="GeneralInfo";
#ifdef WIN32
  page_l=160;
#if defined(SSE_GUI_RICHEDIT)
  page_w=596;
#else
  page_w=82*8; //with Courier New, should give 80 visible columns
#endif
  page_h=INFOBOX_HEIGHT;
  Left=(GuiSM.cx_screen()-(3+page_l+page_w+10+3))/2;
  Top=(GuiSM.cy_screen()-(page_h+GuiSM.cy_caption()))/2;
  FSLeft=(640-(3+page_l+page_w+10+3))/2;
  FSTop=(480-(page_h+GuiSM.cy_caption()))/2;
  BkBrush=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  il=NULL;
  MaxLinkID=0;
#endif
#ifdef UNIX
//  for(int n=0;n<5;n++)tab[n].owner=this;
  page_lv.owner=this;
  Page=INFOPAGE_README;
#endif
}


void TGeneralInfo::CreatePage(int pg) {
  switch(pg) {
  case INFOPAGE_ABOUT:
    CreateAboutPage();
    break;
#if defined(DEBUG_BUILD)
  case INFOPAGE_DRAWSPEED:
    CreateSpeedPage();
    break;
#endif
  case INFOPAGE_LINKS:
#if !defined(SSE_GUI_RICHEDIT2)
    CreateLinksPage();
    break;
#endif
  case INFOPAGE_README:
  case INFOPAGE_HOWTO_DISK:
  case INFOPAGE_HOWTO_CART:
  case INFOPAGE_FAQ:
  case INFOPAGE_FAQ_SSE:
#if defined(UNIX)
  case INFOPAGE_UNIXREADME:
#endif
  case INFOPAGE_README_SSE:
  case INFOPAGE_HINTS:
  case INFOPAGE_LICENCE:
  case INFOPAGE_TRACE:
  case INFOPAGE_BUGS:
  case INFOPAGE_STATS:
    CreateReadmePage(pg);
    break;
  }
}


void TGeneralInfo::CreateAboutPage() {
  int y=10,h=6;
#if defined(SSE_BUILD)
  EasyStr Text=EasyStr("Steem SSE v")+stem_version_text+" R" + SSE_VERSION_R  
    +" (built "+stem_version_date_text+")\n";
#if defined(DEBUG_BUILD)
  Text+="Debugger ";
#endif
#if defined(SSE_BETA)
  Text+="Beta ";
#endif
#if defined(SSE_VID_DD)
  Text+=EasyStr("DD")+(DIRECTDRAW_VERSION>>8)+" ";
#endif
#if defined(SSE_VID_D3D)
  Text+=EasyStr("D3D")+(DIRECT3D_VERSION>>8)+" ";
#endif
#if (_MSC_VER)
  Text+=EasyStr("VC")+_MSC_VER;
#elif defined(BCC_BUILD)
  Text+="BCC";
#elif defined(MINGW_BUILD)
  Text+="MinGW";
#elif defined(SSE_UNIX)
  Text+="GCC";
#endif
#if defined(SSE_X64)
  Text+=" 64bit\n";
#else
  Text+=" 32bit\n";
#endif
#else
  EasyStr Text=EasyStr("Steem Engine v")+(char*)stem_version_text+" (built " __DATE__" "+"- "__TIME__")\n";
  Text+="Written by Anthony && Russell Hayward\n";
  Text+="Copyright 2000-2004\n";
#endif
  if(TranslateBuf)
  {
    Text+="\n";
    Text+=T("Translation by [Your Name]");
  }

#ifdef WIN32
  int TextHeight=GetTextSize(Font,"HyITljq").Height;
  h*=TextHeight;
  CreateWindowEx(0,"Static",Text,WS_CHILD|WS_VISIBLE,
    page_l,y,page_w,h,Handle,(HMENU)200,HInstance,NULL);
  y+=h;
  y+=TextHeight;
  // some hints - created and positioned only once per page creation
  Scroller.CreateEx(512,WS_CHILD|WS_VSCROLL|WS_HSCROLL,0,0,0,0,Handle,
    (HMENU)203,HInstance);
  Scroller.SetBkColour(GetSysColor(COLOR_WINDOW));
  y=TextHeight+2;
  for(INT_PTR n=0;;n++)
  {
    if(Credits[n]==NULL)
      break;
    HWND Win=CreateWindowEx(0,"Steem HyperLink",Credits[n],WS_CHILD|WS_VISIBLE
      |HL_STATIC|HL_WINDOWBK,5,y,500,TextHeight,Scroller,HMENU(100+n),HInstance,
      NULL);
    SendMessage(Win,WM_SETFONT,WPARAM(Font),0);
    y+=TextHeight+2;
  }
  Scroller.AutoSize(2,2);
  ShowWindow(Scroller,SW_SHOW);
  CreateWindowEx(0,"Steem HyperLink",STEEM_WEB,WS_CHILD|WS_VISIBLE,0,0,0,0,
    Handle,(HMENU)201,HInstance,NULL);
  if(Focus==NULL) 
    Focus=PageTree;
  SetPageControlsFont();
  UpdatePositions();
  ShowPageControls();
#endif//WIN32

#ifdef UNIX
  about.border=0;about.pad_x=0;about.sy=0;
  about.textheight=(hxc::font->ascent)+(hxc::font->descent)+2;

  about.set_text(Text);
  // hxc::fontinfo has been set up by the creation of the tab buttons causing a load_res
  int n_lines=about.wordwrap(gb.w-20,hxc::font);
  h=n_lines*about.textheight;
  about.create(XD,gb.handle,10,10,gb.w-20,h,hxc::col_bk);
  EasyStr ta=Credits[0];
  int nta=1;
  while (Credits[nta]){
    ta+=EasyStr("\n")+Credits[nta];
    nta++;
  }
  thanks.set_text(ta);
  thanks_label.create(XD,gb.handle,10,20+h,0,0,NULL,this,
//                      BT_LABEL,T("Thanks To"),-59,hxc::col_bk);
                      BT_LABEL,T("Some hints"),-59,hxc::col_bk);
  thanks.create(XD,gb.handle,10,thanks_label.y+thanks_label.h,
                  gb.w-20,gb.h-h-70,hxc::col_white);
  steem_link.create(XD,gb.handle,10,thanks.y+thanks.h+10,0,0,hyperlink_np,this,
                    BT_LINK|BT_TEXT,STEEM_WEB,-56,hxc::col_bk);
#endif//UNIX

}


void TGeneralInfo::CreateReadmePage(int p) {

#ifdef WIN32
  int id;
  switch(p) {
#if defined(SSE_GUI_RICHEDIT)
  case INFOPAGE_README_SSE:
  case INFOPAGE_LINKS:
  case INFOPAGE_FAQ_SSE:
  case INFOPAGE_HINTS:
  case INFOPAGE_BUGS:
#if defined(SSE_STATS_RTF)
  case INFOPAGE_STATS:
#endif
    id=ID_INFOBOXRICHTEXT;
    break;
#endif
  default:
    id=ID_INFOBOXTEXT;
  }//sw
  if(GetDlgItem(Handle,id)==NULL)
  {
    int wid_of_search=get_text_width(T("Search"));
    int wid_of_find=get_text_width(T("Find"))+20;
    CreateWindow("Static",T("Search"),WS_CHILD|WS_VISIBLE,
      page_l,14,wid_of_search,23,Handle,(HMENU)503,HInstance,NULL);
    CreateWindowEx(512,"Edit",SearchText,WS_CHILD|WS_VISIBLE|WS_TABSTOP,
      page_l+wid_of_search+5,10,page_w-wid_of_find-5-wid_of_search-5,23,
      Handle,(HMENU)504,HInstance,NULL);
    CreateWindow("Button",T("Find"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|
      BS_DEFPUSHBUTTON,page_l+page_w-wid_of_find-5,10,wid_of_find,23,Handle,
      (HMENU)502,HInstance,NULL);
    CreateTextDisplay(Handle,page_l,40,page_w,page_h-50-GuiSM.cy_frame()*2,id);
  }
  Str TextFile=DocDir;
  HWND handle0=GetDlgItem(Handle,id); // group window
  HWND handle=GetDlgItem(handle0,IDC_INFOBOXTEXTCONTROL); // edit control
  switch(p) {
  case INFOPAGE_README: TextFile+="readme.txt"; 
    break;
  case INFOPAGE_README_SSE:
#if defined(SSE_GUI_RICHEDIT)
    TextFile+=STEEM_MANUAL_SSE; TextFile+=EXT_RTF;
    SendMessage(handle,EM_SETLIMITTEXT,(DWORD)-1,0); // manual > richedit default size
    SendMessage(handle,EM_AUTOURLDETECT,1,0); // auto-detect links
    SendMessage(handle,EM_SETEVENTMASK,0,ENM_LINK); // link notifications
#else
    TextFile+=STEEM_MANUAL_SSE; TextFile+=EXT_TXT; 
#endif
    break;
  case INFOPAGE_HOWTO_DISK: TextFile+="disk image howto.txt"; break;
  case INFOPAGE_HOWTO_CART: TextFile+="cart image howto.txt"; break;
  case INFOPAGE_FAQ: TextFile+="faq.txt"; break;
#if defined(SSE_GUI_RICHEDIT2)
  case INFOPAGE_FAQ_SSE: TextFile+=STEEM_SSE_FAQ; TextFile+=EXT_RTF; break;
  case INFOPAGE_HINTS: TextFile+=STEEM_HINTS; TextFile+=EXT_RTF; break;
  case INFOPAGE_BUGS: TextFile+="Bugs"; TextFile+=EXT_RTF; break;
#else
  case INFOPAGE_FAQ_SSE: TextFile+=STEEM_SSE_FAQ; TextFile+=EXT_TXT; break;
  case INFOPAGE_HINTS: TextFile+=STEEM_HINTS; TextFile+=EXT_TXT; break;
#endif
  case INFOPAGE_LICENCE: TextFile+="gpl-3.0.txt"; break;
  case INFOPAGE_TRACE: TextFile=RunDir+SLASH+SSE_TRACE_FILE_NAME; break;
#if defined(SSE_STATS)
  case INFOPAGE_STATS: 
    if(Stats.nFrame && runstate!=RUNSTATE_RUNNING)
    {
      TextFile=RunDir+SLASH+STEEM_STATS_FILENAME;   
      TNotify myNotify(T("Collecting data")); // colour counting can be slow
      Stats.Report();
    }
    break; 
#endif
#if defined(SSE_GUI_RICHEDIT2)
  case INFOPAGE_LINKS: 
    TextFile+="links"; TextFile+=EXT_RTF;
    SendMessage(handle,EM_AUTOURLDETECT,1,0); // auto-detect links
    SendMessage(handle,EM_SETEVENTMASK,0,ENM_LINK); // link notifications
    break;
#endif
  }
  FILE *f=fopen(TextFile,"rb");
  if(f)
  {
    long nbytes=GetFileLength(f);
    if(nbytes)
    {
      char *text=(char*)malloc(nbytes+1);
      text[fread(text,1,nbytes,f)]=0;
      fclose(f);
      SendMessage(handle,WM_SETTEXT,0,(LPARAM)text); // settext works for both txt and rtf
      free(text);
    }
  }
#if !defined(SSE_GUI_KBD)
  if(Focus==NULL) Focus=GetDlgItem(Handle,504);
#endif
  SetPageControlsFont();
#if !defined(SSE_GUI_RICHEDIT) // no more
  if(id==ID_INFOBOXTEXT)
    SendMessage(handle, WM_SETFONT, WPARAM(hFontCourier), TRUE);
#endif
  UpdatePositions();
  ShowPageControls();
#endif//WIN32

#ifdef UNIX
  switch (p){
  case INFOPAGE_README:
    ShowTheReadme("win32.help"); 
    break;
  case INFOPAGE_HOWTO_DISK:
    ShowTheReadme("disk image howto.txt");
    break;
#if !defined(SSE_GUI_INFOBOX_NO_CART)
  case INFOPAGE_HOWTO_CART:
    ShowTheReadme("cart image howto.txt");
    break;
#endif
  case INFOPAGE_FAQ:
    ShowTheReadme("FAQ");
    break;
  case INFOPAGE_UNIXREADME:
    ShowTheReadme("README",true);
    break;
  case INFOPAGE_LICENCE:
    ShowTheReadme("gpl-3.0.txt",true);
    break;
  }//sw
#endif//UNIX
}


#ifdef UNIX

void TGeneralInfo::ShowTheReadme(char* fn,bool stripreturns)
{
  FILE *f=fopen(RunDir+"/"+fn,"rb");
  if (f){
    int Len;
    fseek(f,0,SEEK_END);
    Len=ftell(f);
    fseek(f,0,SEEK_SET);

		readme.text.SetLength(Len);
    fread(readme.text.Text,1,Len,f);
    fclose(f);
		if (stripreturns){
			int ocn;
			for (int cn=0;cn<Len;cn++){
				if (*(readme.text.Text+cn)=='\n'){
					ocn=cn++;
					if (*(readme.text.Text+cn)!='.'){
  					while (*(readme.text.Text+cn)=='\n')cn++;
  					if (cn==ocn+1){ //just one return
  						*(readme.text.Text+ocn)=' ';
  					}
  				}
				}
			}
		}
  }
  if (hxc::find(gb.handle,500)==NULL){
    hxc_button *p_lab=new hxc_button(XD,gb.handle,5,5,0,25,NULL,NULL,BT_LABEL,T("Search"),0,hxc::col_bk);
    
    int w=hxc::get_text_width(XD,T("Find"))+8;
    hxc_button *p_but=new hxc_button(XD,gb.handle,gb.w-5-w,5,w,25,button_notifyproc,this,BT_TEXT,T("Find"),500,hxc::col_bk);
    
    hxc_edit *ed=new hxc_edit(XD,gb.handle,p_lab->x+p_lab->w+5,5,p_but->x-5-(p_lab->x+p_lab->w+5),25,
                              edit_notify_proc,this);
    ed->id=501;
    ed->set_text(GetCSFStr("Info","SearchText","",globalINIFile));
  }

  readme.sy=0;
  readme.wordwrapped=false;
	readme.create(XD,gb.handle,5,35,gb.w-10,gb.h-40,hxc::col_white,true);
  last_find_idx=0;
}

#endif


#if !defined(SSE_GUI_RICHEDIT2)

void TGeneralInfo::GetHyperlinkLists(EasyStringList &desc_sl,
                                      EasyStringList &link_sl) {
  desc_sl.Sort=eslNoSort;
  link_sl.Sort=eslNoSort;
#if defined(SSE_BUILD)
  desc_sl.Add(T("Official Steem website (legacy)"),0);
  link_sl.Add("http:/""/steem.atari.st/");
  desc_sl.Add(T("Steem SSE on Sourceforge"),0);
  //link_sl.Add("http:/""/sourceforge.net/projects/steemsse/");
  link_sl.Add(STEEM_WEB);
  desc_sl.Add(T("Steem SSE Bug Reports"),0);
  link_sl.Add(STEEM_WEB_BUG_REPORTS);
#else
  desc_sl.Add(T("Official Steem website"),0);
  link_sl.Add(STEEM_WEB);
#endif
  // Steem links
#ifdef SSE_FILES_IN_RC
  HRSRC rc=FindResource(NULL,MAKEINTRESOURCE(IDR_STEEMNEW),RT_RCDATA);
  ASSERT(rc);
  if(rc)
  {
    HGLOBAL hglob=LoadResource(NULL,rc);
    if(hglob)
    {
      size_t size=SizeofResource(NULL,rc);
      char *ptxt=(char*)LockResource(hglob);
      if(ptxt)
      {
        // identify tag eg [SCROLLERS], load strings, leave after 2 blank lines
        char tb[5000];
        EasyStr LinkOrDesc;
        int InLinks=0;
        int newline=0;
        int j=0;
        for(size_t i=0;i<size && j<5000;i++)
        {
          tb[j]=ptxt[i];
          if((tb[j]=='\n'||tb[j]=='\r')&&newline<2)
          {
            tb[j]=0;
            newline++; // can be n, nr, rn, r
            j++;
          }
          else if(newline)
          {
            newline=0;
            if(tb[0])
            {
              if(InLinks)
              {
                LinkOrDesc=tb;
                if(LinkOrDesc.NotEmpty())
                {
                  if(IsSameStr_I(tb,"[LINKSEND]"))
                    break;
                  else if(LinkOrDesc.Lefts(3)=="...")
                  {
                    desc_sl.Add(LinkOrDesc.Text+3,1);
                    link_sl.Add("");
                  }
                  else if(LinkOrDesc.Lefts(2)=="//")
                  {
                  // pass commented out line
                  }
                  else
                  {
                    if(InLinks==2)
                      link_sl.Add(LinkOrDesc);
                    else
                      desc_sl.Add(LinkOrDesc,0);
                    InLinks=3-InLinks;
                  }
                }
              }
              else
              {
                if(IsSameStr_I(tb,"[LINKS]"))
                  InLinks=1;
              }
            }
            j=0;
            i--;
          }
          else
            j++;
        }//nxt i
        //FreeResource(ghlob);
      }
    }
  }
#else
  FILE *f=fopen(DocDir + "steem.new","rt");
  if(f)
  {
    char buf[5000];
    EasyStr LinkDesc;
    bool InLinks=0;
    while(fgets(buf,5000,f))
    {
      if(strstr(buf,"[LINKS]")==buf)
      {
        InLinks=true;
      }
      else if(strstr(buf,"[LINKSEND]")==buf)
      {
        break;
      }
      else if(InLinks)
      {
        LinkDesc=buf;
        while(LinkDesc.RightChar()=='\r'||LinkDesc.RightChar()=='\n') *(LinkDesc.Right())=0;
        if(LinkDesc.NotEmpty())
        {
          if(LinkDesc.Lefts(3)=="...")
          {
            desc_sl.Add(LinkDesc.Text+3,1);
            link_sl.Add("");
          }
          else
          {
            if(fgets(buf,5000,f))
            {
              Str Link=buf;
              while(Link.RightChar()=='\r'||Link.RightChar()=='\n') *(Link.Right())=0;

              desc_sl.Add(LinkDesc,0);
              link_sl.Add(Link);
            }
          }
        }
      }
    }
    fclose(f);
  }
#endif
}


void TGeneralInfo::CreateLinksPage() {
#ifdef WIN32
#if defined(SSE_GUI_KBD) 
// we can't do it here, there's the scrolling problem too - it's not vital,
// nobody browses the internet without a mouse
  Scroller.CreateEx(WS_EX_CONTROLPARENT,WS_CHILD|WS_VSCROLL|WS_HSCROLL,
    page_l-10,0,page_w+20,INFOBOX_HEIGHT,Handle,(HMENU)400,HInstance);
#else
  Scroller.Create(WS_CHILD|WS_VSCROLL|WS_HSCROLL,page_l-10,0,page_w+20,INFOBOX_HEIGHT,Handle,400,HInstance);
#endif
  int TextHeight=GetTextSize(Font,"HyITljq").Height;
  int y=10;
  INT_PTR id=INFOPAGE_LINK_ID_BASE,group_id=3000;
  HWND Win;
  EasyStringList desc_sl,link_sl;
  GetHyperlinkLists(desc_sl,link_sl);
  for(int n=0;n<desc_sl.NumStrings;n++)
  {
    if(desc_sl[n].Data[0]==0)
    {
#if defined(SSE_GUI_KBD)
      Win=CreateWindowEx(0,"Steem HyperLink",Str(desc_sl[n].String)+"|"
        +link_sl[n].String,WS_CHILD|WS_VISIBLE|WS_TABSTOP,10,y,250,15,
        Scroller,(HMENU)id++,HInstance,NULL);
#else
      Win=CreateWindowEx(0,"Steem HyperLink",Str(desc_sl[n].String)+"|"+link_sl[n].String,
        WS_CHILD|WS_VISIBLE,10,y,250,15,
        Scroller,(HMENU)id++,HInstance,NULL);
#endif
      ToolAddWindow(ToolTip,Win,link_sl[n].String);
      y+=TextHeight+5;
    }
    else
    {
      y+=10;
#if defined(SSE_GUI_KBD)
      CreateWindowEx(0,"Steem HyperLink",desc_sl[n].String,
        HL_STATIC|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
        10,y,250,15,Scroller,(HMENU)group_id++,HInstance,NULL);
#else
      CreateWindowEx(0,"Steem HyperLink",desc_sl[n].String,
        HL_STATIC|WS_CHILD|WS_VISIBLE,
        10,y,250,15,Scroller,(HMENU)group_id++,HInstance,NULL);
#endif
      y+=TextHeight+5;
    }
  }
  MaxLinkID=id;
  SetWindowAndChildrensFont(Scroller.GetControlPage(),Font);
  Scroller.AutoSize(2,2);
#if defined(SSE_GUI_KBD)
  Focus=PageTree;
#else
  Focus=Scroller;
#endif
  SetPageControlsFont();
  ShowPageControls();
//  PostMessage(Handle,WM_SETFOCUS,0,0); //rather hacky, but this is marginal
#endif//WIN32

#ifdef UNIX
	sa.create(XD,gb.handle,1,1,gb.w-2,gb.h-2);
	int y=10;
  EasyStringList desc_sl,link_sl;
  GetHyperlinkLists(desc_sl,link_sl);

  hxc_button *b;
  for (int n=0;n<desc_sl.NumStrings;n++){
    if (desc_sl[n].Data[0]==0){
      b=new hxc_button(XD,sa.handle,10,y,0,0,
          hyperlink_np,this,BT_LINK|BT_TEXT,
          Str(desc_sl[n].String)+"|"+link_sl[n].String,0,hxc::col_bk);
      y+=b->h+5;
    }else{
      y+=10;
      b=new hxc_button(XD,sa.handle,10,y,0,0,
          NULL,this,BT_STATIC|BT_TEXT,
          desc_sl[n].String,0,hxc::col_bk);
      y+=b->h+5;
    }
  }
  sa.adjust();
#endif//UNIX
}

#endif


void TGeneralInfo::Show() {
  if(Handle!=NULL)
  {
#ifdef WIN32
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
#endif
    return;
  }

#ifdef WIN32
  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT|WS_EX_APPWINDOW,
    "Steem General Info",T("General Info"),WS_CAPTION|WS_SYSMENU|WS_SIZEBOX
    |WS_MAXIMIZEBOX,Left,Top, 0,0,ParentWin,NULL,HInstance,NULL);
  if(HandleIsInvalid())
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
#if !defined(SSE_GUI_RICHEDIT)
  hFontCourier=CreateFont(README_FONT_HEIGHT,0,0,0,FW_DONTCARE,FALSE,
    FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,README_FONT_NAME);
#endif
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
  MakeParent(HWND(FullScreen?StemWin:NULL));
  PageTree=CreateWindowEx(512,WC_TREEVIEW,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP
    |TVS_HASLINES|TVS_SHOWSELALWAYS|TVS_HASBUTTONS|TVS_DISABLEDRAGDROP,0,0,
    0,0,Handle,(HMENU)60000,Inst,NULL);
  LoadIcons();
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  AddPageLabel(T("About"),INFOPAGE_ABOUT);
#if defined(SSE_GUI_RICHEDIT)
  if(Exists(DocDir+STEEM_MANUAL_SSE+".rtf"))
#else
  if(Exists(DocDir+STEEM_MANUAL_SSE+EXT_TXT))
#endif
    AddPageLabel(STEEM_MANUAL_SSE,INFOPAGE_README_SSE);
  if(Exists(DocDir+"readme.txt"))
    AddPageLabel(T("Readme"),INFOPAGE_README);
  if(Exists(DocDir+"faq.txt")) 
    AddPageLabel("FAQ",INFOPAGE_FAQ);
#if defined(SSE_GUI_RICHEDIT2)
  if(Exists(DocDir+STEEM_SSE_FAQ+EXT_RTF))
    AddPageLabel(STEEM_SSE_FAQ,INFOPAGE_FAQ_SSE);
  if(Exists(DocDir+STEEM_HINTS+EXT_RTF))
    AddPageLabel(STEEM_HINTS,INFOPAGE_HINTS);
  if(Exists(DocDir+T("Bugs")+EXT_RTF))
    AddPageLabel(T("Bugs"),INFOPAGE_BUGS);
#else
  if(Exists(DocDir+STEEM_SSE_FAQ+EXT_TXT))
    AddPageLabel(STEEM_SSE_FAQ,INFOPAGE_FAQ_SSE);
  if(Exists(DocDir+STEEM_HINTS+EXT_TXT))
    AddPageLabel(STEEM_HINTS,INFOPAGE_HINTS);
#endif
  if(Exists(DocDir+"disk image howto.txt"))
    AddPageLabel("Disk Image Howto",INFOPAGE_HOWTO_DISK);
  if(Exists(DocDir+"cart image howto.txt"))
    AddPageLabel("Cartridge Image Howto",INFOPAGE_HOWTO_CART);
  if(Exists(DocDir+"gpl-3.0.txt"))
    AddPageLabel("GPL3 Licence",INFOPAGE_LICENCE);
  if(Exists(RunDir+SLASH+SSE_TRACE_FILE_NAME))
    AddPageLabel("Trace",INFOPAGE_TRACE);
#if defined(SSE_GUI_RICHEDIT2)
  if(Exists(DocDir+"links.rtf"))
#endif
    AddPageLabel(T("Links"),INFOPAGE_LINKS);
#ifdef DEBUG_BUILD
  //if(avg_frame_time && FullScreen==0)
    AddPageLabel(T("Draw Speed"),INFOPAGE_DRAWSPEED);
#endif
#if defined(SSE_STATS)
    AddPageLabel(T("Status"),INFOPAGE_STATS);
#endif
  page_l=2+TreeGetMaxItemWidth(PageTree)+5+2+10;
  SetWindowPos(Handle,NULL,0,0,3+page_l+page_w+10+3,GuiSM.cy_caption()+3+page_h+3,
    SWP_NOZORDER|SWP_NOMOVE);
  Focus=NULL;
  while(TreeSelectItemWithData(PageTree,Page)==NULL) 
    Page=INFOPAGE_ABOUT;
  ShowWindow(Handle,SW_SHOW);
#if defined(SSE_GUI_KBD)
  SetFocus(PageTree);
#else
  SetFocus(Focus);
#endif
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
#endif//WIN32

#ifdef UNIX
  bool ShowXReadme=Exists(RunDir+"/README");
  bool ShowFAQ=Exists(RunDir+"/FAQ");
  bool ShowReadme=Exists(RunDir+"/win32.help");
#ifdef DEBUG_BUILD
  bool ShowDrawSpeed=0; /*(avg_frame_time && FullScreen==0);*/
#endif
  bool ShowDiskHowto=Exists(RunDir+"/disk image howto.txt");
  bool ShowCartHowto=Exists(RunDir+"/cart image howto.txt");
  bool ShowLicence=Exists(RunDir+"/gpl-3.0.txt");
  page_lv.sl.DeleteAll();
  page_lv.sl.Sort=eslNoSort;
  page_lv.sl.Add(T("About"),101+ICO16_GENERALINFO,INFOPAGE_ABOUT);
  if(ShowXReadme)
    page_lv.sl.Add(EasyStr("X ")+T("Readme"),101+ICO16_README,INFOPAGE_UNIXREADME);
  if(ShowReadme)
    page_lv.sl.Add(EasyStr("Win32 ")+T("Readme"),101+ICO16_README,INFOPAGE_README);
  if(ShowFAQ)
    page_lv.sl.Add("FAQ",101+ICO16_FAQ,INFOPAGE_FAQ);
#ifdef DEBUG_BUILD
  if(ShowDrawSpeed) 
    page_lv.sl.Add(T("Draw Speed"),101+ICO16_DRAWSPEED,INFOPAGE_DRAWSPEED);
#endif
  page_lv.sl.Add(T("Links"),101+ICO16_LINKS,INFOPAGE_LINKS);
  if(ShowDiskHowto)
    page_lv.sl.Add("Disk Image Howto",101+ICO16_DISK,INFOPAGE_HOWTO_DISK);
#if !defined(SSE_GUI_INFOBOX_NO_CART)
  if(ShowCartHowto)
    page_lv.sl.Add("Cartridge Image Howto",101+ICO16_CHIP,INFOPAGE_HOWTO_CART);
#endif
  if(ShowLicence)
    page_lv.sl.Add("GPL3 Licence",101+ICO16_README,INFOPAGE_LICENCE);
  page_lv.lpig=&Ico16;
  page_lv.display_mode=1;
  int page_lv_w=page_lv.get_max_width(XD);
  if(StandardShow(page_lv_w+500,460,T("General Info"),ICO16_GENERALINFO,0,
    (LPWINDOWPROC)WinProc))
    return;
  while(page_lv.select_item_by_data(Page,1)==-1) 
    Page=INFOPAGE_ABOUT;
  page_lv.id=60000;
  page_lv.create(XD,Handle,0,0,page_lv_w,460,listview_notify_proc,this);
  gb.create(XD,Handle,page_lv.w,0,500,460,
    NULL,this,BT_STATIC | BT_BORDER_NONE,"",-4,hxc::col_bk);
  CreatePage(Page);
  if(StemWin)
    InfBut.set_check(true);
  XMapWindow(XD,Handle);
  XFlush(XD);
#endif//UNIX
}


void TGeneralInfo::Hide() {
  if(Handle==NULL) 
    return;
#ifdef WIN32
  ShowWindow(Handle,SW_HIDE);
  if(FullScreen) 
    SetFocus(StemWin);
#if !defined(SSE_GUI_RICHEDIT)
  DeleteObject(hFontCourier);
#endif
  DestroyCurrentPage();
  DestroyWindow(Handle);Handle=NULL;
  ImageList_Destroy(il); il=NULL;
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
#endif//WIN32

#ifdef UNIX
  if(XD==NULL)
    return;
  StandardHide();
  if(StemWin)
    InfBut.set_check(0);
#endif
}


#ifdef WIN32

void TGeneralInfo::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem General Info";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_INFO);
}


void TGeneralInfo::LoadIcons() {
  if(Handle==NULL) 
    return;
  HIMAGELIST old_il=il;
  il=ImageList_Create(18,20,BPPToILC[BytesPerPixel]|ILC_MASK,7,7);
  if(il)
  {
    ImageList_AddPaddedIcons(il,PAD_ALIGN_RIGHT,
      hGUIIcon[RC_ICO_INFO],
      hGUIIcon[RC_ICO_INFO_CLOCK],
      hGUIIcon[RC_ICO_FUJILINK],
      hGUIIcon[RC_ICO_TEXT],
      hGUIIcon[RC_ICO_INFO],
      hGUIIcon[RC_ICO_DISK_HOWTO],
      hGUIIcon[RC_ICO_CART_HOWTO],
      hGUIIcon[RC_ICO_INFO_FAQ],
      hGUIIcon[RC_ICO_INFO_FAQ],
      hGUIIcon[RC_ICO_INFO_FAQ],//hints
      hGUIIcon[RC_ICO_TEXT],
      hGUIIcon[RC_ICO_TEXT],
      hGUIIcon[RC_ICO_TEXT],
      hGUIIcon[RC_ICO_TEXT],
      hGUIIcon[RC_ICO_INFO_CLOCK], //stats?
      0);
  }
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  if(old_il) 
    ImageList_Destroy(old_il);
}


void TGeneralInfo::DestroyCurrentPage() {
  HWND LinkParent=Scroller.GetControlPage();
  if(LinkParent)
  {
    ToolsDeleteAllChildren(ToolTip,LinkParent);
    MaxLinkID=0;
  }
  TStemDialog::DestroyCurrentPage();
}


#if defined(DEBUG_BUILD)

INT_PTR TGeneralInfo::DrawColumn(int x,int y,INT_PTR id,char *t1,...) {
  va_list vl;
  va_start(vl,t1);
  char* arg=t1;
  while(*arg!='*')
  {
    if(*arg=='-')
      y+=10;
    else
    {
      CreateWindow("Static",arg,WS_CHILD|WS_VISIBLE,x,y,MIN(180,page_l+page_w-x),17,
        Handle,(HMENU)id,HInstance,NULL);
      y+=28;id++;
    }
    arg=va_arg(vl,char*);
  }
  va_end(vl);
  return id;
}


EasyStr TGeneralInfo::dp4_disp(int val) {
  EasyStr ret;
  ret=(val%10000);
  while(ret.Length()<4)
    ret.Insert("0",0);
  ret.Insert(".",0);
  ret.Insert(EasyStr(val/10000),0);
  return ret;
}


void TGeneralInfo::CreateSpeedPage() {
  if(avg_frame_time && Glue.video_freq)
  {
    DWORD draw_time,unlock_time,blit_time,tt; //cpu_time;
    DWORD inst_per_second;
    CreateWindowEx(0,"STATIC",T("Timings per VBL (screen refresh)"),
      WS_CHILD|WS_VISIBLE,page_l,45,250,26,
      Handle,(HMENU)300,HInstance,NULL);
    INT_PTR id=DrawColumn(page_l,80,301,CStrT("Drawing time:"),CStrT("Unlocking time:"),CStrT("Blitting time:"),
      CStrT("Total draw time:"),"-",
      //     CStrT("CPU time:"),
      CStrT("Instructions per second:"),"-",
      CStrT("Total frame time:"),CStrT("% ST VBL rate"),"*");
    draw_end();
    tt=timeGetTime();
    for(int trial=0;trial<10;trial++)
    {
      draw_begin();
      draw_end();
    }
    unlock_time=timeGetTime()-tt;
    tt=timeGetTime();
    for(int trial=0;trial<12;trial++)
      draw(false);
    draw_time=timeGetTime()-unlock_time-tt;
    tt=timeGetTime();
    for(int trial=0;trial<12;trial++)
      draw_blit();
    blit_time=timeGetTime()-tt;
    unlock_time/=MAX(frameskip,1);
    draw_time/=MAX(frameskip,1);
    blit_time/=MAX(frameskip,1);
//    cpu_time=avg_frame_time-(draw_time+unlock_time+blit_time);
    inst_per_second=(((n_cpu_cycles_per_second/4)/Glue.video_freq)*12000)
      /avg_frame_time;
    EasyStr Secs=EasyStr(" ")+T("seconds");
    DrawColumn(page_l+page_w/2,80,id,(dp4_disp(draw_time*10/12)+Secs).Text,
      (dp4_disp(unlock_time*10/12)+Secs).Text,
      (dp4_disp(blit_time*10/12)+Secs).Text,
      (dp4_disp((draw_time+unlock_time+blit_time)*10/12)+Secs).Text,"-",
      //   (dp4_disp(cpu_time*10/12)+Secs).Text,
      EasyStr(inst_per_second).Text,"-",
      (dp4_disp(avg_frame_time*10/12)+Secs).Text,
      (dp4_disp((((10000*12000)/avg_frame_time)*100)/Glue.video_freq)).Text,"*");
    if(Focus==NULL) 
      Focus=PageTree;
    SetPageControlsFont();
    ShowPageControls();
  }
}

#endif//#if defined(DEBUG_BUILD)


void TGeneralInfo::UpdatePositions() {
  HWND Win=Handle;
  SetWindowPos(PageTree,NULL,0,0,page_l-10,page_h,SWP_NOZORDER);
  HWND hAbout=GetDlgItem(Win,200);
  if(hAbout)
  {
    int y=10;
    int TextHeight=GetTextSize(Font,"HyITljq").Height;
    int h=6;
    h*=TextHeight;
    SetWindowPos(hAbout,NULL,page_l,y,page_w,h,SWP_NOZORDER);
    HWND hscroller=GetDlgItem(Win,203);
    y+=h;
    SetWindowPos(hscroller,NULL,page_l,y,page_w,page_h-y-(TextHeight+20),
      SWP_NOZORDER);
    HWND hlink=GetDlgItem(Win,201);
    y=page_h-TextHeight-GuiSM.cy_frame()*2;
    SetWindowPos(hlink,NULL,page_l,y,page_w,TextHeight,SWP_NOZORDER);
    return;
  }
  HWND hTextContainer=GetDlgItem(Win,ID_INFOBOXTEXT);
  if(!hTextContainer)
    hTextContainer=GetDlgItem(Win,ID_INFOBOXRICHTEXT);
  if(!hTextContainer)
    return;
  int wid_of_search=get_text_width(T("Search"));
  int wid_of_find=get_text_width(T("Find"))+20;
  HWND hSearch=GetDlgItem(Win,503);
  HWND hEdit=GetDlgItem(Win,504);
  HWND hFind=GetDlgItem(Win,502);
  HWND hText=GetDlgItem(hTextContainer,IDC_INFOBOXTEXTCONTROL);
  BOOL ec;
  ec=SetWindowPos(hSearch,NULL,page_l,14,wid_of_search,23,SWP_NOZORDER);
  ec=SetWindowPos(hEdit,NULL,page_l+wid_of_search+5,10,
    page_w-wid_of_find-5-wid_of_search-5-8-5,23,SWP_NOZORDER);
  ec=SetWindowPos(hFind,NULL,page_l+page_w-wid_of_find-8-5,
    10,wid_of_find,23,SWP_NOZORDER);
  ec=SetWindowPos(hTextContainer,NULL,page_l,40,page_w,
    page_h-50,SWP_NOZORDER);
  ec=SetWindowPos(hText,NULL,0,0,page_w,page_h-50,SWP_NOZORDER);
}


#define GET_THIS This=(TGeneralInfo*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK TGeneralInfo::WndProc(HWND Win,UINT Mess,
                                       WPARAM wPar,LPARAM lPar) {
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  TGeneralInfo *This;
  switch(Mess) {
  case WM_COMMAND:
    GET_THIS;
    switch(LOWORD(wPar)) {
    case 504:
      if(HIWORD(wPar)==EN_CHANGE)
      {
        int SearchTextLen=(int)SendMessage(GetDlgItem(Win,504),WM_GETTEXTLENGTH,
          0,0);
        This->SearchText.SetLength(SearchTextLen);
        if(SearchTextLen)
          SendMessage(GetDlgItem(Win,504),WM_GETTEXT,SearchTextLen+1,
            LPARAM(This->SearchText.Text));
      }
      break;
    case IDOK:case 502:
      if(This->SearchText.Length())
      {
        HWND te=GetDlgItem(Win,ID_INFOBOXTEXT);
#if defined(SSE_GUI_RICHEDIT)
        BOOL IsRichEdit=(te==NULL);  //dodgy?
        if(IsRichEdit)
          te=GetDlgItem(Win,ID_INFOBOXRICHTEXT);
        te=GetDlgItem(te,IDC_INFOBOXTEXTCONTROL);
        if(IsRichEdit)
        {
          int tp=LOWORD(SendMessage(te,EM_GETSEL,0,0))+1; // +1 'find next'
          FINDTEXT ft;
          ft.chrg.cpMin=tp;
          ft.chrg.cpMax=-1;
          ft.lpstrText=This->SearchText.Text;
          tp=(int)SendMessage(te,EM_FINDTEXT,FR_DOWN,(LPARAM)&ft);
          if(tp>-1) // found
            SendMessage(te,EM_SETSEL,tp,tp+This->SearchText.Length());
          break;
        }
#endif
        int TextLen=(int)SendMessage(te,WM_GETTEXTLENGTH,0,0);
        char *Text=new char[TextLen+1];
        SendMessage(te,WM_GETTEXT,TextLen+1,LPARAM(Text));
        strupr(Text);
        EasyStr SText=This->SearchText.UpperCase();
        int tp=LOWORD(SendMessage(te,EM_GETSEL,0,0));
        int n;
        for(n=0;n<2;n++)
        {
          char *textpos=strstr(Text+tp+1,SText);
          if(textpos)
          {
            tp=(int)(textpos-Text);
            if(tp<TextLen)
            {
              SendMessage(te,EM_SETSEL,tp,tp+This->SearchText.Length());
              int first_line=(int)SendMessage(te,EM_GETFIRSTVISIBLELINE,0,0);
              int sel_line=(int)SendMessage(te,EM_LINEFROMCHAR,tp,0);
              SendMessage(te,EM_LINESCROLL,0,MAX(sel_line-5,0)-first_line);
              break;
            }
          }
          tp=-1;
        }
        if(n==2) 
          MessageBeep(0);
        delete[] Text;
      }
      break;
    }
    break;
  case WM_NOTIFY:
  {
    NMHDR *pnmh=(NMHDR*)lPar;
    if(wPar==60000)
    {
      GET_THIS;
      switch(pnmh->code) {
      case TVN_SELCHANGING:
      {
        NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
        if(Inf->action==4096) 
          return true; //DODGY!!!!!! Undocumented!!!!!
        return 0;
      }
      case TVN_SELCHANGED:
      {
        NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
        if(Inf->itemNew.hItem)
        {
          TV_ITEM tvi;
          tvi.mask=TVIF_PARAM;
          tvi.hItem=(HTREEITEM)Inf->itemNew.hItem;
          SendMessage(This->PageTree,TVM_GETITEM,0,(LPARAM)&tvi);
          bool Destroy=true;
          if(GetDlgItem(Win,ID_INFOBOXTEXT))
          { // On text page
            switch(tvi.lParam) {
            case INFOPAGE_README:
            case INFOPAGE_HOWTO_DISK:
            case INFOPAGE_HOWTO_CART:
            case INFOPAGE_FAQ:
            case INFOPAGE_UNIXREADME:
#if !defined(SSE_GUI_RICHEDIT2)
            case INFOPAGE_FAQ_SSE:
            case INFOPAGE_HINTS:
#endif
            case INFOPAGE_LICENCE:
            case INFOPAGE_TRACE:
              Destroy=false;
              break;
            }
          }
#if defined(SSE_GUI_RICHEDIT2)
          else if(GetDlgItem(Win,ID_INFOBOXRICHTEXT))
          { // On richtext page
            switch(tvi.lParam) {
            case INFOPAGE_README_SSE:
            case INFOPAGE_LINKS:
            case INFOPAGE_FAQ_SSE:
            case INFOPAGE_HINTS:
            case INFOPAGE_BUGS:
              Destroy=false;
              break;
            }
          }
#endif
          if(Destroy) 
            This->DestroyCurrentPage();
          This->Page=(int)tvi.lParam;
          This->CreatePage(This->Page);
        }
        break;
      }
      }
    }
    break;
  }
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
    return MAKELONG(502,DC_HASDEFID);
  case WM_SIZE:
    GET_THIS;
    if(This==NULL)
      return TRUE; //?
    This->page_w=LOWORD(lPar)-(This->page_l+GuiSM.cx_frame()*2);
    This->page_h=HIWORD(lPar)-(GuiSM.cy_frame()*2);
    This->UpdatePositions();
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


bool TGeneralInfo::HasHandledMessage(MSG *mess) {
  if(Handle)
  {
#if defined(SSE_GUI_KBD)
    if(mess->message==WM_KEYDOWN && mess->hwnd != StemWin)
    {
      switch(mess->wParam) {
      case VK_TAB:
        SetFocus(GetParent(GetParent(mess->hwnd)));
        break;
      case VK_ESCAPE: //infobox ESC to close, expected behaviour
        Hide();
        return TRUE;    
      case VK_RETURN: // simulate left click on return (for links)
        //TRACE("focus = %x mess handle %x infobox %x\n",GetFocus(),mess->hwnd,Handle);
        POINT xy;
        GetCaretPos(&xy);
        ClientToScreen(mess->hwnd,&xy);
        SetCursorPos(xy.x,xy.y);
        mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
        Sleep(20);
        mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
        break;
      }
    }
#endif
    return (IsDialogMessage(Handle,mess)!=0);
  }
  else
    return 0;
}

#endif//WIN32


#ifdef UNIX

int TGeneralInfo::WinProc(TGeneralInfo *This,Window Win,XEvent *Ev)
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
int TGeneralInfo::button_notifyproc(hxc_button *b,int mess,int*ip)
{
  if (b->id==500){
    TGeneralInfo *This=(TGeneralInfo*)(b->owner);  
    if (mess==BN_CLICKED) edit_notify_proc((hxc_edit*)hxc::find(This->gb.handle,501),EDN_RETURN,0);
    return 0;
  }
	return 0;
}
//---------------------------------------------------------------------------
int TGeneralInfo::listview_notify_proc(hxc_listview *LV,int Mess,int I)
{
  TGeneralInfo *This=(TGeneralInfo*)(LV->owner);
  if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
    int NewPage=LV->sl[LV->sel].Data[1];
    if (This->Page!=NewPage){
      hxc::destroy_children_of(This->gb.handle);
      This->Page=NewPage;
      This->CreatePage(This->Page);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TGeneralInfo::edit_notify_proc(hxc_edit *ed,int mess,int i)
{
  TGeneralInfo *This=(TGeneralInfo*)(ed->owner);
  if (mess==EDN_CHANGE){
    WriteCSFStr("Info","SearchText",ed->text,globalINIFile);
    This->last_find_idx=0;
  }else if (mess==EDN_RETURN){
    if (ed->text.Empty()) return 0;
    
    Str t=This->readme.text.Text;
    strupr(t);
    for (int n=0;n<2;n++){
      char *s=strstr(t.Text + This->last_find_idx,ed->text.UpperCase());
      if (s){
        int line=This->readme.get_line_from_character_index(DWORD(s-t.Text));
        This->readme.highlight_lines.DeleteAll();
        This->readme.highlight_lines.Add(line);
        This->readme.scrollto(MAX(line-3,0)*This->readme.textheight);
        This->readme.draw(true);

        This->last_find_idx=int(s-t.Text)+1;
        if (line<This->readme.n_lines-1){
          while (This->readme.get_line_from_character_index(This->last_find_idx)==line) This->last_find_idx++;
        }       
        break;
      }else{
        if (This->last_find_idx==0) break;
        This->last_find_idx=0;
      }
    }
  }
  return 0;
}

#endif//UNIX
