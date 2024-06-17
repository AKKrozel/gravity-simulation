#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <iostream>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const double G = 40;
const double DT = 0.01;
const int NUM_DT = 6000;

struct Body {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    double mass;
    double radius;
    sf::Color color;
};

sf::Vector2f calculateAcceleration(const Body& body, const std::vector<Body>& bodies) {
    sf::Vector2f netForce(0.0f, 0.0f);

    for (const auto& other : bodies) {
        if (&body != &other) {
            sf::Vector2f distanceVec = other.position - body.position;
            double distance = std::sqrt(distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y);
            double forceMagnitude = (G * body.mass * other.mass) / (distance * distance);
            sf::Vector2f forceVec = distanceVec / static_cast<float>(distance) * static_cast<float>(forceMagnitude);
            netForce += forceVec;
        }
    }

    return netForce / static_cast<float>(body.mass);
}

void moveBodies(std::vector<Body>& bodies) {
    for (auto& body : bodies) {
        body.position += body.velocity * static_cast<float>(DT) + 0.5f * body.acceleration * static_cast<float>(DT * DT);
    }

    for (auto& body : bodies) {
        body.velocity += 0.5f * body.acceleration * static_cast<float>(DT);
    }

    for (auto& body : bodies) {
        body.acceleration = calculateAcceleration(body, bodies);
    }

    for (auto& body : bodies) {
        body.velocity += 0.5f * body.acceleration * static_cast<float>(DT);
    }
}

bool checkCollision(const Body& body1, const Body& body2) {
    sf::Vector2f distanceVec = body2.position - body1.position;
    double distanceSquared = distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y;
    double minDistanceSquared = (body1.radius + body2.radius) * (body1.radius + body2.radius);
    return distanceSquared <= minDistanceSquared;
}

void resolveCollision(Body& body1, Body& body2) {
    double totalMass = body1.mass + body2.mass;

    sf::Vector2f momentum1 = body1.velocity * static_cast<float>(body1.mass);
    sf::Vector2f momentum2 = body2.velocity * static_cast<float>(body2.mass);
    sf::Vector2f totalMomentum = momentum1 + momentum2;

    sf::Vector2f combinedVelocity = totalMomentum / static_cast<float>(totalMass);

    body1.mass = totalMass;
    body1.velocity = combinedVelocity;

    // Set body2's mass to zero to mark it for removal
    body2.mass = 0;
}

void handleCollisions(std::vector<Body>& bodies) {
    std::vector<size_t> bodiesToRemove;

    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            if (checkCollision(bodies[i], bodies[j])) {
                if (bodies[i].mass > bodies[j].mass) {
                    resolveCollision(bodies[i], bodies[j]);
                    bodiesToRemove.push_back(j);
                }
                else {
                    resolveCollision(bodies[j], bodies[i]);
                    bodiesToRemove.push_back(i);
                }
            }
        }
    }

    // Remove bodies with zero mass in reverse order to avoid invalidating indices
    std::sort(bodiesToRemove.rbegin(), bodiesToRemove.rend());
    for (size_t index : bodiesToRemove) {
        bodies.erase(bodies.begin() + index);
    }
}


void drawBodies(sf::RenderWindow& window, const std::vector<Body>& bodies) {
    for (const auto& body : bodies) {
        double radius = body.radius;
        sf::CircleShape circle(static_cast<float>(radius));
        circle.setFillColor(body.color);
        circle.setPosition(body.position.x - radius, body.position.y - radius);
        window.draw(circle);
    }
}

void drawDashedLine(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end, sf::Color color) {
    const float dashLength = 10.0f;
    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    direction /= length;

    bool draw = true;
    for (float i = 0; i < length; i += dashLength) {
        if (draw) {
            sf::Vertex line[] = {
                sf::Vertex(start + direction * i, color),
                sf::Vertex(start + direction * std::min(i + dashLength, length), color)
            };
            window.draw(line, 2, sf::Lines);
        }
        draw = !draw;
    }
}

void drawTriangle(sf::RenderWindow& window, const Body& bodyA, const Body& bodyB, const Body& bodyC, sf::Color color) {
    drawDashedLine(window, bodyA.position, bodyB.position, color);
    drawDashedLine(window, bodyB.position, bodyC.position, color);
    drawDashedLine(window, bodyC.position, bodyA.position, color);
}

int main() {

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Orbiting Objects Verlet Simulation");
    window.setFramerateLimit(60);
    
    // Form: {{x, y}, {vx, vy}, {ax, ay}, m, r
    std::vector<Body> bodies(4);
    bodies[0] = { {960, 540}, {0, 0}, {0, 0}, 10000, 25, sf::Color::Yellow };   // Central body, most massive
    bodies[1] = { {960, 800}, {39.2232270276, 0}, {0, 0}, 1000, 15, sf::Color::Blue };    // Orbiting body, second most massive
    bodies[2] = { {1185.16660498, 670}, {19.6116135138, -33.9683110243}, {0, 0}, 1, 5, sf::Color::Magenta };   // Small mass body at stable Lagrange point
    bodies[3] = { {960, 280.00045552937092}, {-39.2232613879, 0}, {0, 0}, 1, 5, sf::Color::White };  // Small mass body at unstable Lagrange point

    for (int i = 0; i < NUM_DT; ++i) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();
        moveBodies(bodies);
        handleCollisions(bodies);
        drawBodies(window, bodies);
        if (bodies.size() >= 3 && ((150 < i && i < 350) || (NUM_DT-350 < i && i < NUM_DT-150))) {
            drawTriangle(window, bodies[0], bodies[1], bodies[2], sf::Color::Red
            );
        }
        window.display();
    }
    
    return 0;
}