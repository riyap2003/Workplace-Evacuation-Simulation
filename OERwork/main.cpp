#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <limits>
#include<random>

#define W 800
#define H 800

// Constants for the person's movement
#define PERSON_SIZE 10
#define PERSON_COLOR_RED 0.5f
#define PERSON_COLOR_GREEN 0.0f
#define PERSON_COLOR_BLUE 0.5f

// Clipping region boundaries
#define CLIP_LEFT -750
#define CLIP_RIGHT 550
#define CLIP_BOTTOM -550
#define CLIP_TOP 600

// Cohen-Sutherland line clipping region codes
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

class Person {
public:
    float x, y;        // Current position
    float destX, destY; // Destination coordinates
    float speed;       // Speed of movement
    bool hasDestination;


    Person(float startX, float startY, float destX, float destY, float speed)
        : x(startX), y(startY), destX(destX), destY(destY), speed(speed) {}
    Person(float startX, float startY, float speed)
        : x(startX), y(startY), speed(speed), hasDestination(false) {}

    // Update the position of the person
    void updatePosition() {
        float dx = destX - x;
        float dy = destY - y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > speed) {
            float ratio = speed / distance;
            x += dx * ratio;
            y += dy * ratio;
        }
        else {
            x = destX;
            y = destY;
            hasDestination = false; // Reached the destination
        }
    }

    // Draw the person
    void draw() {
        glColor3f(PERSON_COLOR_RED, PERSON_COLOR_GREEN, PERSON_COLOR_BLUE);
        glPointSize(PERSON_SIZE);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }

    // Set destination coordinates for movement
    void setDestination(float newDestX, float newDestY) {
        destX = newDestX;
        destY = newDestY;
        hasDestination = true;

    }
};

// Global variables
std::vector<Person> persons; // Vector to store all persons
bool animationEnabled = true; // Variable to control animation state

// Exit door coordinates
std::vector<std::pair<float, float>> exitDoors = {
    {-750.0f, 300.0f},  // Exit near Office 1 to Office 2
    {-750.0f, -25.0f},  // Exit near Office 2 to Office 3
    {550.0f, 200.0f},   // Exit near Office 5 to Fax/Copy
    {550.0f, -50.0f}    // Exit near Fax/Copy to Office 5
};

// Entry points for each room
std::vector<std::pair<float, float>> entryPoints = {
    {-550.0f, 500.0f},   // Entry point for Office 1
    {-550.0f, 100.0f},   // Entry point for Office 2
    {-550.0f, -25.0f},   // Entry point for Office 3
    {-450.0f, 300.0f},   // Entry point for Conference Room Left
    {-250.0f, 400.0f},      // Entry point for Conference Room Right
    {350.0f, 500.0f},    // Entry point for Lunchroom
    {350.0f, 200.0f},    // Entry point for Fax/Copy
    {350.0f, -50.0f},   // Entry point for Office 5
    {-200.0f, -50.0f},  // Entry point for WC 1
    {-25.0f, -50.0f},   // Entry point for WC 2
    {-375.0f, -50.0f} ,  // Entry point for Storage
    {-500.0f,120.0f},
    {-450.0f,50.0f},
    {-200.0f,50.0f},
    {350.0f,500.0f}

};
// Function to compute Euclidean distance between two points
float distance(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}


void initializePersons() {
    // Office 1 to Office 2
    persons.push_back(Person(-700, 500, -700, 200, 1.0));

    // Office 2 to Office 3
    persons.push_back(Person(-700, 200, -700, -100, 0.5));

    // Conference Room Left to Conference Room Right
    persons.push_back(Person(-400, 200, 0, 400, 0.8));

    // Lunchroom to Fax/Copy
    persons.push_back(Person(400, 500, 400, 200, 0.7));

    // Fax/Copy to Office 5
    persons.push_back(Person(400, 0, 400, -100, 0.4));

    // WC 1 to Storage
    persons.push_back(Person(-200, -100, -250, -250, 0.6));

    // WC 2 to WC 1
    persons.push_back(Person(-50, -100, -50, -250, 0.7));

    // Storage to WC 1
    persons.push_back(Person(-375, -100, -500, -250, 0.5));
}

