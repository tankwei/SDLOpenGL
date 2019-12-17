#pragma once

#include "dll.h"

// TODO Avoid including such a huge header.
#include <memory>

namespace client
{
	class Client;
}

namespace server
{
	class Server;
}

namespace engine
{
	class Engine
	{
	public:

		Engine();//ENGINE_DLL 
		~Engine();

		bool init();
		void draw();
		bool update();

	private:

		typedef std::auto_ptr<client::Client> ClientPtr;
		typedef std::auto_ptr<server::Server> ServerPtr;

		ServerPtr server;
		ClientPtr client;
	};
};
