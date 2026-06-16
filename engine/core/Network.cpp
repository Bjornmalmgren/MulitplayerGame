#include "config.h"
#include "Network.h"
#include "render/World.h"
#include "protocol.h"
#include <algorithm>
#include <gtx/compatibility.hpp>

using namespace Protocol;
void Network::CreateServer(int port) {
	//enet_address_set_host_ip(&address,host);
	address.host = ENET_HOST_ANY;
	/* Bind the server to port 1234. */
	address.port = port;

	client = enet_host_create(&address /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);


	if (client == NULL)
	{

		fprintf(stderr,
			"An error occurred while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr,
		"Created host succesfully.\n");
	t = std::thread(&Network::Update, this);
	startClock = std::chrono::system_clock::now();
	
}



void Network::ExitServer() {
	enet_host_destroy(client);
}

//server side
void Network::Update() {	
	ENetEvent event;
	while (runLoop == true) {
		while (enet_host_service(client, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				printf("A new client connected from %x:%u.\n",
					event.peer->address.host,
					event.peer->address.port);

				

				/* Store any relevant client information here. */
				JoinData j;
				//create ship
				if (worldInstance->shipManager.Shipid == 0) {
					worldInstance->shipManager.createShip(true, worldInstance->shipManager.Shipid, true);
				}
				else
				{
					worldInstance->shipManager.createShip(false, worldInstance->shipManager.Shipid, true);
				}
				

				int IDs = worldInstance->shipManager.Shipid - 1;
				j.id = IDs;
				worldInstance->shipManager.activeIds.push_back(j.id);
				
				
				event.peer->data = &(j.id);
				peers.insert({ j.id,event.peer });
				Message msg = CreateMessage(j, Join);
				auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-startClock).count();
				
				msg.timeStamp = now;
				currentTime = now;
				enet_peer_send(event.peer, 0, CreateENetPacket(msg));
				//add ships to new client
				for (auto ship : worldInstance->shipManager.NetworkShips)
				{
					
					SpawnPlayerData sp;
					sp.id = ship.second->id;
					sp.Position = ship.second->position;
					sp.Rotation = vec4(ship.second->orientation.x, ship.second->orientation.y, ship.second->orientation.z, ship.second->orientation.w);
					Message msg = CreateMessage(sp, SpawnPlayer);
					
					msg.timeStamp = currentTime;
					
					enet_peer_send(event.peer, 0, CreateENetPacket(msg));
				}

				//send to existing clients
				for (auto var : peers)
				{
					if (var.first == j.id) {
						continue;
					}
					SpawnPlayerData sp;
					sp.id = worldInstance->shipManager.NetworkShips[j.id]->id;
					sp.Position = worldInstance->shipManager.NetworkShips[j.id]->position;
					sp.Rotation = vec4(worldInstance->shipManager.NetworkShips[j.id]->orientation.x, worldInstance->shipManager.NetworkShips[j.id]->orientation.y, worldInstance->shipManager.NetworkShips[j.id]->orientation.z, worldInstance->shipManager.NetworkShips[j.id]->orientation.w);
					Message msg = CreateMessage(sp, SpawnPlayer);
					
					msg.timeStamp = currentTime;
					
					enet_peer_send(var.second, 0, CreateENetPacket(msg));
				}
				//sent to joining client

				break;
			}
				

			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet of length %u containing %s was received from %s on channel %u.\n",
					event.packet->dataLength,
					event.packet->data,
					event.peer->data,
					event.channelID);
				Message mesg = DecodeMessage(event.packet);
				if (mesg.type == Join) {

				}
				if (mesg.type == Position) {
					//don't do anything
					
				}
				if (mesg.type == ClientInput) {
					//update position server side and the send out the new position

					ClientInputData sp = *(ClientInputData*)mesg.data;
					worldInstance->shipManager.NetworkShips[sp.id]->AssigneKey((float)sp.Forward, (float)sp.Left, (float)sp.Right,(float) sp.RollLeft, (float)sp.RollRight,(float) sp.PitchUp, (float)sp.PitchDown,(float) sp.Shoot, (float)sp.Boost);
					if (sp.Shoot == 1) {
						glm::vec3 lpos = (worldInstance->shipManager.NetworkShips[sp.id]->transform * glm::vec4(glm::normalize(worldInstance->shipManager.NetworkShips[sp.id]->leftWeaponPos), 0.0f));
						glm::vec3 rpos = (worldInstance->shipManager.NetworkShips[sp.id]->transform * glm::vec4(glm::normalize(worldInstance->shipManager.NetworkShips[sp.id]->rightWeaponPos), 0.0f));
						worldInstance->SpawnProjectile(worldInstance->shipManager.NetworkShips[sp.id]->position + lpos
							,glm::vec3(worldInstance->shipManager.NetworkShips[sp.id]->transform[2]),
							0.1);
						worldInstance->SpawnProjectile(worldInstance->shipManager.NetworkShips[sp.id]->position + rpos,
							glm::vec3(worldInstance->shipManager.NetworkShips[sp.id]->transform[2]),
							0.1);
						
						sp.Shoot = 0;
						
						for (auto var : peers)
						{
							for (int i = 0; i < worldInstance->activeProjectiles.size(); i++)
							{
								SpawnProjectileData spd;
								spd.position = worldInstance->activeProjectiles[i].position;
								spd.direction =  worldInstance->activeProjectiles[i].direction;
								spd.velocity =  worldInstance->activeProjectiles[i].velocity;
								Message msg = CreateMessage(spd, SpawnProjectile);
								ENetPacket* pack = CreateENetPacket(msg,ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(var.second, 1, pack);
							}
							
						}
					}
					if (currentTick > 20) {
						for (auto var : peers)
						{
							for (auto ship : worldInstance->shipManager.NetworkShips)
							{
								
								PositionData p;
								p.id = ship.second->id;
								p.Position = ship.second->position;
								p.Rotation = glm::vec4(ship.second->orientation.x, ship.second->orientation.y, ship.second->orientation.z, ship.second->orientation.w);
								p.Velocity = ship.second->linearVelocity;
								Message msg = CreateMessage(p, Position);
								auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startClock).count();
								msg.timeStamp = now;
								currentTime = now;

								ENetPacket* pack = CreateENetPacket(msg, ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(var.second, 1, pack);

							}
						}
						currentTick = 0;
					}
					currentTick++;
					

					//old code
					/*PositionData sp = *(PositionData*)mesg.data;
					worldInstance->shipManager.ships[sp.id]->position = sp.Position;
					worldInstance->shipManager.ships[sp.id]->orientation = quat(sp.Rotation.w, sp.Rotation.x, sp.Rotation.y, sp.Rotation.z);
					worldInstance->shipManager.ships[sp.id]->transform = translate(worldInstance->shipManager.ships[sp.id]->position) * (mat4)worldInstance->shipManager.ships[sp.id]->orientation;
					glm::mat4 m(glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[0]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[1]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[2]), worldInstance->shipManager.ships[sp.id]->transform[3]);
					Physics::SetTransform(worldInstance->shipManager.ships[sp.id]->collider, m);

					for (auto var : peers)
					{

						if (var.first == sp.id) {
							continue;
						}
						PositionData p;
						p.id = sp.id;
						p.Position = sp.Position;
						Message msg = CreateMessage(sp, Position);
						enet_peer_send(var.second, 0, CreateENetPacket(msg));
					}*/
				}

				std::cout << mesg.data << std::endl;
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", event.peer->data);
				ExitServer();
				/* Reset the peer's client information. */

				event.peer->data = NULL;
			}
		}
	}
	
}

