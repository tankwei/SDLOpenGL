#include "engine/engine.h"

#include "client/client.h"
#include "server/server.h"

namespace engine
{
	Engine::Engine()
		: server(new server::Server()),
		client(new client::Client())
	{
	}

	Engine::~Engine()
	{
	}

	bool Engine::init()
	{
		return server->init() && client->init();
	}

	void Engine::draw()
	{

	}

	bool Engine::update()
	{
		// Client polls inputs.

		// Client sends inputs to server.

		// Server updates.

		// Server sends state to client.

		// Client handles incoming packets.

		// Client performs update/interpolation.

		return true;
	}
}
