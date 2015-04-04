/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "dcgf.h"
#include "PlatformSDL.h"
#include "BRenderSDL.h"
#include "PathUtil.h"

#ifdef __WIN32__
#	include <dbghelp.h>
#	include <direct.h>
#else
#	include <unistd.h>
#	include <dirent.h>
#   include <sys/stat.h>
#endif

#ifdef __ANDROID__
#	include "android/android.h"
#endif

CBGame* CBPlatform::Game = NULL;


#define CLASS_NAME "GF_FRAME"
//////////////////////////////////////////////////////////////////////////
int CBPlatform::Initialize(CBGame* inGame, int argc, char* argv[])
{
	setlocale(LC_CTYPE, "");

	Game = inGame;
	if(!Game) return 1;


	bool windowedMode = false;
	

	// parse command line
	char* SaveGame = NULL;
	char param[MAX_PATH];
	for(int i = 0; i < argc; i++)
	{
		strcpy(param, argv[i]);

		if(CBPlatform::stricmp(param, "-project")==0)
		{
			if (argc > i) strcpy(param, argv[i + 1]);
			else param[0] = '\0';

			if(strcmp(param, "") != 0)
			{
				char* IniDir = CBUtils::GetPath(param);
				char* IniName = CBUtils::GetFilename(param);

				// switch to ini's dir
				chdir(IniDir);

				// set ini name
				sprintf(param, "./%s", IniName);
				Game->m_Registry->SetIniName(param);

				delete [] IniDir;
				delete [] IniName;
			}
		}
		else if(CBPlatform::stricmp(param, "-windowed")==0) windowedMode = true;
	}


	if(Game->m_Registry->ReadBool("Debug", "DebugMode")) Game->DEBUG_DebugEnable("./wme.log");

	Game->m_DEBUG_ShowFPS = Game->m_Registry->ReadBool("Debug", "ShowFPS");

	if(Game->m_Registry->ReadBool("Debug", "DisableSmartCache"))
	{
		Game->LOG(0, "Smart cache is DISABLED");
		Game->m_SmartCache = false;
	}

	//bool AllowDirectDraw = Game->m_Registry->ReadBool("Debug", "AllowDirectDraw", false);

	// load general game settings
	Game->Initialize1();


	if(FAILED(Game->LoadSettings("startup.settings")))
	{
		Game->LOG(0, "Error loading game settings.");
		SAFE_DELETE(Game);

#ifdef __WIN32__
		::MessageBox(NULL, "Some of the essential files are missing. Please reinstall.", NULL, MB_OK|MB_ICONERROR);
#endif
		return 2;
	}

	Game->Initialize2();

	Game->GetDebugMgr()->OnGameInit();
	Game->m_ScEngine->LoadBreakpoints();

	// read values for up- and downscale stepping from registry
	float upScalingStepping = (float) (((float) Game->m_Registry->ReadInt("Scaling", "UpScalingStepping", 0)) / 100.0);
	float downScalingStepping = (float) (((float) Game->m_Registry->ReadInt("Scaling", "DownScalingStepping", 0)) / 100.0);

	Game->LOG(0, "Scaling steppings found up=%.02f down=%.02f.", upScalingStepping, downScalingStepping);

	bool pixelPerfectRendering = Game->m_Registry->ReadBool("Rendering", "PixelPerfect");

	HRESULT ret;

	AnsiString renderingHint = Game->m_Registry->ReadString("Rendering", "RenderingHint", "default");

	bool vsync = Game->m_Registry->ReadBool("Rendering", "VSync");

	// initialize the renderer
	ret = Game->m_Renderer->InitRenderer(Game->m_SettingsResWidth, Game->m_SettingsResHeight, windowedMode, upScalingStepping, downScalingStepping, pixelPerfectRendering, renderingHint, vsync);
	if (FAILED(ret))
	{
		Game->LOG(ret, "Error initializing renderer. Exiting.");
		SAFE_DELETE(Game);
		return 3;
	}

	Game->Initialize3();	
	
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	SDL_AddEventWatch(CBPlatform::SDLEventWatcher, NULL);
#endif
	
	// initialize sound manager (non-fatal if we fail)
	ret = Game->m_SoundMgr->Initialize();
	if(FAILED(ret))
	{
		Game->LOG(ret, "Sound is NOT available.");
	}
		

	// load game
	DWORD DataInitStart = GetTime();

	if(FAILED(Game->LoadFile(Game->m_SettingsGameFile?Game->m_SettingsGameFile:"default.game")))
	{
		Game->LOG(ret, "Error loading game file. Exiting.");
		SAFE_DELETE(Game);
		return false;
	}
	Game->SetWindowTitle();
	Game->m_Renderer->m_Ready = true;
	Game->m_MiniUpdateEnabled = true;

	Game->LOG(0, "Engine initialized in %d ms", GetTime() - DataInitStart);
	Game->LOG(0, "");

	
	if(SaveGame)
	{
		Game->LoadGame(SaveGame);
		delete [] SaveGame;
	}

	// all set, ready to go
	return 0;
}


