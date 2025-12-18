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
        
        // Update trail
        p.trail[p.trailIndex][0] = p.x;
        p.trail[p.trailIndex][1] = p.y;
        p.trailIndex = (p.trailIndex + 1) % 20;
    }
}

void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < segments; i++) {
        float theta = 2.0f * PI * float(i) / float(segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawFilledCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for(int i = 0; i <= segments; i++) {
        float theta = 2.0f * PI * float(i) / float(segments);
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)WIDTH / (float)HEIGHT;
    glOrtho(-400.0f * aspect * cameraZoom, 400.0f * aspect * cameraZoom, 
            -400.0f * cameraZoom, 400.0f * cameraZoom, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw accretion disk glow
    for(int i = 10; i > 0; i--) {
        float alpha = 0.02f * i;
        glColor4f(1.0f, 0.5f, 0.0f, alpha);
        drawFilledCircle(blackHole.x, blackHole.y, 
                        blackHole.eventHorizon + i * 5.0f, 50);
    }
    
    // Draw event horizon
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    drawFilledCircle(blackHole.x, blackHole.y, blackHole.eventHorizon, 50);
    
    // Draw event horizon boundary
    glColor4f(1.0f, 0.3f, 0.0f, 0.8f);
    glLineWidth(2.0f);
    drawCircle(blackHole.x, blackHole.y, blackHole.eventHorizon, 50);
    
    // Draw photon sphere
    glColor4f(0.5f, 0.5f, 1.0f, 0.3f);
    glLineWidth(1.0f);
    drawCircle(blackHole.x, blackHole.y, blackHole.eventHorizon * 1.5f, 50);
    
    // Draw particles and trails
    for(const auto& p : particles) {
        if(!p.active) continue;
        
        // Draw trail
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i < 20; i++) {
            int idx = (p.trailIndex + i) % 20;
            float alpha = (i / 20.0f) * 0.5f * p.life;
            glColor4f(0.5f, 0.8f, 1.0f, alpha);
            glVertex2f(p.trail[idx][0], p.trail[idx][1]);
        }
        glEnd();
        
        
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glColor4f(0.8f, 0.9f, 1.0f, p.life);
        glVertex2f(p.x, p.y);
        glEnd();
    }
    
    glutSwapBuffers();
}

void timer(int value) {
    updateParticles(0.016f);
    
    
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return !p.active; }),
        particles.end()
    );
    

    if(rand() % 100 < 5 && particles.size() < 100) {
        float angle = (rand() % 360) * PI / 180.0f;
        float dist = 200.0f + (rand() % 150);
        float x = cos(angle) * dist;
        float y = sin(angle) * dist;
        
        float v = sqrt(blackHole.mass / dist) * (0.7f + (rand() % 30) / 100.0f);
        float vx = -sin(angle) * v;
        float vy = cos(angle) * v;
        
        addParticle(x, y, vx, vy);
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON) {
        mousePressed = (state == GLUT_DOWN);
        mouseX = x;
        mouseY = y;
    }
}

void mouseWheel(int button, int dir, int x, int y) {
    if(dir > 0) {
        cameraZoom *= 0.9f;
    } else {
        cameraZoom *= 1.1f;
    }
    if(cameraZoom < 0.5f) cameraZoom = 0.5f;
    if(cameraZoom > 3.0f) cameraZoom = 3.0f;
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 27: // ESC
            exit(0);
            break;
        case ' ':
            // Add random particle
            float angle = (rand() % 360) * PI / 180.0f;
            float dist = 150.0f + (rand() % 100);
            float px = cos(angle) * dist;
            float py = sin(angle) * dist;
            float v = sqrt(blackHole.mass / dist) * 0.8f;
            addParticle(px, py, -sin(angle) * v, cos(angle) * v);
            break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Black Hole Simulation");
    
    init();
    
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    glutMouseFunc(mouse);
    glutMouseWheelFunc(mouseWheel);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}
