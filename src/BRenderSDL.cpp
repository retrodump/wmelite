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
#include "BRenderSDL.h"
#include "BSurfaceSDL.h"
#include "FreeImage.h"
#include "MathUtil.h"
#include "StringUtil.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::CBRenderSDL(CBGame* inGame) : CBRenderer(inGame)
{
	m_Texture = NULL;
	m_Renderer = NULL;
	m_Win = NULL;

	m_BorderLeft = m_BorderRight = m_BorderTop = m_BorderBottom = 0;
	m_RatioX = m_RatioY = 1.0f;
	m_PixelPerfect = false;
}

//////////////////////////////////////////////////////////////////////////
CBRenderSDL::~CBRenderSDL()
{
	if (m_Texture) SDL_DestroyTexture(m_Texture);
	if (m_Renderer) SDL_DestroyRenderer(m_Renderer);
	if (m_Win) SDL_DestroyWindow(m_Win);
	
	SDL_Quit();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::InitRenderer(int width, int height, bool windowed, float upScalingRatioStepping, float downScalingRatioStepping, bool pixelPerfectRendering, const AnsiString& renderingHint, bool vsync)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) return E_FAIL;
	
	const SDL_DisplayMode* current = NULL;

	// for searching matching resolution, always remember "best" pick
	SDL_DisplayMode bestResolution;
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);

	bool doubleBuffering = Game->m_Registry->ReadBool("Rendering", "DoubleBuffering", false);

	Game->LOG(0, "Double buffering flag set to: %s.", (doubleBuffering == true) ? "ON" : "default");
	if (doubleBuffering == true)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	}

	int deviceNumber = Game->m_Registry->ReadInt("Video", "DeviceNumber", 0);

	Game->LOG(0, "Current video device no=%d, max=%d (- 1).", deviceNumber, SDL_GetNumVideoDisplays());

	m_Width = width;
	m_Height = height;

	m_RealWidth = width;
	m_RealHeight = height;

	m_PixelPerfect = pixelPerfectRendering;

	// find suitable resolution
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	// mobile devices: search all resolutions until one is found where width > height

	m_RealWidth = 480;
	m_RealHeight = 320;

	int numModes = SDL_GetNumDisplayModes(deviceNumber);
	for (int i = 0; i < numModes; i++)
	{
		SDL_GetDisplayMode(deviceNumber, i, &bestResolution);

		if (bestResolution.w > bestResolution.h)
		{
			m_RealWidth = bestResolution.w;
			m_RealHeight = bestResolution.h;
			current = &bestResolution;
			break;
		}
	}	
#else
	// windows/linux: search a resolution with same screen width/height ratio
	// inspired by Jan Kavan (wmelite julia branch)
	SDL_DisplayMode testResolution;

	SDL_GetCurrentDisplayMode(deviceNumber, &bestResolution);

	// compute ratios precisely
	float gameRatio = (float)width / (float)height;
	float bestRatio = (float)bestResolution.w / (float)bestResolution.h;
	float bestRatioDiff = gameRatio - bestRatio;

	Game->LOG(0, "Game dimension=%dx%d, ratio=%.2f.", width, height, gameRatio);

	Game->LOG(0, "Starting best screen dimension=%dx%d, ratio=%.2f, ratio diff=%.2f.", bestResolution.w, bestResolution.h, bestRatio, bestRatioDiff);

	if (Game->m_Registry->ReadBool("Video", "DesktopResolution", false) == false)
	{
		// now iterate over all modes of this screen and pick the best one
		for (int i = 0; i < SDL_GetNumDisplayModes(deviceNumber); i++)
		{
			if (SDL_GetDisplayMode(deviceNumber, i, &testResolution) == 0)
			{
				 float testRatio = (float)testResolution.w / (float)testResolution.h;
				 float testRatioDiff = gameRatio - testRatio;

				 Game->LOG(0, "Other screen dimension=%dx%d, ratio=%.2f, ratio diff=%.2f.", testResolution.w, testResolution.h, testRatio, testRatioDiff);

				 // the current screen ratio is not equal to the last best one
				 if (MathUtil::FloatsAreEqual(bestRatioDiff, testRatioDiff) == false)
				 {
					// new mode is bigger than game resolution (do not pick different mode that needs downscaling)
					if ((testResolution.w >= width) && (testResolution.h >= height))
					{
						// the ratio fits better
						if (fabs(testRatioDiff) < fabs(bestRatioDiff))
						{
							Game->LOG(0, "New mode better than current (%.2f < %.2f), picking this one!", testRatioDiff, bestRatioDiff);

							bestRatioDiff  = testRatioDiff;
							bestRatio      = testRatio;
							bestResolution = testResolution;
						}
					}
				 }
			}
		}
	}
	else
	{
		Game->LOG(0, "Desktop mode forced!");
	}

	// this is our resulting mode
	current = &bestResolution;

	m_RealWidth = current->w;
	m_RealHeight = current->h;