//////////////////////////////////////////////////////////////////////////
int CBPlatform::MessageLoop()
{
	bool done = false;

	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			HandleEvent(&event);
		}

		if(Game && Game->m_Renderer->m_Active && Game->m_Renderer->m_Ready)
		{

			Game->DisplayContent();
			Game->DisplayQuickMsg();

			Game->DisplayDebugInfo();

			// ***** flip
			if(!Game->m_SuspendedRendering) Game->m_Renderer->Flip();
			if(Game->m_Loading) Game->LoadGame(Game->m_ScheduledLoadSlot);
		}
		if(Game->m_Quitting) break;

	}

	if(Game)
	{
		// remember previous window position
		/*
		if(Game->m_Renderer && Game->m_Renderer->m_Windowed)
		{
			if(!::IsIconic(Game->m_Renderer->m_Window))
			{
				int PosX = Game->m_Renderer->m_WindowRect.left;
				int PosY = Game->m_Renderer->m_WindowRect.top;
				PosX -= Game->m_Renderer->m_MonitorRect.left;
				PosY -= Game->m_Renderer->m_MonitorRect.top;

				Game->m_Registry->WriteInt("Video", "WindowPosX", PosX);
				Game->m_Registry->WriteInt("Video", "WindowPosY", PosY);
			}
		}
		*/

		SAFE_DELETE(Game);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::HandleEvent(SDL_Event* event)
{
	switch (event->type)
	{

	case SDL_MOUSEBUTTONDOWN:

#if defined(__IPHONEOS__) || defined(__ANDROID__)
		{
			CBRenderSDL* renderer = static_cast<CBRenderSDL*>(Game->m_Renderer);
			POINT p;
			GetCursorPos(&p);			
			Game->SetActiveObject(renderer->GetObjectAt(p.x, p.y));
			
			if (Game->m_ActiveObject != NULL && strcmp(Game->m_ActiveObject->GetClassName(), "CUIButton") == 0)
			{
				CUIButton* btn = static_cast<CUIButton*> (Game->m_ActiveObject);
				if (btn->m_Visible && !btn->m_Disable) btn->m_Press = true;
			}
		}
#endif
		switch (event->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (Game)
			{
				if (Game->IsLeftDoubleClick()) Game->OnMouseLeftDblClick();
				else Game->OnMouseLeftDown();
			}
			break;
		case SDL_BUTTON_RIGHT:
			if (Game)
			{
				if (Game->IsRightDoubleClick()) Game->OnMouseRightDblClick();
				else Game->OnMouseRightDown();
			}
			break;
		case SDL_BUTTON_MIDDLE:
			if (Game) Game->OnMouseMiddleDown();
			break;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		switch (event->button.button)
		{
		case SDL_BUTTON_LEFT:
			if (Game) Game->OnMouseLeftUp();
			break;
		case SDL_BUTTON_RIGHT:
			if (Game) Game->OnMouseRightUp();
			break;
		case SDL_BUTTON_MIDDLE:
			if (Game) Game->OnMouseMiddleUp();
			break;
		}
		break;
	
	case SDL_MOUSEWHEEL:
		if (Game) Game->HandleMouseWheel(event->wheel.y);
		break;
	
	case SDL_KEYDOWN:
	case SDL_TEXTINPUT:
		if (Game) Game->HandleKeypress(event);
		break;

	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_RESTORED:
			if (Game) Game->OnActivate(true, true);
			SDL_ShowCursor(SDL_DISABLE);
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_MINIMIZED:
// #if !defined(__IPHONEOS__) && !defined(__ANDROID__)
			if (Game) Game->OnActivate(false, false);
			SDL_ShowCursor(SDL_ENABLE);
// #endif
			break;

		case SDL_WINDOWEVENT_CLOSE:
			break;

		}
		break;

	case SDL_QUIT:
#if defined(__IPHONEOS__) || defined(__ANDROID__)
		if (Game)
		{
			Game->AutoSaveOnExit();
			Game->m_Quitting = true;
		}
#else
		if (Game) Game->OnWindowClose();
#endif
		
		break;

	}
}

//////////////////////////////////////////////////////////////////////////
int CBPlatform::SDLEventWatcher(void* userdata, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_MINIMIZED)
	{
		if (Game) Game->AutoSaveOnExit();
		if (Game) Game->OnActivate(false, false);
		SDL_ShowCursor(SDL_ENABLE);
	}
	
	return 1;
}


