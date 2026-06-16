#pragma once
#include <vector>
#include "render/physics.h"
#include <map>
#include "core/Network.h"
#include "../projects/spacegame/code/spaceship.h"
namespace Game {
    struct SpaceShip;
}
struct node {
    glm::vec3 position;
};
struct Laser {
    glm::vec3 position;
    glm::vec3 direction;
    float velocity;
    int lifeTime;
    Physics::ColliderId collider;
};

class ShipManager
{
public:
    ShipManager();
	~ShipManager(){}
    void createShip(bool drive, int id, bool netWorkedShips);
    void UpdateShips(float dt);
    void DrawShips();
    void DrawNodes();
    std::map<int,Game::SpaceShip*> ships; //all ships
    std::map<int, Game::SpaceShip*> NetworkShips;
    std::vector<int> activeIds;
    std::vector<node> respawnNodes;
    
    int Shipid = 0;
private:
};
class World {
public:
    Client net;
    Network server;
    int activeid;
    World();
    void SendInput(int id ,bool Forward,
        bool Left,
        bool Right,
        bool RollLeft,
        bool RollRight,
        bool PitchUp,
        bool PitchDown,
        bool Shoot , bool boost);
    ~World() {}
    static World* instance();
    void SpawnProjectile(glm::vec3 position, glm::vec3 direction, float velocity);
    std::vector<Laser> activeProjectiles;
    std::vector<Physics::ColliderId> colliders;
    std::map<int, Physics::RaycastPayload> raycasts; // all active raycasts
    ShipManager shipManager; //ship and ship id
    void UpdateLasers();
    void Deadreckoning();
    std::string defaultIP = "127.0.0.1";
    char serverName[16] = "127.0.0.1";
    Render::ModelId laser;
};
#define worldInstance World::instance()