#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "particle.h"
#include "Spring.h"

using namespace sf;

sf::RenderWindow window(sf::VideoMode(1920, 1080), "2D cloth simulation");

//geometry helpers for cutting springs
static float Dot(Vector2f a, Vector2f b) { return a.x * b.x + a.y * b.y; }
static float Cross(Vector2f a, Vector2f b) { return a.x * b.y - a.y * b.x; }

//shortest distance from point p to segment ab
static float PointSegmentDistance(Vector2f p, Vector2f a, Vector2f b) {
    Vector2f ab = b - a;
    float lenSq = Dot(ab, ab);
    if (lenSq < 0.000001f) {
        Vector2f d = p - a;
        return std::sqrt(Dot(d, d));
    }
    float t = std::max(0.f, std::min(1.f, Dot(p - a, ab) / lenSq));
    Vector2f closest = a + ab * t;
    Vector2f d = p - closest;
    return std::sqrt(Dot(d, d));
}

//true if segment ab properly crosses segment cd
static bool SegmentsIntersect(Vector2f a, Vector2f b, Vector2f c, Vector2f d) {
    float d1 = Cross(b - a, c - a);
    float d2 = Cross(b - a, d - a);
    float d3 = Cross(d - c, a - c);
    float d4 = Cross(d - c, b - c);
    return ((d1 > 0) != (d2 > 0)) && ((d3 > 0) != (d4 > 0));
}

//shortest distance between segment ab and segment cd (0 when they cross)
static float SegmentSegmentDistance(Vector2f a, Vector2f b, Vector2f c, Vector2f d) {
    if (SegmentsIntersect(a, b, c, d))
        return 0.f;
    return std::min(std::min(PointSegmentDistance(a, c, d), PointSegmentDistance(b, c, d)),
                    std::min(PointSegmentDistance(c, a, b), PointSegmentDistance(d, a, b)));
}