#endif

	// last chance to override resolution settings with registry
	if ((Game->m_Registry->ReadInt("Debug", "ForceResWidth", 0) != 0) && (Game->m_Registry->ReadInt("Debug", "ForceResHeight", 0) != 0)) {
		m_RealWidth = Game->m_Registry->ReadInt("Debug", "ForceResWidth", m_Width);
		m_RealHeight = Game->m_Registry->ReadInt("Debug", "ForceResHeight", m_Height);
	}

	float origAspect = (float)m_Width / (float)m_Height;
	float realAspect = (float)m_RealWidth / (float)m_RealHeight;

	float ratio;
	if (origAspect < realAspect)
	{
		// normal to wide
		ratio = (float)m_RealHeight / (float)m_Height;
	}
	else
	{
		// wide to normal
		ratio = (float)m_RealWidth / (float)m_Width;
	}

	Game->LOG(0, "Ratio before stepping application: %.02f", ratio);

	if (ratio > 1.0) {
		ratio = GetAlignedUpscalingRatio(ratio, upScalingRatioStepping);
	}
	if (ratio < 1.0) {
		ratio = GetAlignedDownscalingRatio(ratio, downScalingRatioStepping);
	}

	Game->LOG(0, "Ratio after stepping application: %.02f", ratio);

	m_BorderLeft = (m_RealWidth - (m_Width * ratio)) / 2;
	m_BorderRight = m_RealWidth - (m_Width * ratio) - m_BorderLeft;

	m_BorderTop = (m_RealHeight - (m_Height * ratio)) / 2;
	m_BorderBottom = m_RealHeight - (m_Height * ratio) - m_BorderTop;



	m_RatioX = (float)(m_RealWidth - m_BorderLeft - m_BorderRight) / (float)m_Width;
	m_RatioY = (float)(m_RealHeight - m_BorderTop - m_BorderBottom) / (float)m_Height;

// #ifdef __ANDROID__
	Game->LOG(0, "Orig w=%d h=%d Transformed w=%d h=%d ratiox=%.02f ratioy=%.02f",
			m_Width, m_Height, m_RealWidth, m_RealHeight,
			m_RatioX, m_RatioY);
	Game->LOG(0, "BorderLeft=%d BorderRight=%d BorderTop=%d BorderBottom=%d",
			m_BorderLeft, m_BorderRight, m_BorderTop, m_BorderBottom);
// #endif

	Uint32 flags = SDL_WINDOW_SHOWN;
#ifdef __IPHONEOS__
	flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
#else
#ifdef __ANDROID__
	// just do the same IOS does
	flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
#endif

#endif
	// registry wins over command line
	m_Windowed = Game->m_Registry->ReadBool("Video", "Windowed", windowed);
    
	// as of Novmber 2012 using SDL_WINDOW_FULLSCREEN causes iOS glitch when returning back to the app (the image is shifted several pixels up)
	// so better don't use it on iOS
	if (!m_Windowed) flags |= SDL_WINDOW_FULLSCREEN;

	m_Win = SDL_CreateWindow("WME Lite",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_RealWidth, m_RealHeight,
		flags);
	
	if (!m_Win) return E_FAIL;

	SDL_SetWindowDisplayMode(m_Win, current);	

	SDL_ShowCursor(SDL_DISABLE);

