#include "config.h"
#include "Client.h"
//#include "render/World.h"

//void Client::CreateClient() {
//	client = enet_host_create(NULL /* create a client host */,
//		1 /* only allow 1 outgoing connection */,
//		2 /* allow up 2 channels to be used0, 0 and 1 */,
//		0 /* assume any amount of incoming bandwidth */,
//		0 /* assume any amount of outgoing bandwidth */);
//
//	if (client == NULL)
//	{
//		fprintf(stderr,
//			"An error occurred while trying to create an ENet client host.\n");
//		exit(EXIT_FAILURE);
//	}
//	fprintf(stderr,
//		"Created client succesfully.\n");
//}
//
//void Client::StartServer() {
//	if (enet_initialize() != 0) {
//		std::cout << "An error occured when initializing enet" << std::endl;
//		exit(EXIT_FAILURE);
//	}
//}
//
//void Client::ExitServer() {
//	enet_host_destroy(client);
//}
//
//void Client::ConnectToServer(int port) {
//
//	ENetEvent event;
//	enet_address_set_host(&address, "127.0.0.1");
//	address.port = 5555;
//
//	/* Initiate the connection, allocating the two channels 0 and 1. */
//	peer = enet_host_connect(client, &address, 2, 0);
//
//	if (peer == NULL)
//	{
//		fprintf(stderr,
//			"No available peers for initiating an ENet connection.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	/* Wait up to 5 seconds for the connection attempt to succeed. */
//	if (enet_host_service(client, &event, 5000) > 0 &&
//		event.type == ENET_EVENT_TYPE_CONNECT)
//	{
//		puts("Connection to 127.0.0.1:5555 succeeded.");
//		enet_host_flush(client);
//	}
//	else
//	{
//		/* Either the 5 seconds are up or a disconnect event was */
//		/* received. Reset the peer in the event the 5 seconds   */
//		/* had run out without any significant event.            */
//		enet_peer_reset(peer);
//
//		puts("Connection to 127.0.0.1:5555 failed.");
//	}
//
//}
//
//void Client::Update() {
//	ENetEvent event;
//	while (enet_host_service(client, &event, 0) > 0)
//	{
//		switch (event.type)
//		{
//		case ENET_EVENT_TYPE_CONNECT:
//			printf("A new client connected from %x:%u.\n",
//				event.peer->address.host,
//				event.peer->address.port);
//		
//			/* Store any relevant client information here. */
//
//			
//			break;
//
//		case ENET_EVENT_TYPE_RECEIVE:
//			printf("A packet of length %u containing %s was received from %s on channel %u.\n",
//				event.packet->dataLength,
//				event.packet->data,
//				event.peer->data,
//				event.channelID);
//
//			
//			/* Clean up the packet now that we're done using it. */
//			enet_packet_destroy(event.packet);
//
//			break;
//
//		case ENET_EVENT_TYPE_DISCONNECT:
//			printf("%s disconnected.\n", event.peer->data);
//
//			/* Reset the peer's client information. */
//
//			event.peer->data = NULL;
//			break;
//		}
//	}
//
//}