// Function prototypes
void drawRectangle(float x1, float y1, float x2, float y2);
void drawLine(float x1, float y1, float x2, float y2);
void drawText(float x, float y, const char* string);
void drawRectangleOutline(float x1, float y1, float x2, float y2);
void drawOfficeLayout();
void myinit();
void display();
void animate(int);
void toggleAnimation();
void handleExit();
void keyboard(unsigned char key, int x, int y);

// Function to compute region code for a point (x, y)
int computeCode(float x, float y) {
    int code = INSIDE;

    if (x < CLIP_LEFT)
        code |= LEFT;
    else if (x > CLIP_RIGHT)
        code |= RIGHT;
    if (y < CLIP_BOTTOM)
        code |= BOTTOM;
    else if (y > CLIP_TOP)
        code |= TOP;

    return code;
}

// Function to perform Cohen-Sutherland clipping
bool cohenSutherlandClip(float& x0, float& y0, float& x1, float& y1) {
    int code0 = computeCode(x0, y0);
    int code1 = computeCode(x1, y1);

    bool accept = false;

    while (true) {
        if ((code0 == 0) && (code1 == 0)) {
            accept = true;
            break;
        }
        else if (code0 & code1) {
            break;
        }
        else {
            int codeOut;
            float x, y;

            if (code0 != 0)
                codeOut = code0;
            else
                codeOut = code1;

            if (codeOut & TOP) {
                x = x0 + (x1 - x0) * (CLIP_TOP - y0) / (y1 - y0);
                y = CLIP_TOP;
            }
            else if (codeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (CLIP_BOTTOM - y0) / (y1 - y0);
                y = CLIP_BOTTOM;
            }
            else if (codeOut & RIGHT) {
                y = y0 + (y1 - y0) * (CLIP_RIGHT - x0) / (x1 - x0);
                x = CLIP_RIGHT;
            }
            else if (codeOut & LEFT) {
                y = y0 + (y1 - y0) * (CLIP_LEFT - x0) / (x1 - x0);
                x = CLIP_LEFT;
            }

            if (codeOut == code0) {
                x0 = x;
                y0 = y;
                code0 = computeCode(x0, y0);
            }
            else {
                x1 = x;
                y1 = y;
                code1 = computeCode(x1, y1);
            }
        }
    }

    return accept;
}

// Function definitions

