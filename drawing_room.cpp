#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <math.h>
#define DEG2RAD 0.0174533f
float azimuth = 45.0f, elevation = 85.0f, radius = 25.0f;// Camera spherical coordinates
int lastX, lastY, mouseDown = 0;// For mouse interaction/
float cx = 0.0f, cy = 0.0f, cz = 0.0f;
GLfloat L0_pos[] = { 5.0f, 8.0f, 5.0f, 1.0f };      // Ceiling light (warm)
GLfloat L1_pos[] = { -8.0f, 6.0f, -8.0f, 1.0f };    // Secondary light (cool)
float fanRotation = 0.0f;// For optional animation of ceiling fan
int animationTimer = 0;// For optional animation timing
// Menu option identifiers
enum {
    MENU_CAM_OUTSIDE = 1, MENU_CAM_INSIDE = 2, MENU_CAM_INSIDE_LOW = 3,
    MENU_LIGHT_CEILING = 10, MENU_LIGHT_GROUND = 11,
    MENU_ANIM_TOGGLE = 20, MENU_MAIN_EXIT = 100
};
// Function prototypes
void display();
void render_Scene();
void reshape(int w, int h);
void onKey(unsigned char key, int x, int y);
void onSpec(int key, int x, int y);
void onMouse(int button, int state, int x, int y);
void onMotion(int x, int y);
void onTimer(int value);
void initLightsAndMaterials();
void drawLightMarker(GLfloat pos[4], float r, float g, float b);
void drawCylinder(float radius, float height, int slices, int stacks);
void drawWalls();
void drawFloor();
void drawMirrorWall();

