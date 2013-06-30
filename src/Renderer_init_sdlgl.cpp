#include "Renderer.h"
#include <iostream>
#include "platform.h"

#ifdef _WINDOWS_
	#include <Windows.h>
#endif

#include GLHEADER

#include "Font.h"
#include <SDL.h>
#include "InputManager.h"
#include "Log.h"
#include "ImageIO.h"
#include "../data/Resources.h"
#include "EmulationStation.h"
#include "Settings.h"

namespace Renderer
{
	static bool initialCursorState;

	int display_width = 0;
	int display_height = 0;

	int getScreenWidth() { return display_width; }
	int getScreenHeight() { return display_height; }

    SDL_Window * window = nullptr;
    SDL_GLContext context = nullptr;

	bool createSurface() //unsigned int display_width, unsigned int display_height)
	{
		LOG(LogInfo) << "Creating surface...";

		if(SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			LOG(LogError) << "Error initializing SDL!\n	" << SDL_GetError();
			return false;
		}

        //if width or height are zero, use the desktop resolution of the first desktop
        if (display_width == 0 || display_height == 0) {
            SDL_DisplayMode mode;
            SDL_GetDesktopDisplayMode(0, &mode);
            display_width = mode.w;
            display_height = mode.h;
        }
        
        //first create SDL window
        window = SDL_CreateWindow("EmulationStation", 0, 0, display_width, display_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (Settings::getInstance()->getBool("WINDOWED") ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP));
        if(window == nullptr)
		{
			LOG(LogError) << "Error creating SDL video window!";
			return false;
		}
        //usually display width/height are not specified, i.e. zero, which SDL automatically takes as "native resolution"
		//so, since other things rely on the size of the screen (damn currently unnormalized coordinate system), we set it here
		//even though the system was already initialized
        SDL_GetWindowSize(window, &display_width, &display_height);

		//set window icon. try loading PNG from memory
		size_t width = 0;
		size_t height = 0;
		std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(es_logo_32_data, es_logo_32_data_len, width, height);
		if (!rawData.empty()) {
			//SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Uint32 rmask = 0xff000000; Uint32 gmask = 0x0000ff00; Uint32 bmask = 0x00ff0000; Uint32 amask = 0x000000ff;
#else
			Uint32 rmask = 0x000000ff; Uint32 gmask = 0x00ff0000; Uint32 bmask = 0x0000ff00; Uint32 amask = 0xff000000;
#endif
			//try creating SDL surface from logo data
			SDL_Surface * logoSurface = SDL_CreateRGBSurfaceFrom((void *)rawData.data(), width, height, 32, width*4, rmask, gmask, bmask, amask);
            if (logoSurface != nullptr) {
                SDL_SetWindowIcon(window, logoSurface);
                SDL_FreeSurface(logoSurface);
            }
            else {
                LOG(LogError) << "Error creating SDL surface for icon: " << SDL_GetError() << "!";
            }
		}

        //set up opengl attributes
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        //create opengl context
        context = SDL_GL_CreateContext(window);
		if(context == nullptr)
		{
			LOG(LogError) << "Error creating OpenGL context!";
			return false;
		}

		LOG(LogInfo) << "Created surface successfully.";

		//hide mouse cursor
		initialCursorState = SDL_ShowCursor(0) == 1;

		return true;
	}

	void swapBuffers()
	{
        SDL_GL_SwapWindow(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void destroySurface()
	{
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
		context = nullptr;
        window = nullptr;

		//show mouse cursor
		SDL_ShowCursor(initialCursorState);

		SDL_Quit();
	}

	bool init(int w, int h)
	{
		if(w)
			display_width = w;
		if(h)
			display_height = h;

		bool createdSurface = createSurface();

		if(!createdSurface)
			return false;

		glViewport(0, 0, display_width, display_height);
		glOrtho(0, display_width, display_height, 0, -1.0, 1.0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		onInit();

		return true;
	}

	void deinit()
	{
		onDeinit();

		destroySurface();
	}
};