// Function to draw a filled rectangle
void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// Function to draw a line with clipping
void drawLine(float x1, float y1, float x2, float y2) {
    if (cohenSutherlandClip(x1, y1, x2, y2)) {
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
}

// Function to draw text at a specific position
void drawText(float x, float y, const char* string) {
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// Function to draw an outline of a rectangle
void drawRectangleOutline(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);

    glVertex2f(x2, y1);
    glVertex2f(x2, y2);

    glVertex2f(x2, y2);
    glVertex2f(x1, y2);

    glVertex2f(x1, y2);
    glVertex2f(x1, y1);
    glEnd();
}

// Function to draw exit doors
void drawExitDoors() {
    glPointSize(8);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for exit doors

    glBegin(GL_POINTS);
    for (const auto& exitDoor : exitDoors) {
        glVertex2f(exitDoor.first, exitDoor.second);
    }
    glEnd();
}

void drawOfficeLayout() {

    // Set color for walls
    glColor3f(0.1f, 0.5f, 0.7f);

    // Draw external walls
    drawLine(-750, 600, -750, -250);
    drawLine(-750, -250, 550, -250); 
    drawLine(550, -250, 550, 600); 
    drawLine(-550, 600, 550, 600); 

    // Draw office rooms and other spaces with outlines
    drawRectangleOutline(-750, 350, -550, 600); 
    drawRectangle(-749, 349, -551, 599); 
 
    drawRectangleOutline(-750, 50, -550, 300);
    drawRectangle(-749, 49, -551, 299); 
    
    drawRectangleOutline(-750, -250, -550, 0);
    drawRectangle(-749, -251, -551, -1); 

    
    drawRectangleOutline(-500, 50, -250, 300);
    drawRectangle(-499, 49, -251, 299); 

    drawRectangleOutline(-250, 50, 300, 600);
    drawRectangle(-249, 49, 299, 599); 

    drawRectangleOutline(350, 350, 550, 600);
    drawRectangle(351, 351, 549, 599); 

    drawRectangleOutline(350, 150, 550, 300);
    drawRectangle(351, 149, 549, 299);

    drawRectangleOutline(350, -100, 550, 100);
    drawRectangle(351, -101, 549, 99); 

    drawRectangleOutline(-250, -250, -50, -50);
    drawRectangle(-249, -251, -51, -49); 

    drawRectangleOutline(-50, -250, 150, -50);
    drawRectangle(-49, -251, 149, -49); 

    drawRectangleOutline(-500, -250, -250, -50);
    drawRectangle(-499, -251, -251, -49); 

    // Draw table and chairs
    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(-50, 250, 150, 350); 

    glColor3f(0.2f, 0.2f, 0.2f); 
    drawRectangle(-70, 250, -50, 270); 
    drawRectangle(-70, 330, -50, 350);
    drawRectangle(130, 250, 150, 270); 
    drawRectangle(130, 330, 150, 350); 

    
    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(-675, 425, -625, 475); 

    glColor3f(0.2f, 0.2f, 0.2f); 
    drawRectangle(-670, 430, -660, 440); 
    drawRectangle(-670, 470, -660, 460); 

    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(-675, 125, -625, 175); 

    glColor3f(0.2f, 0.2f, 0.2f); 
    drawRectangle(-670, 130, -660, 140); 
    drawRectangle(-670, 170, -660, 160); 

    
    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(-675, -175, -625, -125); 

    glColor3f(0.2f, 0.2f, 0.2f); 
    drawRectangle(-670, -170, -660, -160); 
    drawRectangle(-670, -130, -660, -140);

    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(50, 350, 100, 400); 

    glColor3f(0.2f, 0.2f, 0.2f); 
    drawRectangle(55, 355, 65, 365); 
    drawRectangle(95, 355, 85, 365); 

    
    glColor3f(0.5f, 0.5f, 0.5f); 
    drawRectangle(400, 450, 450, 500); 

    glColor3f(0.2f, 0.2f, 0.2f); 

    drawRectangle(380, 460, 390, 470);
    drawRectangle(420, 460, 430, 470); 
    drawRectangle(380, 490, 390, 500); 


    // Draw entry points as green points
    glColor3f(0.0f, 1.0f, 0.0f); 
    glPointSize(7.0f);
    for (const auto& entry : entryPoints) {
        glBegin(GL_POINTS);
        glVertex2f(entry.first, entry.second);
        glEnd();
    }

    drawExitDoors();

    // Labels for rooms
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-700, 500, "Office 1");
    drawText(-700, 200, "Office 2");
    drawText(-700, -100, "Office 3");
    drawText(-400, 200, "Conference Room Left");
    drawText(0, 400, "Conference Room Right");
    drawText(400, 500, "Lunchroom");
    drawText(400, 200, "Fax/Copy");
    drawText(400, 0, "Office 5");
    drawText(-200, -100, "WC 1");
    drawText(50, -100, "WC 2");
    drawText(-375, -100, "Storage");
}

// Function to initialize OpenGL settings
void myinit() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-W, W, -H, H);
    glMatrixMode(GL_MODELVIEW);
}

// Function to display the scene
void display() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    drawOfficeLayout();

    // Draw all persons
    for (auto& person : persons) {
        person.draw();
    }


    glFlush();
}



// Function to update person positions and trigger redraw
void animate(int) {
    if (animationEnabled) {
        // Update positions of all persons
        for (auto& person : persons) {
            person.updatePosition();
        }


        glutPostRedisplay(); 
        glutTimerFunc(1000 / 60, animate, 0); 
    }
}

// Function to toggle animation state
void toggleAnimation() {
    animationEnabled = !animationEnabled;
    if (animationEnabled) {
        // Restart animation
        glutTimerFunc(0, animate, 0);
    }
}



