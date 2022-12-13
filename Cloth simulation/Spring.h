#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<particle.h>

using namespace sf;

class Spring {
private:
    Particle* p1;//taking two points to create a spring/stick
    Particle* p2;

    //F=kx
    float k;//spring constant
    float length;//length of the spring when no force is applied
    float x = 0;//extension produced
    Vector2f springForce = { 0,0 };
public:
    Spring(float _k, float _length, Particle* _p1, Particle* _p2) {
        k = _k;
        length = _length;
        p1 = _p1;
        p2 = _p2;
    }

    void Update() {
        //Full equation would be F=-k*(|p2-p1|-length)*direction

        //vector from p1 to p2
        Vector2f _vector = p2->GetPosition() - p1->GetPosition();

        //for magnitude
        float mag = sqrt(_vector.x * _vector.x + _vector.y * _vector.y);
        //for extension
        x = mag - length;

        //for direction (p2-p1)/|p2-p1| and final force
        springForce = { _vector.x / mag * k * x,_vector.y / mag * k * x };

        //new position for partice
        p1->Forces(springForce);
        //reversing the direction as F=-kx
        springForce = { -springForce.x,-springForce.y };
        p2->Forces(springForce);
    }

    //to draw line
    void draw(RenderWindow &window) {
        Vertex line[] = {
            Vector2f(p1->GetPosition()),
            Vector2f(p2->GetPosition())
        };
        window.draw(line, 2, LineStrip);
    }

    //returns particle p1 connected to spring/stick
    Particle* GetParticlep1() {
        return p1;
    }

    //returns particle p2 connected to spring/stick
    Particle* GetParticlep2() {
        return p2;
    }

    float GetSpringLength() {
        Vector2f _vector = p2->GetPosition() - p1->GetPosition();
        float mag = sqrt(_vector.x * _vector.x + _vector.y * _vector.y);
        return mag;
    }

    ~Spring() {
        delete(p1);
        delete(p2);
    }
};