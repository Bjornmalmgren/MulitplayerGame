#pragma once
#include "glm.hpp"
#include <enet/enet.h>
 

namespace Protocol {
	using namespace glm;
	
	enum MessageType : char {
		Join,
		Disconnect,
		ClientInput,
		Position,
		PlayerState,
		SpawnProjectile,
		SpawnPlayer,
		SpawnAsteroid,
		StartGame,
	};

	struct Message {
		MessageType type;
		unsigned timeStamp;
		void* data;			// It is Data
	};

	// When you join the server sends this message to tell you which ship is yours. SpawnPlayerData Tells where. JoinData and SpawnPlayerData can be sent in any order.
	struct JoinData {
		unsigned int id; 
	};

	struct PositionData {
		unsigned int id;				// It is an id
		vec3 Position;
		vec3 Velocity;
		vec3 Acceleration;
		vec4 Rotation;
	};

	struct PlayerStateData {
		unsigned int id;				// It is an id
		bool Dead;
		bool Respawn;
	};

	struct SpawnPlayerData {
		unsigned int id;				// It is an id
		vec3 Position;
		vec4 Rotation;
	};

	struct SpawnProjectileData {
		vec3 position;
		vec3 direction;
		float velocity;
	};

	struct ClientInputData {
		unsigned int id;				// It is an id
		bool Forward : 1;
		bool Boost : 1;
		bool Left : 1;
		bool Right : 1;
		bool RollLeft : 1;
		bool RollRight : 1;
		bool PitchUp : 1;
		bool PitchDown : 1;
		bool Shoot : 1;
	};

	// Send at startup to  make sure all asteroid on all clients are in same place
	struct SpawnAsteroidData { 
		vec3 Position;
		vec4 Rotation;
		unsigned int modelId;
		unsigned int physicsId;
	};

	// Not complete
	ENetPacket* CreateENetPacket(const Protocol::Message& msg, uint32 enet_flag = ENET_PACKET_FLAG_RELIABLE) {
		unsigned int size = 0;
		switch (msg.type) {
		case Protocol::Join:
			size = sizeof(Protocol::JoinData);
			break;
		case Protocol::Disconnect:
			break;
		case Protocol::ClientInput:
			size = sizeof(Protocol::ClientInputData);
			break;
		case Protocol::Position:
			size = sizeof(Protocol::PositionData);
			break;
		case Protocol::PlayerState:
			size = sizeof(Protocol::PlayerStateData);
			break;
		case SpawnProjectile:
			size = sizeof(SpawnProjectileData);
			break;
		case Protocol::SpawnPlayer:
			size = sizeof(Protocol::SpawnPlayerData);
			break;
		case Protocol::SpawnAsteroid:
			size = sizeof(Protocol::SpawnAsteroidData);
			break;
		case Protocol::StartGame:
			break;

		default:
			assert(false);
		}

		ENetPacket* packet = enet_packet_create(nullptr, sizeof(Protocol::Message) + size, enet_flag);

		memcpy(packet->data, &msg, sizeof(Protocol::Message));
		memcpy(packet->data+sizeof(Protocol::Message), msg.data, size);
		
		return packet;
	}
	template<typename T>
	Protocol::Message CreateMessage(T& messageData, Protocol::MessageType type) {
		Protocol::Message msg;
		msg.type = type;
		msg.data = (void*)&messageData;
		return msg;
	}
	// Makes the packet to a message but does not check that the data pointer is correct | data is removed when destroy packet is called.
	Protocol::Message DecodeMessage(ENetPacket* packet) {
		Protocol::Message msg;
		msg.type = ((Protocol::Message*)packet->data)->type;
		msg.data = ((Protocol::Message*)packet->data)+1;
		msg.timeStamp = ((Protocol::Message*)packet->data)->timeStamp;
		return msg;
	}
}