#include <SFML/Graphics.hpp>
#include<vector>
#include<particle.h>
#include<Spring.h>

using namespace sf;

sf::RenderWindow window(sf::VideoMode(1920, 1080), "2D cloth simulation");

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
    //fracture point
    float max_length = 100;
    Vector2f gravity = { 0,5 };
    Vector2f windDirection = { 2,2 };
    float windScale = 0.1;
    float windOffsetSpeed = 1;

    Clock clock;
    bool toggle = false;//to toggle between displaying points
    bool toggle_gravity = true;//to turn gravity on and off
    bool toggle_wind = false;//to turn wind on and off
    bool toggle_collision = false;//interact with object or point

    CircleShape shape(5.f);
    shape.setFillColor(Color::Red);

    Vector2f CollisionPosition = { 1300,500 };
    CircleShape Collision_object(50.f);
    Collision_object.setFillColor(Color::Blue);
    

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
            Spring* spring1;
            Spring* spring2;

            //for right edge particle, no need to add spring on right side
            if (i != column - 1) {
                spring1 = new Spring(k, disX, particles[i][j], particles[i + 1][j]);
            }

            //for lower edge, no need to add spring to the downside
            if (j != row - 1) {
                spring2 = new Spring(k, disY, particles[i][j], particles[i][j+1]);
            }

            springs.push_back(spring1);
            springs.push_back(spring2);
        }
    }


    while (window.isOpen())
    {
        window.setFramerateLimit(144);

        //update spring
        for (int i = 0; i < springs.size(); i++) {
            if (springs[i] == NULL)
                continue;

            //for maximum stretch, spring is cut
            if (springs[i]->GetSpringLength() > max_length) {
                springs[i] = NULL;
                delete(springs[i]);
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

        for (int i = 0; i < column; i++) {
            for (int j = 0; j < row; j++) {
                //distance from the centre of collision object to its surface, since circle, radius = 50
                float disObj = 50.f;
                //distance between particle and object
                float disBetween = float(sqrt(pow(CollisionPosition.x - particles[i][j]->GetPosition().x, 2) + pow(CollisionPosition.y - particles[i][j]->GetPosition().y, 2)));
                if (disBetween < disObj) {
                    //vector from centre to the particle position
                    Vector2f OP = { (particles[i][j]->GetPosition().x - CollisionPosition.x),(particles[i][j]->GetPosition().y - CollisionPosition.y) };
                    //magnitude of vector
                    float mag = sqrt(OP.x * OP.x + OP.y * OP.y);
                    //unit vector
                    Vector2f dir_OP = OP / mag;
                    //vector that pushes the particle from its position to the surface of collision body
                    Vector2f push_force =  {dir_OP.x * (disObj - mag), dir_OP.y * (disObj - mag)};

                    particles[i][j]->newPosition(particles[i][j]->GetPosition().x+push_force.x, particles[i][j]->GetPosition().y+push_force.y);
                    
                }
            }
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            Vector2i mousePosition = Mouse::getPosition(window);
            shape.setPosition(mousePosition.x-5, mousePosition.y-5);//to show position of mouse, wrote it only for testing

            //to cut a spring
            if (Mouse::isButtonPressed(Mouse::Right)) {
                for (int i = 0; i < springs.size(); i++) {
                    if (springs[i] == NULL)
                        continue;

                    //obtaining particle position
                    Vector2f posA = springs[i]->GetParticlep1()->GetPosition();
                    Vector2f posB = springs[i]->GetParticlep2()->GetPosition();

                    //to check whether or not spring collides with mouse head
                    bool collisionLine=false;
                    
                    if ((mousePosition.x >= posA.x && mousePosition.x <= posB.x) || (mousePosition.x <= posA.x && mousePosition.x >= posB.x)) {
                        if (mousePosition.y == int(posA.y) && mousePosition.y == int(posB.y)) {
                            collisionLine = true;
                        }
                        else if ((mousePosition.y > posA.y && mousePosition.y < posB.y) || (mousePosition.y < posA.y && mousePosition.y > posB.y)) {
                            if (mousePosition.x == int(posA.x) && mousePosition.x == int(posB.x)) {
                                collisionLine = true;
                            }
                            else {
                                float m = (posA.y - posB.y) / (posA.x - posB.x);
                                float y = m * (mousePosition.x - posB.x) + posB.y;
                                if ((y - mousePosition.y) > 0.00001 || (- y + mousePosition.y) > 0.00001) {
                                    collisionLine = true;
                                }
                            }
                        }
                    }
                
                    //removing spring if mouse collides
                    if (collisionLine) {
                        springs[i] = NULL;
                        delete(springs[i]);
                        break;
                    }
                }
                
            }

            //to move selected particle
            if (Mouse::isButtonPressed(Mouse::Left)) {
                if (toggle_collision) {
                    //arbitary distance around which the particle for which particle interacts
                    float distance = 10;

                    for (int i = 0; i < column; i++) {
                        for (int j = 0; j < row; j++) {
                            //distance between particle and mouse
                            Vector2f pos = particles[i][j]->GetPosition();
                            float _distance = float(sqrt(pow(pos.x - mousePosition.x, 2) + pow(pos.y - mousePosition.y, 2)));

                            //if inside the bubble where click is noticed
                            if (_distance < distance) {
                                distance = _distance;
                                selected = particles[i][j];
                                //to drag the particle
                                //selected->newPosition(mousePosition.x, mousePosition.y);
                            }
                        }
                    }
                    if (Event::KeyReleased) {
                        //to drag the particle
                        selected->newPosition(mousePosition.x, mousePosition.y);
                    }
                }
                if (!toggle_collision) {
                    float distance_collision = 50.f;
                    float _distane_collision = float(sqrt(pow(CollisionPosition.x - mousePosition.x, 2) + pow(CollisionPosition.y - mousePosition.y, 2)));
                    if (_distane_collision <= distance_collision) {
                        CollisionPosition.x = mousePosition.x;
                        CollisionPosition.y = mousePosition.y;
                    }
                }
            }
            
            if (Mouse::isButtonPressed(Mouse::Middle)) {
                toggle = !toggle;
            }

            if (Keyboard::isKeyPressed(Keyboard::Down)) {
                toggle_gravity = !toggle_gravity;
            }

            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                toggle_wind = !toggle_wind;
            }

            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                toggle_collision = !toggle_collision;
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

        Collision_object.setPosition(CollisionPosition.x-50.f,CollisionPosition.y-50.f);
        window.draw(Collision_object);

        window.display();
    }

    return 0;
}