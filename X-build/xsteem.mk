##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=xsteem
ConfigurationName      :=Debug
WorkspacePath          := "/home/user/Documents/ST/X-build"
ProjectPath            := "/home/user/Documents/ST/X-build"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=user
Date                   :=04/04/20
CodeLitePath           :="/home/user/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)UNIX $(PreprocessorSwitch)LINUX $(PreprocessorSwitch)SSE_DEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="xsteem.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../steem/headers $(IncludeSwitch)../steem/code $(IncludeSwitch)../steem/code/x $(IncludeSwitch)../include $(IncludeSwitch)../3rdparty $(IncludeSwitch)../3rdparty/zlib/contrib/minizip $(IncludeSwitch)../3rdparty/zlib $(IncludeSwitch)/home/user/Documents/ST/steem/asm/ $(IncludeSwitch)/home/user/Documents/ST/steem/ 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)X11 $(LibrarySwitch)Xext $(LibrarySwitch)pthread $(LibrarySwitch)Xxf86vm $(LibrarySwitch)asound $(LibrarySwitch)portaudio $(LibrarySwitch)capsimage 
ArLibs                 :=  "X11" "Xext" "pthread" "Xxf86vm" "libasound" "portaudio" "capsimage" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)usr/local/lib $(LibraryPathSwitch)usr/lib/i386-linux-gnu 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -Wall -Wno-unknown-pragmas $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/nasm


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/steem_acia.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_acc.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_archive.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_associate.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_blitter.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_computer.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_cpu.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_cpu_ea.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_cpuinit.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_cpu_op.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_d2.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_dataloadsave.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_debug.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_debug_emu.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_debug_framereport.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_debugger.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_debugger_trace.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_dir_id.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_disk_dma.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_disk_ghost.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_disk_hfe.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_diskman.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_disk_scp.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_disk_stw.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_display.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_draw.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_dwin_edit.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_emulator.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_fdc.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_floppy_disk.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_floppy_drive.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_glue.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_gui.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_gui_controls.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_harddiskman.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_hd_acsi.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_hd_gemdos.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_historylist.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_ikbd.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_infobox.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_interface_caps.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_interface_stvl.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_iolist.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_ior.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_iow.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_key_table.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_loadsave.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_loadsave_emu.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_macros.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_main.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_mem_browser.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_mfp.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_midi.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_mmu.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_mr_static.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_notifyinit.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_options.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_options_create.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_osd.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_palette.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_patchesbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_psg.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_reset.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_rs232.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_run.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_screen_saver.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_shifter.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_shortcutbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_sound.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_Steem.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/steem_steemintro.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_stemdialogs.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_stemwin.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_stjoy.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_stports.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_translate.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_diskman_diags.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_interface_pa.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_interface_rta.cpp$(ObjectSuffix) $(IntermediateDirectory)/steem_tos.cpp$(ObjectSuffix) \
	