#ifdef __IPHONEOS__
	// SDL defaults to OGL ES2, which doesn't work on old devices
	AnsiString defaultRenderingHint = "opengles";
#else

#ifdef __ANDROID__
	// lets assume almost every active device has OpenGL ES2 now...
	AnsiString defaultRenderingHint = "opengles2";
#else

#ifdef __WIN32__
    // directx for Windows
	AnsiString defaultRenderingHint = "direct3d";

#else
	// OpenGL for Linux and all others
	AnsiString defaultRenderingHint = "opengl";

#endif

#endif

#endif

	if (StringUtil::CompareNoCase(renderingHint, "default") == true) {
		Game->LOG(0, "Set rendering hint default value: %s", defaultRenderingHint.c_str());
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, defaultRenderingHint.c_str());
	} else {
		Game->LOG(0, "Set rendering hint registry value: %s", renderingHint.c_str());
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, renderingHint.c_str());
	}


	if (vsync == true) SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

#ifdef __ANDROID__

	SDL_SetHint(SDL_HINT_ANDROID_USE_UI_LOW_PROFILE, "1");

#endif

	// m_Renderer = SDL_CreateRenderer(m_Win, -1, SDL_RENDERER_PRESENTVSYNC);
	m_Renderer = SDL_CreateRenderer(m_Win, -1, 0);
	if (!m_Renderer) return E_FAIL;

	if (m_PixelPerfect == true) {
		// try to make texture scaling look as nice as possible
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_Width, m_Height);
		if (!m_Texture) return E_FAIL;
		SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(m_Renderer, m_Texture);
		SDL_RenderSetViewport(m_Renderer, NULL);

		m_PixelPerfectTargetRect.x = m_BorderLeft;
		m_PixelPerfectTargetRect.y = m_BorderTop;
		m_PixelPerfectTargetRect.w = m_RealWidth;
		m_PixelPerfectTargetRect.h = m_RealHeight;

		Game->AddMem(m_Width * m_Height * 4);

		m_RenderOffscreen = true;

		Game->LOG(0, "PixelPerfect rendering enabled!");
	} else {
		m_RenderOffscreen = false;

		SDL_RenderSetViewport(m_Renderer, NULL);

		Game->LOG(0, "PixelPerfect rendering disabled!");
	}

	// necessary init and computations for experimental frame rate limit
	m_LastRenderTime = 0;
	m_SecondTickTime = 0;
	m_FrameCounter   = 0;
	m_FrameRateLimit = Game->m_Registry->ReadInt("Rendering", "FrameRateLimit", 0);
	
	// be as little invasive as possible, if the framerate is already ok or even too low
	// there's no need to sleep
	m_FrameRateLimitBrakeActive = false;
	m_AccumulatedSleepTime = 0;
	
	if (m_FrameRateLimit > 0)
	{
		m_FrameSleepTime = MathUtil::Round(1000.0f / ((float) m_FrameRateLimit));
		
	}

	m_Active = true;
	

	return S_OK;
}

