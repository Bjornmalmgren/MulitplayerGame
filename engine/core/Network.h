#pragma once
#include <enet/enet.h>
#include "iostream"
#include <thread>
#include <map>
#include <vector>

class Network
{

public:
	std::map<int, ENetPeer*> peers;
	bool runLoop = true;
	std::thread t;
	bool isHost = false;
	int port;
	ENetAddress address;
	ENetHost* client;
	void CreateServer(int port);
	void ExitServer();
	void Update();
	//message type
	std::vector<int> DelayedMessages;
	//index of which spaceship it affects
	std::vector<int> DelayedMessagesIndex;
	std::chrono::time_point<std::chrono::system_clock> startClock;
	int start;
	int currentTime;

private:
	int currentTick = 0;
};

class Client
{
public:
	int port = 7777;
	bool connect = false;
	bool clientStart = false;
	std::thread t;
	ENetPeer* peer;
	ENetAddress address;
	ENetHost* client;
	glm::vec3 predicted = glm::vec3(0);
	glm::vec3 corrected;
	void CreateClient();
	void StartServer();
	void ConnectToServer(int port, const char* host);
	void ExitServer();
	void Update();
	void SendInput(int id, bool Forward,
		bool Left,
		bool Right,
		bool RollLeft,
		bool RollRight,
		bool PitchUp,
		bool PitchDown,
		bool Shoot, bool boost);
	std::chrono::time_point<std::chrono::system_clock> startClock;
	int start;
	int currentTime;
	int lastUpdate;

};