Objects1=$(IntermediateDirectory)/asm_asm_draw.asm$(ObjectSuffix) $(IntermediateDirectory)/asm_asm_osd_draw.asm$(ObjectSuffix) $(IntermediateDirectory)/rc_resource.asm$(ObjectSuffix) $(IntermediateDirectory)/include_circularbuffer.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_configstorefile.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_di_get_contents.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_dynamicarray.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_easycompress.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_easystr.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_easystringlist.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/include_mymisc.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_notwin_mymisc.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_portio.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_wordwrapper.cpp$(ObjectSuffix) $(IntermediateDirectory)/include_dirsearch.cpp$(ObjectSuffix) $(IntermediateDirectory)/6301_6301.c$(ObjectSuffix) $(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(ObjectSuffix) $(IntermediateDirectory)/dsp_dsp.cpp$(ObjectSuffix) $(IntermediateDirectory)/FIR-filter-class_filt.cpp$(ObjectSuffix) 

Objects2=$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/zlib_adler32.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_compress.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_crc32.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_deflate.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_inffast.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_inflate.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_inftrees.c$(ObjectSuffix) $(IntermediateDirectory)/zlib_zutil.c$(ObjectSuffix) $(IntermediateDirectory)/minizip_ioapi.c$(ObjectSuffix) $(IntermediateDirectory)/minizip_unzip.c$(ObjectSuffix) \
	$(IntermediateDirectory)/zlib_trees.c$(ObjectSuffix) $(IntermediateDirectory)/caps_CapsPlug.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_alert.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_dir_lv.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_fileselect.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_popup.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_popuphints.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_hxc_prompt.cpp$(ObjectSuffix) $(IntermediateDirectory)/x_x_mymisc.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/x_x_portio.cpp$(ObjectSuffix) 



Objects=$(Objects0) $(Objects1) $(Objects2) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	@echo $(Objects2) >> $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/steem_acia.cpp$(ObjectSuffix): ../steem/acia.cpp $(IntermediateDirectory)/steem_acia.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/acia.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_acia.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_acia.cpp$(DependSuffix): ../steem/acia.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_acia.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_acia.cpp$(DependSuffix) -MM "../steem/acia.cpp"

$(IntermediateDirectory)/steem_acia.cpp$(PreprocessSuffix): ../steem/acia.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_acia.cpp$(PreprocessSuffix) "../steem/acia.cpp"

$(IntermediateDirectory)/steem_acc.cpp$(ObjectSuffix): ../steem/acc.cpp $(IntermediateDirectory)/steem_acc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/acc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_acc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_acc.cpp$(DependSuffix): ../steem/acc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_acc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_acc.cpp$(DependSuffix) -MM "../steem/acc.cpp"

$(IntermediateDirectory)/steem_acc.cpp$(PreprocessSuffix): ../steem/acc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_acc.cpp$(PreprocessSuffix) "../steem/acc.cpp"

$(IntermediateDirectory)/steem_archive.cpp$(ObjectSuffix): ../steem/archive.cpp $(IntermediateDirectory)/steem_archive.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/archive.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_archive.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_archive.cpp$(DependSuffix): ../steem/archive.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_archive.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_archive.cpp$(DependSuffix) -MM "../steem/archive.cpp"

$(IntermediateDirectory)/steem_archive.cpp$(PreprocessSuffix): ../steem/archive.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_archive.cpp$(PreprocessSuffix) "../steem/archive.cpp"

$(IntermediateDirectory)/steem_associate.cpp$(ObjectSuffix): ../steem/associate.cpp $(IntermediateDirectory)/steem_associate.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/associate.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_associate.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_associate.cpp$(DependSuffix): ../steem/associate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_associate.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_associate.cpp$(DependSuffix) -MM "../steem/associate.cpp"

$(IntermediateDirectory)/steem_associate.cpp$(PreprocessSuffix): ../steem/associate.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_associate.cpp$(PreprocessSuffix) "../steem/associate.cpp"

$(IntermediateDirectory)/steem_blitter.cpp$(ObjectSuffix): ../steem/blitter.cpp $(IntermediateDirectory)/steem_blitter.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/blitter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_blitter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_blitter.cpp$(DependSuffix): ../steem/blitter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_blitter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_blitter.cpp$(DependSuffix) -MM "../steem/blitter.cpp"

$(IntermediateDirectory)/steem_blitter.cpp$(PreprocessSuffix): ../steem/blitter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_blitter.cpp$(PreprocessSuffix) "../steem/blitter.cpp"

$(IntermediateDirectory)/steem_computer.cpp$(ObjectSuffix): ../steem/computer.cpp $(IntermediateDirectory)/steem_computer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/computer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_computer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_computer.cpp$(DependSuffix): ../steem/computer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_computer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_computer.cpp$(DependSuffix) -MM "../steem/computer.cpp"

$(IntermediateDirectory)/steem_computer.cpp$(PreprocessSuffix): ../steem/computer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_computer.cpp$(PreprocessSuffix) "../steem/computer.cpp"

$(IntermediateDirectory)/steem_cpu.cpp$(ObjectSuffix): ../steem/cpu.cpp $(IntermediateDirectory)/steem_cpu.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/cpu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_cpu.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_cpu.cpp$(DependSuffix): ../steem/cpu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_cpu.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_cpu.cpp$(DependSuffix) -MM "../steem/cpu.cpp"

$(IntermediateDirectory)/steem_cpu.cpp$(PreprocessSuffix): ../steem/cpu.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_cpu.cpp$(PreprocessSuffix) "../steem/cpu.cpp"

$(IntermediateDirectory)/steem_cpu_ea.cpp$(ObjectSuffix): ../steem/cpu_ea.cpp $(IntermediateDirectory)/steem_cpu_ea.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/cpu_ea.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_cpu_ea.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_cpu_ea.cpp$(DependSuffix): ../steem/cpu_ea.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_cpu_ea.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_cpu_ea.cpp$(DependSuffix) -MM "../steem/cpu_ea.cpp"

$(IntermediateDirectory)/steem_cpu_ea.cpp$(PreprocessSuffix): ../steem/cpu_ea.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_cpu_ea.cpp$(PreprocessSuffix) "../steem/cpu_ea.cpp"

$(IntermediateDirectory)/steem_cpuinit.cpp$(ObjectSuffix): ../steem/cpuinit.cpp $(IntermediateDirectory)/steem_cpuinit.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/cpuinit.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_cpuinit.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_cpuinit.cpp$(DependSuffix): ../steem/cpuinit.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_cpuinit.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_cpuinit.cpp$(DependSuffix) -MM "../steem/cpuinit.cpp"

$(IntermediateDirectory)/steem_cpuinit.cpp$(PreprocessSuffix): ../steem/cpuinit.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_cpuinit.cpp$(PreprocessSuffix) "../steem/cpuinit.cpp"

$(IntermediateDirectory)/steem_cpu_op.cpp$(ObjectSuffix): ../steem/cpu_op.cpp $(IntermediateDirectory)/steem_cpu_op.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/cpu_op.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_cpu_op.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_cpu_op.cpp$(DependSuffix): ../steem/cpu_op.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_cpu_op.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_cpu_op.cpp$(DependSuffix) -MM "../steem/cpu_op.cpp"

$(IntermediateDirectory)/steem_cpu_op.cpp$(PreprocessSuffix): ../steem/cpu_op.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_cpu_op.cpp$(PreprocessSuffix) "../steem/cpu_op.cpp"

$(IntermediateDirectory)/steem_d2.cpp$(ObjectSuffix): ../steem/d2.cpp $(IntermediateDirectory)/steem_d2.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/d2.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_d2.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_d2.cpp$(DependSuffix): ../steem/d2.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_d2.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_d2.cpp$(DependSuffix) -MM "../steem/d2.cpp"

$(IntermediateDirectory)/steem_d2.cpp$(PreprocessSuffix): ../steem/d2.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_d2.cpp$(PreprocessSuffix) "../steem/d2.cpp"

$(IntermediateDirectory)/steem_dataloadsave.cpp$(ObjectSuffix): ../steem/dataloadsave.cpp $(IntermediateDirectory)/steem_dataloadsave.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/dataloadsave.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_dataloadsave.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_dataloadsave.cpp$(DependSuffix): ../steem/dataloadsave.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_dataloadsave.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_dataloadsave.cpp$(DependSuffix) -MM "../steem/dataloadsave.cpp"

$(IntermediateDirectory)/steem_dataloadsave.cpp$(PreprocessSuffix): ../steem/dataloadsave.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_dataloadsave.cpp$(PreprocessSuffix) "../steem/dataloadsave.cpp"

$(IntermediateDirectory)/steem_debug.cpp$(ObjectSuffix): ../steem/debug.cpp $(IntermediateDirectory)/steem_debug.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/debug.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_debug.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_debug.cpp$(DependSuffix): ../steem/debug.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_debug.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_debug.cpp$(DependSuffix) -MM "../steem/debug.cpp"

$(IntermediateDirectory)/steem_debug.cpp$(PreprocessSuffix): ../steem/debug.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_debug.cpp$(PreprocessSuffix) "../steem/debug.cpp"

$(IntermediateDirectory)/steem_debug_emu.cpp$(ObjectSuffix): ../steem/debug_emu.cpp $(IntermediateDirectory)/steem_debug_emu.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/debug_emu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_debug_emu.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_debug_emu.cpp$(DependSuffix): ../steem/debug_emu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_debug_emu.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_debug_emu.cpp$(DependSuffix) -MM "../steem/debug_emu.cpp"

$(IntermediateDirectory)/steem_debug_emu.cpp$(PreprocessSuffix): ../steem/debug_emu.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_debug_emu.cpp$(PreprocessSuffix) "../steem/debug_emu.cpp"

$(IntermediateDirectory)/steem_debug_framereport.cpp$(ObjectSuffix): ../steem/debug_framereport.cpp $(IntermediateDirectory)/steem_debug_framereport.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/debug_framereport.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_debug_framereport.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_debug_framereport.cpp$(DependSuffix): ../steem/debug_framereport.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_debug_framereport.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_debug_framereport.cpp$(DependSuffix) -MM "../steem/debug_framereport.cpp"

$(IntermediateDirectory)/steem_debug_framereport.cpp$(PreprocessSuffix): ../steem/debug_framereport.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_debug_framereport.cpp$(PreprocessSuffix) "../steem/debug_framereport.cpp"

$(IntermediateDirectory)/steem_debugger.cpp$(ObjectSuffix): ../steem/debugger.cpp $(IntermediateDirectory)/steem_debugger.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/debugger.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_debugger.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_debugger.cpp$(DependSuffix): ../steem/debugger.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_debugger.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_debugger.cpp$(DependSuffix) -MM "../steem/debugger.cpp"

$(IntermediateDirectory)/steem_debugger.cpp$(PreprocessSuffix): ../steem/debugger.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_debugger.cpp$(PreprocessSuffix) "../steem/debugger.cpp"

$(IntermediateDirectory)/steem_debugger_trace.cpp$(ObjectSuffix): ../steem/debugger_trace.cpp $(IntermediateDirectory)/steem_debugger_trace.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/debugger_trace.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_debugger_trace.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_debugger_trace.cpp$(DependSuffix): ../steem/debugger_trace.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_debugger_trace.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_debugger_trace.cpp$(DependSuffix) -MM "../steem/debugger_trace.cpp"

$(IntermediateDirectory)/steem_debugger_trace.cpp$(PreprocessSuffix): ../steem/debugger_trace.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_debugger_trace.cpp$(PreprocessSuffix) "../steem/debugger_trace.cpp"

$(IntermediateDirectory)/steem_dir_id.cpp$(ObjectSuffix): ../steem/dir_id.cpp $(IntermediateDirectory)/steem_dir_id.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/dir_id.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_dir_id.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_dir_id.cpp$(DependSuffix): ../steem/dir_id.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_dir_id.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_dir_id.cpp$(DependSuffix) -MM "../steem/dir_id.cpp"

$(IntermediateDirectory)/steem_dir_id.cpp$(PreprocessSuffix): ../steem/dir_id.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_dir_id.cpp$(PreprocessSuffix) "../steem/dir_id.cpp"

$(IntermediateDirectory)/steem_disk_dma.cpp$(ObjectSuffix): ../steem/disk_dma.cpp $(IntermediateDirectory)/steem_disk_dma.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/disk_dma.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_disk_dma.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_disk_dma.cpp$(DependSuffix): ../steem/disk_dma.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_disk_dma.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_disk_dma.cpp$(DependSuffix) -MM "../steem/disk_dma.cpp"

$(IntermediateDirectory)/steem_disk_dma.cpp$(PreprocessSuffix): ../steem/disk_dma.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_disk_dma.cpp$(PreprocessSuffix) "../steem/disk_dma.cpp"

$(IntermediateDirectory)/steem_disk_ghost.cpp$(ObjectSuffix): ../steem/disk_ghost.cpp $(IntermediateDirectory)/steem_disk_ghost.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/disk_ghost.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_disk_ghost.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_disk_ghost.cpp$(DependSuffix): ../steem/disk_ghost.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_disk_ghost.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_disk_ghost.cpp$(DependSuffix) -MM "../steem/disk_ghost.cpp"

$(IntermediateDirectory)/steem_disk_ghost.cpp$(PreprocessSuffix): ../steem/disk_ghost.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_disk_ghost.cpp$(PreprocessSuffix) "../steem/disk_ghost.cpp"

$(IntermediateDirectory)/steem_disk_hfe.cpp$(ObjectSuffix): ../steem/disk_hfe.cpp $(IntermediateDirectory)/steem_disk_hfe.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/disk_hfe.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_disk_hfe.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_disk_hfe.cpp$(DependSuffix): ../steem/disk_hfe.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_disk_hfe.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_disk_hfe.cpp$(DependSuffix) -MM "../steem/disk_hfe.cpp"

$(IntermediateDirectory)/steem_disk_hfe.cpp$(PreprocessSuffix): ../steem/disk_hfe.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_disk_hfe.cpp$(PreprocessSuffix) "../steem/disk_hfe.cpp"

$(IntermediateDirectory)/steem_diskman.cpp$(ObjectSuffix): ../steem/diskman.cpp $(IntermediateDirectory)/steem_diskman.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/diskman.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_diskman.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_diskman.cpp$(DependSuffix): ../steem/diskman.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_diskman.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_diskman.cpp$(DependSuffix) -MM "../steem/diskman.cpp"

$(IntermediateDirectory)/steem_diskman.cpp$(PreprocessSuffix): ../steem/diskman.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_diskman.cpp$(PreprocessSuffix) "../steem/diskman.cpp"

$(IntermediateDirectory)/steem_disk_scp.cpp$(ObjectSuffix): ../steem/disk_scp.cpp $(IntermediateDirectory)/steem_disk_scp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/disk_scp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_disk_scp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_disk_scp.cpp$(DependSuffix): ../steem/disk_scp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_disk_scp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_disk_scp.cpp$(DependSuffix) -MM "../steem/disk_scp.cpp"

$(IntermediateDirectory)/steem_disk_scp.cpp$(PreprocessSuffix): ../steem/disk_scp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_disk_scp.cpp$(PreprocessSuffix) "../steem/disk_scp.cpp"

$(IntermediateDirectory)/steem_disk_stw.cpp$(ObjectSuffix): ../steem/disk_stw.cpp $(IntermediateDirectory)/steem_disk_stw.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/disk_stw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_disk_stw.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_disk_stw.cpp$(DependSuffix): ../steem/disk_stw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_disk_stw.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_disk_stw.cpp$(DependSuffix) -MM "../steem/disk_stw.cpp"

$(IntermediateDirectory)/steem_disk_stw.cpp$(PreprocessSuffix): ../steem/disk_stw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_disk_stw.cpp$(PreprocessSuffix) "../steem/disk_stw.cpp"

$(IntermediateDirectory)/steem_display.cpp$(ObjectSuffix): ../steem/display.cpp $(IntermediateDirectory)/steem_display.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/display.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_display.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_display.cpp$(DependSuffix): ../steem/display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_display.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_display.cpp$(DependSuffix) -MM "../steem/display.cpp"

$(IntermediateDirectory)/steem_display.cpp$(PreprocessSuffix): ../steem/display.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_display.cpp$(PreprocessSuffix) "../steem/display.cpp"

$(IntermediateDirectory)/steem_draw.cpp$(ObjectSuffix): ../steem/draw.cpp $(IntermediateDirectory)/steem_draw.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/draw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_draw.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_draw.cpp$(DependSuffix): ../steem/draw.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_draw.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_draw.cpp$(DependSuffix) -MM "../steem/draw.cpp"

$(IntermediateDirectory)/steem_draw.cpp$(PreprocessSuffix): ../steem/draw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_draw.cpp$(PreprocessSuffix) "../steem/draw.cpp"

$(IntermediateDirectory)/steem_dwin_edit.cpp$(ObjectSuffix): ../steem/dwin_edit.cpp $(IntermediateDirectory)/steem_dwin_edit.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/dwin_edit.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_dwin_edit.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_dwin_edit.cpp$(DependSuffix): ../steem/dwin_edit.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_dwin_edit.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_dwin_edit.cpp$(DependSuffix) -MM "../steem/dwin_edit.cpp"

$(IntermediateDirectory)/steem_dwin_edit.cpp$(PreprocessSuffix): ../steem/dwin_edit.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_dwin_edit.cpp$(PreprocessSuffix) "../steem/dwin_edit.cpp"

$(IntermediateDirectory)/steem_emulator.cpp$(ObjectSuffix): ../steem/emulator.cpp $(IntermediateDirectory)/steem_emulator.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/emulator.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_emulator.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_emulator.cpp$(DependSuffix): ../steem/emulator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_emulator.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_emulator.cpp$(DependSuffix) -MM "../steem/emulator.cpp"

$(IntermediateDirectory)/steem_emulator.cpp$(PreprocessSuffix): ../steem/emulator.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_emulator.cpp$(PreprocessSuffix) "../steem/emulator.cpp"

$(IntermediateDirectory)/steem_fdc.cpp$(ObjectSuffix): ../steem/fdc.cpp $(IntermediateDirectory)/steem_fdc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/fdc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_fdc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_fdc.cpp$(DependSuffix): ../steem/fdc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_fdc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_fdc.cpp$(DependSuffix) -MM "../steem/fdc.cpp"

$(IntermediateDirectory)/steem_fdc.cpp$(PreprocessSuffix): ../steem/fdc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_fdc.cpp$(PreprocessSuffix) "../steem/fdc.cpp"

$(IntermediateDirectory)/steem_floppy_disk.cpp$(ObjectSuffix): ../steem/floppy_disk.cpp $(IntermediateDirectory)/steem_floppy_disk.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/floppy_disk.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_floppy_disk.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_floppy_disk.cpp$(DependSuffix): ../steem/floppy_disk.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_floppy_disk.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_floppy_disk.cpp$(DependSuffix) -MM "../steem/floppy_disk.cpp"

$(IntermediateDirectory)/steem_floppy_disk.cpp$(PreprocessSuffix): ../steem/floppy_disk.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_floppy_disk.cpp$(PreprocessSuffix) "../steem/floppy_disk.cpp"

$(IntermediateDirectory)/steem_floppy_drive.cpp$(ObjectSuffix): ../steem/floppy_drive.cpp $(IntermediateDirectory)/steem_floppy_drive.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/floppy_drive.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_floppy_drive.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_floppy_drive.cpp$(DependSuffix): ../steem/floppy_drive.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_floppy_drive.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_floppy_drive.cpp$(DependSuffix) -MM "../steem/floppy_drive.cpp"

$(IntermediateDirectory)/steem_floppy_drive.cpp$(PreprocessSuffix): ../steem/floppy_drive.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_floppy_drive.cpp$(PreprocessSuffix) "../steem/floppy_drive.cpp"

$(IntermediateDirectory)/steem_glue.cpp$(ObjectSuffix): ../steem/glue.cpp $(IntermediateDirectory)/steem_glue.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/glue.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_glue.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_glue.cpp$(DependSuffix): ../steem/glue.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_glue.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_glue.cpp$(DependSuffix) -MM "../steem/glue.cpp"

$(IntermediateDirectory)/steem_glue.cpp$(PreprocessSuffix): ../steem/glue.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_glue.cpp$(PreprocessSuffix) "../steem/glue.cpp"

$(IntermediateDirectory)/steem_gui.cpp$(ObjectSuffix): ../steem/gui.cpp $(IntermediateDirectory)/steem_gui.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/gui.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_gui.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_gui.cpp$(DependSuffix): ../steem/gui.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_gui.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_gui.cpp$(DependSuffix) -MM "../steem/gui.cpp"

$(IntermediateDirectory)/steem_gui.cpp$(PreprocessSuffix): ../steem/gui.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_gui.cpp$(PreprocessSuffix) "../steem/gui.cpp"

$(IntermediateDirectory)/steem_gui_controls.cpp$(ObjectSuffix): ../steem/gui_controls.cpp $(IntermediateDirectory)/steem_gui_controls.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/gui_controls.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_gui_controls.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_gui_controls.cpp$(DependSuffix): ../steem/gui_controls.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_gui_controls.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_gui_controls.cpp$(DependSuffix) -MM "../steem/gui_controls.cpp"

$(IntermediateDirectory)/steem_gui_controls.cpp$(PreprocessSuffix): ../steem/gui_controls.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_gui_controls.cpp$(PreprocessSuffix) "../steem/gui_controls.cpp"

$(IntermediateDirectory)/steem_harddiskman.cpp$(ObjectSuffix): ../steem/harddiskman.cpp $(IntermediateDirectory)/steem_harddiskman.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/harddiskman.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_harddiskman.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_harddiskman.cpp$(DependSuffix): ../steem/harddiskman.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_harddiskman.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_harddiskman.cpp$(DependSuffix) -MM "../steem/harddiskman.cpp"

$(IntermediateDirectory)/steem_harddiskman.cpp$(PreprocessSuffix): ../steem/harddiskman.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_harddiskman.cpp$(PreprocessSuffix) "../steem/harddiskman.cpp"

$(IntermediateDirectory)/steem_hd_acsi.cpp$(ObjectSuffix): ../steem/hd_acsi.cpp $(IntermediateDirectory)/steem_hd_acsi.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/hd_acsi.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_hd_acsi.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_hd_acsi.cpp$(DependSuffix): ../steem/hd_acsi.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_hd_acsi.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_hd_acsi.cpp$(DependSuffix) -MM "../steem/hd_acsi.cpp"

$(IntermediateDirectory)/steem_hd_acsi.cpp$(PreprocessSuffix): ../steem/hd_acsi.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_hd_acsi.cpp$(PreprocessSuffix) "../steem/hd_acsi.cpp"

$(IntermediateDirectory)/steem_hd_gemdos.cpp$(ObjectSuffix): ../steem/hd_gemdos.cpp $(IntermediateDirectory)/steem_hd_gemdos.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/hd_gemdos.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_hd_gemdos.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_hd_gemdos.cpp$(DependSuffix): ../steem/hd_gemdos.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_hd_gemdos.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_hd_gemdos.cpp$(DependSuffix) -MM "../steem/hd_gemdos.cpp"

$(IntermediateDirectory)/steem_hd_gemdos.cpp$(PreprocessSuffix): ../steem/hd_gemdos.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_hd_gemdos.cpp$(PreprocessSuffix) "../steem/hd_gemdos.cpp"

$(IntermediateDirectory)/steem_historylist.cpp$(ObjectSuffix): ../steem/historylist.cpp $(IntermediateDirectory)/steem_historylist.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/historylist.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_historylist.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_historylist.cpp$(DependSuffix): ../steem/historylist.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_historylist.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_historylist.cpp$(DependSuffix) -MM "../steem/historylist.cpp"

$(IntermediateDirectory)/steem_historylist.cpp$(PreprocessSuffix): ../steem/historylist.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_historylist.cpp$(PreprocessSuffix) "../steem/historylist.cpp"

$(IntermediateDirectory)/steem_ikbd.cpp$(ObjectSuffix): ../steem/ikbd.cpp $(IntermediateDirectory)/steem_ikbd.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/ikbd.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_ikbd.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_ikbd.cpp$(DependSuffix): ../steem/ikbd.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_ikbd.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_ikbd.cpp$(DependSuffix) -MM "../steem/ikbd.cpp"

$(IntermediateDirectory)/steem_ikbd.cpp$(PreprocessSuffix): ../steem/ikbd.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_ikbd.cpp$(PreprocessSuffix) "../steem/ikbd.cpp"

$(IntermediateDirectory)/steem_infobox.cpp$(ObjectSuffix): ../steem/infobox.cpp $(IntermediateDirectory)/steem_infobox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/infobox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_infobox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_infobox.cpp$(DependSuffix): ../steem/infobox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_infobox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_infobox.cpp$(DependSuffix) -MM "../steem/infobox.cpp"

$(IntermediateDirectory)/steem_infobox.cpp$(PreprocessSuffix): ../steem/infobox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_infobox.cpp$(PreprocessSuffix) "../steem/infobox.cpp"

$(IntermediateDirectory)/steem_interface_caps.cpp$(ObjectSuffix): ../steem/interface_caps.cpp $(IntermediateDirectory)/steem_interface_caps.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/interface_caps.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_interface_caps.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_interface_caps.cpp$(DependSuffix): ../steem/interface_caps.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_interface_caps.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_interface_caps.cpp$(DependSuffix) -MM "../steem/interface_caps.cpp"

$(IntermediateDirectory)/steem_interface_caps.cpp$(PreprocessSuffix): ../steem/interface_caps.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_interface_caps.cpp$(PreprocessSuffix) "../steem/interface_caps.cpp"

$(IntermediateDirectory)/steem_interface_stvl.cpp$(ObjectSuffix): ../steem/interface_stvl.cpp $(IntermediateDirectory)/steem_interface_stvl.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/interface_stvl.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_interface_stvl.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_interface_stvl.cpp$(DependSuffix): ../steem/interface_stvl.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_interface_stvl.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_interface_stvl.cpp$(DependSuffix) -MM "../steem/interface_stvl.cpp"

$(IntermediateDirectory)/steem_interface_stvl.cpp$(PreprocessSuffix): ../steem/interface_stvl.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_interface_stvl.cpp$(PreprocessSuffix) "../steem/interface_stvl.cpp"

$(IntermediateDirectory)/steem_iolist.cpp$(ObjectSuffix): ../steem/iolist.cpp $(IntermediateDirectory)/steem_iolist.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/iolist.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_iolist.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_iolist.cpp$(DependSuffix): ../steem/iolist.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_iolist.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_iolist.cpp$(DependSuffix) -MM "../steem/iolist.cpp"

$(IntermediateDirectory)/steem_iolist.cpp$(PreprocessSuffix): ../steem/iolist.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_iolist.cpp$(PreprocessSuffix) "../steem/iolist.cpp"

$(IntermediateDirectory)/steem_ior.cpp$(ObjectSuffix): ../steem/ior.cpp $(IntermediateDirectory)/steem_ior.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/ior.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_ior.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_ior.cpp$(DependSuffix): ../steem/ior.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_ior.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_ior.cpp$(DependSuffix) -MM "../steem/ior.cpp"

$(IntermediateDirectory)/steem_ior.cpp$(PreprocessSuffix): ../steem/ior.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_ior.cpp$(PreprocessSuffix) "../steem/ior.cpp"

$(IntermediateDirectory)/steem_iow.cpp$(ObjectSuffix): ../steem/iow.cpp $(IntermediateDirectory)/steem_iow.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/iow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_iow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_iow.cpp$(DependSuffix): ../steem/iow.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_iow.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_iow.cpp$(DependSuffix) -MM "../steem/iow.cpp"

$(IntermediateDirectory)/steem_iow.cpp$(PreprocessSuffix): ../steem/iow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_iow.cpp$(PreprocessSuffix) "../steem/iow.cpp"

$(IntermediateDirectory)/steem_key_table.cpp$(ObjectSuffix): ../steem/key_table.cpp $(IntermediateDirectory)/steem_key_table.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/key_table.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_key_table.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_key_table.cpp$(DependSuffix): ../steem/key_table.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_key_table.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_key_table.cpp$(DependSuffix) -MM "../steem/key_table.cpp"

$(IntermediateDirectory)/steem_key_table.cpp$(PreprocessSuffix): ../steem/key_table.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_key_table.cpp$(PreprocessSuffix) "../steem/key_table.cpp"

$(IntermediateDirectory)/steem_loadsave.cpp$(ObjectSuffix): ../steem/loadsave.cpp $(IntermediateDirectory)/steem_loadsave.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/loadsave.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_loadsave.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_loadsave.cpp$(DependSuffix): ../steem/loadsave.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_loadsave.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_loadsave.cpp$(DependSuffix) -MM "../steem/loadsave.cpp"

$(IntermediateDirectory)/steem_loadsave.cpp$(PreprocessSuffix): ../steem/loadsave.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_loadsave.cpp$(PreprocessSuffix) "../steem/loadsave.cpp"

$(IntermediateDirectory)/steem_loadsave_emu.cpp$(ObjectSuffix): ../steem/loadsave_emu.cpp $(IntermediateDirectory)/steem_loadsave_emu.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/loadsave_emu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_loadsave_emu.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_loadsave_emu.cpp$(DependSuffix): ../steem/loadsave_emu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_loadsave_emu.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_loadsave_emu.cpp$(DependSuffix) -MM "../steem/loadsave_emu.cpp"

$(IntermediateDirectory)/steem_loadsave_emu.cpp$(PreprocessSuffix): ../steem/loadsave_emu.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_loadsave_emu.cpp$(PreprocessSuffix) "../steem/loadsave_emu.cpp"

$(IntermediateDirectory)/steem_macros.cpp$(ObjectSuffix): ../steem/macros.cpp $(IntermediateDirectory)/steem_macros.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/macros.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_macros.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_macros.cpp$(DependSuffix): ../steem/macros.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_macros.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_macros.cpp$(DependSuffix) -MM "../steem/macros.cpp"

$(IntermediateDirectory)/steem_macros.cpp$(PreprocessSuffix): ../steem/macros.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_macros.cpp$(PreprocessSuffix) "../steem/macros.cpp"

$(IntermediateDirectory)/steem_main.cpp$(ObjectSuffix): ../steem/main.cpp $(IntermediateDirectory)/steem_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_main.cpp$(DependSuffix): ../steem/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_main.cpp$(DependSuffix) -MM "../steem/main.cpp"

$(IntermediateDirectory)/steem_main.cpp$(PreprocessSuffix): ../steem/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_main.cpp$(PreprocessSuffix) "../steem/main.cpp"

$(IntermediateDirectory)/steem_mem_browser.cpp$(ObjectSuffix): ../steem/mem_browser.cpp $(IntermediateDirectory)/steem_mem_browser.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/mem_browser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_mem_browser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_mem_browser.cpp$(DependSuffix): ../steem/mem_browser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_mem_browser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_mem_browser.cpp$(DependSuffix) -MM "../steem/mem_browser.cpp"

$(IntermediateDirectory)/steem_mem_browser.cpp$(PreprocessSuffix): ../steem/mem_browser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_mem_browser.cpp$(PreprocessSuffix) "../steem/mem_browser.cpp"

$(IntermediateDirectory)/steem_mfp.cpp$(ObjectSuffix): ../steem/mfp.cpp $(IntermediateDirectory)/steem_mfp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/mfp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_mfp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_mfp.cpp$(DependSuffix): ../steem/mfp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_mfp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_mfp.cpp$(DependSuffix) -MM "../steem/mfp.cpp"

$(IntermediateDirectory)/steem_mfp.cpp$(PreprocessSuffix): ../steem/mfp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_mfp.cpp$(PreprocessSuffix) "../steem/mfp.cpp"

$(IntermediateDirectory)/steem_midi.cpp$(ObjectSuffix): ../steem/midi.cpp $(IntermediateDirectory)/steem_midi.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/midi.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_midi.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_midi.cpp$(DependSuffix): ../steem/midi.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_midi.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_midi.cpp$(DependSuffix) -MM "../steem/midi.cpp"

$(IntermediateDirectory)/steem_midi.cpp$(PreprocessSuffix): ../steem/midi.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_midi.cpp$(PreprocessSuffix) "../steem/midi.cpp"

$(IntermediateDirectory)/steem_mmu.cpp$(ObjectSuffix): ../steem/mmu.cpp $(IntermediateDirectory)/steem_mmu.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/mmu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_mmu.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_mmu.cpp$(DependSuffix): ../steem/mmu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_mmu.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_mmu.cpp$(DependSuffix) -MM "../steem/mmu.cpp"

$(IntermediateDirectory)/steem_mmu.cpp$(PreprocessSuffix): ../steem/mmu.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_mmu.cpp$(PreprocessSuffix) "../steem/mmu.cpp"

$(IntermediateDirectory)/steem_mr_static.cpp$(ObjectSuffix): ../steem/mr_static.cpp $(IntermediateDirectory)/steem_mr_static.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/mr_static.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_mr_static.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_mr_static.cpp$(DependSuffix): ../steem/mr_static.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_mr_static.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_mr_static.cpp$(DependSuffix) -MM "../steem/mr_static.cpp"

$(IntermediateDirectory)/steem_mr_static.cpp$(PreprocessSuffix): ../steem/mr_static.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_mr_static.cpp$(PreprocessSuffix) "../steem/mr_static.cpp"

$(IntermediateDirectory)/steem_notifyinit.cpp$(ObjectSuffix): ../steem/notifyinit.cpp $(IntermediateDirectory)/steem_notifyinit.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/notifyinit.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_notifyinit.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_notifyinit.cpp$(DependSuffix): ../steem/notifyinit.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_notifyinit.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_notifyinit.cpp$(DependSuffix) -MM "../steem/notifyinit.cpp"

$(IntermediateDirectory)/steem_notifyinit.cpp$(PreprocessSuffix): ../steem/notifyinit.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_notifyinit.cpp$(PreprocessSuffix) "../steem/notifyinit.cpp"

$(IntermediateDirectory)/steem_options.cpp$(ObjectSuffix): ../steem/options.cpp $(IntermediateDirectory)/steem_options.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/options.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_options.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_options.cpp$(DependSuffix): ../steem/options.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_options.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_options.cpp$(DependSuffix) -MM "../steem/options.cpp"

$(IntermediateDirectory)/steem_options.cpp$(PreprocessSuffix): ../steem/options.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_options.cpp$(PreprocessSuffix) "../steem/options.cpp"

$(IntermediateDirectory)/steem_options_create.cpp$(ObjectSuffix): ../steem/options_create.cpp $(IntermediateDirectory)/steem_options_create.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/options_create.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_options_create.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_options_create.cpp$(DependSuffix): ../steem/options_create.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_options_create.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_options_create.cpp$(DependSuffix) -MM "../steem/options_create.cpp"

$(IntermediateDirectory)/steem_options_create.cpp$(PreprocessSuffix): ../steem/options_create.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_options_create.cpp$(PreprocessSuffix) "../steem/options_create.cpp"

$(IntermediateDirectory)/steem_osd.cpp$(ObjectSuffix): ../steem/osd.cpp $(IntermediateDirectory)/steem_osd.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/osd.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_osd.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_osd.cpp$(DependSuffix): ../steem/osd.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_osd.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_osd.cpp$(DependSuffix) -MM "../steem/osd.cpp"

$(IntermediateDirectory)/steem_osd.cpp$(PreprocessSuffix): ../steem/osd.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_osd.cpp$(PreprocessSuffix) "../steem/osd.cpp"

$(IntermediateDirectory)/steem_palette.cpp$(ObjectSuffix): ../steem/palette.cpp $(IntermediateDirectory)/steem_palette.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/palette.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_palette.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_palette.cpp$(DependSuffix): ../steem/palette.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_palette.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_palette.cpp$(DependSuffix) -MM "../steem/palette.cpp"

$(IntermediateDirectory)/steem_palette.cpp$(PreprocessSuffix): ../steem/palette.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_palette.cpp$(PreprocessSuffix) "../steem/palette.cpp"

$(IntermediateDirectory)/steem_patchesbox.cpp$(ObjectSuffix): ../steem/patchesbox.cpp $(IntermediateDirectory)/steem_patchesbox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/patchesbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_patchesbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_patchesbox.cpp$(DependSuffix): ../steem/patchesbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_patchesbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_patchesbox.cpp$(DependSuffix) -MM "../steem/patchesbox.cpp"

$(IntermediateDirectory)/steem_patchesbox.cpp$(PreprocessSuffix): ../steem/patchesbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_patchesbox.cpp$(PreprocessSuffix) "../steem/patchesbox.cpp"

$(IntermediateDirectory)/steem_psg.cpp$(ObjectSuffix): ../steem/psg.cpp $(IntermediateDirectory)/steem_psg.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/psg.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_psg.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_psg.cpp$(DependSuffix): ../steem/psg.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_psg.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_psg.cpp$(DependSuffix) -MM "../steem/psg.cpp"

$(IntermediateDirectory)/steem_psg.cpp$(PreprocessSuffix): ../steem/psg.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_psg.cpp$(PreprocessSuffix) "../steem/psg.cpp"

$(IntermediateDirectory)/steem_reset.cpp$(ObjectSuffix): ../steem/reset.cpp $(IntermediateDirectory)/steem_reset.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/reset.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_reset.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_reset.cpp$(DependSuffix): ../steem/reset.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_reset.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_reset.cpp$(DependSuffix) -MM "../steem/reset.cpp"

$(IntermediateDirectory)/steem_reset.cpp$(PreprocessSuffix): ../steem/reset.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_reset.cpp$(PreprocessSuffix) "../steem/reset.cpp"

$(IntermediateDirectory)/steem_rs232.cpp$(ObjectSuffix): ../steem/rs232.cpp $(IntermediateDirectory)/steem_rs232.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/rs232.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_rs232.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_rs232.cpp$(DependSuffix): ../steem/rs232.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_rs232.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_rs232.cpp$(DependSuffix) -MM "../steem/rs232.cpp"

$(IntermediateDirectory)/steem_rs232.cpp$(PreprocessSuffix): ../steem/rs232.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_rs232.cpp$(PreprocessSuffix) "../steem/rs232.cpp"

$(IntermediateDirectory)/steem_run.cpp$(ObjectSuffix): ../steem/run.cpp $(IntermediateDirectory)/steem_run.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/run.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_run.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_run.cpp$(DependSuffix): ../steem/run.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_run.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_run.cpp$(DependSuffix) -MM "../steem/run.cpp"

$(IntermediateDirectory)/steem_run.cpp$(PreprocessSuffix): ../steem/run.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_run.cpp$(PreprocessSuffix) "../steem/run.cpp"

$(IntermediateDirectory)/steem_screen_saver.cpp$(ObjectSuffix): ../steem/screen_saver.cpp $(IntermediateDirectory)/steem_screen_saver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/screen_saver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_screen_saver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_screen_saver.cpp$(DependSuffix): ../steem/screen_saver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_screen_saver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_screen_saver.cpp$(DependSuffix) -MM "../steem/screen_saver.cpp"

$(IntermediateDirectory)/steem_screen_saver.cpp$(PreprocessSuffix): ../steem/screen_saver.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_screen_saver.cpp$(PreprocessSuffix) "../steem/screen_saver.cpp"

$(IntermediateDirectory)/steem_shifter.cpp$(ObjectSuffix): ../steem/shifter.cpp $(IntermediateDirectory)/steem_shifter.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/shifter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_shifter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_shifter.cpp$(DependSuffix): ../steem/shifter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_shifter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_shifter.cpp$(DependSuffix) -MM "../steem/shifter.cpp"

$(IntermediateDirectory)/steem_shifter.cpp$(PreprocessSuffix): ../steem/shifter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_shifter.cpp$(PreprocessSuffix) "../steem/shifter.cpp"

$(IntermediateDirectory)/steem_shortcutbox.cpp$(ObjectSuffix): ../steem/shortcutbox.cpp $(IntermediateDirectory)/steem_shortcutbox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/shortcutbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_shortcutbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_shortcutbox.cpp$(DependSuffix): ../steem/shortcutbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_shortcutbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_shortcutbox.cpp$(DependSuffix) -MM "../steem/shortcutbox.cpp"

$(IntermediateDirectory)/steem_shortcutbox.cpp$(PreprocessSuffix): ../steem/shortcutbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_shortcutbox.cpp$(PreprocessSuffix) "../steem/shortcutbox.cpp"

$(IntermediateDirectory)/steem_sound.cpp$(ObjectSuffix): ../steem/sound.cpp $(IntermediateDirectory)/steem_sound.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/sound.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_sound.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_sound.cpp$(DependSuffix): ../steem/sound.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_sound.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_sound.cpp$(DependSuffix) -MM "../steem/sound.cpp"

$(IntermediateDirectory)/steem_sound.cpp$(PreprocessSuffix): ../steem/sound.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_sound.cpp$(PreprocessSuffix) "../steem/sound.cpp"

$(IntermediateDirectory)/steem_Steem.cpp$(ObjectSuffix): ../steem/Steem.cpp $(IntermediateDirectory)/steem_Steem.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/Steem.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_Steem.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_Steem.cpp$(DependSuffix): ../steem/Steem.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_Steem.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_Steem.cpp$(DependSuffix) -MM "../steem/Steem.cpp"

$(IntermediateDirectory)/steem_Steem.cpp$(PreprocessSuffix): ../steem/Steem.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_Steem.cpp$(PreprocessSuffix) "../steem/Steem.cpp"

$(IntermediateDirectory)/steem_steemintro.cpp$(ObjectSuffix): ../steem/steemintro.cpp $(IntermediateDirectory)/steem_steemintro.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/steemintro.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_steemintro.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_steemintro.cpp$(DependSuffix): ../steem/steemintro.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_steemintro.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_steemintro.cpp$(DependSuffix) -MM "../steem/steemintro.cpp"

$(IntermediateDirectory)/steem_steemintro.cpp$(PreprocessSuffix): ../steem/steemintro.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_steemintro.cpp$(PreprocessSuffix) "../steem/steemintro.cpp"

$(IntermediateDirectory)/steem_stemdialogs.cpp$(ObjectSuffix): ../steem/stemdialogs.cpp $(IntermediateDirectory)/steem_stemdialogs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/stemdialogs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_stemdialogs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_stemdialogs.cpp$(DependSuffix): ../steem/stemdialogs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_stemdialogs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_stemdialogs.cpp$(DependSuffix) -MM "../steem/stemdialogs.cpp"

$(IntermediateDirectory)/steem_stemdialogs.cpp$(PreprocessSuffix): ../steem/stemdialogs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_stemdialogs.cpp$(PreprocessSuffix) "../steem/stemdialogs.cpp"

$(IntermediateDirectory)/steem_stemwin.cpp$(ObjectSuffix): ../steem/stemwin.cpp $(IntermediateDirectory)/steem_stemwin.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/stemwin.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_stemwin.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_stemwin.cpp$(DependSuffix): ../steem/stemwin.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_stemwin.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_stemwin.cpp$(DependSuffix) -MM "../steem/stemwin.cpp"

$(IntermediateDirectory)/steem_stemwin.cpp$(PreprocessSuffix): ../steem/stemwin.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_stemwin.cpp$(PreprocessSuffix) "../steem/stemwin.cpp"

$(IntermediateDirectory)/steem_stjoy.cpp$(ObjectSuffix): ../steem/stjoy.cpp $(IntermediateDirectory)/steem_stjoy.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/stjoy.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_stjoy.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_stjoy.cpp$(DependSuffix): ../steem/stjoy.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_stjoy.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_stjoy.cpp$(DependSuffix) -MM "../steem/stjoy.cpp"

$(IntermediateDirectory)/steem_stjoy.cpp$(PreprocessSuffix): ../steem/stjoy.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_stjoy.cpp$(PreprocessSuffix) "../steem/stjoy.cpp"

$(IntermediateDirectory)/steem_stports.cpp$(ObjectSuffix): ../steem/stports.cpp $(IntermediateDirectory)/steem_stports.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/stports.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_stports.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_stports.cpp$(DependSuffix): ../steem/stports.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_stports.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_stports.cpp$(DependSuffix) -MM "../steem/stports.cpp"

$(IntermediateDirectory)/steem_stports.cpp$(PreprocessSuffix): ../steem/stports.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_stports.cpp$(PreprocessSuffix) "../steem/stports.cpp"

$(IntermediateDirectory)/steem_translate.cpp$(ObjectSuffix): ../steem/translate.cpp $(IntermediateDirectory)/steem_translate.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/translate.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_translate.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_translate.cpp$(DependSuffix): ../steem/translate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_translate.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_translate.cpp$(DependSuffix) -MM "../steem/translate.cpp"

$(IntermediateDirectory)/steem_translate.cpp$(PreprocessSuffix): ../steem/translate.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_translate.cpp$(PreprocessSuffix) "../steem/translate.cpp"

$(IntermediateDirectory)/steem_diskman_diags.cpp$(ObjectSuffix): ../steem/diskman_diags.cpp $(IntermediateDirectory)/steem_diskman_diags.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/diskman_diags.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_diskman_diags.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_diskman_diags.cpp$(DependSuffix): ../steem/diskman_diags.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_diskman_diags.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_diskman_diags.cpp$(DependSuffix) -MM "../steem/diskman_diags.cpp"

$(IntermediateDirectory)/steem_diskman_diags.cpp$(PreprocessSuffix): ../steem/diskman_diags.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_diskman_diags.cpp$(PreprocessSuffix) "../steem/diskman_diags.cpp"

$(IntermediateDirectory)/steem_interface_pa.cpp$(ObjectSuffix): ../steem/interface_pa.cpp $(IntermediateDirectory)/steem_interface_pa.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/interface_pa.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_interface_pa.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_interface_pa.cpp$(DependSuffix): ../steem/interface_pa.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_interface_pa.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_interface_pa.cpp$(DependSuffix) -MM "../steem/interface_pa.cpp"

$(IntermediateDirectory)/steem_interface_pa.cpp$(PreprocessSuffix): ../steem/interface_pa.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_interface_pa.cpp$(PreprocessSuffix) "../steem/interface_pa.cpp"

$(IntermediateDirectory)/steem_interface_rta.cpp$(ObjectSuffix): ../steem/interface_rta.cpp $(IntermediateDirectory)/steem_interface_rta.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/interface_rta.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_interface_rta.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_interface_rta.cpp$(DependSuffix): ../steem/interface_rta.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_interface_rta.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_interface_rta.cpp$(DependSuffix) -MM "../steem/interface_rta.cpp"

$(IntermediateDirectory)/steem_interface_rta.cpp$(PreprocessSuffix): ../steem/interface_rta.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_interface_rta.cpp$(PreprocessSuffix) "../steem/interface_rta.cpp"

$(IntermediateDirectory)/steem_tos.cpp$(ObjectSuffix): ../steem/tos.cpp $(IntermediateDirectory)/steem_tos.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/steem/tos.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/steem_tos.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/steem_tos.cpp$(DependSuffix): ../steem/tos.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/steem_tos.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/steem_tos.cpp$(DependSuffix) -MM "../steem/tos.cpp"

$(IntermediateDirectory)/steem_tos.cpp$(PreprocessSuffix): ../steem/tos.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/steem_tos.cpp$(PreprocessSuffix) "../steem/tos.cpp"

$(IntermediateDirectory)/asm_asm_draw.asm$(ObjectSuffix): ../steem/asm/asm_draw.asm $(IntermediateDirectory)/asm_asm_draw.asm$(DependSuffix)
	$(AS) -felf "/home/user/Documents/ST/steem/asm/asm_draw.asm" $(ASFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/asm_asm_draw.asm$(ObjectSuffix) -I$(IncludePath)
$(IntermediateDirectory)/asm_asm_draw.asm$(DependSuffix): ../steem/asm/asm_draw.asm
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/asm_asm_draw.asm$(ObjectSuffix) -MF$(IntermediateDirectory)/asm_asm_draw.asm$(DependSuffix) -MM "../steem/asm/asm_draw.asm"

$(IntermediateDirectory)/asm_asm_draw.asm$(PreprocessSuffix): ../steem/asm/asm_draw.asm
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/asm_asm_draw.asm$(PreprocessSuffix) "../steem/asm/asm_draw.asm"

$(IntermediateDirectory)/asm_asm_osd_draw.asm$(ObjectSuffix): ../steem/asm/asm_osd_draw.asm $(IntermediateDirectory)/asm_asm_osd_draw.asm$(DependSuffix)
	$(AS) -felf "/home/user/Documents/ST/steem/asm/asm_osd_draw.asm" $(ASFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/asm_asm_osd_draw.asm$(ObjectSuffix) -I$(IncludePath)
$(IntermediateDirectory)/asm_asm_osd_draw.asm$(DependSuffix): ../steem/asm/asm_osd_draw.asm
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/asm_asm_osd_draw.asm$(ObjectSuffix) -MF$(IntermediateDirectory)/asm_asm_osd_draw.asm$(DependSuffix) -MM "../steem/asm/asm_osd_draw.asm"

$(IntermediateDirectory)/asm_asm_osd_draw.asm$(PreprocessSuffix): ../steem/asm/asm_osd_draw.asm
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/asm_asm_osd_draw.asm$(PreprocessSuffix) "../steem/asm/asm_osd_draw.asm"

$(IntermediateDirectory)/rc_resource.asm$(ObjectSuffix): ../steem/rc/resource.asm $(IntermediateDirectory)/rc_resource.asm$(DependSuffix)
	$(AS) -felf "/home/user/Documents/ST/steem/rc/resource.asm" $(ASFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rc_resource.asm$(ObjectSuffix) -I$(IncludePath)
$(IntermediateDirectory)/rc_resource.asm$(DependSuffix): ../steem/rc/resource.asm
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rc_resource.asm$(ObjectSuffix) -MF$(IntermediateDirectory)/rc_resource.asm$(DependSuffix) -MM "../steem/rc/resource.asm"

$(IntermediateDirectory)/rc_resource.asm$(PreprocessSuffix): ../steem/rc/resource.asm
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rc_resource.asm$(PreprocessSuffix) "../steem/rc/resource.asm"

$(IntermediateDirectory)/include_circularbuffer.cpp$(ObjectSuffix): ../include/circularbuffer.cpp $(IntermediateDirectory)/include_circularbuffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/circularbuffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_circularbuffer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_circularbuffer.cpp$(DependSuffix): ../include/circularbuffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_circularbuffer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_circularbuffer.cpp$(DependSuffix) -MM "../include/circularbuffer.cpp"

$(IntermediateDirectory)/include_circularbuffer.cpp$(PreprocessSuffix): ../include/circularbuffer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_circularbuffer.cpp$(PreprocessSuffix) "../include/circularbuffer.cpp"

$(IntermediateDirectory)/include_configstorefile.cpp$(ObjectSuffix): ../include/configstorefile.cpp $(IntermediateDirectory)/include_configstorefile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/configstorefile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_configstorefile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_configstorefile.cpp$(DependSuffix): ../include/configstorefile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_configstorefile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_configstorefile.cpp$(DependSuffix) -MM "../include/configstorefile.cpp"

$(IntermediateDirectory)/include_configstorefile.cpp$(PreprocessSuffix): ../include/configstorefile.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_configstorefile.cpp$(PreprocessSuffix) "../include/configstorefile.cpp"

$(IntermediateDirectory)/include_di_get_contents.cpp$(ObjectSuffix): ../include/di_get_contents.cpp $(IntermediateDirectory)/include_di_get_contents.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/di_get_contents.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_di_get_contents.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_di_get_contents.cpp$(DependSuffix): ../include/di_get_contents.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_di_get_contents.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_di_get_contents.cpp$(DependSuffix) -MM "../include/di_get_contents.cpp"

$(IntermediateDirectory)/include_di_get_contents.cpp$(PreprocessSuffix): ../include/di_get_contents.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_di_get_contents.cpp$(PreprocessSuffix) "../include/di_get_contents.cpp"

$(IntermediateDirectory)/include_dynamicarray.cpp$(ObjectSuffix): ../include/dynamicarray.cpp $(IntermediateDirectory)/include_dynamicarray.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/dynamicarray.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_dynamicarray.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_dynamicarray.cpp$(DependSuffix): ../include/dynamicarray.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_dynamicarray.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_dynamicarray.cpp$(DependSuffix) -MM "../include/dynamicarray.cpp"

$(IntermediateDirectory)/include_dynamicarray.cpp$(PreprocessSuffix): ../include/dynamicarray.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_dynamicarray.cpp$(PreprocessSuffix) "../include/dynamicarray.cpp"

$(IntermediateDirectory)/include_easycompress.cpp$(ObjectSuffix): ../include/easycompress.cpp $(IntermediateDirectory)/include_easycompress.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/easycompress.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_easycompress.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_easycompress.cpp$(DependSuffix): ../include/easycompress.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_easycompress.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_easycompress.cpp$(DependSuffix) -MM "../include/easycompress.cpp"

$(IntermediateDirectory)/include_easycompress.cpp$(PreprocessSuffix): ../include/easycompress.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_easycompress.cpp$(PreprocessSuffix) "../include/easycompress.cpp"

$(IntermediateDirectory)/include_easystr.cpp$(ObjectSuffix): ../include/easystr.cpp $(IntermediateDirectory)/include_easystr.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/easystr.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_easystr.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_easystr.cpp$(DependSuffix): ../include/easystr.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_easystr.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_easystr.cpp$(DependSuffix) -MM "../include/easystr.cpp"

$(IntermediateDirectory)/include_easystr.cpp$(PreprocessSuffix): ../include/easystr.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_easystr.cpp$(PreprocessSuffix) "../include/easystr.cpp"

$(IntermediateDirectory)/include_easystringlist.cpp$(ObjectSuffix): ../include/easystringlist.cpp $(IntermediateDirectory)/include_easystringlist.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/easystringlist.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_easystringlist.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_easystringlist.cpp$(DependSuffix): ../include/easystringlist.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_easystringlist.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_easystringlist.cpp$(DependSuffix) -MM "../include/easystringlist.cpp"

$(IntermediateDirectory)/include_easystringlist.cpp$(PreprocessSuffix): ../include/easystringlist.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_easystringlist.cpp$(PreprocessSuffix) "../include/easystringlist.cpp"

$(IntermediateDirectory)/include_mymisc.cpp$(ObjectSuffix): ../include/mymisc.cpp $(IntermediateDirectory)/include_mymisc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/mymisc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_mymisc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_mymisc.cpp$(DependSuffix): ../include/mymisc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_mymisc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_mymisc.cpp$(DependSuffix) -MM "../include/mymisc.cpp"

$(IntermediateDirectory)/include_mymisc.cpp$(PreprocessSuffix): ../include/mymisc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_mymisc.cpp$(PreprocessSuffix) "../include/mymisc.cpp"

$(IntermediateDirectory)/include_notwin_mymisc.cpp$(ObjectSuffix): ../include/notwin_mymisc.cpp $(IntermediateDirectory)/include_notwin_mymisc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/notwin_mymisc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_notwin_mymisc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_notwin_mymisc.cpp$(DependSuffix): ../include/notwin_mymisc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_notwin_mymisc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_notwin_mymisc.cpp$(DependSuffix) -MM "../include/notwin_mymisc.cpp"

$(IntermediateDirectory)/include_notwin_mymisc.cpp$(PreprocessSuffix): ../include/notwin_mymisc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_notwin_mymisc.cpp$(PreprocessSuffix) "../include/notwin_mymisc.cpp"

$(IntermediateDirectory)/include_portio.cpp$(ObjectSuffix): ../include/portio.cpp $(IntermediateDirectory)/include_portio.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/portio.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_portio.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_portio.cpp$(DependSuffix): ../include/portio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_portio.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_portio.cpp$(DependSuffix) -MM "../include/portio.cpp"

$(IntermediateDirectory)/include_portio.cpp$(PreprocessSuffix): ../include/portio.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_portio.cpp$(PreprocessSuffix) "../include/portio.cpp"

$(IntermediateDirectory)/include_wordwrapper.cpp$(ObjectSuffix): ../include/wordwrapper.cpp $(IntermediateDirectory)/include_wordwrapper.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/wordwrapper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_wordwrapper.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_wordwrapper.cpp$(DependSuffix): ../include/wordwrapper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_wordwrapper.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_wordwrapper.cpp$(DependSuffix) -MM "../include/wordwrapper.cpp"

$(IntermediateDirectory)/include_wordwrapper.cpp$(PreprocessSuffix): ../include/wordwrapper.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_wordwrapper.cpp$(PreprocessSuffix) "../include/wordwrapper.cpp"

$(IntermediateDirectory)/include_dirsearch.cpp$(ObjectSuffix): ../include/dirsearch.cpp $(IntermediateDirectory)/include_dirsearch.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/dirsearch.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/include_dirsearch.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/include_dirsearch.cpp$(DependSuffix): ../include/dirsearch.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/include_dirsearch.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/include_dirsearch.cpp$(DependSuffix) -MM "../include/dirsearch.cpp"

$(IntermediateDirectory)/include_dirsearch.cpp$(PreprocessSuffix): ../include/dirsearch.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/include_dirsearch.cpp$(PreprocessSuffix) "../include/dirsearch.cpp"

$(IntermediateDirectory)/6301_6301.c$(ObjectSuffix): ../3rdparty/6301/6301.c $(IntermediateDirectory)/6301_6301.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/6301/6301.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/6301_6301.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/6301_6301.c$(DependSuffix): ../3rdparty/6301/6301.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/6301_6301.c$(ObjectSuffix) -MF$(IntermediateDirectory)/6301_6301.c$(DependSuffix) -MM "../3rdparty/6301/6301.c"

$(IntermediateDirectory)/6301_6301.c$(PreprocessSuffix): ../3rdparty/6301/6301.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/6301_6301.c$(PreprocessSuffix) "../3rdparty/6301/6301.c"

$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(ObjectSuffix): ../3rdparty/pasti/div68kCycleAccurate.c $(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/pasti/div68kCycleAccurate.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(DependSuffix): ../3rdparty/pasti/div68kCycleAccurate.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(ObjectSuffix) -MF$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(DependSuffix) -MM "../3rdparty/pasti/div68kCycleAccurate.c"

$(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(PreprocessSuffix): ../3rdparty/pasti/div68kCycleAccurate.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/pasti_div68kCycleAccurate.c$(PreprocessSuffix) "../3rdparty/pasti/div68kCycleAccurate.c"

$(IntermediateDirectory)/dsp_dsp.cpp$(ObjectSuffix): ../3rdparty/dsp/dsp.cpp $(IntermediateDirectory)/dsp_dsp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/dsp/dsp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dsp_dsp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dsp_dsp.cpp$(DependSuffix): ../3rdparty/dsp/dsp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/dsp_dsp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/dsp_dsp.cpp$(DependSuffix) -MM "../3rdparty/dsp/dsp.cpp"

$(IntermediateDirectory)/dsp_dsp.cpp$(PreprocessSuffix): ../3rdparty/dsp/dsp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dsp_dsp.cpp$(PreprocessSuffix) "../3rdparty/dsp/dsp.cpp"

$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(ObjectSuffix): ../3rdparty/dsp/FIR-filter-class/filt.cpp $(IntermediateDirectory)/FIR-filter-class_filt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/dsp/FIR-filter-class/filt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(DependSuffix): ../3rdparty/dsp/FIR-filter-class/filt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(DependSuffix) -MM "../3rdparty/dsp/FIR-filter-class/filt.cpp"

$(IntermediateDirectory)/FIR-filter-class_filt.cpp$(PreprocessSuffix): ../3rdparty/dsp/FIR-filter-class/filt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/FIR-filter-class_filt.cpp$(PreprocessSuffix) "../3rdparty/dsp/FIR-filter-class/filt.cpp"

$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(ObjectSuffix): ../3rdparty/rtaudio/RtAudio.cpp $(IntermediateDirectory)/rtaudio_RtAudio.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/rtaudio/RtAudio.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(DependSuffix): ../3rdparty/rtaudio/RtAudio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(DependSuffix) -MM "../3rdparty/rtaudio/RtAudio.cpp"

$(IntermediateDirectory)/rtaudio_RtAudio.cpp$(PreprocessSuffix): ../3rdparty/rtaudio/RtAudio.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtaudio_RtAudio.cpp$(PreprocessSuffix) "../3rdparty/rtaudio/RtAudio.cpp"

$(IntermediateDirectory)/zlib_adler32.c$(ObjectSuffix): ../3rdparty/zlib/adler32.c $(IntermediateDirectory)/zlib_adler32.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/adler32.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_adler32.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_adler32.c$(DependSuffix): ../3rdparty/zlib/adler32.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_adler32.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_adler32.c$(DependSuffix) -MM "../3rdparty/zlib/adler32.c"

$(IntermediateDirectory)/zlib_adler32.c$(PreprocessSuffix): ../3rdparty/zlib/adler32.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_adler32.c$(PreprocessSuffix) "../3rdparty/zlib/adler32.c"

$(IntermediateDirectory)/zlib_compress.c$(ObjectSuffix): ../3rdparty/zlib/compress.c $(IntermediateDirectory)/zlib_compress.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/compress.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_compress.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_compress.c$(DependSuffix): ../3rdparty/zlib/compress.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_compress.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_compress.c$(DependSuffix) -MM "../3rdparty/zlib/compress.c"

$(IntermediateDirectory)/zlib_compress.c$(PreprocessSuffix): ../3rdparty/zlib/compress.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_compress.c$(PreprocessSuffix) "../3rdparty/zlib/compress.c"

$(IntermediateDirectory)/zlib_crc32.c$(ObjectSuffix): ../3rdparty/zlib/crc32.c $(IntermediateDirectory)/zlib_crc32.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/crc32.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_crc32.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_crc32.c$(DependSuffix): ../3rdparty/zlib/crc32.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_crc32.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_crc32.c$(DependSuffix) -MM "../3rdparty/zlib/crc32.c"

$(IntermediateDirectory)/zlib_crc32.c$(PreprocessSuffix): ../3rdparty/zlib/crc32.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_crc32.c$(PreprocessSuffix) "../3rdparty/zlib/crc32.c"

$(IntermediateDirectory)/zlib_deflate.c$(ObjectSuffix): ../3rdparty/zlib/deflate.c $(IntermediateDirectory)/zlib_deflate.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/deflate.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_deflate.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_deflate.c$(DependSuffix): ../3rdparty/zlib/deflate.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_deflate.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_deflate.c$(DependSuffix) -MM "../3rdparty/zlib/deflate.c"

$(IntermediateDirectory)/zlib_deflate.c$(PreprocessSuffix): ../3rdparty/zlib/deflate.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_deflate.c$(PreprocessSuffix) "../3rdparty/zlib/deflate.c"

$(IntermediateDirectory)/zlib_inffast.c$(ObjectSuffix): ../3rdparty/zlib/inffast.c $(IntermediateDirectory)/zlib_inffast.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/inffast.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_inffast.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_inffast.c$(DependSuffix): ../3rdparty/zlib/inffast.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_inffast.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_inffast.c$(DependSuffix) -MM "../3rdparty/zlib/inffast.c"

$(IntermediateDirectory)/zlib_inffast.c$(PreprocessSuffix): ../3rdparty/zlib/inffast.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_inffast.c$(PreprocessSuffix) "../3rdparty/zlib/inffast.c"

$(IntermediateDirectory)/zlib_inflate.c$(ObjectSuffix): ../3rdparty/zlib/inflate.c $(IntermediateDirectory)/zlib_inflate.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/inflate.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_inflate.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_inflate.c$(DependSuffix): ../3rdparty/zlib/inflate.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_inflate.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_inflate.c$(DependSuffix) -MM "../3rdparty/zlib/inflate.c"

$(IntermediateDirectory)/zlib_inflate.c$(PreprocessSuffix): ../3rdparty/zlib/inflate.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_inflate.c$(PreprocessSuffix) "../3rdparty/zlib/inflate.c"

$(IntermediateDirectory)/zlib_inftrees.c$(ObjectSuffix): ../3rdparty/zlib/inftrees.c $(IntermediateDirectory)/zlib_inftrees.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/inftrees.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_inftrees.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_inftrees.c$(DependSuffix): ../3rdparty/zlib/inftrees.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_inftrees.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_inftrees.c$(DependSuffix) -MM "../3rdparty/zlib/inftrees.c"

$(IntermediateDirectory)/zlib_inftrees.c$(PreprocessSuffix): ../3rdparty/zlib/inftrees.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_inftrees.c$(PreprocessSuffix) "../3rdparty/zlib/inftrees.c"

$(IntermediateDirectory)/zlib_zutil.c$(ObjectSuffix): ../3rdparty/zlib/zutil.c $(IntermediateDirectory)/zlib_zutil.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/zutil.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_zutil.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_zutil.c$(DependSuffix): ../3rdparty/zlib/zutil.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_zutil.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_zutil.c$(DependSuffix) -MM "../3rdparty/zlib/zutil.c"

$(IntermediateDirectory)/zlib_zutil.c$(PreprocessSuffix): ../3rdparty/zlib/zutil.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_zutil.c$(PreprocessSuffix) "../3rdparty/zlib/zutil.c"

$(IntermediateDirectory)/minizip_ioapi.c$(ObjectSuffix): ../3rdparty/zlib/contrib/minizip/ioapi.c $(IntermediateDirectory)/minizip_ioapi.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/contrib/minizip/ioapi.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/minizip_ioapi.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/minizip_ioapi.c$(DependSuffix): ../3rdparty/zlib/contrib/minizip/ioapi.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/minizip_ioapi.c$(ObjectSuffix) -MF$(IntermediateDirectory)/minizip_ioapi.c$(DependSuffix) -MM "../3rdparty/zlib/contrib/minizip/ioapi.c"

$(IntermediateDirectory)/minizip_ioapi.c$(PreprocessSuffix): ../3rdparty/zlib/contrib/minizip/ioapi.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/minizip_ioapi.c$(PreprocessSuffix) "../3rdparty/zlib/contrib/minizip/ioapi.c"

$(IntermediateDirectory)/minizip_unzip.c$(ObjectSuffix): ../3rdparty/zlib/contrib/minizip/unzip.c $(IntermediateDirectory)/minizip_unzip.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/contrib/minizip/unzip.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/minizip_unzip.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/minizip_unzip.c$(DependSuffix): ../3rdparty/zlib/contrib/minizip/unzip.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/minizip_unzip.c$(ObjectSuffix) -MF$(IntermediateDirectory)/minizip_unzip.c$(DependSuffix) -MM "../3rdparty/zlib/contrib/minizip/unzip.c"

$(IntermediateDirectory)/minizip_unzip.c$(PreprocessSuffix): ../3rdparty/zlib/contrib/minizip/unzip.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/minizip_unzip.c$(PreprocessSuffix) "../3rdparty/zlib/contrib/minizip/unzip.c"

$(IntermediateDirectory)/zlib_trees.c$(ObjectSuffix): ../3rdparty/zlib/trees.c $(IntermediateDirectory)/zlib_trees.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/zlib/trees.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zlib_trees.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zlib_trees.c$(DependSuffix): ../3rdparty/zlib/trees.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zlib_trees.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zlib_trees.c$(DependSuffix) -MM "../3rdparty/zlib/trees.c"

$(IntermediateDirectory)/zlib_trees.c$(PreprocessSuffix): ../3rdparty/zlib/trees.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zlib_trees.c$(PreprocessSuffix) "../3rdparty/zlib/trees.c"

$(IntermediateDirectory)/caps_CapsPlug.cpp$(ObjectSuffix): ../3rdparty/caps/CapsPlug.cpp $(IntermediateDirectory)/caps_CapsPlug.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/3rdparty/caps/CapsPlug.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/caps_CapsPlug.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/caps_CapsPlug.cpp$(DependSuffix): ../3rdparty/caps/CapsPlug.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/caps_CapsPlug.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/caps_CapsPlug.cpp$(DependSuffix) -MM "../3rdparty/caps/CapsPlug.cpp"

$(IntermediateDirectory)/caps_CapsPlug.cpp$(PreprocessSuffix): ../3rdparty/caps/CapsPlug.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/caps_CapsPlug.cpp$(PreprocessSuffix) "../3rdparty/caps/CapsPlug.cpp"

$(IntermediateDirectory)/x_hxc.cpp$(ObjectSuffix): ../include/x/hxc.cpp $(IntermediateDirectory)/x_hxc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc.cpp$(DependSuffix): ../include/x/hxc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc.cpp$(DependSuffix) -MM "../include/x/hxc.cpp"

$(IntermediateDirectory)/x_hxc.cpp$(PreprocessSuffix): ../include/x/hxc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc.cpp$(PreprocessSuffix) "../include/x/hxc.cpp"

$(IntermediateDirectory)/x_hxc_alert.cpp$(ObjectSuffix): ../include/x/hxc_alert.cpp $(IntermediateDirectory)/x_hxc_alert.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_alert.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_alert.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_alert.cpp$(DependSuffix): ../include/x/hxc_alert.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_alert.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_alert.cpp$(DependSuffix) -MM "../include/x/hxc_alert.cpp"

$(IntermediateDirectory)/x_hxc_alert.cpp$(PreprocessSuffix): ../include/x/hxc_alert.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_alert.cpp$(PreprocessSuffix) "../include/x/hxc_alert.cpp"

$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(ObjectSuffix): ../include/x/hxc_dir_lv.cpp $(IntermediateDirectory)/x_hxc_dir_lv.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_dir_lv.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(DependSuffix): ../include/x/hxc_dir_lv.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(DependSuffix) -MM "../include/x/hxc_dir_lv.cpp"

$(IntermediateDirectory)/x_hxc_dir_lv.cpp$(PreprocessSuffix): ../include/x/hxc_dir_lv.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_dir_lv.cpp$(PreprocessSuffix) "../include/x/hxc_dir_lv.cpp"

$(IntermediateDirectory)/x_hxc_fileselect.cpp$(ObjectSuffix): ../include/x/hxc_fileselect.cpp $(IntermediateDirectory)/x_hxc_fileselect.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_fileselect.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_fileselect.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_fileselect.cpp$(DependSuffix): ../include/x/hxc_fileselect.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_fileselect.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_fileselect.cpp$(DependSuffix) -MM "../include/x/hxc_fileselect.cpp"

$(IntermediateDirectory)/x_hxc_fileselect.cpp$(PreprocessSuffix): ../include/x/hxc_fileselect.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_fileselect.cpp$(PreprocessSuffix) "../include/x/hxc_fileselect.cpp"

$(IntermediateDirectory)/x_hxc_popup.cpp$(ObjectSuffix): ../include/x/hxc_popup.cpp $(IntermediateDirectory)/x_hxc_popup.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_popup.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_popup.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_popup.cpp$(DependSuffix): ../include/x/hxc_popup.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_popup.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_popup.cpp$(DependSuffix) -MM "../include/x/hxc_popup.cpp"

$(IntermediateDirectory)/x_hxc_popup.cpp$(PreprocessSuffix): ../include/x/hxc_popup.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_popup.cpp$(PreprocessSuffix) "../include/x/hxc_popup.cpp"

$(IntermediateDirectory)/x_hxc_popuphints.cpp$(ObjectSuffix): ../include/x/hxc_popuphints.cpp $(IntermediateDirectory)/x_hxc_popuphints.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_popuphints.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_popuphints.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_popuphints.cpp$(DependSuffix): ../include/x/hxc_popuphints.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_popuphints.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_popuphints.cpp$(DependSuffix) -MM "../include/x/hxc_popuphints.cpp"

$(IntermediateDirectory)/x_hxc_popuphints.cpp$(PreprocessSuffix): ../include/x/hxc_popuphints.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_popuphints.cpp$(PreprocessSuffix) "../include/x/hxc_popuphints.cpp"

$(IntermediateDirectory)/x_hxc_prompt.cpp$(ObjectSuffix): ../include/x/hxc_prompt.cpp $(IntermediateDirectory)/x_hxc_prompt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/hxc_prompt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_hxc_prompt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_hxc_prompt.cpp$(DependSuffix): ../include/x/hxc_prompt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_hxc_prompt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_hxc_prompt.cpp$(DependSuffix) -MM "../include/x/hxc_prompt.cpp"

$(IntermediateDirectory)/x_hxc_prompt.cpp$(PreprocessSuffix): ../include/x/hxc_prompt.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_hxc_prompt.cpp$(PreprocessSuffix) "../include/x/hxc_prompt.cpp"

$(IntermediateDirectory)/x_x_mymisc.cpp$(ObjectSuffix): ../include/x/x_mymisc.cpp $(IntermediateDirectory)/x_x_mymisc.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/x_mymisc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_x_mymisc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_x_mymisc.cpp$(DependSuffix): ../include/x/x_mymisc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_x_mymisc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_x_mymisc.cpp$(DependSuffix) -MM "../include/x/x_mymisc.cpp"

$(IntermediateDirectory)/x_x_mymisc.cpp$(PreprocessSuffix): ../include/x/x_mymisc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_x_mymisc.cpp$(PreprocessSuffix) "../include/x/x_mymisc.cpp"

$(IntermediateDirectory)/x_x_portio.cpp$(ObjectSuffix): ../include/x/x_portio.cpp $(IntermediateDirectory)/x_x_portio.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/user/Documents/ST/include/x/x_portio.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/x_x_portio.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/x_x_portio.cpp$(DependSuffix): ../include/x/x_portio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/x_x_portio.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/x_x_portio.cpp$(DependSuffix) -MM "../include/x/x_portio.cpp"

$(IntermediateDirectory)/x_x_portio.cpp$(PreprocessSuffix): ../include/x/x_portio.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/x_x_portio.cpp$(PreprocessSuffix) "../include/x/x_portio.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