void cameraMenu(int option);
void lightMenu(int option);
void mainMenu(int option);
void drawCylinder(float radius, float height, int slices, int stacks)// Utility function to draw a cylinder using GLU quadrics
{
	GLUquadricObj* quad = gluNewQuadric();// Create a new quadric object
	gluQuadricNormals(quad, GLU_SMOOTH);// Set normal generation for smooth shading
	gluCylinder(quad, radius, radius, height, slices, stacks);// Draw the circular base
}
void drawWalls()
{
	glPushMatrix();// Room dimensions: 20x10x20 (width x height x depth)

    // Disable lighting for wall geometry (we'll use color only)
    glColor3f(0.85f, 0.82f, 0.78f);  // Beige walls
    // Front wall (positive Z)
	glPushMatrix();// Front wall (positive Z)
	glTranslatef(0.0f, 0.0f, -10.0f);// Move to front wall position
	glScalef(20.0f, 10.0f, 0.2f);// Scale to wall dimensions (thin depth)
	glutSolidCube(1.0f);// Draw the wall as a scaled cube
	glPopMatrix();// Restore matrix for next wall
    // Back wall (negative Z)
    glColor3f(0.82f, 0.80f, 0.76f);  // Slightly darker back wall
	glPushMatrix();// Back wall (negative Z)
	glTranslatef(0.0f, 0.0f, 10.0f);// Move to back wall position
	glScalef(20.0f, 10.0f, 0.2f);// Scale to wall dimensions (thin depth)
	glutSolidCube(1.0f);// Draw the wall as a scaled cube
	glPopMatrix();// Restore matrix for next wall
    // Left wall (negative X)
	glColor3f(0.87f, 0.84f, 0.80f);// Slightly lighter left wall
	glPushMatrix();// Left wall (negative X)
	glTranslatef(-10.0f, 0.0f, 0.0f);// Move to left wall position
	glScalef(0.2f, 10.0f, 20.0f);// Scale to wall dimensions (thin depth)
	glutSolidCube(1.0f);// Draw the wall as a scaled cube
	glPopMatrix();// Restore matrix for next wall
    // Right wall (positive X)
	glColor3f(0.83f, 0.81f, 0.77f);// Slightly darker right wall
	glPushMatrix();// Right wall (positive X)
	glTranslatef(10.0f, 0.0f, 0.0f);// Move to right wall position
	glScalef(0.2f, 10.0f, 20.0f);// Scale to wall dimensions (thin depth)
	glutSolidCube(1.0f);// Draw the wall as a scaled cube
	glPopMatrix();// Restore matrix after all walls
    // Ceiling
	glColor3f(0.90f, 0.88f, 0.85f);// Light ceiling color
	glPushMatrix();// Ceiling
	glTranslatef(0.0f, 5.0f, 0.0f);// Move to ceiling position
	glScalef(20.0f, 0.2f, 20.0f);// Scale to ceiling dimensions (thin height)
	glutSolidCube(1.0f);// Draw the ceiling as a scaled cube
	glPopMatrix();// Restore matrix after ceiling
	glPopMatrix();// Restore matrix after all walls
}
void drawFloor()
{
	glDisable(GL_COLOR_MATERIAL);// Disable color material to use material properties for floor
    // Matte rubber floor material
	GLfloat floor_diff[] = { 0.15f, 0.15f, 0.15f, 1.0f };// Dark gray diffuse
	GLfloat floor_spec[] = { 0.1f, 0.1f, 0.1f, 1.0f };// Very low specular highlights
	GLfloat floor_ambi[] = { 0.1f, 0.1f, 0.1f, 1.0f };// Low ambient reflection
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, floor_ambi);// Set ambient, diffuse, and specular properties for the floor
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, floor_diff);// Set ambient, diffuse, and specular properties for the floor
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, floor_spec);// Set ambient, diffuse, and specular properties for the floor
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 5.0f);// Very low shininess for a matte surface
    glColor3f(0.2f, 0.2f, 0.2f);  // Dark gray floor
	glPushMatrix();// Floor
	glTranslatef(0.0f, -4.95f, 0.0f);// Move to floor position
	glScalef(20.0f, 0.1f, 20.0f);// Scale to floor dimensions (thin height)
	glutSolidCube(1.0f);// Draw the floor as a scaled cube
	glPopMatrix();// Restore matrix after floor
	glEnable(GL_COLOR_MATERIAL);// Re-enable color material for other objects
}
void drawMirrorWall()
{
	glDisable(GL_COLOR_MATERIAL);// Disable color material to use material properties for mirror
    GLfloat mirror_diff[] = { 0.7f, 0.8f, 0.85f, 1.0f };// Light blue-silver diffuse
    GLfloat mirror_spec[] = { 0.95f, 0.95f, 1.0f, 1.0f };// Strong white specular highlights
	GLfloat mirror_ambi[] = { 0.4f, 0.4f, 0.45f, 1.0f };// Slightly bluish ambient reflection
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mirror_ambi);// Slightly bluish ambient
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mirror_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mirror_spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0f);
    glColor3f(0.75f, 0.82f, 0.88f);  // Light blue-silver mirror
	glPushMatrix();// Mirror wall (right wall)
	glTranslatef(9.8f, 0.0f, 0.0f);// Move to mirror wall position (right wall)
	glScalef(0.4f, 9.0f, 18.0f);// Scale to mirror wall dimensions (thin depth, tall height)
	glutSolidCube(1.0f);// Draw the mirror wall as a scaled cube
	glPopMatrix();// Restore matrix after mirror wall
	glEnable(GL_COLOR_MATERIAL);// Re-enable color material for other objects
}
void initLightsAndMaterials()
{
	glEnable(GL_DEPTH_TEST);// Enable depth testing for proper occlusion
	glEnable(GL_NORMALIZE);// Enable automatic normal normalization for correct lighting on scaled objects
	glShadeModel(GL_SMOOTH);// Use smooth shading for better lighting transitions
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);// Enable two-sided lighting for better appearance on thin geometry
	glDisable(GL_CULL_FACE);// Disable back-face culling to allow lighting on all surfaces
	GLfloat ambient[] = { 0.4f, 0.4f, 0.42f, 1.0f };// Global ambient light (soft gray)
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);// Set global ambient light
	GLfloat L0_diff[] = { 0.95f, 0.9f, 0.8f, 1.0f };// Warm ceiling light (soft yellow-white)
	GLfloat L0_spec[] = { 1.0f, 1.0f, 0.95f, 1.0f };// Strong specular for 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, L0_diff);// Set properties for light 0 
	glLightfv(GL_LIGHT0, GL_SPECULAR, L0_spec);// Set properties for light 0 
	glLightfv(GL_LIGHT0, GL_POSITION, L0_pos);// Set position for light 0
	glEnable(GL_LIGHT0);// Enable light 0 ceiling light
	GLfloat L1_diff[] = { 0.6f, 0.7f, 0.85f, 1.0f };// Cool secondary light soft blue-white
	GLfloat L1_spec[] = { 0.7f, 0.8f, 0.95f, 1.0f };// Moderate specular for secondary light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, L1_diff);// Set properties for light 1
	glLightfv(GL_LIGHT1, GL_SPECULAR, L1_spec);// Set properties for light 1
	glLightfv(GL_LIGHT1, GL_POSITION, L1_pos);// Set position for light 1
	glEnable(GL_LIGHT1);// Enable light 1 secondary light
	glEnable(GL_LIGHTING);// Enable lighting calculations
	GLfloat mat_diff[] = { 0.8f, 0.8f, 0.8f, 1.0f };// Default material properties for non-floor, non-mirror objects
	GLfloat mat_spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };// High specular for a slightly shiny look
	GLfloat mat_ambi[] = { 0.3f, 0.3f, 0.3f, 1.0f };// Moderate ambient reflection for general objects
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambi);// Set default material properties for general objects
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diff);// Set default material properties for general objects
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);// Set default material properties for general objects
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80.0f);// Moderate shininess for general objects
	glEnable(GL_COLOR_MATERIAL);// Enable color material mode to allow glColor to affect material properties
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);// Use glColor to set diffuse color, while ambient and specular come from material settings
	glEnable(GL_BLEND);// Enable blending for light markers
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Use standard alpha blending for light markers
}
void drawLightMarker(GLfloat pos[4], float r, float g, float b)
{
    glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);// Move to light position
	GLfloat emiss[] = { r, g, b, 1.0f };// Set emission color to represent the light's color
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emiss);// Draw a small sphere to represent the light source
	glutSolidSphere(0.25f, 16, 16);// Reset emission to zero after drawing the marker
	GLfloat noEmiss[] = { 0.0f, 0.0f, 0.0f, 1.0f };// Reset emission to zero after drawing the marker
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmiss);// Restore matrix after drawing the light marker
	glPopMatrix();// Restore matrix after drawing the light marker
}
void onKey(unsigned char key, int x, int y)
{
    switch (key)
    {
	case 'u': case 'U': cx -= 0.5f; break;// Move camera left
	case 'o': cx += 0.5f; break;// Move camera right
	case 'i': cy -= 0.5f; break;// Move camera down
	case 'k': cy += 0.5f; break;// Move camera up
	case 'j': cz -= 0.5f; break;// Move camera forward
	case 'l': cz += 0.5f; break;// Move camera backward
	case '+': radius -= 1.0f; if (radius < 3.0f) radius = 3.0f; break;// Zoom in (decrease radius)
	case '-': radius += 1.0f; break;// Zoom out (increase radius)
    case 'r': case 'R':
		azimuth = 45.0f; elevation = 25.0f; radius = 25.0f;// Reset to default outside view
		cx = cy = cz = 0.0f;// Reset camera target to center of the room
		break;
	case 27: exit(0);// Exit on 'Esc' key
    }
	glutPostRedisplay();// Request a redraw after changing camera parameters
}
void onSpec(int key, int x, int y)
{
    switch (key)
    {
	case GLUT_KEY_LEFT: azimuth -= 3.0f; break;// Rotate left
	case GLUT_KEY_RIGHT: azimuth += 3.0f; break;// Rotate right
	case GLUT_KEY_UP: elevation += 3.0f; if (elevation > 89.9f) elevation = 89.9f; break;// Rotate up (limit to avoid flipping)
	case GLUT_KEY_DOWN: elevation -= 3.0f; if (elevation < -89.9f) elevation = -89.9f; break;// Rotate down (limit to avoid flipping)
    }
	glutPostRedisplay();// Request a redraw after changing camera angles
}
void onMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN) { mouseDown = 1; lastX = x; lastY = y; }
        else { mouseDown = 0; }
    }
}
void onMotion(int x, int y)
{
	if (!mouseDown) return;// Only rotate if left mouse button is held down
	azimuth += (x - lastX) * 0.5f;// Adjust azimuth based on horizontal mouse movement
	elevation += (y - lastY) * 0.5f;// Adjust elevation based on vertical mouse movement
	if (elevation > 89.9f) elevation = 89.9f;// Limit elevation to avoid flipping the camera upside down
	if (elevation < -89.9f) elevation = -89.9f;// Limit elevation to avoid flipping the camera upside down
	lastX = x; lastY = y;// Update last mouse position for the next movement
	glutPostRedisplay();// Request a redraw after changing camera angles
}
void onTimer(int value)
{
    fanRotation += 6.0f;  // Rotate 6 degrees per frame
	if (fanRotation >= 360.0f) fanRotation -= 360.0f;// Keep rotation within 0-360 degrees
    glutPostRedisplay();
    glutTimerFunc(50, onTimer, 0);  // Update every 50ms (~20 FPS)
}
void cameraMenu(int option)
{
    switch (option)
    {
    case MENU_CAM_OUTSIDE:
		radius = 30.0f; azimuth = 45.0f; elevation = 25.0f;// Default outside view
		cx = cy = cz = 0.0f;// Look at the center of the room
		break;// Inside view with moderate distance and angle
    case MENU_CAM_INSIDE:
		radius = 12.0f; azimuth = 0.0f; elevation = 20.0f;// Look at the center of the room from a closer position
		cx = 0.0f; cy = 0.0f; cz = 2.0f;// Look slightly down at the center of the room
		break;// Inside view very close to the floor, looking up at the room
    case MENU_CAM_INSIDE_LOW:
		radius = 8.0f; azimuth = 90.0f; elevation = 10.0f;// Look at the center of the room from a low angle
		cx = 0.0f; cy = -1.5f; cz = 0.0f;// Look slightly up at the center of the room from a low position
		break;// Add more camera views as needed
    }
	glutPostRedisplay();// Request a redraw after changing camera parameters
}
void lightMenu(int option)
{
    switch (option)
    {
	case MENU_LIGHT_CEILING: L0_pos[1] = 4.8f; break;// Move ceiling light up to the ceiling
	case MENU_LIGHT_GROUND: L0_pos[1] = 0.5f; break;// Move ceiling light down to near the floor
    }
    glutPostRedisplay();
}
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);// Enable double buffering, RGB color mode, and depth testing
	glutInitWindowSize(1200, 900);// Set initial window size
	glutInitWindowPosition(50, 50);// Set initial window position
	glutCreateWindow("drawing room");// Create the window with a title
	glClearColor(0.05f, 0.05f, 0.08f, 1.0f);// Set a dark background color for better contrast with the room
	initLightsAndMaterials();// Initialize lighting and material properties
	glutDisplayFunc(display);// Set the display callback function to render the scene
	glutReshapeFunc(reshape);// Set the reshape callback function to handle window resizing
	glutKeyboardFunc(onKey);// Set the keyboard callback function for camera controls
	glutSpecialFunc(onSpec);// Set the special keys callback function for camera rotation
	glutMouseFunc(onMouse);// Set the mouse callback function for handling mouse button events
	glutMotionFunc(onMotion);// Set the mouse motion callback function for handling mouse movement while a button is pressed
	glutTimerFunc(50, onTimer, 0);// Set the timer callback function for optional animation (e.g., ceiling fan)
	int camMenu = glutCreateMenu(cameraMenu);// Create camera menu and add options
	glutAddMenuEntry("Outside View (Far)", MENU_CAM_OUTSIDE);// Inside view with moderate distance and angle
	glutAddMenuEntry("Inside View (Medium)", MENU_CAM_INSIDE);// Inside view very close to the floor, looking up at the room
	glutAddMenuEntry("Inside View (Close Low)", MENU_CAM_INSIDE_LOW);// Create lighting menu and add options
	int lightMenu_id = glutCreateMenu(lightMenu);// Create lighting menu and add options
	glutAddMenuEntry("Light on Ceiling", MENU_LIGHT_CEILING);// Move ceiling light up to the ceiling
	glutAddMenuEntry("Light on Ground", MENU_LIGHT_GROUND);// Move ceiling light down to near the floor
    // Main menu
	int main_menu = glutCreateMenu(mainMenu);// Create main menu and add options
	glutAddSubMenu("Camera Views", camMenu);// Add camera submenu to main menu
	glutAddSubMenu("Lighting", lightMenu_id);// Add lighting submenu to main menu
	glutAddMenuEntry("Exit Program", MENU_MAIN_EXIT);// Add exit option to main menu
	glutAttachMenu(GLUT_RIGHT_BUTTON);// Attach the main menu to the right mouse button
	glutMainLoop();// Start the main event loop
	return 0;// This line will never be reached, but it's good practice to include it
}
void mainMenu(int option)
{
	if (option == MENU_MAIN_EXIT) exit(0);// Exit the program if the exit option is selected from the main menu
}
void render_Scene()
{
    // Draw room structure
	drawWalls();// Draw walls and ceiling
	drawFloor();// Draw floor
	drawMirrorWall();// Draw the mirror wall on the right side of the room
    // Draw lighting fixtures
	drawLightMarker(L0_pos, 1.0f, 0.95f, 0.7f); // Draw ceiling light marker (warm color)
	drawLightMarker(L1_pos, 0.5f, 0.7f, 0.95f);// Draw secondary light marker (cool color)
   // Stand
	glColor3f(0.45f, 0.25f, 0.1f);// Dark wood color for the stand
	glPushMatrix();// Leg 1
	glTranslatef(7.5f, -5.0f, 7.0f);// Move to leg position
	glRotatef(-75, 1, 0, 0);// Rotate to make the leg vertical
	drawCylinder(0.08f, 3.0f, 20, 20);// Draw the leg as a cylinder
	glPopMatrix();// Leg 2
	glPushMatrix();// Move to leg position
	glTranslatef(8.5f, -5.0f, 7.0f);// Rotate to make the leg vertical
	glRotatef(-75, 1, 0, 0);// Draw the leg as a cylinder
	drawCylinder(0.08f, 3.0f, 20, 20);// Restore matrix after drawing leg 2
	glPopMatrix();// Draw the leg as a cylinder
    // Board frame
	glColor3f(0.35f, 0.2f, 0.05f);// Brown color for the board frame
	glPushMatrix();// Move to board position and scale to frame dimensions
	glTranslatef(8.0f, -2.5f, 7.8f);// Draw the board frame as a scaled cube
	glScalef(3.0f, 2.2f, 0.15f);// Restore matrix after drawing the board frame
    glutSolidCube(1.0f);
    glPopMatrix();
    // White drawing area
	glColor3f(0.95f, 0.95f, 0.95f);// White color for the drawing area of the board
	glPushMatrix();//  Move to board position and scale to drawing area dimensions
	glTranslatef(8.0f, -2.5f, 7.5f);//  Draw the drawing area as a slightly smaller scaled cube in front of the frame
	glScalef(2.5f, 1.8f, 0.05f);// Restore matrix after drawing the drawing area
    glutSolidCube(1.0f);
	glPopMatrix();// Add some simple drawings on the board using colored quads or lines
    // Seat
	glColor3f(0.25f, 0.25f, 0.28f);// Dark gray color for the seat
	glPushMatrix();//Move to seat position and scale to seat dimensions
	glTranslatef(3.5f, -3.6f, 4.5f);// Draw the seat as a scaled cube
	glScalef(1.5f, 0.2f, 1.5f);//	Restore matrix after drawing the seat
	glutSolidCube(1.0f);// Backrest
	glPopMatrix();// Dark gray color for the backrest
    // Back
	glPushMatrix();// Move to backrest position and scale to backrest dimensions
	glTranslatef(3.5f, -2.8f, 3.9f);// Draw the backrest as a scaled cube
	glScalef(1.5f, 1.6f, 0.2f);// Restore matrix after drawing the backrest
    glutSolidCube(1.0f);
    glPopMatrix();
    // Leg 1
	glColor3f(0.15f, 0.15f, 0.15f);// Dark gray color for the legs
	glPushMatrix();// Move to leg position and scale to leg dimensions
	glTranslatef(3.0f, -4.3f, 4.0f);// Draw the leg as a scaled cube
	glScalef(0.12f, 1.4f, 0.12f);// Restore matrix after drawing leg 1
    glutSolidCube(1.0f);
    glPopMatrix();
    // Leg 2
    glPushMatrix();
	glTranslatef(4.0f, -4.3f, 4.0f);// Move to leg position and scale to leg dimensions
	glScalef(0.12f, 1.4f, 0.12f);// Draw the leg as a scaled cube
	glutSolidCube(1.0f);// Restore matrix after drawing leg 2
    glPopMatrix();
    // Leg 3
	glPushMatrix();// Move to leg position and scale to leg dimensions
	glTranslatef(3.0f, -4.3f, 5.0f);// Draw the leg as a scaled cube
	glScalef(0.12f, 1.4f, 0.12f);// Restore matrix after drawing leg 3
	glutSolidCube(1.0f);// Restore matrix after drawing leg 3
	glPopMatrix();//
    // Leg 4
	glPushMatrix();// Move to leg position and scale to leg dimensions
	glTranslatef(4.0f, -4.3f, 5.0f);// Draw the leg as a scaled cube
	glScalef(0.12f, 1.4f, 0.12f);// Restore matrix after drawing leg 4
	glutSolidCube(1.0f);// Restore matrix after drawing leg 4
	glPopMatrix();// Add some simple drawings on the board using colored quads or lines
	glColor3f(0.4f, 0.2f, 0.0f);// Brown color for the first drawing on the board
	glPushMatrix();// Move to drawing position and scale to drawing dimensions
	glTranslatef(-7.0f, 2.0f, -9.7f);// Draw the first drawing as a scaled cube on the board
	glScalef(2.0f, 1.5f, 0.1f);// Restore matrix after drawing the first drawing
	glutSolidCube(1.0f);// Second drawing on the board
	glPopMatrix();// Green color for the second drawing on the board
	glColor3f(0.2f, 0.5f, 0.8f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the second drawing as a scaled cube on the board
	glTranslatef(-7.0f, 2.0f, -9.6f);// Restore matrix after drawing the second drawing
	glScalef(1.6f, 1.1f, 0.05f);// Draw the second drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Third drawing on the board
	glPopMatrix();// Brown color for the third drawing on the board
	glColor3f(0.4f, 0.2f, 0.0f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the third drawing as a scaled cube on the board
	glTranslatef(-2.5f, 2.0f, -9.7f);//	Restore matrix after drawing the third drawing
	glScalef(2.0f, 1.5f, 0.1f);// Draw the third drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Fourth drawing on the board
	glPopMatrix();// Green color for the fourth drawing on the board
	glColor3f(0.8f, 0.4f, 0.2f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the fourth drawing as a scaled cube on the board
	glTranslatef(-2.5f, 2.0f, -9.6f);// Restore matrix after drawing the fourth drawing
	glScalef(1.6f, 1.1f, 0.05f);//Draw the fourth drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Fifth drawing on the board
	glPopMatrix();// Brown color for the fifth drawing on the board
	glColor3f(0.4f, 0.2f, 0.0f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the fifth drawing as a scaled cube on the board
	glTranslatef(2.5f, 2.0f, -9.7f);// Restore matrix after drawing the fifth drawing
	glScalef(2.0f, 1.5f, 0.1f);// Draw the fifth drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Sixth drawing on the board
	glPopMatrix();// Green color for the sixth drawing on the board
	glColor3f(0.3f, 0.7f, 0.3f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the sixth drawing as a scaled cube on the board
	glTranslatef(2.5f, 2.0f, -9.6f);// Restore matrix after drawing the sixth drawing
	glScalef(1.6f, 1.1f, 0.05f);// Draw the sixth drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Seventh drawing on the board
	glPopMatrix();// Brown color for the seventh drawing on the board
	glColor3f(0.4f, 0.2f, 0.0f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the seventh drawing as a scaled cube on the board
	glTranslatef(7.0f, 2.0f, -9.7f);// Restore matrix after drawing the seventh drawing
	glScalef(2.0f, 1.5f, 0.1f);// Draw the seventh drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Eighth drawing on the board
	glPopMatrix();// Green color for the eighth drawing on the board
	glColor3f(0.7f, 0.2f, 0.5f);// Move to drawing position and scale to drawing dimensions
    glPushMatrix();//Draw the eighth drawing as a scaled cube on the board
	glTranslatef(7.0f, 2.0f, -9.6f);// Restore matrix after drawing the eighth drawing
	glScalef(1.6f, 1.1f, 0.05f);// Draw the eighth drawing as a scaled cube on the board
    glutSolidCube(1.0f);
	glPopMatrix();// Add some simple drawings on the board using colored quads or lines
	glColor3f(0.45f, 0.25f, 0.1f);// Brown color for the first drawing on the board
	glPushMatrix();// Move to drawing position and scale to drawing dimensions
	glTranslatef(-9.6f, 1.5f, 5.0f);// Draw the first drawing as a scaled cube on the board
	glScalef(0.1f, 2.0f, 2.5f);// Restore matrix after drawing the first drawing
	glutSolidCube(1.0f);// Second drawing on the board
	glPopMatrix();// Green color for the second drawing on the board
	glColor3f(0.2f, 0.6f, 0.7f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the second drawing as a scaled cube on the board
	glTranslatef(-9.5f, 1.5f, 5.0f);// Restore matrix after drawing the second drawing
	glScalef(0.05f, 1.6f, 2.0f);// Draw the second drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Add some simple drawings on the board using colored quads or lines
	glPopMatrix();// Brown color for the first drawing on the board
	glColor3f(0.45f, 0.25f, 0.1f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();//Draw the first drawing as a scaled cube on the board
	glTranslatef(9.6f, 1.5f, -4.0f);// Restore matrix after drawing the first drawing
	glScalef(0.1f, 2.0f, 2.5f);// Draw the first drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Second drawing on the board
	glPopMatrix();// Green color for the second drawing on the board
	glColor3f(0.8f, 0.5f, 0.2f);// Move to drawing position and scale to drawing dimensions
	glPushMatrix();// Draw the second drawing as a scaled cube on the board
	glTranslatef(9.5f, 1.5f, -4.0f);//Restore matrix after drawing the second drawing
	glScalef(0.05f, 1.6f, 2.0f);//Draw the second drawing as a scaled cube on the board
	glutSolidCube(1.0f);// Restore matrix after drawing the second drawing
	glPopMatrix();// Add more objects and details to the scene as desired
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear the color and depth buffers to prepare for new frame rendering
	glMatrixMode(GL_MODELVIEW);// Set the matrix mode to modelview for camera transformations
	glLoadIdentity();//	Reset the modelview matrix to the identity matrix before applying camera transformations
	float ex = radius * cosf(elevation * DEG2RAD) * cosf(azimuth * DEG2RAD);//	Calculate the camera's eye position based on spherical coordinates (radius, azimuth, elevation)
	float ey = radius * sinf(elevation * DEG2RAD);// Calculate the camera's eye position based on spherical coordinates (radius, azimuth, elevation)
	float ez = radius * cosf(elevation * DEG2RAD) * sinf(azimuth * DEG2RAD);// Calculate the camera's eye position based on spherical coordinates (radius, azimuth, elevation)
	gluLookAt(ex, ey, ez, cx, cy, cz, 0.0f, 1.0f, 0.0f);//	Set up the camera view using gluLookAt with the calculated eye position, target position (cx, cy, cz), and up vector (0, 1, 0)
	glLightfv(GL_LIGHT0, GL_POSITION, L0_pos);// Update light positions in case they were changed through the menu
	glLightfv(GL_LIGHT1, GL_POSITION, L1_pos);//	Update light positions in case they were changed through the menu
	render_Scene();//	Call the function to render the entire scene (room, objects, lights)
	glutSwapBuffers();//	Swap the front and back buffers to display the rendered frame on the screen
}
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);// Set the viewport to cover the new window dimensions
	glMatrixMode(GL_PROJECTION);//	Set the matrix mode to projection to adjust the perspective
	glLoadIdentity();// Reset the projection matrix to the identity matrix before setting up the perspective
	GLfloat aspect = (h == 0) ? 1.0f : (GLfloat)w / (GLfloat)h;//	Set up a perspective projection with a 60-degree field of view, aspect ratio based on window dimensions, and near/far clipping planes
    gluPerspective(60.0, aspect, 0.3, 200.0);
    glMatrixMode(GL_MODELVIEW);
}
