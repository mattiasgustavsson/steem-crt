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
FILE: diskman_diags.cpp
DESCRIPTION: The dialogs that can be shown by the Disk Manager.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <diskman.h>
#include <computer.h>
#include <translate.h>
#include <display.h>
#include <mymisc.h>
#include <gui.h>
#include <archive.h>
#include <choosefolder.h>
#include <di_get_contents.h>


void TDiskManager::ShowDatabaseDiag() {
  if(GetContentsCheckExist()==0) 
    return;

#ifdef WIN32
  if(DatabaseDiag!=NULL) 
    return;  
  int th=GetTextSize(Font,T("To download disks see Steem's ")).Height;
  DatabaseDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Search Disk Image Database"),WS_CAPTION|WS_SYSMENU,100,100,506,
    10+30+300+10+th+10+GuiSM.cy_caption()+6,Handle,NULL,HInstance,
    NULL);
  if(DatabaseDiag==NULL||IsWindow(DatabaseDiag)==0)
    return;
  //EnableWindow(Handle,0);
  SetWindowLongPtr(DatabaseDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(DatabaseDiag,StemWin);
  int y=10,w,page_r=500-10;
  HWND Win;
  w=GetTextSize(Font,T("Search for")).Width;
  CreateWindow("Static",T("Search for"),WS_CHILD|WS_VISIBLE,
    10,y+4,w,23,DatabaseDiag,(HMENU)102,HInstance,NULL);
  CreateWindowEx(512,"Edit","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,
    w+15,y,page_r-50-(w+15),23,DatabaseDiag,(HMENU)103,HInstance,NULL);
  SendDlgItemMessage(DatabaseDiag,103,WM_SETTEXT,0,(LPARAM)(DatabaseFind.Text));
  CreateWindow("Button",T("Go"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
    page_r-45,y,45,23,DatabaseDiag,(HMENU)IDOK,HInstance,NULL);
  y+=30;
  Win=CreateWindowEx(512,WC_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|
    LVS_SINGLESEL|LVS_REPORT,
    10,y,page_r-10,300,DatabaseDiag,(HMENU)111,HInstance,NULL);
  RECT rc;
  GetClientRect(Win,&rc);
  LV_COLUMN lvc;
  lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
  lvc.fmt=LVCFMT_LEFT;
  lvc.cx=180;
  lvc.pszText=StaticT("Name");
  lvc.iSubItem=0;
  SendMessage(Win,LVM_INSERTCOLUMN,0,LPARAM(&lvc));
  lvc.fmt=LVCFMT_LEFT;
  lvc.cx=300;
  lvc.pszText=StaticT("Contents");
  lvc.iSubItem=1;
  SendMessage(Win,LVM_INSERTCOLUMN,1,LPARAM(&lvc));
  y+=310;
  w=GetTextSize(Font,T("To download disks see Steem's ")).Width;
  CreateWindow("Static",T("To download disks see Steem's "),WS_CHILD|WS_VISIBLE,
    10,y,w,th,DatabaseDiag,(HMENU)300,HInstance,NULL);
  CreateWindowEx(0,"Steem HyperLink",T("links page")+"|"+STEEM_WEB_LEGACY+"links.htm",
    WS_CHILD|WS_VISIBLE,10+w,y,200,th,DatabaseDiag,(HMENU)301,HInstance,NULL);
  SetWindowAndChildrensFont(DatabaseDiag,Font);
  CentreWindow(DatabaseDiag,0);
  DiagFocus=GetDlgItem(DatabaseDiag,103);
  ShowWindow(DatabaseDiag,SW_SHOW);
#endif

#ifdef UNIX
  int w=600,h=10+30+300+10+hxc::font->ascent+hxc::font->descent+10,y=10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Search Disk Image Database"),0);
  if (handle==0) return;

  hxc_button *p_but,*p_but2;
  hxc_edit *p_ed;
  hxc_textdisplay result_td;

  p_but=new hxc_button(XD,handle,10,y,0,25,NULL,this,BT_LABEL,T("Search for"),0,hxc::col_bk);

  p_but2=new hxc_button(XD,handle,w-10,y,0,25,diag_but_np,this,BT_TEXT,T("Go"),100,hxc::col_bk);
  p_but2->x-=p_but2->w;
  XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

  p_ed=new hxc_edit(XD,handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-5,25,diag_ed_np,this);
  p_ed->set_text("");
  p_ed->id=101;
  y+=30;

  result_td.id=200;
  result_td.border=1;
  result_td.pad_x=5;
  result_td.sy=0;
  result_td.textheight=(hxc::font->ascent)+(hxc::font->descent)+2;
  result_td.create(XD,handle,10,y,w-20,300,hxc::col_white,true);
  y+=310;

  p_but=new hxc_button(XD,handle,10,y,0,0,NULL,this,BT_LABEL,
          T("To download disks see Steem's "),0,hxc::col_bk);

  new hxc_button(XD,handle,10+p_but->w,y,0,0,hyperlink_np,this,BT_LINK|BT_TEXT,
          T("links page")+"|"+STEEM_WEB+"links.htm",0,hxc::col_bk);

  hxc::show_modal_dialog(XD,handle,true,p_ed->handle);
  hxc::destroy_modal_dialog(XD,handle);
#endif//UNIX
}


void TDiskManager::ShowContentDiag() {

#ifdef WIN32
  int h=GetTextSize(Font,T("Contents")).Height;
  ContentDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Disk Image Contents"),WS_CAPTION|WS_SYSMENU,100,100,406,10+30+30+h+2
    +160+110+10+GuiSM.cy_caption()+6,Handle,NULL,HInstance,NULL);
  if(ContentDiag==NULL||IsWindow(ContentDiag)==0)
    return;
  EnableWindow(Handle,0);
  SetWindowLongPtr(ContentDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(ContentDiag,StemWin);
  int y=10,w,page_r=400-10;
  HWND Win;
  w=GetTextSize(Font,T("Disk path")).Width;
  CreateWindow("Static",T("Disk path"),WS_CHILD|WS_VISIBLE,
    10,y+4,w,23,ContentDiag,(HMENU)102,HInstance,NULL);
  CreateWindowEx(512,"Edit",contents_sl[0].String,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_READONLY,
    w+15,y,page_r-(w+15),23,ContentDiag,(HMENU)103,HInstance,NULL);
  y+=30;
  w=GetTextSize(Font,T("Short TOSEC name")).Width;
  CreateWindow("Static",T("Short TOSEC name"),WS_CHILD|WS_VISIBLE,
    10,y+4,w,23,ContentDiag,(HMENU)100,HInstance,NULL);
  CreateWindowEx(512,"Edit",contents_sl[1].String,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_READONLY,
    w+15,y,page_r-(w+15),23,ContentDiag,(HMENU)101,HInstance,NULL);
  y+=30;
  CreateWindow("Static",T("Contents"),WS_CHILD|WS_VISIBLE,
    10,y,page_r-10,h+1,ContentDiag,(HMENU)110,HInstance,NULL);
  y+=h+2;
  Win=CreateWindowEx(512,WC_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|
    LVS_SINGLESEL|LVS_REPORT|LVS_NOCOLUMNHEADER,
    10,y,page_r-10,150,ContentDiag,(HMENU)111,HInstance,NULL);
  ListView_SetExtendedListViewStyle(Win,LVS_EX_CHECKBOXES);
  RECT rc;
  GetClientRect(Win,&rc);
  LV_COLUMN lvc;
  lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
  lvc.fmt=LVCFMT_LEFT;
  lvc.cx=rc.right-GuiSM.cx_vscroll();
  lvc.pszText="";
  lvc.iSubItem=0;
  SendMessage(Win,LVM_INSERTCOLUMN,0,LPARAM(&lvc));
  LV_ITEM lvi;
  lvi.mask=LVIF_TEXT|LVIF_PARAM;
  for(int i=2;i<contents_sl.NumStrings;i++)
  {
    lvi.iSubItem=0;
    lvi.pszText=contents_sl[i].String;
    lvi.lParam=i;
    lvi.iItem=i-2;
    SendMessage(Win,LVM_INSERTITEM,0,(LPARAM)&lvi);

    ListView_SetItemState(Win,i-2,LVI_SI_CHECKED,LVIS_STATEIMAGEMASK);
  }
  y+=160;
  int Disable=0;
  if(contents_sl.NumStrings<=2) 
    Disable=WS_DISABLED;
  CreateWindow("Button",T("Create Shortcuts To Selected Contents"),WS_CHILD|WS_VISIBLE|BS_GROUPBOX|Disable,
    10,y,page_r-10,110,ContentDiag,(HMENU)200,HInstance,NULL);
  y+=20;
  w=GetTextSize(Font,T("In folder")).Width;
  CreateWindow("Static",T("In folder"),WS_CHILD|WS_VISIBLE|Disable,
    20,y+4,w,23,ContentDiag,(HMENU)200,HInstance,NULL);
  Win=CreateWindowEx(512,"Edit",ContentsLinksPath,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|Disable,
    25+w,y,page_r-80-5-(w+25),23,ContentDiag,(HMENU)201,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);
  CreateWindow("Button",T("Browse"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE|Disable,page_r-80,y,70,23,ContentDiag,(HMENU)202,HInstance,NULL);
  y+=30;
  w=GetCheckBoxSize(Font,T("Append disk name")).Width;
  Win=CreateWindow("Button",T("Append disk name"),WS_CHILD|WS_VISIBLE|Disable|
    BS_AUTOCHECKBOX,20,y,w,23,ContentDiag,(HMENU)220,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,BST_CHECKED,0);
  Str ShortName=GetContentsGetAppendName(contents_sl[1].String);
  Win=CreateWindowEx(512,"Edit",ShortName,WS_CHILD|WS_VISIBLE|WS_TABSTOP
    |ES_AUTOHSCROLL|Disable,25+w,y,page_r-10-(w+25),23,ContentDiag,(HMENU)221,
    HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,50,0);
  y+=30;
  w=GetTextSize(Font,T("On name conflict")).Width;
  CreateWindow("Static",T("On name conflict"),WS_CHILD|WS_VISIBLE|Disable,
    20,y+4,w,23,ContentDiag,(HMENU)210,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|Disable,
    25+w,y,page_r-100-(25+w)-10,200,ContentDiag,(HMENU)211,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Skip"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Overwrite"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Rename new"));
  SendMessage(Win,CB_SETCURSEL,ContentConflictAction,0);
  CreateWindow("Button",T("Create"),WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON|WS_TABSTOP|Disable,
    page_r-100,y,90,23,ContentDiag,(HMENU)IDOK,HInstance,NULL);
  SetWindowAndChildrensFont(ContentDiag,Font);
  CentreWindow(ContentDiag,0);
  DiagFocus=GetDlgItem(ContentDiag,101);
  ShowWindow(ContentDiag,SW_SHOW);
#endif//WIN32

#ifdef UNIX
  if (contents_sl.NumStrings<2) return;

  int w=500,h=10+30+20+185+30+30+30+30+10,y=10;
  if (contents_sl.NumStrings==2) h=10+30+10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Disk Image Contents"),0);
  if (handle==0) return;

  hxc_button *p_but,*p_but2,*p_group,*p_append_but=NULL;
  hxc_edit *p_path_ed=NULL,*p_append_ed=NULL;
  hxc_dropdown *p_conflict_dd=NULL;
  hxc_listview cont_lv;
  Window focus=0;

  new hxc_button(XD,handle,10,y,0,25,NULL,this,BT_LABEL,T("Short TOSEC name")+" - "+contents_sl[1].String,0,hxc::col_bk);
  y+=30;

  if (contents_sl.NumStrings>2){
    p_but=new hxc_button(XD,handle,10,y,0,0,NULL,this,BT_LABEL,T("Contents"),0,hxc::col_bk);
    y+=p_but->h+2;

    cont_lv.lpig=&Ico16;
    cont_lv.display_mode=1;
    cont_lv.checkbox_mode=true;
    cont_lv.sl.DeleteAll();
    cont_lv.sl.Sort=eslNoSort;
    for (int i=2;i<contents_sl.NumStrings;i++){
      cont_lv.additem(contents_sl[i].String,101+ICO16_TICKED);
    }
    cont_lv.create(XD,handle,10,y,w-20,180,diag_lv_np,this);
    y+=185;

    p_group=new hxc_button(XD,handle,10,y,w-20,120,NULL,this,BT_GROUPBOX,T("Create Links To Selected Contents"),0,hxc::col_bk);

    w=p_group->w;
    y=25;

    p_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_LABEL,T("In folder"),0,hxc::col_bk);

    p_but2=new hxc_button(XD,p_group->handle,w-10,y,0,25,diag_but_np,this,BT_TEXT,T("Browse"),200,hxc::col_bk);
    p_but2->x-=p_but2->w;
    XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

    p_path_ed=new hxc_edit(XD,p_group->handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-5,25,NULL,this);
    p_path_ed->set_text(ContentsLinksPath);
    p_path_ed->id=201;
    y+=30;
    focus=p_path_ed->handle;

    p_append_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_CHECKBOX,T("Append disk name"),101,hxc::col_bk);
    p_append_but->set_check(true);

    p_append_ed=new hxc_edit(XD,p_group->handle,10+p_append_but->w+5,y,w-10-10-p_append_but->w-5,25,NULL,this);
    p_append_ed->set_text(GetContentsGetAppendName(contents_sl[1].String));
    y+=30;

    p_but=new hxc_button(XD,p_group->handle,10,y,0,25,NULL,this,BT_LABEL,T("On name conflict"),0,hxc::col_bk);

    p_but2=new hxc_button(XD,p_group->handle,w-10,y,0,25,hxc::modal_but_np,this,BT_TEXT,T("Create"),1,hxc::col_bk);
    p_but2->x-=p_but2->w;
    XMoveWindow(XD,p_but2->handle,p_but2->x,p_but2->y);

    p_conflict_dd=new hxc_dropdown(XD,p_group->handle,10+p_but->w+5,y,w-10-10-p_but->w-5-p_but2->w-10,200,NULL,this);
    p_conflict_dd->additem(T("Skip"));
    p_conflict_dd->additem(T("Overwrite"));
    p_conflict_dd->additem(T("Rename new"));
    p_conflict_dd->changesel(ContentConflictAction);
  }

  Str DestFol="///",DiskName,NewLink;
  for(;;){
    int id=hxc::show_modal_dialog(XD,handle,true,focus);

    if (contents_sl.NumStrings<=2) break;

    ContentConflictAction=p_conflict_dd->sel;
    if (id==1){ // Create links
      DestFol=p_path_ed->text;
      NO_SLASH(DestFol);
      if (p_append_but->checked) DiskName=Str(" (")+p_append_ed->text+")";
      CreateDirectory(DestFol,NULL);
      if (GetFileAttributes(DestFol)==0xffffffff){
        Alert(T("Invalid directory"),T("Error"),MB_ICONEXCLAMATION);
      }else{
        char *ext;
        ext=strrchr(contents_sl[0].String,'.');
        if (ext==NULL) ext="";
        for (int i=2;i<contents_sl.NumStrings;i++){
          if (cont_lv.sl[i].Data[0]==101+ICO16_TICKED){
            NewLink=DestFol+SLASH+Str(contents_sl[i].String)+DiskName+ext;
            if (Exists(NewLink)){
              if (ContentConflictAction==0){
                NewLink="";
              }else if (ContentConflictAction==2){
                NewLink=GetUniquePath(DestFol,Str(contents_sl[i].String)+DiskName+ext);
              }
            }
            if (NewLink.NotEmpty()) symlink(contents_sl[0].String,NewLink);
          }
        }
        break;
      }
    }else{
      break;
    }
  }
  hxc::destroy_modal_dialog(XD,handle);

  if (IsSameStr_I(DestFol,DisksFol)){
    set_path(DisksFol);
    dir_lv.select_item_by_name(GetFileNameFromPath(NewLink));
  }
#endif//UNIX
}


#ifdef WIN32

void TDiskManager::ShowDiskDiag() {
#if defined(SSE_DISK_CREATE_MSA_DIM) // modifying the GUI is the hardest part
  DiskDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Create Custom Disk Image"),WS_CAPTION|WS_SYSMENU,100,100,256,171+30
    +GuiSM.cy_caption(),
    Handle,NULL,HInstance,NULL);
  if(DiskDiag==NULL||IsWindow(DiskDiag)==0)
    return;
  EnableWindow(Handle,0);
  SetWindowLongPtr(DiskDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(DiskDiag,StemWin);
  int y=14,x=10,HorizontalSeparation=5;
  // radio buttons for image type, sticky but not persistent
  int mask=WS_CHILD|BS_AUTORADIOBUTTON|WS_VISIBLE; 
  int Wid=GetCheckBoxSize(Font,T(extension_list[EXT_ST])).Width;
  HWND Win=CreateWindow("Button",T(extension_list[EXT_ST]),mask|WS_GROUP,x,
    y,Wid,25,DiskDiag,(HMENU)7341,HInstance,NULL);
  int Offset=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T(extension_list[EXT_MSA])).Width;
  Win=CreateWindow("Button",T(extension_list[EXT_MSA]),mask,x+Offset,
   y,Wid,25,DiskDiag,(HMENU)7342,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T(extension_list[EXT_DIM])).Width;
  Win=CreateWindow("Button",T(extension_list[EXT_DIM]),mask,x+Offset,
   y,Wid,25,DiskDiag,(HMENU)7343,HInstance,NULL);
  SendMessage(GetDlgItem(DiskDiag,7341+SSEConfig.DiskImageCreated-1),
    BM_SETCHECK,TRUE,0);
  y+=30;
  Wid=get_text_width(T("Sides"));
  CreateWindow("Static",T("Sides"),WS_CHILD|WS_VISIBLE,
    10,y,Wid,23,DiskDiag,(HMENU)100,HInstance,NULL);
  // updown controls for sides/tracks/sectors, we also change min values (T40, S6)
  HWND hEdit0=CreateWindow("Edit",NULL,WS_CHILD|WS_TABSTOP|WS_BORDER|WS_VISIBLE,
      150,y-2,40,21,DiskDiag,(HMENU)101,HInstance,NULL);
  Win=CreateWindow(UPDOWN_CLASS,NULL,
    WS_CHILD|WS_TABSTOP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_VISIBLE,
    0,0,0,0,DiskDiag,(HMENU)101,HInstance,NULL);
  SendMessage(Win,UDM_SETBUDDY,(WPARAM)hEdit0,0);
  SendMessageW(Win,UDM_SETRANGE,0,MAKELPARAM(2,1));
  SendMessageW(Win,UDM_SETPOS32,0,SidesIdx+1);
  y+=30;
  Wid=get_text_width(T("Tracks"));
  CreateWindow("Static",T("Tracks"),WS_CHILD|WS_VISIBLE,
    10,y,Wid,23,DiskDiag,(HMENU)104,HInstance,NULL);
  HWND hEdit1=CreateWindow("Edit",NULL,WS_CHILD|WS_TABSTOP|WS_BORDER|WS_VISIBLE,
      150,y-2,40,21,DiskDiag,(HMENU)105,HInstance,NULL);
  Win=CreateWindow(UPDOWN_CLASS,NULL,
    WS_CHILD|WS_TABSTOP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_VISIBLE,
    0,0,0,0,DiskDiag,(HMENU)105,HInstance,NULL);
  SendMessage(Win,UDM_SETBUDDY,(WPARAM)hEdit1,0);
  SendMessageW(Win,UDM_SETRANGE,0,MAKELPARAM(FLOPPY_MAX_TRACK_NUM,40));
  SendMessageW(Win,UDM_SETPOS32,0,TracksIdx);
  y+=30;
  Wid=get_text_width(T("Sectors"));
  CreateWindow("Static",T("Sectors"),WS_CHILD|WS_VISIBLE,
    10,y,Wid,23,DiskDiag,(HMENU)102,HInstance,NULL);
  HWND hEdit2=CreateWindow("Edit",NULL,WS_CHILD|WS_TABSTOP|WS_BORDER|WS_VISIBLE,
      150,y-2,40,21,DiskDiag,(HMENU)103,HInstance,NULL);
  Win=CreateWindow(UPDOWN_CLASS,NULL,
    WS_CHILD|WS_TABSTOP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_VISIBLE,
    0,0,0,0,DiskDiag,(HMENU)103,HInstance,NULL);
  SendMessage(Win,UDM_SETBUDDY,(WPARAM)hEdit2,0);
  SendMessageW(Win,UDM_SETRANGE,0,MAKELPARAM(FLOPPY_MAX_SECTOR_NUM,6));
  SendMessageW(Win,UDM_SETPOS32,0,SecsPerTrackIdx);
  int databytes=GetDiskSelectionSize();
  y+=30-4;
  int kb=databytes/1024;
  CreateWindow("Static",T("Disk size")+": "+kb+T(" KB"),WS_CHILD|WS_VISIBLE,
    10,y,230,23,DiskDiag,(HMENU)106,HInstance,NULL);
  y+=30;
  CreateWindow("Button",T("OK"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
    70,y,80,23,DiskDiag,(HMENU)IDOK,HInstance,NULL);
  CreateWindow("Button",T("Cancel"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    160,y,80,23,DiskDiag,(HMENU)IDCANCEL,HInstance,NULL);
  SetWindowAndChildrensFont(DiskDiag,Font);
  CentreWindow(DiskDiag,0);
  DiagFocus=GetDlgItem(DiskDiag,101);
  ShowWindow(DiskDiag,SW_SHOW);
#else
  DiskDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Create Custom Disk Image"),WS_CAPTION|WS_SYSMENU,100,100,256,171
    +GuiSM.cy_caption(),
    Handle,NULL,HInstance,NULL);
  if(DiskDiag==NULL||IsWindow(DiskDiag)==0)
    return;
  EnableWindow(Handle,0);
  SetWindowLongPtr(DiskDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(DiskDiag,StemWin);
  int Wid=get_text_width(T("Sides"));
  CreateWindow("Static",T("Sides"),WS_CHILD|WS_VISIBLE,
    10,14,Wid,23,DiskDiag,(HMENU)100,HInstance,NULL);
  HWND Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP
    |CBS_DROPDOWNLIST,150,10,90,200,DiskDiag,(HMENU)101,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"1");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"2");
  SendMessage(Win,CB_SETCURSEL,SidesIdx,0);
  Wid=get_text_width(T("Tracks"));
  CreateWindow("Static",T("Tracks"),WS_CHILD|WS_VISIBLE,
    10,44,Wid,23,DiskDiag,(HMENU)104,HInstance,NULL);
  Wid=get_text_width(T("0 to "));
  CreateWindow("Static",T("0 to "),WS_CHILD|WS_VISIBLE,
    150-Wid,44,Wid,23,DiskDiag,(HMENU)99,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|WS_VSCROLL,
    150,40,90,300,DiskDiag,(HMENU)105,HInstance,NULL);
  int n;
  for(n=75;n<=FLOPPY_MAX_TRACK_NUM;n++)
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)EasyStr(n).Text);
  SendMessage(Win,CB_SETCURSEL,MIN(int(TracksIdx),(FLOPPY_MAX_TRACK_NUM+1)-75),0);
  Wid=get_text_width(T("Sectors"));
  CreateWindow("Static",T("Sectors"),WS_CHILD|WS_VISIBLE,
    10,74,Wid,23,DiskDiag,(HMENU)102,HInstance,NULL);
  Wid=get_text_width(T("1 to "));
  CreateWindow("Static",T("1 to "),WS_CHILD|WS_VISIBLE,
    150-Wid,74,Wid,23,DiskDiag,(HMENU)98,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|WS_VSCROLL,
    150,70,90,300,DiskDiag,(HMENU)103,HInstance,NULL);
  for(n=8;n<=FLOPPY_MAX_SECTOR_NUM;n++)
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)EasyStr(n).Text);
  SendMessage(Win,CB_SETCURSEL,SecsPerTrackIdx,0);
  EasyStr SizeBytes=GetDiskSelectionSize();
  if(SizeBytes.Length()>6)
  {
    SizeBytes.Insert(",",1);
    SizeBytes.Insert(",",5);
  }
  else
    SizeBytes.Insert(",",3);
  CreateWindow("Static",T("Disk size")+": "+SizeBytes+" "+T("bytes"),WS_CHILD|WS_VISIBLE,
    10,100,230,23,DiskDiag,(HMENU)106,HInstance,NULL);
  CreateWindow("Button",T("OK"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
    70,130,80,23,DiskDiag,(HMENU)IDOK,HInstance,NULL);
  CreateWindow("Button",T("Cancel"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    160,130,80,23,DiskDiag,(HMENU)IDCANCEL,HInstance,NULL);
  SetWindowAndChildrensFont(DiskDiag,Font);
  CentreWindow(DiskDiag,0);
  DiagFocus=GetDlgItem(DiskDiag,101);
  ShowWindow(DiskDiag,SW_SHOW);
#endif
}


void TDiskManager::ShowLinksDiag() {
  LinksDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Create Multiple Shortcuts"),WS_CAPTION,100,100,406,376
    +GuiSM.cy_caption(),Handle,NULL,HInstance,NULL);
  if(LinksDiag==NULL||IsWindow(LinksDiag)==0)
    return;
  EnableWindow(Handle,0);
  SetWindowLongPtr(LinksDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(LinksDiag,StemWin);
  LONG Wid=GetTextSize(Font,T("Create shortcuts to")).Width;
  CreateWindow("Static",T("Create shortcuts to"),WS_CHILD|WS_VISIBLE,
    10,14,Wid,23,LinksDiag,(HMENU)100,HInstance,NULL);
  HWND Win=CreateWindowEx(512,"Edit",LinksTargetPath,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,
    Wid+15,10,300-(Wid+5),23,LinksDiag,(HMENU)101,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);
  CreateWindow("Button",T("Browse"),WS_CHILD|WS_VISIBLE|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,315,10,75,23,LinksDiag,(HMENU)102,HInstance,NULL);
  Wid=GetTextSize(Font,T("In folder")).Width;
  CreateWindow("Static",T("In folder"),WS_CHILD|WS_VISIBLE,
    10,44,Wid,23,LinksDiag,(HMENU)200,HInstance,NULL);
  if(MultipleLinksPath.IsEmpty()) 
    MultipleLinksPath=HomeFol;
  Win=CreateWindowEx(512,"Edit",MultipleLinksPath,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,
    Wid+15,40,300-(Wid+5),23,LinksDiag,(HMENU)201,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);
  CreateWindow("Button",T("Browse"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE,315,40,75,23,LinksDiag,(HMENU)202,HInstance,NULL);
  EasyStr TargetName=GetFileNameFromPath(LinksTargetPath);
  char *dot=strrchr(TargetName,'.');
  if(dot) 
    *dot=0;
  for(int n=0;n<9;n++)
  {
    Wid=GetTextSize(Font,EasyStr("#")+(n+1)).Width;
    CreateWindow("Static",EasyStr("#")+(n+1),WS_CHILD|WS_VISIBLE,
      10,74+n*30,Wid,23,LinksDiag,HMENU(300+n*100),HInstance,NULL);
    Win=CreateWindowEx(512,"Edit",LPSTR((n==0)?TargetName.Text:""),
      WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,
      Wid+15,70+n*30,380-(Wid+5),23,LinksDiag,HMENU(301+n*100),HInstance,NULL);
    SendMessage(Win,EM_LIMITTEXT,100,0);
  }
  CreateWindow("Button",T("OK"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
    200,340,90,23,LinksDiag,(HMENU)IDOK,HInstance,NULL);
  CreateWindow("Button",T("Cancel"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    300,340,90,23,LinksDiag,(HMENU)IDCANCEL,HInstance,NULL);
  SetWindowAndChildrensFont(LinksDiag,Font);
  Wid=(LONG)SendMessage(GetDlgItem(LinksDiag,101),WM_GETTEXTLENGTH,0,0);
  SendMessage(GetDlgItem(LinksDiag,101),EM_SETSEL,Wid,Wid);
  SendMessage(GetDlgItem(LinksDiag,101),EM_SCROLLCARET,0,0);
  Wid=(LONG)SendMessage(GetDlgItem(LinksDiag,201),WM_GETTEXTLENGTH,0,0);
  SendMessage(GetDlgItem(LinksDiag,201),EM_SETSEL,Wid,Wid);
  SendMessage(GetDlgItem(LinksDiag,201),EM_SCROLLCARET,0,0);
  DiagFocus=GetDlgItem(LinksDiag,301);
  SendMessage(DiagFocus,EM_SETSEL,0,-1);
  SendMessage(DiagFocus,EM_SCROLLCARET,0,0);
  CentreWindow(LinksDiag,0);
  ShowWindow(LinksDiag,SW_SHOW);
}


#ifndef SSE_NO_WINSTON_IMPORT

void TDiskManager::ShowImportDiag()
{
  ImportDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",T("Import WinSTon Favourites"),WS_CAPTION,
                           100,100,406,196+GuiSM.cy_caption(),
                           Handle,NULL,HInstance,NULL);
  if (ImportDiag==NULL || IsWindow(ImportDiag)==0){
    return;
  }
  EnableWindow(Handle,0);

  SetWindowLongPtr(ImportDiag,GWLP_USERDATA,(LONG_PTR)this);

  if (FullScreen) SetParent(ImportDiag,StemWin);

  long Wid;
  HWND Win;

  Wid=GetTextSize(Font,T("WinSTon folder")).Width;
  CreateWindow("Static",T("WinSTon folder"),WS_CHILD | WS_VISIBLE,
                          10,14,Wid,23,ImportDiag,(HMENU)100,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit",WinSTonPath,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                Wid+15,10,300-(Wid+5),23,ImportDiag,(HMENU)101,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);

  CreateWindow("Button",T("Browse"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                    315,10,75,23,ImportDiag,(HMENU)102,HInstance,NULL);


  Wid=GetTextSize(Font,T("WinSTon discs folder")).Width;
  CreateWindow("Static",T("WinSTon discs folder"),WS_CHILD | WS_VISIBLE,
                          10,44,Wid,23,ImportDiag,(HMENU)150,HInstance,NULL);

  Win=CreateWindowEx(512,"Edit",WinSTonDiskPath,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                Wid+15,40,300-(Wid+5),23,ImportDiag,(HMENU)151,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);

  CreateWindow("Button",T("Browse"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                    315,40,75,23,ImportDiag,(HMENU)152,HInstance,NULL);


  Wid=GetTextSize(Font,T("Import to")).Width;
  CreateWindow("Static",T("Import to"),WS_CHILD | WS_VISIBLE,
                          10,74,Wid,23,ImportDiag,(HMENU)200,HInstance,NULL);

  if (ImportPath.IsEmpty()) ImportPath=HomeFol+"\\"+T("Favourites");
  Win=CreateWindowEx(512,"Edit",ImportPath,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                Wid+15,70,300-(Wid+5),23,ImportDiag,(HMENU)201,HInstance,NULL);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);

  CreateWindow("Button",T("Browse"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CHECKBOX | BS_PUSHLIKE,
                    315,70,75,23,ImportDiag,(HMENU)202,HInstance,NULL);

  Wid=GetCheckBoxSize(Font,T("Only downloaded disks")).Width;  //If off create broken links!
  Win=CreateWindow("Button",T("Only downloaded disks"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                          10,100,Wid,23,ImportDiag,(HMENU)300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,ImportOnlyIfExist,0);

  Wid=GetTextSize(Font,T("On name conflict")).Width;
  CreateWindow("Static",T("On name conflict"),WS_CHILD | WS_VISIBLE,
                          10,134,Wid,23,ImportDiag,(HMENU)301,HInstance,NULL);

  Win=CreateWindow("Combobox","",WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                          15+Wid,130,200,200,ImportDiag,(HMENU)302,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Skip"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Overwrite"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Rename new"));
  SendMessage(Win,CB_ADDSTRING,0,(long)CStrT("Rename existing"));
  SendMessage(Win,CB_SETCURSEL,ImportConflictAction,0);


  CreateWindow("Button",T("OK"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                    200,160,90,23,ImportDiag,(HMENU)IDOK,HInstance,NULL);

  CreateWindow("Button",T("Cancel"),WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    300,160,90,23,ImportDiag,(HMENU)IDCANCEL,HInstance,NULL);

  CreateWindow(PROGRESS_CLASS,"",WS_CHILD | PBS_SMOOTH,10,160,280,23,ImportDiag,(HMENU)400,Inst,NULL);


  SetWindowAndChildrensFont(ImportDiag,Font);

  Wid=SendMessage(GetDlgItem(ImportDiag,101),WM_GETTEXTLENGTH,0,0);
  SendMessage(GetDlgItem(ImportDiag,101),EM_SETSEL,Wid,Wid);
  SendMessage(GetDlgItem(ImportDiag,101),EM_SCROLLCARET,0,0);

  Wid=SendMessage(GetDlgItem(ImportDiag,201),WM_GETTEXTLENGTH,0,0);
  SendMessage(GetDlgItem(ImportDiag,201),EM_SETSEL,Wid,Wid);
  SendMessage(GetDlgItem(ImportDiag,201),EM_SCROLLCARET,0,0);

  CentreWindow(ImportDiag,0);
  DiagFocus=GetDlgItem(ImportDiag,IDOK);
  ShowWindow(ImportDiag,SW_SHOW);
}

#endif


void TDiskManager::ShowPropDiag() {
#if USE_PASTI
  if(hPasti)
  {
    // sl will contain all pasti disks in the archive (if PropInf.Path is an
    //archive)
    EasyStringList sl(eslNoSort);
    if(FileIsDisk(PropInf.Path)==DISK_COMPRESSED)
    {
      // disks_sl will contain all disks (pasti and non-pasti) in the archive
      EasyStringList disks_sl(eslNoSort);
      zippy.list_contents(PropInf.Path,&disks_sl,true);
      for(int i=0;i<disks_sl.NumStrings;i++)
      {
        if(FileIsDisk(disks_sl[i].String)==DISK_PASTI)
        {
          // have to pass correct name to pasti
          Str temp_out=WriteDir+SLASH+GetFileNameFromPath(disks_sl[i].String);
          sl.Add(temp_out);
          zippy.extract_file(PropInf.Path,(int)disks_sl[i].Data[0],temp_out,true,0);
        }
      }
    }
    if(sl.NumStrings||FileIsDisk(PropInf.Path)==DISK_PASTI)
    {
      // pass null-term list, disks first, followed by archive 
      char buf[8192],*p;
      ZeroMemory(buf,sizeof(buf));
      p=buf;
      for(int i=0;i<sl.NumStrings;i++)
      {
        strcpy(p,sl[i].String);
        p+=strlen(p)+1;
      }
      strcpy(p,PropInf.Path);
      pasti->DlgFileProps(Handle,buf);
      // clean up unwanted files
      for(int i=0;i<sl.NumStrings;i++) 
        DeleteFile(sl[i].String);
      return;
    }
  }
  else
  {
    if(has_extension(PropInf.Path,dot_ext(EXT_STX))) 
      return;
  }
#endif
  PropDiag=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Dialog",
    T("Disk Properties"),WS_CAPTION|WS_SYSMENU,100,100,100,199,Handle,NULL,
    HInstance,NULL);
  if(PropDiag==NULL||IsWindow(PropDiag)==0)
    return;
  EnableWindow(Handle,0);
  SetWindowLongPtr(PropDiag,GWLP_USERDATA,(LONG_PTR)this);
  if(FullScreen) 
    SetParent(PropDiag,StemWin);
  HWND Win;
  long Wid;
  int y=10;
  Wid=GetTextSize(Font,T("Disk path")).Width;
  CreateWindow("Static",T("Disk path"),WS_CHILD|WS_VISIBLE,
    10,y+4,Wid,23,PropDiag,(HMENU)100,HInstance,NULL);
  CreateWindowEx(512,"Edit",PropInf.Path,
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_READONLY,
    Wid+15,y,290-(Wid+15),23,PropDiag,(HMENU)101,HInstance,NULL);
  y+=30;
  if(PropInf.LinkPath.NotEmpty())
  {
    Wid=GetTextSize(Font,T("Shortcut path")).Width;
    CreateWindow("Static",T("Shortcut path"),WS_CHILD|WS_VISIBLE,
      10,y+4,Wid,23,PropDiag,(HMENU)110,HInstance,NULL);
    CreateWindowEx(512,"Edit",PropInf.LinkPath,
      WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_READONLY,
      Wid+15,y,290-(Wid+15),23,PropDiag,(HMENU)111,HInstance,NULL);
    y+=30;
  }
  if(has_extension(PropInf.Path,dot_ext(EXT_STT))==0)
  {
    if(FileIsDisk(PropInf.Path)==DISK_COMPRESSED)
    {
      TWidthHeight wh=GetTextSize(Font,T("Contents"));
      CreateWindow("Static",T("Contents"),WS_CHILD|WS_VISIBLE,
        10,y,wh.Width,wh.Height,PropDiag,(HMENU)120,HInstance,NULL);
      y+=wh.Height;
      Win=CreateWindowEx(512,"Listbox","",
        WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|
        LBS_NOINTEGRALHEIGHT|LBS_NOTIFY,
        10,y,280,50,PropDiag,(HMENU)121,HInstance,NULL);
      SendMessage(Win,WM_SETFONT,(WPARAM)Font,0);
      y+=60;
      EasyStringList esl;
      esl.Sort=eslSortByNameI;
      zippy.list_contents(PropInf.Path,&esl,0);
      for(int i=0;i<esl.NumStrings;i++)
        SendMessage(Win,LB_SETITEMDATA,SendMessage(Win,LB_ADDSTRING,0,
          LPARAM(esl[i].String)),esl[i].Data[0]);
      SendMessage(Win,LB_SETCURSEL,0,0);
    }
    CreateWindow("Button",T("Disk Parameters"),WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
      10,y,280,215,PropDiag,(HMENU)130,HInstance,NULL);
    CreateWindowEx(512,"Edit","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|
      ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_READONLY,
      10,y,280,215,PropDiag,(HMENU)190,HInstance,NULL);
    y+=20;
    CreateWindow("Static","",WS_CHILD|WS_VISIBLE,
      20,y,190,20,PropDiag,(HMENU)131,HInstance,NULL);
    y+=20;
    CreateWindow("Static","",WS_CHILD|WS_VISIBLE,
      20,y,260,20,PropDiag,(HMENU)132,HInstance,NULL);
    y+=20;
    Wid=GetTextSize(Font,T("Sides")).Width;
    CreateWindow("Static",T("Sides"),WS_CHILD|WS_VISIBLE,
      20,y+4,Wid,23,PropDiag,(HMENU)140,HInstance,NULL);
    Win=CreateWindowEx(512,"Combobox","",
      WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST,
      200,y,80,200,PropDiag,(HMENU)141,HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,LPARAM("1"));
    SendMessage(Win,CB_ADDSTRING,0,LPARAM("2"));
    y+=30;
    Wid=GetTextSize(Font,T("Tracks per side")).Width;
    CreateWindow("Static",T("Tracks per side"),WS_CHILD|WS_VISIBLE,
      20,y+4,Wid,23,PropDiag,(HMENU)150,HInstance,NULL);
    Win=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP
      |WS_VSCROLL|CBS_DROPDOWNLIST,200,y,80,300,PropDiag,(HMENU)151,HInstance,
      NULL);
    for(int i=10;i<=FLOPPY_MAX_TRACK_NUM+1;i++) 
      SendMessage(Win,CB_ADDSTRING,0,LPARAM(EasyStr(i).Text));
    y+=30;
    Wid=GetTextSize(Font,T("Sectors per track")).Width;
    CreateWindow("Static",T("Sectors per track"),WS_CHILD|WS_VISIBLE,
      20,y+4,Wid,23,PropDiag,(HMENU)160,HInstance,NULL);
    Win=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP
      |CBS_DROPDOWNLIST|WS_VSCROLL,200,y,80,300,PropDiag,(HMENU)161,HInstance,
      NULL);
    for(int i=3;i<=FLOPPY_MAX_SECTOR_NUM;i++) 
      SendMessage(Win,CB_ADDSTRING,0,LPARAM(EasyStr(i).Text));
    y+=30;
    Wid=GetTextSize(Font,T("Bytes per sector")).Width;
    CreateWindow("Static",T("Bytes per sector"),WS_CHILD|WS_VISIBLE,
      20,y+4,Wid,23,PropDiag,(HMENU)170,HInstance,NULL);
    Win=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP
      |CBS_DROPDOWNLIST,200,y,80,200,PropDiag,(HMENU)171,HInstance,NULL);
    for(int i=128;i<=1024;i<<=1) 
      SendMessage(Win,CB_ADDSTRING,0,LPARAM(EasyStr(i).Text));
    y+=30;
    CreateWindow("Button",T("Auto Detect"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHLIKE
      |BS_CHECKBOX,20,y,125,23,PropDiag,(HMENU)180,HInstance,NULL);
    CreateWindow("Button",T("Apply Changes"),WS_CHILD|WS_VISIBLE|WS_TABSTOP
      |BS_PUSHLIKE|BS_CHECKBOX|WS_DISABLED,155,y,125,23,PropDiag,(HMENU)181,
      HInstance,NULL);
    y+=50;
  }
  else
  {
    FILE *f=fopen(PropInf.Path,"rb");
    if(f)
    {
      CreateWindow("Static",T("Size in bytes")+": "+GetFileLength(f),WS_CHILD
        |WS_VISIBLE,10,y,280,20,PropDiag,(HMENU)112,HInstance,NULL);
      fclose(f);
      y+=22;
    }
    else
      y+=5;
  }
  SetWindowPos(PropDiag,NULL,0,0,306,y+GuiSM.cy_caption()+6,
    SWP_NOZORDER|SWP_NOMOVE);
  PropShowFileInfo(0);
  SetWindowAndChildrensFont(PropDiag,Font);
  Wid=(int)SendMessage(GetDlgItem(PropDiag,101),WM_GETTEXTLENGTH,0,0);
  SendMessage(GetDlgItem(PropDiag,101),EM_SETSEL,0,Wid);
  SendMessage(GetDlgItem(PropDiag,101),EM_SCROLLCARET,0,0);
  if(PropInf.LinkPath.NotEmpty())
  {
    Wid=(int)SendMessage(GetDlgItem(PropDiag,111),WM_GETTEXTLENGTH,0,0);
    SendMessage(GetDlgItem(PropDiag,111),EM_SETSEL,0,Wid);
    SendMessage(GetDlgItem(PropDiag,111),EM_SCROLLCARET,0,0);
  }
  CentreWindow(PropDiag,0);
  DiagFocus=GetDlgItem(PropDiag,101);
  ShowWindow(PropDiag,SW_SHOW);
  SetFocus(DiagFocus);
}


void TDiskManager::PropShowFileInfo(int i) {
  EasyStr FileInZip;
  DWORD FileHOffset=0;
  if(FileIsDisk(PropInf.Path)==DISK_COMPRESSED)
  {
    FileInZip.SetLength(MAX_PATH+1);
    SendDlgItemMessage(PropDiag,121,LB_GETTEXT,i,LPARAM(FileInZip.Text));
    FileHOffset=(int)SendDlgItemMessage(PropDiag,121,LB_GETITEMDATA,i,0);
  }
  if(FileInZip.Empty()||FileIsDisk(FileInZip))
  {
    ShowWindow(GetDlgItem(PropDiag,190),SW_HIDE);
    for(int j=130;j<190;j++)
      if(GetDlgItem(PropDiag,j)) 
        ShowWindow(GetDlgItem(PropDiag,j),SW_SHOW);
    TSF314 &TempDrive=FloppyDrive[2];
    TFloppyDisk &TempDisk=FloppyDisk[2];
    Str ErrMsg;
    Str DiskPath=PropInf.Path;
    if(FileInZip.NotEmpty()) 
      DiskPath=FileInZip;
    // for CTR, IPF, SCP, we don't try to show disk properties (would be 
    // complicated and especially unreliable since those disks are protected)
    if((has_extension(DiskPath,dot_ext(EXT_CTR))!=0)
      ||(has_extension(DiskPath,dot_ext(EXT_IPF))!=0)
      ||(has_extension(DiskPath,dot_ext(EXT_SCP))!=0))
      ErrMsg=T("Protected disk");
    else if(has_extension(DiskPath,dot_ext(EXT_STT))==0&&
      FileIsDisk(DiskPath)!=DISK_PASTI)
    {
      // bpbi is what Steem detects the BPB should be (not including the .steembpb file)
      // file_bpbi is what the raw BPB from the disk is
      if(TempDrive.SetDisk(PropInf.Path,FileInZip,&bpbi,&file_bpbi)!=FIMAGE_OK)
        ErrMsg=T("No BPB information");
    }
    else
      ErrMsg=T("No BPB information");
    if(ErrMsg.Empty())
    {
      final_bpbi.BytesPerSector=TempDisk.BytesPerSector;
      final_bpbi.Sectors=TempDisk.SectorsPerTrack*TempDisk.TracksPerSide
        *TempDisk.Sides;
      final_bpbi.SectorsPerTrack=TempDisk.SectorsPerTrack;
      final_bpbi.Sides=TempDisk.Sides;
      BOOL can_edit=(has_extension(DiskPath,dot_ext(EXT_STW))==0);
      for(int j=140;j<190;j++)
        if(GetDlgItem(PropDiag,j)) 
          EnableWindow(GetDlgItem(PropDiag,j),can_edit);
      EnableWindow(GetDlgItem(PropDiag,181),0);
      EasyStr StrValBPB=T("BPB is valid");
      if(TempDisk.ValidBPB==0)
      {
        int TracksPerSide=0;
        if(file_bpbi.SectorsPerTrack>0&&file_bpbi.Sides>0&&file_bpbi.Sectors>0)
          TracksPerSide=(file_bpbi.Sectors/file_bpbi.SectorsPerTrack)
          /file_bpbi.Sides;
        StrValBPB=T("BPB is not valid")+" ("+file_bpbi.Sides+","+TracksPerSide
          +","+file_bpbi.SectorsPerTrack+","+file_bpbi.BytesPerSector+")";
      }
      SetWindowText(GetDlgItem(PropDiag,131),StrValBPB);
      SetWindowText(GetDlgItem(PropDiag,132),T("Data bytes: ")
        +TempDisk.DiskFileLen);
      SetWindowLongPtr(GetDlgItem(PropDiag,132),GWLP_USERDATA,
        (LONG_PTR)TempDisk.DiskFileLen);
      SendDlgItemMessage(PropDiag,141,CB_SETCURSEL,TempDisk.Sides-1,0);
      SendDlgItemMessage(PropDiag,151,CB_SETCURSEL,TempDisk.TracksPerSide-10,0);
      SendDlgItemMessage(PropDiag,161,CB_SETCURSEL,TempDisk.SectorsPerTrack-3,0);
      switch(TempDisk.BytesPerSector) {
      case 128: SendDlgItemMessage(PropDiag,171,CB_SETCURSEL,0,0); break;
      case 256: SendDlgItemMessage(PropDiag,171,CB_SETCURSEL,1,0); break;
      case 512: SendDlgItemMessage(PropDiag,171,CB_SETCURSEL,2,0); break;
      case 1024: SendDlgItemMessage(PropDiag,171,CB_SETCURSEL,3,0); break;
      }
      TempDrive.RemoveDisk(true);
    }
    else
    {
      SetWindowText(GetDlgItem(PropDiag,131),ErrMsg);
      SetWindowText(GetDlgItem(PropDiag,132),"");
      for(int j=140;j<190;j++)
        if(GetDlgItem(PropDiag,j)) 
          EnableWindow(GetDlgItem(PropDiag,j),0);
    }
  }
  else
  {
    Str ZipTemp;
    ZipTemp.SetLength(MAX_PATH);
    GetTempFileName(WriteDir,"ZIP",0,ZipTemp);
    if(zippy.extract_file(PropInf.Path,FileHOffset,ZipTemp,true)==ZIPPY_SUCCEED)
    {
      char Text[20001];
      FILE *f=fopen(ZipTemp,"rb");
      int Len=(int)fread(Text,1,20000,f);
      Text[Len]=0;
      fclose(f);
      SetWindowText(GetDlgItem(PropDiag,190),Text);
    }
    DeleteFile(ZipTemp);
    for(int j=130;j<190;j++) 
      if(GetDlgItem(PropDiag,j)) 
        ShowWindow(GetDlgItem(PropDiag,j),SW_HIDE);
    ShowWindow(GetDlgItem(PropDiag,190),SW_SHOW);
  }
}


LRESULT CALLBACK TDiskManager::Dialog_WndProc(HWND Win,UINT Mess,
                            WPARAM wPar,LPARAM lPar) {
  WORD wpar_lo=LOWORD(wPar);
  WORD wpar_hi=HIWORD(wPar);
  TDiskManager *This=(TDiskManager*)GetWindowLongPtr(Win,GWLP_USERDATA);
  if(This==NULL)
  {
    if(Mess==WM_CREATE)
      SetClassLongPtr(Win,GCLP_HICON,(LONG_PTR)(hGUIIconSmall[RC_ICO_DRIVE]));
    return DefWindowProc(Win,Mess,wPar,lPar);
  }
  if(Win==This->DatabaseDiag)
  {
    switch(Mess)  {
    case WM_COMMAND:
      switch(wpar_lo) {
      case IDOK:
      {
        Str Find=GetWindowTextStr(GetDlgItem(Win,103));
        This->DatabaseFind=Find; // remember
        if(Find.Empty())
        {
          MessageBeep(0);
          break;
        }
        char buf[65536],*p=buf;
        GetContents_SearchDatabase(Find,buf,sizeof(buf));
        int n_items=0;
        while(p[0])
        {
          p+=strlen(p)+1;
          while(p[0])
            p+=strlen(p)+1;
          p++;
          n_items++;
        }
        if(n_items==0)
        {
          MessageBeep(0);
          break;
        }
        SendDlgItemMessage(Win,111,LVM_DELETEALLITEMS,0,0);
        SendDlgItemMessage(Win,111,LVM_SETITEMCOUNT,n_items,0);
        SendDlgItemMessage(Win,111,WM_SETREDRAW,0,0);
        int i=0;
        LV_ITEM lvi;
        lvi.mask=LVIF_TEXT;
        Str Contents;
        p=buf;
        while(p[0])
        {
          lvi.iItem=i;
          lvi.iSubItem=0;
          lvi.pszText=p;
          SendDlgItemMessage(Win,111,LVM_INSERTITEM,0,LPARAM(&lvi));
          p+=strlen(p)+1;
          Contents="";
          while(p[0])
          {
            if(Contents[0]) Contents+=", ";
            Contents+=p;
            p+=strlen(p)+1;
          }
          lvi.iSubItem=1;
          lvi.pszText=Contents;
          SendDlgItemMessage(Win,111,LVM_SETITEM,0,LPARAM(&lvi));
          p++; // skip content list null
          i++;
        }
        SendDlgItemMessage(Win,111,LVM_SETCOLUMNWIDTH,0,LVSCW_AUTOSIZE);
        SendDlgItemMessage(Win,111,LVM_SETCOLUMNWIDTH,1,LVSCW_AUTOSIZE);
        SendDlgItemMessage(Win,111,WM_SETREDRAW,1,0);
        break;
      }
      case IDCANCEL:
        SetForegroundWindow(This->Handle);
        //EnableWindow(This->Handle,true);
        DestroyWindow(Win);
        return 0;
      }
      break;
    case WM_CLOSE:
      SetForegroundWindow(This->Handle);
      //EnableWindow(This->Handle,true);
      break;
    case WM_DESTROY:
      This->DatabaseDiag=NULL;
      break;
    }
  }
  else if(Win==This->ContentDiag)
  {
    switch(Mess) {
    case WM_COMMAND:
      switch(wpar_lo) {
      case IDOK:
      {
        Str DestFol=GetWindowTextStr(GetDlgItem(Win,201));
        NO_SLASH(DestFol);
        Str NewLink;
        Str DiskName;
        if(SendDlgItemMessage(Win,220,BM_GETCHECK,BST_CHECKED,0)==BST_CHECKED)
          DiskName=Str(" (")+GetWindowTextStr(GetDlgItem(Win,221))+")";
        CreateDirectory(DestFol,NULL);
        if(GetFileAttributes(DestFol)==0xffffffff)
        {
          Alert(T("Invalid directory"),T("Error"),0);
          break;
        }
        for(int i=2;i<This->contents_sl.NumStrings;i++)
        {
          LV_ITEM lvi;
          lvi.iItem=i-2;
          lvi.iSubItem=0;
          lvi.mask=LVIF_STATE;
          lvi.stateMask=LVIS_STATEIMAGEMASK;
          SendDlgItemMessage(Win,111,LVM_GETITEM,0,(LPARAM)&lvi);
          if(lvi.state & LVI_SI_CHECKED)
          {
            NewLink=DestFol+SLASH+Str(This->contents_sl[i].String)
              +DiskName+".lnk";
            if(Exists(NewLink))
            {
              if(This->ContentConflictAction==0)
                NewLink="";
              else if(This->ContentConflictAction==2)
                NewLink=GetUniquePath(DestFol,Str(This->contents_sl[i].String)+DiskName+".lnk");
            }
            if(NewLink.NotEmpty())
              CreateLink(NewLink,This->contents_sl[0].String);
          }
        }
        if(IsSameStr_I(DestFol,This->DisksFol))
          This->RefreshDiskView("",0,NewLink);
      }
      case IDCANCEL:
        EnableWindow(This->Handle,true);
        SetForegroundWindow(This->Handle);
        DestroyWindow(Win);
        return 0;
      case 202:
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        HWND Edit=GetDlgItem(Win,201);
        EnableAllWindows(0,Win);
        Str CurText=GetWindowTextStr(Edit);
        NO_SLASH(CurText);
        EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
          T("Pick a Folder"),CurText);
        if(NewFol.NotEmpty())
          SendMessage(Edit,WM_SETTEXT,0,(LPARAM)NewFol.Text);
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        break;
      }
      case 211:
        if(wpar_hi==CBN_SELENDOK)
          This->ContentConflictAction=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
        break;
      }
      break;
    case WM_CLOSE:
      SendMessage(Win,WM_COMMAND,IDCANCEL,0);
      break;
    case WM_DESTROY:
      This->ContentDiag=NULL;
      This->contents_sl.DeleteAll();
      break;
    }
  }
  else if(Win==This->DiskDiag)
  {
    switch(Mess) {
    case WM_COMMAND:
      switch(wpar_lo) {
#if defined(SSE_DISK_CREATE_MSA_DIM)
      case 7341:
      case 7342:
      case 7343:
        if(wpar_hi==BN_CLICKED)
          if(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
            SSEConfig.DiskImageCreated=(BYTE)(wpar_lo-7341+1); // sticky, used by CreateDiskImage()
        break;
#endif
      case IDCANCEL:
      case IDOK:
      {
#if defined(SSE_DISK_CREATE_MSA_DIM)
        EasyStr STName=This->DisksFol+SLASH+T("Blank Disk.")
            +extension_list[SSEConfig.DiskImageCreated];
        if(wpar_lo==IDOK)
        {
          int n=2;
          while(Exists(STName))
            STName=This->DisksFol+SLASH+T("Blank Disk")+" ("+(n++)+")."
            +extension_list[SSEConfig.DiskImageCreated];
          char buf[12];
          SendDlgItemMessage(Win,101,WM_GETTEXT,2,(LPARAM)buf);
          WORD Sides=(WORD)atoi(buf);
          This->SidesIdx=Sides-1;
          SendDlgItemMessage(Win,103,WM_GETTEXT,3,(LPARAM)buf);
          This->SecsPerTrackIdx=(WORD)atoi(buf);
          SendDlgItemMessage(Win,105,WM_GETTEXT,3,(LPARAM)buf);
          This->TracksIdx=(WORD)atoi(buf);
          WORD Sectors=WORD(Sides*(This->TracksIdx)*(This->SecsPerTrackIdx));
          //TRACE("create disk image %s %d sectors, %d tracks %d sectors/track, %d sides\n",
            //STName.Text,Sectors,This->TracksIdx,This->SecsPerTrackIdx,Sides);
          if(This->CreateDiskImage(STName,Sectors,
            This->SecsPerTrackIdx,Sides)==0)
          {
            Alert(EasyStr(T("Could not create the disk image"))+" "+STName,
              T("Error"),MB_ICONEXCLAMATION);
            return 0;
          }
        }
#else
        EasyStr STName=This->DisksFol+"\\"+T("Blank Disk")+".st";
        if(wpar_lo==IDOK)
        {
          int n=2;
          while(Exists(STName))
            STName=This->DisksFol+"\\"+T("Blank Disk")+" ("+(n++)+").st";
          This->SidesIdx=(WORD)SendDlgItemMessage(Win,101,CB_GETCURSEL,0,0);
          WORD Sides=WORD(This->SidesIdx+1);
          This->SecsPerTrackIdx=(WORD)SendDlgItemMessage(Win,103,CB_GETCURSEL,
            0,0);
          This->TracksIdx=(WORD)SendDlgItemMessage(Win,105,CB_GETCURSEL,0,0);
          WORD Sectors=WORD(Sides*(This->TracksIdx+76)
            *(This->SecsPerTrackIdx+8));
          if(This->CreateDiskImage(STName,Sectors,
            This->SecsPerTrackIdx+8,Sides)==0)
          {
            Alert(EasyStr(T("Could not create the disk image"))+" "+STName,
              T("Error"),MB_ICONEXCLAMATION);
            return 0;
          }
        }
#endif
        EnableWindow(This->Handle,true);
        SetForegroundWindow(This->Handle);
        DestroyWindow(Win);
        if(wpar_lo==IDOK)
          This->RefreshDiskView(STName,true);
        return 0;
      }
      case 101:case 103:case 105:
#if defined(SSE_DISK_CREATE_MSA_DIM)
        if(wpar_hi==EN_CHANGE)
        {
          int databytes=This->GetDiskSelectionSize();
          int kb=databytes/1024;
          SendDlgItemMessage(Win,106,WM_SETTEXT,0,LPARAM((T("Disk size")
            +": "+kb+T(" KB")).Text));
        }
#else
        if(wpar_hi==CBN_SELENDOK)
        {
          EasyStr SizeBytes=This->GetDiskSelectionSize();
          if(SizeBytes.Length()>6)
          {
            SizeBytes.Insert(",",1);
            SizeBytes.Insert(",",5);
          }
          else
            SizeBytes.Insert(",",3);
          SendDlgItemMessage(Win,106,WM_SETTEXT,0,LPARAM((T("Disk size")
            +": "+SizeBytes+" "+T("bytes")).Text));
        }
#endif
        break;
      }
      break;
    case WM_DESTROY:
      This->DiskDiag=NULL;
      EnableWindow(This->Handle,true);
      break;
    case WM_CLOSE:
      SendMessage(Win,WM_COMMAND,IDCANCEL,0);
      return 0;
    }
  }
  else if(Win==This->LinksDiag)
  {
    switch(Mess)
    {
    case WM_COMMAND:
      switch(wpar_lo) {
      case IDOK:
        if(This->DoCreateMultiLinks()==0)
          break;
      case IDCANCEL:
        EnableWindow(This->Handle,true);
        SetForegroundWindow(This->Handle);
        DestroyWindow(Win);
        return 0;
      case 102:
      case 202:
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        char CurText[MAX_PATH+1];
        HWND Edit=GetDlgItem(Win,wpar_lo-1);
        EnableAllWindows(0,Win);
        SendMessage(Edit,WM_GETTEXT,MAX_PATH,(LPARAM)CurText);
        NO_SLASH(CurText);
        if(wpar_lo==202)
        {
          EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
            T("Pick a Folder"),CurText);
          if(NewFol.NotEmpty())
            SendMessage(Edit,WM_SETTEXT,0,(LPARAM)NewFol.Text);
        }
        else
        {
          EasyStr CurFol=CurText;
          char *CurDiskName=GetFileNameFromPath(CurFol);
          if(CurDiskName>CurFol.Text)
            *(CurDiskName-1)=0;
          EasyStr Target=FileSelect(HWND(FullScreen?StemWin:Win),
            T("Select Shortcut Target"),CurFol,
            FSTypes(2,NULL),1,true,"st",CurDiskName);
          if(Target.NotEmpty())
            SendMessage(Edit,WM_SETTEXT,0,(LPARAM)Target.Text);
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        break;
      }
      break;
    case WM_CLOSE:
      SendMessage(Win,WM_COMMAND,IDCANCEL,0);
      return 0;
    case WM_DESTROY:
      This->LinksDiag=NULL;
      EnableWindow(This->Handle,true);
      break;
    }
  }
#if !defined(SSE_NO_WINSTON_IMPORT)
  else if(Win==This->ImportDiag)
  {
    switch(Mess) {
    case WM_COMMAND:
      switch(wpar_lo) {
      case IDOK:
        if(This->DoImport()==0) break;
      case IDCANCEL:
        if(This->Importing)
          This->Importing=0;
        else
        {
          EnableWindow(This->Handle,true);
          SetForegroundWindow(This->Handle);
          DestroyWindow(Win);
        }
        return 0;
      case 102:
      case 152:
      case 202:
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        char CurFol[MAX_PATH+1];
        HWND Edit=GetDlgItem(Win,wpar_lo-1);
        EnableAllWindows(0,Win);
        SendMessage(Edit,WM_GETTEXT,MAX_PATH,(long)CurFol);
        NO_SLASH(CurFol);
        EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),T("Pick a Folder"),CurFol);
        if(NewFol.NotEmpty())
          SendMessage(Edit,WM_SETTEXT,0,(long)NewFol.Text);
        if(wpar_lo==102)
          SendDlgItemMessage(Win,151,WM_SETTEXT,0,(LPARAM)((NewFol+"\\Discs").Text));
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        break;
      }
      }
      break;
    case WM_CLOSE:
      SendMessage(Win,WM_COMMAND,IDCANCEL,0);
      return 0;
    case WM_DESTROY:
      This->ImportDiag=NULL;
      EnableWindow(This->Handle,true);
      break;
    }
  }
#endif
  else if(Win==This->PropDiag)
  {
    switch(Mess) {
    case WM_COMMAND:
      switch(wpar_lo) {
      case 121:
        if(wpar_hi==LBN_SELCHANGE)
          This->PropShowFileInfo((int)SendMessage(HWND(lPar),LB_GETCURSEL,0,0));
        break;
      case 141:case 151:case 161:case 171:
        if(wpar_hi==CBN_SELENDOK)
        {
          int TracksPerSide=0;
          if(This->final_bpbi.SectorsPerTrack>0&&This->final_bpbi.Sides>0
            &&This->final_bpbi.Sectors>0)
            TracksPerSide=(This->final_bpbi.Sectors/
              This->final_bpbi.SectorsPerTrack)/This->final_bpbi.Sides;
          bool Enable=
            ((SendDlgItemMessage(Win,141,CB_GETCURSEL,0,0)+1)
              !=This->final_bpbi.Sides)|
            ((SendDlgItemMessage(Win,151,CB_GETCURSEL,0,0)+10)!=TracksPerSide)|
            ((SendDlgItemMessage(Win,161,CB_GETCURSEL,0,0)+3)
              !=This->final_bpbi.SectorsPerTrack)|
            ((128<<SendDlgItemMessage(Win,171,CB_GETCURSEL,0,0))
              !=This->final_bpbi.BytesPerSector);
          EnableWindow(GetDlgItem(This->PropDiag,181),Enable);
        }
        break;
      case 181:
        if(wpar_hi==BN_CLICKED)
        {
          int nFile=0;
          EasyStr DiskInZip;
          if(FileIsDisk(This->PropInf.Path)==DISK_COMPRESSED)
          {
            nFile=(int)SendDlgItemMessage(Win,121,LB_GETCURSEL,0,0);
            DiskInZip.SetLength(MAX_PATH);
            SendDlgItemMessage(Win,121,LB_GETTEXT,nFile,(LPARAM)DiskInZip.Text);
          }
          EasyStr File=This->PropInf.Path+DiskInZip+".steembpb";
          int Sides=(int)SendDlgItemMessage(Win,141,CB_GETCURSEL,0,0)+1;
          int TracksPerSide=(int)SendDlgItemMessage(Win,151,CB_GETCURSEL,0,0)+10;
          int SectorsPerTrack=(int)SendDlgItemMessage(Win,161,CB_GETCURSEL,0,0)+3;
          int BytesPerSector=128<<(int)SendDlgItemMessage(Win,171,CB_GETCURSEL,0,0);
          int Sectors=Sides*TracksPerSide*SectorsPerTrack;
          int Ret=IDYES;
          if((Sectors*BytesPerSector)>GetWindowLongPtr(GetDlgItem(Win,132),GWLP_USERDATA))
            Ret=Alert(T("This disk configuration is too big for the size of the\
 file, this could cause disk problems. Would you like to use it anyway?"),
              T("Use Configuration?"),MB_ICONQUESTION|MB_YESNO);
          if(Ret==IDYES)
          {
            TConfigStoreFile CSF(File);
            CSF.SetStr("BPB","Sides",Str(Sides));
            CSF.SetStr("BPB","SectorsPerTrack",Str(SectorsPerTrack));
            CSF.SetStr("BPB","BytesPerSector",Str(BytesPerSector));
            CSF.SetStr("BPB","Sectors",Str(Sectors));
            CSF.Close();
            This->PropShowFileInfo(nFile);
            for(int disk=0;disk<2;disk++)
            {
              if(IsSameStr_I(FloppyDrive[disk].GetDisk(),This->PropInf.Path))
              {
                EasyStr DiskInZip2=FloppyDisk[disk].DiskInZip;
                FloppyDrive[disk].RemoveDisk();
                FloppyDrive[disk].SetDisk(This->PropInf.Path,DiskInZip2);
              }
            }
          }
        }
        break;
      case 180:
        if(wpar_hi==BN_CLICKED)
        {
          int TracksPerSide=0;
          if(This->bpbi.SectorsPerTrack>0&&This->bpbi.Sides>0
            &&This->bpbi.Sectors>0)
            TracksPerSide=(This->bpbi.Sectors/This->bpbi.SectorsPerTrack)
            /This->bpbi.Sides;
          SendDlgItemMessage(Win,141,CB_SETCURSEL,This->bpbi.Sides-1,0);
          SendDlgItemMessage(Win,151,CB_SETCURSEL,TracksPerSide-10,0);
          SendDlgItemMessage(Win,161,CB_SETCURSEL,
            This->bpbi.SectorsPerTrack-3,0);
          switch(This->bpbi.BytesPerSector) {
          case 128: SendDlgItemMessage(Win,171,CB_SETCURSEL,0,0); break;
          case 256: SendDlgItemMessage(Win,171,CB_SETCURSEL,1,0); break;
          case 512: SendDlgItemMessage(Win,171,CB_SETCURSEL,2,0); break;
          case 1024: SendDlgItemMessage(Win,171,CB_SETCURSEL,3,0); break;
          }
          SendMessage(Win,WM_COMMAND,MAKEWPARAM(141,CBN_SELENDOK),
            (LPARAM)GetDlgItem(Win,141));
        }
        break;
      }
      break;
    case WM_CLOSE:
      This->PropDiag=NULL;
      SetForegroundWindow(This->Handle);
      EnableWindow(This->Handle,true);
      break;
    }
  }
  switch(Mess) {
  case WM_MOVING:case WM_SIZING:
    if(FullScreen)
    {
      RECT *rc=(RECT*)lPar;
      if(rc->top<MENUHEIGHT)
      {
        if(Mess==WM_MOVING) 
          rc->bottom+=MENUHEIGHT-rc->top;
        rc->top=MENUHEIGHT;
        return true;
      }
      RECT LimRC={0,MENUHEIGHT+GuiSM.cy_frame(),GuiSM.cx_screen(),GuiSM.cy_screen()};
      ClipCursor(&LimRC);
    }
    break;
  case WM_CAPTURECHANGED:   //Finished
    if(FullScreen) 
      ClipCursor(NULL);
    break;
  case WM_ACTIVATE:
    if(wPar==WA_INACTIVE) 
      This->DiagFocus=GetFocus();
    break;
  case WM_SETFOCUS:
    SetFocus(This->DiagFocus);
    break;
  case DM_GETDEFID:
    return MAKELONG(IDOK,DC_HASDEFID);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


int TDiskManager::GetDiskSelectionSize() {
#if defined(SSE_DISK_CREATE_MSA_DIM)
  char buf[12];
  SendDlgItemMessage(DiskDiag,101,WM_GETTEXT,2,(LPARAM)buf);
  WORD Sides=(WORD)atoi(buf);
  SendDlgItemMessage(DiskDiag,105,WM_GETTEXT,3,(LPARAM)buf);
  WORD Tracks=(WORD)atoi(buf);
  SendDlgItemMessage(DiskDiag,103,WM_GETTEXT,3,(LPARAM)buf);
  WORD Sectors=(WORD)atoi(buf);
  int bytes=Sides*Tracks*Sectors*512;
  //TRACE("%d bytes = %d Sides * %d Tracks * %d Sectors *512\n",bytes,Sides,Tracks,Sectors);
  return bytes;
#else
  return (SendDlgItemMessage(DiskDiag,101,CB_GETCURSEL,0,0)+1) *
    (SendDlgItemMessage(DiskDiag,103,CB_GETCURSEL,0,0)+8) *
    (SendDlgItemMessage(DiskDiag,105,CB_GETCURSEL,0,0)+76)*512;
#endif
}


bool TDiskManager::DoCreateMultiLinks() {
  LinksTargetPath.SetLength(MAX_PATH);
  SendMessage(GetDlgItem(LinksDiag,101),WM_GETTEXT,MAX_PATH,(LPARAM)LinksTargetPath.Text);
  NO_SLASH(LinksTargetPath);
  if(LinksTargetPath[0]==0)
  {
    Alert(T("Please enter a file/folder to be the target for the shortcuts."),
      T("Multiple Shortcuts Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  else if(GetFileAttributes(LinksTargetPath)==0xffffffff)
  {
    Alert(LinksTargetPath+" "+T("does not exist."),T("Multiple Shortcuts Error")
      ,MB_ICONEXCLAMATION);
    return 0;
  }
  MultipleLinksPath.SetLength(MAX_PATH);
  SendMessage(GetDlgItem(LinksDiag,201),WM_GETTEXT,MAX_PATH,
    (LPARAM)MultipleLinksPath.Text);
  NO_SLASH(MultipleLinksPath);
  if(MultipleLinksPath[0]==0)
  {
    Alert(T("Please enter a folder to create the shortcuts in."),T("Multiple Shortcuts Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  if(GetFileAttributes(MultipleLinksPath)==0xffffffff)
  {
    if(CreateDirectory(MultipleLinksPath,NULL)==0)
    {
      Alert(T("Couldn't create the folder to create the shortcuts in")+" "+MultipleLinksPath,T("Multiple Shortcuts Error"),MB_ICONEXCLAMATION);
      return 0;
    }
  }
  EasyStr LinkFileName,Name;
  for(int n=0;n<9;n++)
  {
    LinkFileName=MultipleLinksPath+SLASH;
    Name.SetLength(200);
    SendMessage(GetDlgItem(LinksDiag,301+n*100),WM_GETTEXT,200,
      (LPARAM)Name.Text);
    if(Name.NotEmpty())
    {
      RemoveIllegalFromPath(Name,0,true,'-');
      while(strchr(Name,SLASHCHAR)) 
        *(strchr(Name,SLASHCHAR))='-';
      LinkFileName+=Name+".lnk";
      CreateLink(LinkFileName,LinksTargetPath);
    }
  }
  if(IsSameStr_I(MultipleLinksPath,DisksFol))
    PostMessage(Handle,WM_COMMAND,IDCANCEL,0);
  return true;
}

#endif//WIN32


#ifdef UNIX

int TDiskManager::diag_but_np(hxc_button *b,int mess,int *p_i)
{
  if (mess!=BN_CLICKED) return 0;
  if (b->id==100){
//    TDiskManager *This=(TDiskManager*)(b->owner);
    hxc_textdisplay *p_td=(hxc_textdisplay*)hxc::find(b->parent,200);
    hxc_edit *p_ed=(hxc_edit*)hxc::find(b->parent,101);

    Str Find=p_ed->text;
    if (Find.Empty()) return 0;

    char buf[65536],*p=buf;
    GetContents_SearchDatabase(Find,buf,sizeof(buf));

    Str Name,Contents;
    Str outtext;
    while (p[0]){
      Name=p;
      p+=strlen(p)+1;

      Contents="";
      while (p[0]){
        if (Contents[0]) Contents+=", ";
        Contents+=p;
        p+=strlen(p)+1;
      }
      p++; // skip content list null

      outtext+=Name+" - "+Contents+"\n\n";
    }
    p_td->sy=0;
    p_td->set_text(outtext);
    p_td->draw(true);
  }else if (b->id==200){
    b->set_check(true);
    hxc_edit *p_ed=(hxc_edit*)hxc::find(b->parent,201);
    fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
    EasyStr new_path=fileselect.choose(XD,p_ed->text,"",T("Pick a Folder"),
      FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
    if (new_path[0]){
      NO_SLASH(new_path);
      p_ed->set_text(new_path+"/");
    }
    b->set_check(0);
  }
  return 0;
}
//---------------------------------------------------------------------------
int TDiskManager::diag_ed_np(hxc_edit *ed,int mess,int i)
{
  if (mess==EDN_RETURN){
    hxc_button *p_but=(hxc_button*)hxc::find(ed->parent,100);
    int i[1]={Button1};
    diag_but_np(p_but,BN_CLICKED,i);
  }
  return 0;
}

#endif//UNIX


#ifndef SSE_NO_WINSTON_IMPORT

bool TDiskManager::ImportDiskExists(char *Disk,EasyStr &FullDisk) {
  if(Disk==NULL) return 0;
  if(FloppyDisk[1]==':')
    FullDisk=Disk;
  else
    FullDisk=WinSTonDiskPath+"\\"+Disk;
  if(Exists(FullDisk)) return true;
  // Default to zip if no extension and disk doesn't exist
  EasyStr OldExt=".zip";
  // Strip current extension and save it
  char *dot=strrchr(FullDisk,'.');
  if(dot)
  {
    OldExt=dot;
    *dot=0;
  }
  // Go through list of extensions and see if any of them exist
  char *WinSTonDiskExts[4]={".msa",".st",".zip",NULL};
  int i=0;
  while(WinSTonDiskExts[i]) {
    if(Exists(FullDisk+WinSTonDiskExts[i]))
    {
      FullDisk+=WinSTonDiskExts[i];
      return true;
    }
    i++;
  }
  // Doesn't exist, still set most likely file name
  FullDisk+=OldExt;
  return 0;
}


HRESULT TDiskManager::CreateLinkCheckForOverwrite(char *LinkPath,char *TargetPath,
                          IShellLink *Link,IPersistFile* File){
  if (ImportConflictAction!=1){ // Not overwrite
    EasyStr NewLinkPath=LinkPath;
    if (Exists(LinkPath)){
      switch (ImportConflictAction){
        case 0:
          NewLinkPath="";
          break;
        case 2:case 3:
        {
          EasyStr UniqueLink=LinkPath;
          int n=2;
          UniqueLink.Insert(" (2)",UniqueLink.Length()-4);
          while (Exists(UniqueLink)) UniqueLink[UniqueLink.Length()-6]=char('1'+(n++));
          if (ImportConflictAction==2){
            NewLinkPath=UniqueLink;
          }else{
            if (MoveFile(LinkPath,UniqueLink)==0) NewLinkPath="";
          }
          break;
        }
      }
    }
    if (NewLinkPath.NotEmpty()) return CreateLink(NewLinkPath,TargetPath,NULL,Link,File);
    return 0;
  }
  return CreateLink(LinkPath,TargetPath,NULL,Link,File);
}


bool TDiskManager::DoImport() {
  WinSTonPath.SetLength(MAX_PATH);
  SendMessage(GetDlgItem(ImportDiag,101),WM_GETTEXT,MAX_PATH,(LPARAM)WinSTonPath.Text);
  NO_SLASH(WinSTonPath);
  if (WinSTonPath.Text[0]==0){
    Alert(T("Please enter the full path of the folder WinSTon is in."),T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }else if (GetFileAttributes(WinSTonPath)==0xffffffff){
    Alert(WinSTonPath+" "+T("does not exist."),T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  WinSTonDiskPath.SetLength(MAX_PATH);
  SendMessage(GetDlgItem(ImportDiag,151),WM_GETTEXT,MAX_PATH,(LPARAM)WinSTonDiskPath.Text);
  NO_SLASH(WinSTonDiskPath);
  if (WinSTonDiskPath.Text[0]==0){
    Alert(T("Please enter the full path of the folder WinSTon stores its disks in."),T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }else if (GetFileAttributes(WinSTonDiskPath)==0xffffffff){
    Alert(WinSTonDiskPath+" "+T("does not exist."),T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  ImportPath.SetLength(MAX_PATH);
  SendMessage(GetDlgItem(ImportDiag,201),WM_GETTEXT,MAX_PATH,(LPARAM)ImportPath.Text);
  NO_SLASH(ImportPath);
  if (ImportPath.Text[0]==0){
    Alert(T("Please specify a folder to import the favourites to."),T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  if (GetFileAttributes(ImportPath)==0xffffffff){
    if (CreateDirectory(ImportPath,NULL)==0){
      Alert(T("Couldn't create the import folder")+" "+ImportPath,T("Import Error"),MB_ICONEXCLAMATION);
      return 0;
    }
  }
  ImportOnlyIfExist=SendMessage(GetDlgItem(ImportDiag,300),BM_GETCHECK,0,0);
  ImportConflictAction=SendMessage(GetDlgItem(ImportDiag,302),CB_GETCURSEL,0,0);
  FILE *f=fopen(WinSTonPath+"\\favourites.txt","rb");
  if (f==NULL){
    Alert(T("Couldn't open favourites.txt, please check that the WinSTon folder is correct."),
            T("Import Error"),MB_ICONEXCLAMATION);
    return 0;
  }
  long Len=GetFileLength(f);
  char *Data=new char[Len+1];
  fread(Data,1,Len,f);
  Data[Len]=0;
  fclose(f);
  Importing=true;
  EnableAllWindows(0,ImportDiag);
  EnableWindow(GetDlgItem(ImportDiag,100),0);
  EnableWindow(GetDlgItem(ImportDiag,101),0);
  EnableWindow(GetDlgItem(ImportDiag,102),0);
  EnableWindow(GetDlgItem(ImportDiag,150),0);
  EnableWindow(GetDlgItem(ImportDiag,151),0);
  EnableWindow(GetDlgItem(ImportDiag,152),0);
  EnableWindow(GetDlgItem(ImportDiag,200),0);
  EnableWindow(GetDlgItem(ImportDiag,201),0);
  EnableWindow(GetDlgItem(ImportDiag,202),0);
  EnableWindow(GetDlgItem(ImportDiag,300),0);
  EnableWindow(GetDlgItem(ImportDiag,301),0);
  EnableWindow(GetDlgItem(ImportDiag,302),0);
  long OneBit=MAX(Len/280,1L);
  SendMessage(GetDlgItem(ImportDiag,400),PBM_SETRANGE,0,MAKELPARAM(0,280));
  SendMessage(GetDlgItem(ImportDiag,400),PBM_SETPOS,0,0);
  ShowWindow(GetDlgItem(ImportDiag,IDOK),SW_HIDE);
  ShowWindow(GetDlgItem(ImportDiag,400),SW_SHOW);
  IShellLink *LinkObj=NULL;
  IPersistFile *FileObj=NULL;
  HRESULT hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&LinkObj);
  if (SUCCEEDED(hres)==0) LinkObj=NULL;
  if (LinkObj){
    hres=LinkObj->QueryInterface(IID_IPersistFile,(void**)&FileObj);
    if (SUCCEEDED(hres)==0) FileObj=NULL;
  }
  EasyStr CurFol=ImportPath,LinkPath;
  char *FloppyDisk[2][2]={{NULL,NULL},{NULL,NULL}};
  char *Command=Data,*pRet,*CommandStart,*NextCommand;
  long BitPos,OldBitPos=0;
  while (Importing){
    pRet=strchr(Command,'\n');
    if (pRet==NULL) break;
    NextCommand=pRet+1;
    if (*(pRet-1)=='\r') pRet--;
    *pRet=0;
    if ((CommandStart=strstr(Command,"[FOLDER]"))!=NULL){
      CurFol+="\\";
      CurFol+=RemoveIllegalFromPath(CommandStart+8,0);
      CreateDirectory(CurFol,NULL);
    }else if ((CommandStart=strstr(Command,"[ENDFOLDER]"))!=NULL){
      *(GetFileNameFromPath(CurFol)-1)=0;
      FloppyDisk[0][0]=NULL;FloppyDisk[1][0]=NULL;FloppyDisk[0][1]=NULL;FloppyDisk[1][1]=NULL;
    }else if ((CommandStart=strstr(Command,"[TITLE]"))!=NULL){
      LinkPath=CurFol+"\\"+RemoveIllegalFromPath(CommandStart+7,0)+".lnk";
    }else if ((CommandStart=strstr(Command,"[ENDTITLE]"))!=NULL){
      if (FloppyDisk[1][0] || FloppyDisk[1][1]) LinkPath.Insert(" (Disk 1)",LinkPath.Length()-4);
      EasyStr TargetPath,NewLinkPath;
      for (int d=0;d<2;d++){
        if (FloppyDisk[d][0]){
          if (ImportDiskExists(FloppyDisk[d][0],TargetPath)){
            CreateLinkCheckForOverwrite(LinkPath,TargetPath,LinkObj,FileObj);
          }else if (ImportDiskExists(FloppyDisk[d][1],TargetPath)){
            CreateLinkCheckForOverwrite(LinkPath,TargetPath,LinkObj,FileObj);
          }else if (ImportOnlyIfExist==0){
            ImportDiskExists(FloppyDisk[d][0],TargetPath);
            CreateLinkCheckForOverwrite(LinkPath,TargetPath,LinkObj,FileObj);
          }
        }
        LinkPath[LinkPath.Length()-6]='2';
      }
      LinkPath[0]=0;
      FloppyDisk[0][0]=NULL;FloppyDisk[1][0]=NULL;FloppyDisk[0][1]=NULL;FloppyDisk[1][1]=NULL;
    }else if ((CommandStart=strstr(Command,"[DISC1]"))!=NULL){
      FloppyDisk[0][0]=RemoveIllegalFromPath(CommandStart+7,CommandStart[8]==':');
    }else if ((CommandStart=strstr(Command,"[DISC2]"))!=NULL){
      FloppyDisk[1][0]=RemoveIllegalFromPath(CommandStart+7,CommandStart[8]==':');
    }else if ((CommandStart=strstr(Command,"[ALTDISC1]"))!=NULL){
      FloppyDisk[0][1]=RemoveIllegalFromPath(CommandStart+10,CommandStart[11]==':');
    }else if ((CommandStart=strstr(Command,"[ALTDISC2]"))!=NULL){
      FloppyDisk[1][1]=RemoveIllegalFromPath(CommandStart+10,CommandStart[11]==':');
    }
    Command=NextCommand;
    BitPos=(long(Command)-long(Data))/OneBit;
    if (BitPos!=OldBitPos){
      SendMessage(GetDlgItem(ImportDiag,400),PBM_SETPOS,WPARAM(BitPos),0);
      OldBitPos=BitPos;
    }
    PeekEvent();
  }
  if (LinkObj) LinkObj->Release();
  if (FileObj) FileObj->Release();
  delete[] Data;
  EnableWindow(GetDlgItem(ImportDiag,100),true);
  EnableWindow(GetDlgItem(ImportDiag,101),true);
  EnableWindow(GetDlgItem(ImportDiag,102),true);
  EnableWindow(GetDlgItem(ImportDiag,150),true);
  EnableWindow(GetDlgItem(ImportDiag,151),true);
  EnableWindow(GetDlgItem(ImportDiag,152),true);
  EnableWindow(GetDlgItem(ImportDiag,200),true);
  EnableWindow(GetDlgItem(ImportDiag,201),true);
  EnableWindow(GetDlgItem(ImportDiag,202),true);
  EnableWindow(GetDlgItem(ImportDiag,300),true);
  EnableWindow(GetDlgItem(ImportDiag,301),true);
  EnableWindow(GetDlgItem(ImportDiag,302),true);
  EnableAllWindows(true,ImportDiag);
  if (Importing==0){ //Cancelled
    ShowWindow(GetDlgItem(ImportDiag,400),SW_HIDE);
    ShowWindow(GetDlgItem(ImportDiag,IDOK),SW_SHOW);
    return 0;
  }
  SetDir(ImportPath,true);
  Importing=0;
  return true;
}

#endif//#ifndef SSE_NO_WINSTON_IMPORT