void Client::CreateClient() {
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used0, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);

	if (client == NULL)
	{
		fprintf(stderr,
			"An error occurred while trying to create an ENet client host.\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr,
		"Created client succesfully.\n");
	t = std::thread(&Client::Update, this);
}

void Client::StartServer() {
	if (enet_initialize() != 0) {
		std::cout << "An error occured when initializing enet" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Client::ExitServer() {
	enet_host_destroy(client);
}

void Client::ConnectToServer(int port, const char* host) {
	std::string s = host;
	ENetEvent event;
	std::cout << "in connect" << std::endl;

	enet_address_set_host(&address, host);
	address.port = port;
	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect(client, &address, 2, 0);

	if (peer == NULL)
	{
		fprintf(stderr,
			"No available peers for initiating an ENet connection.\n");
		exit(EXIT_FAILURE);
	}

	/* Wait up to 5 seconds for the connection attempt to succeed. */
	if (enet_host_service(client, &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		
		puts("Connection to host succeeded.");
		enet_host_flush(client);
		
	}
	else
	{
		/* Either the 5 seconds are up or a disconnect event was */
		/* received. Reset the peer in the event the 5 seconds   */
		/* had run out without any significant event.            */
		enet_peer_reset(peer);

		puts("Connection to host failed.");
	}


}

void Client::Update() {
	ENetEvent event;
	
	while (enet_host_service(client, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			printf("A new client connected from %x:%u.\n",
				event.peer->address.host,
				event.peer->address.port);

			/* Store any relevant client information here. */


			break;

		case ENET_EVENT_TYPE_RECEIVE:
			printf("A packet of length %u containing %s was received from %s on channel %u.\n",
				event.packet->dataLength,
				event.packet->data,
				event.peer->data,
				event.channelID);

			Message mesg = DecodeMessage(event.packet);
			
			lastUpdate = mesg.timeStamp;
			if (currentTime < lastUpdate)
				currentTime = lastUpdate;
			if (mesg.type == SpawnPlayer) {
				SpawnPlayerData sp = *(SpawnPlayerData*)mesg.data;
				if (sp.id == worldInstance->activeid) {
					
					if (!worldInstance->server.isHost) {
						worldInstance->shipManager.createShip(true, sp.id, false);

						worldInstance->shipManager.ships[sp.id]->position = sp.Position;
						worldInstance->shipManager.ships[sp.id]->orientation = quat(sp.Rotation.w, sp.Rotation.x, sp.Rotation.y, sp.Rotation.z);
						glm::mat4 m(glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[0]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[1]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[2]), worldInstance->shipManager.ships[sp.id]->transform[3]);
						Physics::SetTransform(worldInstance->shipManager.ships[sp.id]->collider, m);
					}
					else
					{
						worldInstance->shipManager.NetworkShips[sp.id]->position = sp.Position;
						worldInstance->shipManager.NetworkShips[sp.id]->orientation = quat(sp.Rotation.w, sp.Rotation.x, sp.Rotation.y, sp.Rotation.z);
						glm::mat4 m(glm::normalize(worldInstance->shipManager.NetworkShips[sp.id]->transform[0]), glm::normalize(worldInstance->shipManager.NetworkShips[sp.id]->transform[1]), glm::normalize(worldInstance->shipManager.NetworkShips[sp.id]->transform[2]), worldInstance->shipManager.NetworkShips[sp.id]->transform[3]);
						Physics::SetTransform(worldInstance->shipManager.NetworkShips[sp.id]->collider, m);
					}
						
					
					
					
				}
				else
				{
					worldInstance->shipManager.createShip(false,sp.id, false);
					worldInstance->shipManager.ships[sp.id]->position = sp.Position;
					worldInstance->shipManager.ships[sp.id]->orientation = quat(sp.Rotation.w, sp.Rotation.x, sp.Rotation.y, sp.Rotation.z);
					glm::mat4 m(glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[0]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[1]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[2]), worldInstance->shipManager.ships[sp.id]->transform[3]);
					Physics::SetTransform(worldInstance->shipManager.ships[sp.id]->collider, m);
				}
				
			}
			if (mesg.type == Join) {
				JoinData* sp = (JoinData*)mesg.data;
				//if (worldInstance->server.isHost == false){
				//	worldInstance->shipManager.activeIds.push_back(sp->id);
				//	Render::ParticleSystem::Instance()->RemoveEmitter(worldInstance->shipManager.ships[0]->particleEmitterLeft);
				//	Render::ParticleSystem::Instance()->RemoveEmitter(worldInstance->shipManager.ships[0]->particleEmitterRight);
				//	worldInstance->shipManager.ships.clear();
				//}
				currentTime = mesg.timeStamp;
				worldInstance->activeid = sp->id;
				std::cout << sp->id << std::endl;
			}
			if (mesg.type == Position) {
				//update position
				
				PositionData sp = *(PositionData*)mesg.data;
				if (worldInstance->shipManager.ships[sp.id]->prediction == glm::vec3(0))
				{
					worldInstance->shipManager.ships[sp.id]->prediction = sp.Position;
				}
				
				worldInstance->shipManager.ships[sp.id]->position = sp.Position;
				
				
				
				worldInstance->shipManager.ships[sp.id]->orientation = quat(sp.Rotation.w, sp.Rotation.x, sp.Rotation.y, sp.Rotation.z);
				worldInstance->shipManager.ships[sp.id]->transform = translate(worldInstance->shipManager.ships[sp.id]->position) * (mat4)worldInstance->shipManager.ships[sp.id]->orientation;
				
				glm::mat4 m(glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[0]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[1]), glm::normalize(worldInstance->shipManager.ships[sp.id]->transform[2]), worldInstance->shipManager.ships[sp.id]->transform[3]);
				Physics::SetTransform(worldInstance->shipManager.ships[sp.id]->collider, m);
				//dead reckoning

			}
			if (mesg.type == ClientInput) {
				//update position server side and the send out the new position
				
			}
			if (mesg.type == SpawnProjectile)
			{
				SpawnProjectileData sp = *(SpawnProjectileData*)mesg.data;
				if (worldInstance->server.isHost == false)
				{
					worldInstance->SpawnProjectile(sp.position, sp.direction, sp.velocity);	
				}
				
			}
			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy(event.packet);

			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			printf("%s disconnected.\n", event.peer->data);

			/* Reset the peer's client information. */
			ExitServer();
			event.peer->data = NULL;
			break;
		}
	}

}

void Client::SendInput(int id, bool Forward,bool Left,bool Right,bool RollLeft,bool RollRight,bool PitchUp,bool PitchDown,bool Shoot, bool boost) {
	ClientInputData ci;
	ci.id = id;
	ci.Forward = Forward;
	ci.Left = Left;
	ci.Right = Right;
	ci.RollRight = RollRight;
	ci.RollLeft = RollLeft;
	ci.PitchUp = PitchUp;
	ci.PitchDown = PitchDown;
	ci.Shoot = Shoot;
	ci.Boost = boost;
	Message msg = CreateMessage(ci, ClientInput);
	ENetPacket* pack = CreateENetPacket(msg);
	int sent  = enet_peer_send(client->peers,0,pack);

}