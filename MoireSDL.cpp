#include <SDL/SDL.h>
#include <cmath>
#include <ctime>
#include <queue>

#ifdef WIN32
#include <windows.h>
#endif


#define VERTICES 7
#define LENGTH 100


Uint32 CycleColor( const std::vector<Uint32> *colors, double phase, const SDL_PixelFormat *format )
{
	size_t index1 = (size_t) phase;
	Uint32 color1 = colors->at( index1 % colors->size() ), color2 = colors->at( (index1 + 1) % colors->size() );
	double along = phase - index1;
	Uint8 r1 = (color1 & 0xFF0000) >> 16, g1 = (color1 & 0x00FF00) >> 8, b1 = color1 & 0x0000FF;
	Uint8 r2 = (color2 & 0xFF0000) >> 16, g2 = (color2 & 0x00FF00) >> 8, b2 = color2 & 0x0000FF;
	Uint8 r = r1 * (1. - along) + r2 * along + 0.5;
	Uint8 g = g1 * (1. - along) + g2 * along + 0.5;
	Uint8 b = b1 * (1. - along) + b2 * along + 0.5;
	return SDL_MapRGB( format, r, g, b );
}


void DrawLine( SDL_Surface *drawto, int x1, int y1, int x2, int y2, Uint32 color )
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double length = std::max<double>( fabs(dx), fabs(dy) );
	dx /= length;
	dy /= length;
	
	Uint32 *pixels = (Uint32*) drawto->pixels;
	double x = x1;
	double y = y1;
	
	for( double i = 0.; i <= length; i += 1. )
	{
		pixels[ ((int)y) * drawto->w + (int)x ] = color;
		x += dx;
		y += dy;
	}
}


int main( int argc, char **argv )
{
	if( (argc >= 2) && ((strcasecmp( argv[ 1 ], "/c" ) == 0) || (strcasecmp( argv[ 1 ], "/p" ) == 0)) )
		return 0;
	
#ifdef WIN32
	BOOL (WINAPI *SetProcessDPIAware)( void ) = GetProcAddress( LoadLibraryA("user32.dll"), "SetProcessDPIAware" );
	if( SetProcessDPIAware )
		SetProcessDPIAware();
#endif
	
	SDL_Init( SDL_INIT_VIDEO );
	
	int w = 0, h = 0;
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if( info )
	{
		w = info->current_w;
		h = info->current_h;
	}
	
	SDL_WM_SetCaption( "Moire SDL", NULL );
	SDL_Surface *screen = SDL_SetVideoMode( w, h, 0, SDL_SWSURFACE | SDL_FULLSCREEN );
	if( ! screen )
		return -1;
	
	SDL_LockSurface( screen );
	memset( screen->pixels, 0, screen->w * screen->h * 4 );
	SDL_UnlockSurface( screen );
	SDL_UpdateRect( screen, 0, 0, screen->w, screen->h );
	
	SDL_ShowCursor( SDL_DISABLE );
	
	std::vector<Uint32> colors;
	colors.push_back(0x00FF00); // Green
	colors.push_back(0x00FFFF); // Cyan
	colors.push_back(0x0000FF); // Blue
	colors.push_back(0xFF00FF); // Magenta
	colors.push_back(0xFF0000); // Red
	colors.push_back(0xFFFF00); // Yellow
	
	double x[VERTICES], y[VERTICES], dx[VERTICES], dy[VERTICES];
	std::queue<int> history;
	
	srand( time(NULL) );
	
	for( int i = 0; i < VERTICES; i ++ )
	{
		x[ i ] = rand() % screen->w;
		y[ i ] = rand() % screen->h;
		dx[ i ] = ((rand() % 2) * 2 - 1) * 4;
		dy[ i ] = ((rand() % 2) * 2 - 1) * 4;
	}
	
	SDL_Event event;
	while( SDL_PollEvent( &event ) ) {}
	
	double phase = 0.;
	
	bool running = true;
	while( running )
	{
		Uint32 color = CycleColor( &colors, phase, screen->format );
		phase += 0.01;
		
		SDL_LockSurface( screen );
		
		while( history.size() > VERTICES * 4 * LENGTH )
		{
			int x1 = history.front();
			history.pop();
			int y1 = history.front();
			history.pop();
			int x2 = history.front();
			history.pop();
			int y2 = history.front();
			history.pop();
			
			DrawLine( screen, x1, y1, x2, y2, 0 );
		}
		
		for( int i = 0; i < VERTICES; i ++ )
		{
			DrawLine( screen, x[ i ], y[ i ], x[ (i + 1) % VERTICES ], y[ (i + 1) % VERTICES ], color );
			
			history.push( x[ i ] );
			history.push( y[ i ] );
			history.push( x[ (i + 1) % VERTICES ] );
			history.push( y[ (i + 1) % VERTICES ] );
		}
		
		SDL_UnlockSurface( screen );
		SDL_UpdateRect( screen, 0, 0, screen->w, screen->h );
		
		for( int i = 0; i < VERTICES; i ++ )
		{
			if( (x[ i ] + dx[ i ] >= screen->w) || (x[ i ] + dx[ i ] < 0) )
				dx[ i ] *= -1;
			if( (y[ i ] + dy[ i ] >= screen->h) || (y[ i ] + dy[ i ] < 0) )
				dy[ i ] *= -1;
			
			x[ i ] += dx[ i ];
			y[ i ] += dy[ i ];
		}
		
		SDL_Delay( 12 );
		
		while( SDL_PollEvent( &event ) )
		{
			if( (event.type == SDL_QUIT) || (event.type == SDL_MOUSEMOTION) || (event.type == SDL_MOUSEBUTTONDOWN) || (event.type == SDL_KEYDOWN) )
				running = false;
		}
	}
	
	SDL_Quit();
	return 0;
}
