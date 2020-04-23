; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 
;  Win32ASM (MASM32) UnRAR Example
;
;  Author: Florian Mücke
;  Version: 2, 11.09.05 (complete re-do)
;
; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

      .586
      .model flat, stdcall
      option casemap :none		;case sensitive

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

      include windows.inc
      include user32.inc
      include kernel32.inc
	  include comctl32.inc 
	  include unrar.inc
	  
	  includelib comctl32.lib 
      includelib user32.lib
      includelib kernel32.lib
	  includelib unrar.lib

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	WndProc             PROTO :DWORD,:DWORD,:DWORD,:DWORD
	ThreadProc          PROTO :DWORD
    RARCallbackProc    	PROTO :DWORD,:DWORD,:DWORD,:DWORD    
                        

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

.data
	hInstance		dd 0
	hWnd            dd 0
	hIcon			dd 0
	hArchive		dd 0
	ArcName			db "test.rar",0
	DestPath		db "EXTRACTED_DATA",0
	caption			db "W32ASM UNRAR example",0
	ArcOpenErr		db "archive open error",0
	NextVolQuery	db "next volume not found",0
;	NextVolOk		db "next volume successfully opened",0
	ArchiveData		RAROpenArchiveData <>
	HeaderData		RARHeaderData <>
	buffer			db 260 dup (0)
	unpSize			dd 0
	FormatStr		db "%lu of %lu bytes",0
	bExtracted		dd 0
	fbExtracted		dd 0
	dummy_dd		dd 0
	FileCRC			dd 0
	IDC_PROGRESS	equ 1
	ThreadID		dd 0
	
; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

.code

start:
	invoke GetModuleHandle, NULL
    mov hInstance, eax
	invoke InitCommonControls

	mov ArchiveData.lpArcName, OFFSET ArcName
	mov ArchiveData.OpenMode, RAR_OM_LIST
	invoke RAROpenArchive, ADDR ArchiveData
	mov hArchive, eax
	.if eax == 0
		invoke MessageBox, NULL, ADDR ArcOpenErr, ADDR caption, MB_OK
		jmp the_end
	.endif
   ;get unpacked filesize
	invoke RARReadHeader, hArchive, ADDR HeaderData
    getunpsize:
	invoke RARProcessFile, hArchive,RAR_SKIP,0,0
	mov eax, HeaderData.UnpSize
	add unpSize, eax
	invoke RARReadHeader, hArchive, ADDR HeaderData
	.if eax != ERAR_END_ARCHIVE
		jmp getunpsize
	.endif
	invoke RARCloseArchive, hArchive
    
    invoke DialogBoxParam,hInstance,200,0,ADDR WndProc,0
		
the_end:
	invoke ExitProcess, 0

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

WndProc proc hWin   :DWORD,
             uMsg   :DWORD,
             wParam :DWORD,
             lParam :DWORD

	.if uMsg == WM_INITDIALOG
		push hWin
		pop hWnd
		invoke LoadIcon,hInstance,500
        mov hIcon, eax
        invoke SendMessage,hWin,WM_SETICON,1,hIcon
		;mov eax, 1000
		;shl eax,16					;create a high and low word
		;invoke SendDlgItemMessage, hWin, 1001, PBM_SETRANGE,0,eax

	.elseif uMsg == WM_COMMAND
		mov eax, wParam

		.if eax == IDOK					;LAUNCH
			invoke CreateThread,NULL,NULL,ADDR ThreadProc,NULL,NORMAL_PRIORITY_CLASS,ADDR ThreadID

		.elseif eax == IDCANCEL				;QUIT
			invoke EndDialog,hWin,0

		.endif

	.elseif uMsg == WM_CLOSE
		invoke EndDialog,hWin,0

	.endif
	xor eax,eax
    ret

WndProc endp

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ThreadProc PROC RetVal:DWORD
	invoke SendDlgItemMessage, hWnd, 1002, PBM_SETPOS,eax,0
	mov ArchiveData.lpArcName, OFFSET ArcName
	mov ArchiveData.OpenMode, RAR_OM_EXTRACT
	invoke RAROpenArchive, ADDR ArchiveData
	mov hArchive, eax

	invoke RARSetCallback, hArchive, offset RARCallbackProc, 0
