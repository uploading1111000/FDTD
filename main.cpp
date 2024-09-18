#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <array>

#include "sim2D.h"

enum mode {
	MAG,
	EFIELD,
	HYFIELD,
	HXFIELD
};

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* tex;
sim2D* sim;
Uint64 last_change;

const int WIDTH = 200;
const int HEIGHT = 200;
float SCALE = 5.0;
mode MODE = EFIELD;



bool STOP;

const int MAX_FPS = 60;
constexpr Uint64 NS_PERTICK = 1000000000 / MAX_FPS;

std::array<Uint8, 3> hsv2rgb(double H, double S, double V) {
	
	auto f = [H, S, V](int n) {
		double k = fmod((n + H / 60),6.0);
		return V - V * S * fmax(0, fmin(1, fmin(k, 4 - k)));
		};
	int Rint = f(5) * 255, Bint = f(3) * 255, Gint = f(1) * 255;
	Uint8 R = Rint & 0xff, B = Bint & 0xff, G = Gint & 0xff;
	return { R,G,B};
}

float getVal(int x, int y) {
	switch(MODE) {
	case MAG:
		return sim->getMag(x, y);
	case EFIELD:
		return 1 + sim->getEField(x, y);
	case HYFIELD:
		return 1 + sim->getHyField(x, y);
	case HXFIELD:
		return 1 + sim->getHxField(x, y);
	}
}

int graph() {
	void* pixels;
	int pitch;
	SDL_LockTexture(tex, NULL, &pixels, &pitch);
	if (pitch < WIDTH * 4) {
		SDL_UnlockTexture(tex);
		return 1;
	}
	auto updatePixel = [pitch, pixels](int x, int y, Uint8 r, Uint8 g, Uint8 b) {
		Uint8* base = ((Uint8*)pixels) + (pitch * y) + (4 * x);
		base[0] = r;
		base[1] = g;
		base[2] = b;
		base[3] = 0xff;
		};
	int x, y;
	double val;
	std::array<Uint8, 3> rgb;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			val = getVal(x, y);
			rgb = hsv2rgb(120 * (val*SCALE), 1, 1);
			updatePixel(x, y, rgb[0], rgb[1], rgb[2]);
		}
	}

	SDL_UnlockTexture(tex);
	SDL_RenderTexture(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);
	return 0;
}


bool loop() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			return true;
		case SDL_EVENT_KEY_DOWN:
			switch (event.key.key) {
			case SDLK_ESCAPE:
				return true;
			case SDLK_SPACE:
				STOP = !STOP;
				break;
			case SDLK_RIGHT:
				sim->step();
				SDL_Log("%f", sim->getEField(50, 50));
				break;
			case SDLK_R:
				switch(MODE) {
				case MAG:
					MODE = EFIELD;
					SDL_Log("Mode: EField");
					break;
				case EFIELD:
					MODE = HYFIELD;
					SDL_Log("Mode: HyField");
					break;
				case HYFIELD:
					MODE = HXFIELD;
					SDL_Log("Mode: HxField");
					break;
				case HXFIELD:
					MODE = MAG;
					SDL_Log("Mode: FullMagnitude");
					break;
				}
				break;
			}
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			SCALE = fmax(0.2, SCALE + event.wheel.y * 0.1);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				float x, y;
				SDL_RenderCoordinatesFromWindow(ren, event.button.x, event.button.y, &x, &y);
				int xint = x, yint = y;
				sim->addSource(xint,yint);
			}
			break;
		}
	}

	if (!STOP) {
		sim->step();
	}

	if (graph() != 0) {
		return true;
	}

	return false;
}


int main(int argc, char* argv[])
{
	//Make sim
	sim = new sim2D(WIDTH, HEIGHT, 377.0);

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	// Create a window and renderer
	if (SDL_CreateWindowAndRenderer("SDL Test", 800, 600, SDL_WINDOW_RESIZABLE, &win, &ren) != 0) {
		SDL_Log("Unable to create window: %s", SDL_GetError());
		return 2;
	}

	// Create texture to stream from
	SDL_SetRenderLogicalPresentation(ren, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_LINEAR);
	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	// Load Calibri font



	SDL_Log("Init Complete");

	//Main loop with timing
	bool quit = false;
	Uint64 tick = SDL_GetPerformanceCounter(), tock;
	while (!quit) {
		tick = SDL_GetPerformanceCounter();

		quit = loop();

		tock = SDL_GetPerformanceCounter();
		Uint64 dt = 1000000000 * (tock - tick) / SDL_GetPerformanceFrequency();
		if (dt < NS_PERTICK) {
			SDL_DelayNS(NS_PERTICK - dt - 50000);
		}
	}

	// Clean up
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	SDL_Log("Program exited successfully");

	return 0;
}