# Makefile for MinGW32/MinGW-W64

ifndef PREFIX
PREFIX=
RCPREFIX=
else
ifeq ($(PREFIX),x86_64-w64-mingw32-)
RCPREFIX=$(PREFIX)
else
ifeq ($(PREFIX),i686-w64-mingw32-)
ifeq ($(OS),Windows_NT)
RCPREFIX=
else
RCPREFIX=$(PREFIX)
endif
endif
endif
endif

CC= $(PREFIX)gcc
CXX= $(PREFIX)g++
RC= $(RCPREFIX)windres

DEFINES= \
	-DWIN32 \
	-DWINVER=0x500 \
	-D_WIN32_WINNT=0x500 \
	-D_WIN32_IE=0x501 \
	-DNOMINMAX \
	-D_UNICODE \
	-DUNICODE \
	-DNDEBUG \
	-D_WINDOWS \
	-D_USRDLL \
	-DPLUGIN_EXPORTS

CFLAGS= -O2 \
	-finput-charset=cp932 -fexec-charset=cp932 \
	-I. \
	-I../common \
	-I../utility \
	-I../../../sakura_core \
	$(DEFINES) $(MYCFLAGS)

CXXFLAGS= $(CFLAGS) $(MYCXXFLAGS)

LIBS_SUB=
#	-lxmlite.lib \
#	-lshlwapi.lib

LIBS= \
	-lkernel32 -luser32 -lgdi32 \
	-lwinspool -lcomdlg32 -ladvapi32 \
	-lshell32 -lole32 -loleaut32 \
	-luuid -lcomctl32 -limm32 \
	-lmpr -limagehlp \
	-static-libgcc -static-libstdc++ -mwindows -shared -s \
	$(LIBS_SUB)

plugin= HelloPlugin.dll
plugin_1041= HelloPlugin_1041.dll
plugin_1033= HelloPlugin_1033.dll

IMPDEF= HelloPlugin.def

OBJSCOMMON= \
	../common/CBasePluginService.o \
	../common/CExternalComplementIfObj.o \
	../common/CExternalEditorIfObj.o \
	../common/CExternalMacroIfObj.o \
	../common/CExternalOutlineIfObj.o \
	../common/CExternalPluginIfObj.o \
	../common/CExternalSmartIndentIfObj.o \
	../common/SakuraMeetsPlugin.o

OBJSUTILITY= 

OBJS_1041= \
	HelloPlugin_1041.o

OBJS_1033= \
	HelloPlugin_1033.o

OBJS= \
	CPluginService.o \
	dllmain.o \
	HelloPlugin.o \
	stdafx.o \
	$(OBJS_1041) \
	$(OBJSCOMMON) \
	$(OBJSUTILITY)

RCTOOLDIR=../../../btool
RCTOOL=$(RCTOOLDIR)/mrc2grc.exe

all: $(RCTOOL) $(plugin) $(plugin_1033)

$(plugin): stdafx $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS) --def $(IMPDEF)

#$(plugin_1041): $(OBJS_1041)
#	$(CXX) -o $@ $(OBJS_1041) -shared -s

$(plugin_1033): $(OBJS_1033)
	$(CXX) -o $@ $(OBJS_1033) -shared -s

stdafx:
	$(CXX) $(CXXFLAGS) -c StdAfx.h

$(RCTOOL): $(RCTOOLDIR)/mrc2grc.cpp
	$(CXX) $(CXXFLAGS) $(RCTOOLDIR)/mrc2grc.cpp -o $@ -static-libgcc

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.rc.o:
	$(RCTOOL) $< $*_grc.rc
	$(RC) --language=0411 $(DEFINES) $*_grc.rc -o $@
	$(RM) $*_grc.rc

clean:
	$(RM) $(plugin) $(OBJS) StdAfx.h.gch

depend: 
	$(CXX) -E -MM -w $(DEFINES) $(CXXFLAGS) *.cpp ../common/*.cpp ../utility/*.cpp > depend.mak

.SUFFIXES: .cpp .o .rc
.PHONY: all clean depend

-include depend.mak