//////////////////////////////////////////////////////////////////////////
// Win32 API bindings
//////////////////////////////////////////////////////////////////////////
HINSTANCE CBPlatform::ShellExecute(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
#ifdef __WIN32__
	return ::ShellExecute(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::OutputDebugString(LPCSTR lpOutputString)
{
#ifdef __WIN32__
	::OutputDebugString(lpOutputString);
#endif
}


//////////////////////////////////////////////////////////////////////////
DWORD CBPlatform::GetTime()
{
	return SDL_GetTicks();
}

//////////////////////////////////////////////////////////////////////////
void CBPlatform::SleepMs(DWORD millis)
{
	SDL_Delay(millis);
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::GetCursorPos(LPPOINT lpPoint)
{
	CBRenderSDL* renderer = static_cast<CBRenderSDL*>(Game->m_Renderer);

	int x, y;

	SDL_GetMouseState(&x, &y);
	lpPoint->x = x;
	lpPoint->y = y;

	renderer->PointFromScreen(lpPoint);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetCursorPos(int X, int Y)
{
	CBRenderSDL* renderer = static_cast<CBRenderSDL*>(Game->m_Renderer);

	POINT p;
	p.x = X;
	p.y = Y;
	renderer->PointToScreen(&p);

	SDL_WarpMouseInWindow(renderer->GetSdlWindow(), p.x, p.y);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::ShowWindow(HWND hWnd, int nCmdShow)
{
#ifdef __WIN32__
	return ::ShowWindow(hWnd, nCmdShow);
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::DeleteFile(const char* lpFileName)
{
	return remove(lpFileName) ? true : false;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::CopyFile(const char* from, const char* to, bool failIfExists)
{
	if (PathUtil::FileExists(to)) return false;

	FILE* source = fopen(from, "rb");
	if (!source) return false;

	FILE* target = fopen(to, "wb");
	if (!target)
	{
		fclose(source);
		return false;
	}

	bool ret = true;

	char buf[4096];
	size_t numBytes;

	while ((numBytes = fread(buf, sizeof(char), sizeof(buf), source)) > 0)
	{
		if (fwrite(buf, sizeof(char), numBytes, target) != numBytes)
		{
			ret = false;
			break;
		}
	}
	fclose(source);
	fclose(target);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CBPlatform::CreateDirectory(const char* path)
{
#ifdef _WIN32
	int ret = _mkdir(path);
#else
	int ret = mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	return (ret == 0);
}

//////////////////////////////////////////////////////////////////////////
HWND CBPlatform::SetCapture(HWND hWnd)
{
#ifdef __WIN32__
	return ::SetCapture(hWnd);
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::ReleaseCapture()
{
#ifdef __WIN32__
	return ::ReleaseCapture();
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetForegroundWindow(HWND hWnd)
{
#ifdef __WIN32__
	return ::SetForegroundWindow(hWnd);
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetRectEmpty(LPRECT lprc)
{
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::IsRectEmpty(CONST LPRECT lprc)
{
	return (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::PtInRect(LPRECT lprc, POINT p)
{
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::SetRect(LPRECT lprc, int left, int top, int right, int bottom)
{
	lprc->left   = left;
	lprc->top    = top;
	lprc->right  = right;
	lprc->bottom = bottom;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::IntersectRect(LPRECT lprcDst, CONST LPRECT lprcSrc1, CONST LPRECT lprcSrc2)
{
	if (IsRectEmpty(lprcSrc1) || IsRectEmpty(lprcSrc2) ||
		lprcSrc1->left >= lprcSrc2->right || lprcSrc2->left >= lprcSrc1->right ||
		lprcSrc1->top >= lprcSrc2->bottom || lprcSrc2->top >= lprcSrc1->bottom)
	{
		SetRectEmpty(lprcDst);
		return FALSE;
	}
	lprcDst->left   = max(lprcSrc1->left, lprcSrc2->left);
	lprcDst->right  = min(lprcSrc1->right, lprcSrc2->right);
	lprcDst->top    = max(lprcSrc1->top, lprcSrc2->top);
	lprcDst->bottom = min(lprcSrc1->bottom, lprcSrc2->bottom);
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::UnionRect(LPRECT lprcDst, RECT* lprcSrc1, RECT* lprcSrc2)
{
	if (IsRectEmpty(lprcSrc1))
	{
		if (IsRectEmpty(lprcSrc2))
		{
			SetRectEmpty(lprcDst);
			return FALSE;
		}
		else
		{
			*lprcDst = *lprcSrc2;
		}
	}
	else
	{
		if (IsRectEmpty(lprcSrc2))
		{
			*lprcDst = *lprcSrc1;
		}
		else
		{
			lprcDst->left   = min(lprcSrc1->left, lprcSrc2->left);
			lprcDst->top    = min(lprcSrc1->top, lprcSrc2->top);
			lprcDst->right  = max(lprcSrc1->right, lprcSrc2->right);
			lprcDst->bottom = max(lprcSrc1->bottom, lprcSrc2->bottom);
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::CopyRect(LPRECT lprcDst, RECT* lprcSrc)
{
	if (lprcDst == NULL || lprcSrc == NULL) return FALSE;

	*lprcDst = *lprcSrc;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::OffsetRect(LPRECT lprc, int dx, int dy)
{
	if (lprc == NULL) return FALSE;

	lprc->left   += dx;
	lprc->top    += dy;
	lprc->right  += dx;
	lprc->bottom += dy;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CBPlatform::EqualRect(LPRECT rect1, LPRECT rect2)
{
	return rect1->left == rect2->left && rect1->right == rect2->right && rect1->top == rect2->top && rect1->bottom == rect2->bottom;
}


//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::GetSystemFontPath()
{
#ifdef __WIN32__
	// we're looking for something like "c:\windows\fonts\";
	char winDir[MAX_PATH + 1];
	winDir[MAX_PATH] = '\0';
	::GetWindowsDirectory(winDir, MAX_PATH);
	return PathUtil::Combine(AnsiString(winDir), "fonts");
#elif __ANDROID__
	// there is no system font path but an app can bring
	// custom fonts which could be referenced
	char androidPath[1024];
	android_getFontPath(androidPath, 1024);
	return AnsiString(androidPath);
#else
	// !PORTME
	return "/Library/Fonts/";
#endif
}

//////////////////////////////////////////////////////////////////////////
AnsiString CBPlatform::GetPlatformName()
{
	return AnsiString(SDL_GetPlatform());
}

//////////////////////////////////////////////////////////////////////////
int CBPlatform::stricmp(const char* str1, const char* str2)
{
#ifdef __WIN32__
	return ::stricmp(str1, str2);
#else
	return ::strcasecmp(str1, str2);
#endif
}

//////////////////////////////////////////////////////////////////////////
int CBPlatform::strnicmp(const char* str1, const char* str2, size_t maxCount)
{
#ifdef __WIN32__
	return ::strnicmp(str1, str2, maxCount);
#else
	return ::strncasecmp(str1, str2, maxCount);
#endif
}


//////////////////////////////////////////////////////////////////////////
char* CBPlatform::strupr(char* string)
{
	if (string)
	{
		for (size_t i = 0; i < strlen(string); ++i)
		{
			string[i] = toupper(string[i]);
		}
	}
	return string;
}

//////////////////////////////////////////////////////////////////////////
char* CBPlatform::strlwr(char* string)
{
	if (string)
	{
		for (size_t i = 0; i < strlen(string); ++i)
		{
			string[i] = tolower(string[i]);
		}
	}
	return string;
}
