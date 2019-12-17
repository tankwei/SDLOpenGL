#include "edit/app.h"
#include <SDL2/SDL_main.h>
#include "util/model.h"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	editor::App app;
	if (!app.init()){
		return 1;
	}
    while(app.run_frame()){

	}

    return 0;
}

	
