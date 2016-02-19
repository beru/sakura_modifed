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
SUBWCREV= SubWCRev.exe

DEFINES= \
 -DWIN32 \
 -DWINVER=0x500 \
 -D_WIN32_WINNT=0x500 \
 -D_WIN32_IE=0x501 \
 -DNOMINMAX \
 -D_UNICODE \
 -DUNICODE \
 -DNDEBUG
CFLAGS= -O2 \
 -finput-charset=cp932 -fexec-charset=cp932 \
 -I. \
 $(DEFINES) $(MYCFLAGS)
CXXFLAGS= $(CFLAGS) $(MYCXXFLAGS)
LIBS= \
 -lkernel32 -luser32 -lgdi32 \
 -lwinspool -lcomdlg32 -ladvapi32 \
 -lshell32 -lole32 -loleaut32 \
 -luuid -lcomctl32 -limm32 \
 -lmpr -limagehlp -lshlwapi \
 -static-libgcc -static-libstdc++ -mwindows -s

exe= sakura.exe

# ls *.cpp */*.cpp */*/*.cpp | sed -E -e "s/([[:alnum:]_]+)\.[[:alnum:]]+/\1.o \\\\/"
OBJS= \
AutoReloadAgent.o \
AutoSaveAgent.o \
BackupAgent.o \
CodeChecker.o \
DataProfile.o \
DicMgr.o \
EditApp.o \
Eol.o \
FileExt.o \
GrepAgent.o \
GrepEnumKeys.o \
HokanMgr.o \
KeyWordSetMgr.o \
LoadAgent.o \
MarkMgr.o \
Ope.o \
OpeBlk.o \
OpeBuf.o \
Profile.o \
PropertyManager.o \
ReadManager.o \
RegexKeyword.o \
SaveAgent.o \
SearchAgent.o \
SelectLang.o \
SortedTagJumpList.o \
StdAfx.o \
WriteManager.o \
apiwrap/StdApi.o \
apiwrap/StdControl.o \
basis/LaxInteger.o \
basis/MyPoint.o \
basis/MyRect.o \
basis/MySize.o \
basis/MyString.o \
basis/SakuraBasis.o \
basis/StrictInteger.o \
basis/StrictPoint.o \
basis/StrictRange.o \
basis/StrictRect.o \
charset/Cesu8.o \
charset/charcode.o \
charset/charset.o \
charset/CodeBase.o \
charset/codecheck.o \
charset/CodeFactory.o \
charset/CodeMediator.o \
charset/CodePage.o \
charset/codeutil.o \
charset/ESI.o \
charset/Euc.o \
charset/Jis.o \
charset/Latin1.o \
charset/ShiftJis.o \
charset/Unicode.o \
charset/UnicodeBe.o \
charset/Utf7.o \
charset/Utf8.o \
cmd/ViewCommander.o \
cmd/ViewCommander_Bookmark.o \
cmd/ViewCommander_Clipboard.o \
cmd/ViewCommander_Convert.o \
cmd/ViewCommander_Cursor.o \
cmd/ViewCommander_CustMenu.o \
cmd/ViewCommander_Diff.o \
cmd/ViewCommander_Edit.o \
cmd/ViewCommander_Edit_advanced.o \
cmd/ViewCommander_Edit_word_line.o \
cmd/ViewCommander_File.o \
cmd/ViewCommander_Grep.o \
cmd/ViewCommander_Insert.o \
cmd/ViewCommander_Macro.o \
cmd/ViewCommander_ModeChange.o \
cmd/ViewCommander_Outline.o \
cmd/ViewCommander_Search.o \
cmd/ViewCommander_Select.o \
cmd/ViewCommander_Settings.o \
cmd/ViewCommander_Support.o \
cmd/ViewCommander_TagJump.o \
cmd/ViewCommander_Window.o \
convert/Convert.o \
convert/Convert_HaneisuToZeneisu.o \
convert/Convert_HankataToZenhira.o \
convert/Convert_HankataToZenkata.o \
convert/Convert_SpaceToTab.o \
convert/Convert_TabToSpace.o \
convert/Convert_ToHankaku.o \
convert/Convert_ToLower.o \
convert/Convert_ToUpper.o \
convert/Convert_ToZenhira.o \
convert/Convert_ToZenkata.o \
convert/Convert_Trim.o \
convert/convert_util.o \
convert/convert_util2.o \
convert/Convert_ZeneisuToHaneisu.o \
convert/Convert_ZenkataToHankata.o \
convert/Decode_Base64Decode.o \
convert/Decode_UuDecode.o \
debug/Debug1.o \
debug/Debug2.o \
debug/Debug3.o \
debug/RunningTimer.o \
dlg/Dialog.o \
dlg/DlgAbout.o \
dlg/DlgCancel.o \
dlg/DlgCompare.o \
dlg/DlgCtrlCode.o \
dlg/DlgDiff.o \
dlg/DlgExec.o \
dlg/DlgFavorite.o \
dlg/DlgFileUpdateQuery.o \
dlg/DlgFind.o \
dlg/DlgGrep.o \
dlg/DlgGrepReplace.o \
dlg/DlgInput1.o \
dlg/DlgJump.o \
dlg/DlgOpenFile.o \
dlg/DlgPluginOption.o \
dlg/DlgPrintSetting.o \
dlg/DlgProfileMgr.o \
dlg/DlgProperty.o \
dlg/DlgReplace.o \
dlg/DlgSetCharSet.o \
dlg/DlgTagJumpList.o \
dlg/DlgTagsMake.o \
dlg/DlgWinSize.o \
doc/BlockComment.o \
doc/DocEditor.o \
doc/DocFile.o \
doc/DocFileOperation.o \
doc/DocListener.o \
doc/DocLocker.o \
doc/DocOutline.o \
doc/DocReader.o \
doc/DocType.o \
doc/DocTypeSetting.o \
doc/DocVisitor.o \
doc/EditDoc.o \
doc/LineComment.o \
doc/layout/Layout.o \
doc/layout/LayoutMgr.o \
doc/layout/LayoutMgr_DoLayout.o \
doc/layout/LayoutMgr_New.o \
doc/layout/LayoutMgr_New2.o \
doc/logic/DocLine.o \
doc/logic/DocLineMgr.o \
docplus/BookmarkManager.o \
docplus/DiffManager.o \
docplus/FuncListManager.o \
docplus/ModifyManager.o \
env/AppNodeManager.o \
env/CommonSetting.o \
env/DllSharedData.o \
env/DocTypeManager.o \
env/FileNameManager.o \
env/FormatManager.o \
env/HelpManager.o \
env/SakuraEnvironment.o \
env/SearchKeywordManager.o \
env/ShareData.o \
env/ShareData_IO.o \
env/TagJumpManager.o \
extmodule/Bregexp.o \
extmodule/BregexpDll2.o \
extmodule/DllHandler.o \
extmodule/HtmlHelp.o \
extmodule/Migemo.o \
extmodule/UxTheme.o \
func/Funccode.o \
func/FuncKeyWnd.o \
func/FuncLookup.o \
func/KeyBind.o \
io/BinaryStream.o \
io/File.o \
io/FileLoad.o \
io/IoBridge.o \
io/Stream.o \
io/TextStream.o \
io/ZipFile.o \
macro/CookieManager.o \
macro/EditorIfObj.o \
macro/IfObj.o \
macro/KeyMacroMgr.o \
macro/Macro.o \
macro/MacroFactory.o \
macro/MacroManagerBase.o \
macro/PluginMacroManager.o \
macro/PPA.o \
macro/PPAMacroMgr.o \
macro/SMacroMgr.o \
macro/WSH.o \
macro/WSHIfObj.o \
macro/WSHManager.o \
mem/Memory.o \
mem/Native.o \
mem/NativeA.o \
mem/NativeW.o \
mem/RecycledBuffer.o \
mfclike/MyWnd.o \
outline/DlgFileTree.o \
outline/DlgFuncList.o \
outline/FuncInfo.o \
outline/FuncInfoArr.o \
parse/WordParse.o \
plugin/DllPlugin.o \
plugin/JackManager.o \
plugin/Plugin.o \
plugin/PluginIfObj.o \
plugin/PluginManager.o \
plugin/WSHPlugin.o \
print/Print.o \
print/PrintPreview.o \
prop/PropComBackup.o \
prop/PropComCustmenu.o \
prop/PropComEdit.o \
prop/PropComFile.o \
prop/PropComFileName.o \
prop/PropComFormat.o \
prop/PropComGeneral.o \
prop/PropComGrep.o \
prop/PropComHelper.o \
prop/PropComKeybind.o \
prop/PropComKeyword.o \
prop/PropComMacro.o \
prop/PropComMainMenu.o \
prop/PropCommon.o \
prop/PropComPlugin.o \
prop/PropComStatusbar.o \
prop/PropComTab.o \
prop/PropComToolbar.o \
prop/PropComWin.o \
recent/MRUFile.o \
recent/MRUFolder.o \
recent/MruListener.o \
recent/Recent.o \
recent/RecentCmd.o \
recent/RecentCurDir.o \
recent/RecentEditNode.o \
recent/RecentExceptMru.o \
recent/RecentFile.o \
recent/RecentFolder.o \
recent/RecentGrepFile.o \
recent/RecentGrepFolder.o \
recent/RecentImp.o \
recent/RecentReplace.o \
recent/RecentSearch.o \
recent/RecentTagjumpKeyword.o \
typeprop/DlgKeywordSelect.o \
typeprop/DlgSameColor.o \
typeprop/DlgTypeAscertain.o \
typeprop/DlgTypeList.o \
typeprop/ImpExpManager.o \
typeprop/PropTypes.o \
typeprop/PropTypesColor.o \
typeprop/PropTypesKeyHelp.o \
typeprop/PropTypesRegex.o \
typeprop/PropTypesScreen.o \
typeprop/PropTypesSupport.o \
typeprop/PropTypesWindow.o \
types/Type.o \
types/TypeSupport.o \
types/Type_Asm.o \
types/Type_Awk.o \
types/Type_Basis.o \
types/Type_Cobol.o \
types/Type_CorbaIdl.o \
types/Type_Cpp.o \
types/Type_Dos.o \
types/Type_Erlang.o \
types/Type_Html.o \
types/Type_Ini.o \
types/Type_Java.o \
types/Type_Others.o \
types/Type_Pascal.o \
types/Type_Perl.o \
types/Type_Python.o \
types/Type_Rich.o \
types/Type_Sql.o \
types/Type_Tex.o \
types/Type_Text.o \
types/Type_Vb.o \
uiparts/Graphics.o \
uiparts/ImageListMgr.o \
uiparts/MenuDrawer.o \
uiparts/SoundSet.o \
uiparts/VisualProgress.o \
uiparts/WaitCursor.o \
util/fileUtil.o \
util/format.o \
util/input.o \
util/MessageBoxF.o \
util/module.o \
util/ole_convert.o \
util/os.o \
util/other_util.o \
util/relation_tool.o \
util/shell.o \
util/string_ex.o \
util/string_ex2.o \
util/tchar_convert.o \
util/tchar_printf.o \
util/tchar_receive.o \
util/tchar_template.o \
util/window.o \
view/Caret.o \
view/DispPos.o \
view/EditView.o \
view/EditView_Cmdgrep.o \
view/EditView_CmdHokan.o \
view/EditView_Cmdisrch.o \
view/EditView_Command.o \
view/EditView_Command_New.o \
view/EditView_Diff.o \
view/EditView_ExecCmd.o \
view/EditView_Ime.o \
view/EditView_Mouse.o \
view/EditView_Paint.o \
view/EditView_Paint_Bracket.o \
view/EditView_Scroll.o \
view/EditView_Search.o \
view/Ruler.o \
view/TextArea.o \
view/TextDrawer.o \
view/TextMetrics.o \
view/ViewCalc.o \
view/ViewFont.o \
view/ViewParser.o \
view/ViewSelect.o \
view/colors/ColorStrategy.o \
view/colors/Color_Comment.o \
view/colors/Color_Found.o \
view/colors/Color_Heredoc.o \
view/colors/Color_KeywordSet.o \
view/colors/Color_Numeric.o \
view/colors/Color_Quote.o \
view/colors/Color_RegexKeyword.o \
view/colors/Color_Url.o \
view/figures/FigureManager.o \
view/figures/FigureStrategy.o \
view/figures/Figure_CtrlCode.o \
view/figures/Figure_Eol.o \
view/figures/Figure_HanSpace.o \
view/figures/Figure_Tab.o \
view/figures/Figure_ZenSpace.o \
window/AutoScrollWnd.o \
window/EditWnd.o \
window/MainStatusBar.o \
window/MainToolBar.o \
window/SplitBoxWnd.o \
window/SplitterWnd.o \
window/TabWnd.o \
window/TipWnd.o \
window/Wnd.o \
_main/AppMode.o \
_main/CommandLine.o \
_main/ControlProcess.o \
_main/ControlTray.o \
_main/global.o \
_main/NormalProcess.o \
_main/Process.o \
_main/ProcessFactory.o \
_main/WinMain.o \
_os/Clipboard.o \
_os/DropTarget.o \
_os/OsVersionInfo.o \
sakura_rc.o \

