#pragma once
#include<SFML/Graphics.hpp>
#include<vector>

using namespace sf;

class Particle
{
private:
    Vector2f position = { 0,0 };
    Vector2f velocity = { 0,0 };
    Vector2f acceleration = { 0,0 };
    float mass = 10;
    bool _static = false;
    Color color = Color::Green;
    float radius = 5.f;

public:
    Particle() {}
    //Setting initial conditions for particles
    Particle(float posX, float posY, bool pinned)
    {
        position = { posX,posY };
        _static = pinned;
    }

    //Adding forces to the particle (Euler integration method)
    void Forces(Vector2f force) {
        if (!_static) {
            Vector2f _acceleration = { force.x / mass, force.y / mass };
            acceleration += _acceleration;
        }
    }

    //update properties of particles
    void Update() {
        if (!_static) {
            //for energy loss overtime->decrease in velocity i.e. k*v where k is damping constant
            velocity.x *= float(0.9);
            velocity.y *= float(0.9);

            //for some value of acceleration, velocity will increase
            velocity += acceleration;

            //new position
            position += velocity;

            //resetting acceleration to zero
            acceleration = { 0,0 };
            //velocity = { 0,0 };
        }
    }

    //new position for the particles
    void newPosition(float posX, float posY) {
        velocity = { 0,0 };
        position = { posX,posY };
    }

    //move the particle without touching its velocity (used by collision resolution)
    void SetPosition(Vector2f newPos) {
        position = newPos;
    }

    //obtaining position of the particle
    Vector2f GetPosition()
    {
        return position;
    }

    Vector2f GetVelocity()
    {
        return velocity;
    }

    void SetVelocity(Vector2f newVelocity)
    {
        velocity = newVelocity;
    }

    bool IsStatic()
    {
        return _static;
    }

    void Draw(RenderWindow &window)
    {
        CircleShape points(radius);
        points.setFillColor(color);
        points.setPosition(position.x - radius, position.y - radius);
        window.draw(points);
    }
    ~Particle() {}
};