int main()
{
    //particles per row and column
    const int row=15;
    const int column = 30;

    //distance between particles in x and y axis
    float disX = 40;
    float disY = 40;

    //offset of particles
    Vector2f Offset = { 50,20 };

    Particle* particles[column][row];
    //selected particle
    Particle* selected = NULL;
    //for springs
    std::vector<Spring*> springs;

    //stiffness
    float k=5;
    //a spring tears once it is stretched beyond this multiple of its rest length
    const float maxStretchFactor = 2.5f;
    //how close (in pixels) the mouse path must pass to a spring to cut it
    const float cutTolerance = 4.f;
    //distance (in pixels) within which a click picks up a particle
    const float pickRadius = 10.f;

    Vector2f gravity = { 0,5 };
    Vector2f windDirection = { 2,2 };
    float windScale = 0.1f;
    float windOffsetSpeed = 1;

    const float physicsStep = 1.f / 144.f;
    const float maxFrameTime = 0.25f;//avoid a spiral of death after a long stall
    float accumulator = 0.f;

    Clock clock;
    bool toggle = false;//to toggle between displaying points
    bool toggle_gravity = true;//to turn gravity on and off
    bool toggle_wind = false;//to turn wind on and off
    bool toggle_collision = false;//interact with object or point

    CircleShape shape(5.f);
    shape.setFillColor(Color::Red);

    //collision object (circle)
    const float collisionRadius = 50.f;
    Vector2f CollisionPosition = { 1300,500 };
    CircleShape Collision_object(collisionRadius);
    Collision_object.setOrigin(collisionRadius, collisionRadius);
    Collision_object.setFillColor(Color::Blue);
    bool draggingCollider = false;

    //mouse state
    Vector2f mousePos = { 0,0 };
    Vector2f prevMousePos = { 0,0 };
    bool leftHeld = false;
    bool rightHeld = false;


    //creating grid of particles
    for (int i = 0; i < column; i++) {
        for (int j = 0; j < row; j++) {
            bool staticParticle = j == 0;//top particles

            Particle* particle = new Particle(i * disX + Offset.x, j * disY + Offset.y,staticParticle);
            particles[i][j] = particle;
        }
    }

    //for springs attached to each particle
    for (int i = 0; i < column; i++) {
        for (int j = 0; j < row; j++) {
            //for right edge particle, no need to add spring on right side
            if (i != column - 1) {
                springs.push_back(new Spring(k, disX, particles[i][j], particles[i + 1][j]));
            }

            //for lower edge, no need to add spring to the downside
            if (j != row - 1) {
                springs.push_back(new Spring(k, disY, particles[i][j], particles[i][j + 1]));
            }
        }
    }

    window.setFramerateLimit(144);
    prevMousePos = mousePos = Vector2f(Mouse::getPosition(window));

    while (window.isOpen())
    {
        //input: discrete actions come from events, so each press acts exactly once
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == Keyboard::Down)
                    toggle_gravity = !toggle_gravity;
                else if (event.key.code == Keyboard::Up)
                    toggle_wind = !toggle_wind;
                else if (event.key.code == Keyboard::Left) {
                    toggle_collision = !toggle_collision;
                    selected = NULL;
                    draggingCollider = false;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                Vector2f p((float)event.mouseButton.x, (float)event.mouseButton.y);

                if (event.mouseButton.button == Mouse::Left) {
                    leftHeld = true;
                    if (toggle_collision) {
                        //pick the nearest particle within the pick radius
                        selected = NULL;
                        float best = pickRadius;
                        for (int i = 0; i < column; i++) {
                            for (int j = 0; j < row; j++) {
                                Vector2f d = particles[i][j]->GetPosition() - p;
                                float dist = std::sqrt(Dot(d, d));
                                if (dist < best) {
                                    best = dist;
                                    selected = particles[i][j];
                                }
                            }
                        }
                    }
                    else {
                        //grab the collision object when clicking on it
                        Vector2f d = CollisionPosition - p;
                        draggingCollider = std::sqrt(Dot(d, d)) <= collisionRadius;
                    }
                }
                else if (event.mouseButton.button == Mouse::Right) {
                    rightHeld = true;
                }
                else if (event.mouseButton.button == Mouse::Middle) {
                    toggle = !toggle;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == Mouse::Left) {
                    leftHeld = false;
                    selected = NULL;
                    draggingCollider = false;
                }
                else if (event.mouseButton.button == Mouse::Right) {
                    rightHeld = false;
                }
            }

            if (event.type == sf::Event::LostFocus) {
                leftHeld = rightHeld = false;
                selected = NULL;
                draggingCollider = false;
            }
        }

        //continuous state: current mouse position
        prevMousePos = mousePos;
        mousePos = Vector2f(Mouse::getPosition(window));
        shape.setPosition(mousePos.x - 5, mousePos.y - 5);//to show position of mouse, wrote it only for testing

        //to cut springs: any spring crossed by (or very close to) the mouse path this frame
        if (rightHeld) {
            for (int i = 0; i < springs.size(); i++) {
                if (springs[i] == NULL)
                    continue;

                Vector2f posA = springs[i]->GetParticlep1()->GetPosition();
                Vector2f posB = springs[i]->GetParticlep2()->GetPosition();

                if (SegmentSegmentDistance(prevMousePos, mousePos, posA, posB) <= cutTolerance) {
                    delete springs[i];
                    springs[i] = NULL;
                }
            }
        }

        //to move the collision object
        if (draggingCollider)
            CollisionPosition = mousePos;

        //advance in fixed steps
        accumulator += std::min(clock.restart().asSeconds(), maxFrameTime);

        while (accumulator >= physicsStep) {
            accumulator -= physicsStep;

            //keep the dragged particle glued to the mouse
            if (leftHeld && toggle_collision && selected != NULL)
                selected->newPosition(mousePos.x, mousePos.y);

            //update spring
            for (int i = 0; i < springs.size(); i++) {
                if (springs[i] == NULL)
                    continue;

                //for maximum stretch, spring is cut
                if (springs[i]->GetSpringLength() > springs[i]->GetRestLength() * maxStretchFactor) {
                    delete springs[i];
                    springs[i] = NULL;
                    continue;
                }

                springs[i]->Update();
            }

            //update particles
            for (int i = 0; i < column; i++) {
                for (int j = 0; j < row; j++) {
                    if (toggle_gravity) {
                        //Adding gravity
                        particles[i][j]->Forces(gravity);
                    }

                    if (toggle_wind) {
                        //wind force
                        float noise = windScale + rand() % 2 * windOffsetSpeed;
                        Vector2f wind = windDirection * noise;
                        particles[i][j]->Forces(wind);
                    }

                    particles[i][j]->Update();
                }
            }

            //collision with the circle: push particles to the surface and cancel the velocity component pointing into the object, so they slide instead of sinking/jittering
            for (int i = 0; i < column; i++) {
                for (int j = 0; j < row; j++) {
                    Particle* p = particles[i][j];
                    if (p->IsStatic())
                        continue;

                    //vector from centre to the particle position
                    Vector2f OP = p->GetPosition() - CollisionPosition;
                    float mag = std::sqrt(Dot(OP, OP));
                    if (mag >= collisionRadius)
                        continue;

                    //unit vector (arbitrary direction if the particle is exactly at the centre)
                    Vector2f dir_OP = mag > 0.0001f ? OP / mag : Vector2f(0.f, -1.f);

                    p->SetPosition(CollisionPosition + dir_OP * collisionRadius);

                    Vector2f v = p->GetVelocity();
                    float vn = Dot(v, dir_OP);
                    if (vn < 0.f)
                        p->SetVelocity(v - dir_OP * vn);
                }
            }
        }

        window.clear();

        for (int i = 0; i < column; i++) {
            for (int j = 0; j < row; j++) {
                if (!toggle) {
                    if (j == 0)
                        particles[i][j]->Draw(window);
                }
                else
                    particles[i][j]->Draw(window);
            }
        }


        for (auto spring : springs) {
            if (spring == NULL)
                continue;
            spring->draw(window);
        }

        window.draw(shape);

        Collision_object.setPosition(CollisionPosition);
        window.draw(Collision_object);

        window.display();
    }

    return 0;
}
