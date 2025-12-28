# black-hole-simulation-in-cpp
i simulated blackhole using cpp and basic physics and math
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

const int WIDTH = 800;
const int HEIGHT = 600;
const float PI = 3.14159265359f;

struct BlackHole {
    float x, y;
    float mass;
    float radius;
    float eventHorizon;
} blackHole;

struct Particle {
    float x, y;
    float vx, vy;
    float trail[20][2];
    int trailIndex;
    bool active;
    float life;
};

std::vector<Particle> particles;
float cameraZoom = 1.0f;
int mouseX = 0, mouseY = 0;
bool mousePressed = false;

void initBlackHole() {
    blackHole.x = 0.0f;
    blackHole.y = 0.0f;
    blackHole.mass = 1000.0f;
    blackHole.radius = 20.0f;
    blackHole.eventHorizon = 40.0f;
}

void addParticle(float x, float y, float vx, float vy) {
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.trailIndex = 0;
    p.active = true;
    p.life = 1.0f;
    
    for(int i = 0; i < 20; i++) {
        p.trail[i][0] = x;
        p.trail[i][1] = y;
    }
    
    particles.push_back(p);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    initBlackHole();
    srand(time(NULL));
  
    for(int i = 0; i < 50; i++) {
        float angle = (i / 50.0f) * 2.0f * PI;
        float dist = 150.0f + (rand() % 100);
        float x = cos(angle) * dist;
        float y = sin(angle) * dist;
        

        float v = sqrt(blackHole.mass / dist) * 0.8f;
        float vx = -sin(angle) * v;
        float vy = cos(angle) * v;
        
        addParticle(x, y, vx, vy);
    }
}

void updateParticles(float dt) {
    for(auto& p : particles) {
        if(!p.active) continue;
        

        float dx = blackHole.x - p.x;
        float dy = blackHole.y - p.y;
        float dist = sqrt(dx*dx + dy*dy);
        
        
        if(dist < blackHole.eventHorizon) {
            p.life -= 0.05f;
            if(p.life <= 0.0f) {
                p.active = false;
                continue;
            }
        }
         
        if(dist > 1.0f) {
            float force = blackHole.mass / (dist * dist);
            float ax = (dx / dist) * force;
            float ay = (dy / dist) * force;
            
            
            p.vx += ax * dt;
            p.vy += ay * dt;
        }
        
        
        p.x += p.vx * dt;
        p.y += p.vy * dt;
           