/////////////////////////////////////////////////////
HRESULT CBRenderSDL::SendRenderingHintSceneComplete()
{
	if (m_PixelPerfect == true) {
		if (m_RenderOffscreen == true) {
			SDL_SetRenderTarget(m_Renderer, NULL);
			SDL_RenderSetViewport(m_Renderer, NULL);
			SDL_RenderCopy(m_Renderer, m_Texture, NULL, &m_PixelPerfectTargetRect);

#ifndef __IPHONEOS__
			SDL_RenderSetViewport(m_Renderer, &m_PixelPerfectTargetRect);
#endif

			m_RenderOffscreen = false;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Flip()
{
	
#ifdef __IPHONEOS__
	// hack: until viewports work correctly, we just paint black bars instead
    
	SDL_SetRenderDrawColor(m_Renderer, 0x00, 0x00, 0x00, 0xFF);

	static bool firstRefresh = true; // prevents a weird color glitch
	if (firstRefresh)
	{
		firstRefresh = false;
	}
	else
	{
		SDL_Rect rect;
		if (m_BorderLeft > 0)
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = m_BorderLeft;
			rect.h = m_RealHeight;		
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderRight > 0)
		{
			rect.x = (m_RealWidth - m_BorderRight);
			rect.y = 0;
			rect.w = m_BorderRight;
			rect.h = m_RealHeight;		
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderTop > 0)
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = m_RealWidth;
			rect.h = m_BorderTop;		
			SDL_RenderFillRect(m_Renderer, &rect);
		}
		if (m_BorderBottom > 0)
		{
			rect.x = 0;
			rect.y = m_RealHeight - m_BorderBottom;
			rect.w = m_RealWidth;
			rect.h = m_BorderBottom;		
			SDL_RenderFillRect(m_Renderer, &rect);
		}
	}
    
#endif

	// if not already done, draw the offscreen image onto the final screen
	SendRenderingHintSceneComplete();

	// last resort to limit frame rate if vsync does not work
	if (m_FrameRateLimit > 0)
	{
		// check how many milliseconds the rendering is "too fast"
		DWORD currRenderTime = CBPlatform::GetTime();
		DWORD renderDiffTime = currRenderTime - m_LastRenderTime;

		// accumulate the "requested" overall sleep time per second
		m_AccumulatedSleepTime += m_FrameSleepTime - renderDiffTime;

		// if the brake is active, sleep here
		if ((m_FrameRateLimitBrakeActive == true) && (renderDiffTime < m_FrameSleepTime))
		{
			CBPlatform::SleepMs(m_FrameSleepTime - renderDiffTime);
		}
		m_LastRenderTime = CBPlatform::GetTime();

		// once per second, check whether the brake was necessary 
		// and whether the sleep time needs adjustment
		m_FrameCounter++;
		if ((m_LastRenderTime - m_SecondTickTime) > 1000)
		{
			// modify the sleep time if the actual frame rate diverts from the requested one
			if ((m_FrameRateLimitBrakeActive == true) && (m_FrameCounter != m_FrameRateLimit))
			{
				if (m_FrameCounter > (m_FrameRateLimit + 1))
				{
					m_FrameSleepTime++;
				}
				else 
				{
					if (m_FrameCounter < (m_FrameRateLimit - 1))
					{
						if (m_FrameSleepTime > 0)
						{
							m_FrameSleepTime--;
						}
					}
				}
			}

			// check whether the "brake" needs to be switched on or off
			// depending on the rendering/sleeping time during the last second
			if (m_AccumulatedSleepTime > 60)
			{
				m_FrameRateLimitBrakeActive = true;
			}
			else
			{
				m_FrameRateLimitBrakeActive = false;
			}
			m_AccumulatedSleepTime = 0;

			m_FrameCounter = 0;
			m_SecondTickTime = m_LastRenderTime;
		}
	}

	SDL_RenderPresent(m_Renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::EraseBackground()
{
	Fill(0, 0, 0, NULL);

	// in case of pixelperfect rendering, both on- and offscreen
	// image need to be erased
	if (m_PixelPerfect == true) {
		SDL_SetRenderTarget(m_Renderer, m_Texture);
		SDL_RenderSetViewport(m_Renderer, NULL);
		Fill(0, 0, 0, NULL);
		m_RenderOffscreen = true;
	} else {
		// paranoia
		m_RenderOffscreen = false;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fill(BYTE r, BYTE g, BYTE b, RECT* rect)
{
	SDL_SetRenderDrawColor(m_Renderer, r, g, b, 0xFF);
	SDL_RenderClear(m_Renderer);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::Fade(WORD Alpha)
{
	DWORD dwAlpha = 255 - Alpha;
	return FadeToColor(dwAlpha<<24);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::FadeToColor(DWORD Color, RECT* rect)
{
	SDL_Rect fillRect;

	if (rect)
	{
		fillRect.x = rect->left;
		fillRect.y = rect->top;
		fillRect.w = rect->right - rect->left;
		fillRect.h = rect->bottom - rect->top;		
	}
	else
	{
		RECT rc;
		Game->GetCurrentViewportRect(&rc);
		fillRect.x = rc.left;
		fillRect.y = rc.top;
		fillRect.w = rc.right - rc.left;
		fillRect.h = rc.bottom - rc.top;
	}
	ModTargetRect(&fillRect);

	BYTE r = D3DCOLGetR(Color);
	BYTE g = D3DCOLGetG(Color);
	BYTE b = D3DCOLGetB(Color);
	BYTE a = D3DCOLGetA(Color);

	SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderFillRect(m_Renderer, &fillRect);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::DrawLine(int X1, int Y1, int X2, int Y2, DWORD Color)
{
	BYTE r = D3DCOLGetR(Color);
	BYTE g = D3DCOLGetG(Color);
	BYTE b = D3DCOLGetB(Color);
	BYTE a = D3DCOLGetA(Color);

	SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);

#ifdef __IPHONEOS__
	// maybe on iOS viewport setting still doesn't work...
	POINT point1, point2;
	point1.x = X1;
	point1.y = Y1;
	PointToScreen(&point1);

	point2.x = X2;
	point2.y = Y2;
	PointToScreen(&point2);
#else
	// setviewport() is respected, so only scale the coordinates
	POINT point1, point2;
	point1.x = X1 * m_RatioX;
	point1.y = Y1 * m_RatioY;

	point2.x = X2 * m_RatioX;
	point2.y = Y2 * m_RatioY;
#endif

	SDL_RenderDrawLine(m_Renderer, point1.x, point1.y, point2.x, point2.y);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBImage* CBRenderSDL::TakeScreenshot()
{
	SDL_Rect viewport;

	SDL_RenderGetViewport(m_Renderer, &viewport);
	
	SDL_Surface* surface = SDL_CreateRGBSurface(0, viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0x00000000);
	if (!surface) return NULL;

	if (SDL_RenderReadPixels(m_Renderer, NULL, surface->format->format, surface->pixels, surface->pitch) < 0) return NULL;


	FIBITMAP* dib = FreeImage_Allocate(viewport.w, viewport.h, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	
	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
	
	for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++)
	{
		BYTE* bits = FreeImage_GetScanLine(dib, y);
		BYTE* src = (BYTE*)surface->pixels + (viewport.h - y - 1) * surface->pitch;
		memcpy(bits, src, bytespp * viewport.w);
	}
	
	return new CBImage(Game, dib);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::SwitchFullscreen()
{
	if (m_Windowed) SDL_SetWindowFullscreen(m_Win, SDL_TRUE);
	else SDL_SetWindowFullscreen(m_Win, SDL_FALSE);

	m_Windowed = !m_Windowed;

	Game->m_Registry->WriteBool("Video", "Windowed", m_Windowed);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
const char* CBRenderSDL::GetName()
{
	if (m_Name.empty())
	{
		if (m_Renderer)
		{
			SDL_RendererInfo info;
			SDL_GetRendererInfo(m_Renderer, &info);
			m_Name = AnsiString(info.name);
		}
	}
	return m_Name.c_str();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBRenderSDL::SetViewport(int left, int top, int right, int bottom)
{
	SDL_Rect rect;
	
	if (m_RenderOffscreen == false)
	{
		// original behaviour --> modify viewport rect
		rect.x = left + m_BorderLeft;
		rect.y = top + m_BorderTop;
		rect.w = (right - left) * m_RatioX;
		rect.h = (bottom - top) * m_RatioY;

		// TODO fix this once viewports work correctly in SDL/landscape
#ifndef __IPHONEOS__	
		SDL_RenderSetViewport(GetSdlRenderer(), &rect);
#endif
	}
	else
	{
		// pixelperfect behaviour --> remember viewport for final onscreen drawing only
		m_PixelPerfectTargetRect.x = left + m_BorderLeft;
		m_PixelPerfectTargetRect.y = top + m_BorderTop;
		m_PixelPerfectTargetRect.w = (right - left) * m_RatioX;
		m_PixelPerfectTargetRect.h = (bottom - top) * m_RatioY;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::ModTargetRect(SDL_Rect* rect)
{
	SDL_Rect viewportRect;

	if (m_RenderOffscreen == false)
	{
		// only apply offsets/scaling if pixelperfect rendering is off
		SDL_RenderGetViewport(GetSdlRenderer(), &viewportRect);

		rect->x = MathUtil::Round(rect->x * m_RatioX + m_BorderLeft - viewportRect.x);
		rect->y = MathUtil::Round(rect->y * m_RatioY + m_BorderTop - viewportRect.y);
		rect->w = MathUtil::RoundUp(rect->w * m_RatioX);
		rect->h = MathUtil::RoundUp(rect->h * m_RatioY);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::ModOrigin(SDL_Point* origin)
{
	// FIXME I don't know if this is correct...
	if (m_RenderOffscreen == false)
	{
		// only apply scaling if no pixelperfect rendering is requested
		origin->x *= m_RatioX;
		origin->y *= m_RatioY;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointFromScreen(POINT* point)
{
	point->x = point->x / m_RatioX - m_BorderLeft;
	point->y = point->y / m_RatioY - m_BorderTop;
}


//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::PointToScreen(POINT* point)
{
	point->x = MathUtil::Round(point->x * m_RatioX + m_BorderLeft);
	point->y = MathUtil::Round(point->y * m_RatioY + m_BorderTop);
}

//////////////////////////////////////////////////////////////////////////
void CBRenderSDL::DumpData(char* Filename)
{
	FILE* f = fopen(Filename, "wt");
	if(!f) return;

	CBSurfaceStorage* Mgr = Game->m_SurfaceStorage;

	int TotalKB = 0;
	fprintf(f, "Filename;Usage;Size;KBytes\n");
	for(int i=0; i<Mgr->m_Surfaces.GetSize(); i++)
	{
		CBSurfaceSDL* Surf = (CBSurfaceSDL*)Mgr->m_Surfaces[i];
		if(!Surf->m_Filename) continue;
		if(!Surf->m_Valid) continue;

		fprintf(f, "%s;%d;", Surf->m_Filename, Surf->m_ReferenceCount);
		fprintf(f, "%dx%d;", Surf->GetWidth(), Surf->GetHeight());

		int kb = Surf->GetWidth() * Surf->GetHeight() * 4 / 1024;

		TotalKB+=kb;
		fprintf(f, "%d;", kb);
		fprintf(f, "\n");
	}
	fprintf(f, "Total %d;;;%d\n", Mgr->m_Surfaces.GetSize(), TotalKB);


	fclose(f);
	Game->LOG(0, "Texture Stats Dump completed.");
	Game->QuickMessage("Texture Stats Dump completed.");
}

//////////////////////////////////////////////////////////////////////////
float CBRenderSDL::GetAlignedUpscalingRatio(float ratio, float stepping)
{
	float newRatio = 1.0;

	// sort out unreasonable values
	if (stepping < 0.001)
	{
		return ratio;
	}

	while ((newRatio + stepping) < ratio)
	{
		// only increase newRatio until it is at most equal to the old one
		if ((newRatio + stepping) <= ratio)
		{
			newRatio += stepping;
		}
	}

	return newRatio;
}

//////////////////////////////////////////////////////////////////////////
float CBRenderSDL::GetAlignedDownscalingRatio(float ratio, float stepping)
{
	float newRatio = 1.0;

	// sort out unreasonable values
	if (stepping < 0.001)
	{
		return ratio;
	}

	while ((newRatio - stepping) > ratio)
	{
		// decrease until newRatio is equal or lower than ratio
		newRatio -= stepping;
	}
	return newRatio;
}

