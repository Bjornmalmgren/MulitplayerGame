#include "config.h"
#include "World.h"

#include <algorithm>
#include <gtx/compatibility.hpp>

#include "gtx/quaternion.hpp"
#include "render/debugrender.h"
World::World() {
}

World* World::instance() {
    static World w;
    return &w;
}

void ShipManager::createShip(bool drive, int id, bool netWorkedShips) {
   
    Game::SpaceShip* ship = new Game::SpaceShip();
    ship->model = Render::LoadModel("assets/space/spaceship.glb");
    int i = rand() % respawnNodes.size();
    glm::vec3 pos = respawnNodes[i].position;
    ship->position = pos;
    ship->CreateRays();
    ship->collider = Physics::CreateCollider(Physics::LoadColliderMesh("assets/space/spaceship_physics.glb"),ship->transform);
    worldInstance->colliders.push_back(ship->collider);
    ship->id = id;
    ship->isLocalPlayer = drive;

   

    glm::mat4 T = glm::translate(ship->position) * (glm::mat4)ship->orientation;
    ship->transform = T;
    if (netWorkedShips)
        NetworkShips.insert({ id,ship });
    else
        ships.insert({ id,ship });
    Shipid++;
}

void ShipManager::UpdateShips(float dt) {
    for (auto obj : NetworkShips)
    {
        obj.second->Update(dt);
        obj.second->CheckCollisions();
        glm::mat4 m(glm::normalize(obj.second->transform[0]), glm::normalize(obj.second->transform[1]), glm::normalize(obj.second->transform[2]), obj.second->transform[3]);
        Physics::SetTransform(obj.second->collider, m);
    }
    for (auto obj : ships)
    {
        obj.second->Update(dt);
        obj.second->CheckCollisions();
        glm::mat4 m(glm::normalize(obj.second->transform[0]), glm::normalize(obj.second->transform[1]), glm::normalize(obj.second->transform[2]), obj.second->transform[3]);
        Physics::SetTransform(obj.second->collider, m);
    }
}

void ShipManager::DrawShips() {
    for (auto ship : NetworkShips) {
        Render::RenderDevice::Draw(ship.second->model, ship.second->transform);
    }
    for (auto ship : ships) {
        Render::RenderDevice::Draw(ship.second->model, ship.second->transform);
    }
}


void World::SendInput(int id, bool Forward,bool Left,bool Right,bool RollLeft,bool RollRight,bool PitchUp,bool PitchDown,bool Shoot, bool boost) {
    if (net.connect == true) {
        net.SendInput(id, Forward, Left, Right, RollLeft, RollRight, PitchUp, PitchDown, Shoot, boost);
    }
}

void ShipManager::DrawNodes() {
    for(auto var : respawnNodes)
    {
        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x+1,var.position.y,var.position.z } , 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);
        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x-1,var.position.y,var.position.z }, 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);

        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x,var.position.y+1,var.position.z }, 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);
        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x,var.position.y-1,var.position.z }, 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);

        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x,var.position.y,var.position.z+1 }, 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);
        Debug::DrawLine({ var.position.x,var.position.y,var.position.z }, { var.position.x,var.position.y,var.position.z-1 }, 1.0f, glm::vec4(1, 1, 0, 1), glm::vec4(1, 1, 0, 1), Debug::RenderMode::Normal);
    }
}

ShipManager::ShipManager() {
    respawnNodes.push_back(node{ {21,-4,3} });
    respawnNodes.push_back(node{ {26,-19,-9} });
    respawnNodes.push_back(node{ {23,-10,-25} });
    respawnNodes.push_back(node{ {7,0,-30} });
    respawnNodes.push_back(node{ {-10,10,-33} });
    respawnNodes.push_back(node{ {-25,10,-11} });
    respawnNodes.push_back(node{ {-15,22,25} });
    srand(0);
}