RCTOOLDIR=../btool
RCTOOL=$(RCTOOLDIR)/mrc2grc.exe
HEADERMAKETOOLDIR= ../HeaderMake
HEADERMAKE= $(HEADERMAKETOOLDIR)/HeaderMake.exe

all: $(RCTOOL) $(HEADERMAKE) $(exe)

$(exe): Funccode_define.h Funccode_enum.h svnrev stdafx $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

Funccode_define.h: Funccode_x.hsrc
	$(HEADERMAKE) -in=../sakura_core/Funccode_x.hsrc -out=../sakura_core/Funccode_define.h -mode=define

Funccode_enum.h: Funccode_x.hsrc
	$(HEADERMAKE) -in=../sakura_core/Funccode_x.hsrc -out=../sakura_core/Funccode_enum.h -mode=enum -enum=EFunctionCode

svnrev:
	cp svnrev_unknown.h svnrev.h
	-$(SUBWCREV) ".\\" ".\svnrev_template.h" ".\svnrev.h"

stdafx:
	$(CXX) $(CXXFLAGS) -c StdAfx.h

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(RCTOOL): $(RCTOOLDIR)/mrc2grc.cpp
	$(CXX) $(CXXFLAGS) $(RCTOOLDIR)/mrc2grc.cpp -o $@ -static-libgcc

$(HEADERMAKE): $(HEADERMAKETOOLDIR)/HeaderMake.cpp
	$(CXX) $(CXXFLAGS) $(HEADERMAKETOOLDIR)/HeaderMake.cpp -o $@ -static-libgcc

.rc.o:
	$(RCTOOL) $< sakura_grc.rc
	$(RC) --language=0411 $(DEFINES) sakura_grc.rc -o $@
	$(RM) sakura_grc.rc

clean:
	$(RM) $(exe) $(OBJS) $(RCTOOL) $(HEADERMAKE) StdAfx.h.gch

depend: svnrev
	$(CXX) -E -MM -w $(DEFINES) $(CXXFLAGS) *.cpp */*.cpp */*/*.cpp > depend.mak

.SUFFIXES: .cpp .o .rc
.PHONY: all clean depend

-include depend.mak