// Function to handle exit from nearest exit door
void handleExit() {
    for (auto& person : persons) {
        float minDistance = std::numeric_limits<float>::max();
        std::pair<float, float> nearestExit;

        for (const auto& door : exitDoors) {
            float dx = door.first - person.x;
            float dy = door.second - person.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < minDistance) {
                minDistance = distance;
                nearestExit = door;
            }
        }

        // Set person's destination to the nearest exit
        person.setDestination(nearestExit.first, nearestExit.second);
    }
}
void handle() {
    srand(time(nullptr)); // Seed random number generator

    static std::vector<int> currentEntryPointIndex(persons.size(), 0); // Track the current entry point index for each person

    for (size_t i = 0; i < persons.size(); ++i) {
        auto& person = persons[i];
        int& currentIndex = currentEntryPointIndex[i];

        // Check distances to all entry points
        float minDistance = std::numeric_limits<float>::max();
        std::pair<float, float> nearestEntry;

        for (size_t j = 0; j < entryPoints.size(); ++j) {
            float dist = distance(person.x, person.y, entryPoints[j].first, entryPoints[j].second);

            if (dist < minDistance) {
                minDistance = dist;
                nearestEntry = entryPoints[j];
                currentIndex = j; // Update the current index to the nearest entry point
            }
        }

        // Set person's destination to the nearest entry point
        person.setDestination(nearestEntry.first, nearestEntry.second);
    }
}



void handleEntry() {
    const int maxIterations = 1000;
    const float stepSize = 20.0f;
    const float goalThreshold = 30.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(CLIP_LEFT, CLIP_RIGHT);
    std::uniform_real_distribution<float> disY(CLIP_BOTTOM, CLIP_TOP);

    for (auto& person : persons) {
        // Skip if the person already has a destination
        if (person.hasDestination) continue;

        // Initialize RRT structure for the current person
        std::vector<std::pair<float, float>> tree;
        tree.push_back(std::make_pair(person.x, person.y)); // Start with the person's current position

        // Expand the tree
        for (int i = 0; i < maxIterations; ++i) {
            // Generate a random point
            float randomX = disX(gen);
            float randomY = disY(gen);
            std::pair<float, float> randomPoint(randomX, randomY);

            // Find the closest point in the tree
            float minDist = std::numeric_limits<float>::max();
            std::pair<float, float> closestPoint;
            for (const auto& node : tree) {
                float dist = distance(node.first, node.second, randomPoint.first, randomPoint.second);
                if (dist < minDist) {
                    minDist = dist;
                    closestPoint = node;
                }
            }

            // Move from the closest point towards the random point
            float dx = randomPoint.first - closestPoint.first;
            float dy = randomPoint.second - closestPoint.second;
            float distToRandom = sqrt(dx * dx + dy * dy);

            if (distToRandom > stepSize) {
                float newX = closestPoint.first + (dx / distToRandom) * stepSize;
                float newY = closestPoint.second + (dy / distToRandom) * stepSize;
                tree.push_back(std::make_pair(newX, newY));
            }
            else {
                tree.push_back(randomPoint);
            }

            // Check if we have reached any entry point
            for (size_t j = 0; j < entryPoints.size(); ++j) {
                float dist = distance(tree.back().first, tree.back().second, entryPoints[j].first, entryPoints[j].second);
                if (dist < goalThreshold) {
                    // Update the person's destination to this entry point
                    person.setDestination(entryPoints[j].first, entryPoints[j].second);
                    break; // Exit loop once the goal is reached
                }
            }

            if (person.hasDestination) {
                break; // Exit loop if the person has a destination
            }
        }
    }
}



void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case ' ': 
        toggleAnimation();
        break;
    case 's':handleEntry(); 
        break;
    case 'S':handle();break;
    case 'd': 
        handleExit();
        break;
    case '1': 
        if (!persons.empty()) {
            persons[0].setDestination(100.0f, 100.0f); 
        }
        break;
    case '2':
        if (persons.size() > 1) {
            persons[1].setDestination(-100.0f, -100.0f); 
        }
        break;
    case '3': 
        if (persons.size() > 2) {
            persons[2].setDestination(200.0f, 200.0f); 
        }
        break;
    case '+': 
        for (auto& person : persons) {
            person.speed += 0.1f;
        }
        break;
    case '-': 
        for (auto& person : persons) {
            if (person.speed > 0.1f) {
                person.speed -= 0.1f;
            }
        }
        break;
    case 'r': 
        initializePersons();
        break;
    case 'c':
        persons.clear();
        break;
    case 'a':
        persons.push_back(Person(0.0f, 0.0f, 100.0f, 100.0f, 0.5f)); 
        break;
    default:exit(0);
        break;
    }
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("Office Layout with Routes");
    myinit();
    initializePersons();
    glutDisplayFunc(display);
    glutTimerFunc(0, animate, 0);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}