void World::SpawnProjectile(glm::vec3 position, glm::vec3 direction, float velocity) {
    //left laser
    Laser ls;

    
    ls.direction = direction, ls.position = position+direction*2.0f, ls.lifeTime = 0;
    //right laser
    
    ls.velocity = velocity;
    
    activeProjectiles.push_back(ls);
 
}

void World::UpdateLasers() {
    std::vector<int> lasersToDelete;
    for (int i = 0; i < activeProjectiles.size(); i++)
    {
        
        
        activeProjectiles[i].position += activeProjectiles[i].velocity*activeProjectiles[i].direction*(float)(activeProjectiles[i].lifeTime/10);
        glm::quat q = glm::identity<glm::quat>();
        q = glm::quat(glm::vec3(activeProjectiles[i].direction.y, 0, activeProjectiles[i].direction.x));
        glm::mat4 m = glm::translate(activeProjectiles[i].position)*glm::identity<glm::mat4>();

       
        Render::RenderDevice::Draw(worldInstance->laser, m );
        activeProjectiles[i].lifeTime++;
        if (activeProjectiles[i].lifeTime == 50)
        {
           activeProjectiles[i].collider = Physics::CreateCollider(Physics::LoadColliderMesh("assets/space/spaceship_physics.glb"),glm::translate( activeProjectiles[i].position)*glm::identity<glm::mat4>());
        }else if (activeProjectiles[i].lifeTime > 50)
        {
            SetTransform(activeProjectiles[i].collider,glm::translate( activeProjectiles[i].position)*glm::identity<glm::mat4>() );  
        }
        if (activeProjectiles[i].lifeTime >= 500) {
            lasersToDelete.push_back(i);
        }
    }
    if (lasersToDelete.size() > 0)
    {
        for (int i = 0; i < lasersToDelete.size(); i++)
        {
            if (i > activeProjectiles.size())
            {
                int j = activeProjectiles.size()-1;
               
                
                activeProjectiles.erase(activeProjectiles.begin() + j);
                
            }
            else
            {
                activeProjectiles.erase(activeProjectiles.begin() + lasersToDelete[i]-i);    
            }
            
        }    
    }
    
    
}

void World::Deadreckoning() {
    if (shipManager.ships.size() <= 1) return;

    
    float dt = net.currentTime - net.lastUpdate;
    dt *= 0.001f;
    dt = glm::clamp(dt, 0.0f, 0.2f);
    for (int i = 0; i < shipManager.ships.size(); i++) {
        if (shipManager.ships[i] == NULL) continue;
           

        float timeSinceLastUpdate = net.currentTime - net.lastUpdate;
        if (timeSinceLastUpdate < 1.0f) {
            glm::vec3 desiredVelocity = glm::vec3(0, 0, shipManager.ships[i]->currentSpeed);
            desiredVelocity = shipManager.ships[i]->transform * glm::vec4(desiredVelocity, 0.0f);

            shipManager.ships[i]->prediction = shipManager.ships[i]->position +
                shipManager.ships[i]->linearVelocity * timeSinceLastUpdate +
                shipManager.ships[i]->accelerationFactor * timeSinceLastUpdate * timeSinceLastUpdate;

            float alpha = glm::clamp(timeSinceLastUpdate / 0.1f, 0.0f, 1.0f);
            worldInstance->shipManager.ships[i]->position = glm::mix(worldInstance->shipManager.ships[i]->position, worldInstance->shipManager.ships[i]->prediction, alpha);
        }
        


    }
    
    
        //deadreckoning
       
        //glm::vec3 desiredVelocity = glm::vec3(0, 0, shipManager.NetworkShips[local]->currentSpeed);
        //desiredVelocity = shipManager.NetworkShips[local]->transform * glm::vec4(desiredVelocity, 1.0f);
        //
        //net.predicted = shipManager.NetworkShips[local]->position + shipManager.NetworkShips[local]->linearVelocity * dt + desiredVelocity * (0.5f * dt * dt);
   
}