_extract:	
	invoke RARReadHeader, hArchive, ADDR HeaderData
    .if eax == ERAR_END_ARCHIVE
        jmp extract_
    .endif 
	invoke SendDlgItemMessage, hWnd, 302, WM_SETTEXT,0, ADDR HeaderData.ArcName
	invoke RARProcessFile, hArchive, RAR_EXTRACT, ADDR DestPath, NULL
    jmp _extract
extract_:
	invoke RARCloseArchive, hArchive
	ret
ThreadProc endp

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

RARCallbackProc	PROC msg:DWORD, userData:DWORD, p1:DWORD, p2:DWORD
;  RAR callback proc for data/volume processing

    .if msg == UCM_CHANGEVOLUME
        .if p2 == RAR_VOL_NOTIFY
            invoke SendDlgItemMessage, hWnd, 302, WM_SETTEXT,0, p1
            mov eax, 1
            ret
        .elseif p2 == RAR_VOL_ASK
            invoke MessageBox,0, ADDR NextVolQuery, addr caption, MB_OK
            mov eax, -1
            ret
        .endif

    .elseif msg == UCM_PROCESSDATA
;;        .if abort_thread == 1  ;has thread been aborted?
;;            return -1
;;        .endif
      ;check if processing file has changed
        mov eax, FileCRC
        .if HeaderData.FileCRC != eax ;new file
            mov eax, p2         ;n of extracted bytes
            mov bExtracted, eax
            mov eax, HeaderData.FileCRC
            mov FileCRC, eax
            invoke SendDlgItemMessage, hWnd, 301, WM_SETTEXT,0, ADDR HeaderData.FileName
        .else                   ;still on the same file
            mov eax, p2
            add bExtracted,eax
        .endif

        .if HeaderData.UnpSize > eax
          ;get percent of current extraction
            finit               ;init fpu
            fild bExtracted     ;load nBytesExtracted on TOS
            mov dummy_dd, 100
            fimul dummy_dd
            fidiv HeaderData.UnpSize			;divides both
            fistp dummy_dd		;get integer from TOS
          ;--same without floating point--
            ;mov eax, nBytesExtracted
            ;mov ebx, 100
            ;mul ebx ;result in edx:eax
            ;div HeaderData.UnpSize ;source=edx:eax, result in eax
            ;mov dummy_dd,eax
          ;----------------------------
        .else
            mov eax,100
            mov dummy_dd,eax
        .endif
	    invoke SendDlgItemMessage, hWnd, 1001, PBM_SETPOS,dummy_dd,0
	    invoke wsprintf, ADDR buffer, ADDR FormatStr, bExtracted, HeaderData.UnpSize
	    invoke SendDlgItemMessage, hWnd, 303, WM_SETTEXT,0, ADDR buffer
	    	    
      ;get overall progress percentage
        mov eax, fbExtracted
        add eax, p2         ;add n of extracted bytes
        mov fbExtracted,eax
        finit               ;init fpu
        fld fbExtracted     ;load again
        mov dummy_dd,100
        fimul dummy_dd      ;mul tos by 100
        fld unpSize         ;load max extracted size
        fdivp ST(1),ST(0)   ;divide to get percentage
        fistp dummy_dd      ;save value
        invoke SendDlgItemMessage, hWnd, 1002, PBM_SETPOS,dummy_dd,0
	    invoke wsprintf, ADDR buffer, ADDR FormatStr, fbExtracted, unpSize
	    invoke SendDlgItemMessage, hWnd, 304, WM_SETTEXT,0, ADDR buffer
    	mov eax, 1
    	ret

    .elseif msg == UCM_NEEDPASSWORD
;;        invoke lstrcpy,p1,addr pass
    .endif
    xor eax,eax
    ret

RARCallbackProc ENDP

; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

end start