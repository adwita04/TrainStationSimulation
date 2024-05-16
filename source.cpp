/*
Name: Adwita Deshpande
Roll no.: 2203303
Branch: Mathematics and computing

Name: V.Nikita Sairam
Roll no.: 2003329
Branch: Mathematics and computing

Modelling a train and a station using blender and importing obj files
*/

#define _USE_MATH_DEFINES

#include <iostream>
#include <sstream>    
#include <string> 
#include <fstream> 
#include <vector>
#include "getBMP.h"

#include <GL/glew.h>
#include <GL/freeglut.h> 

using namespace std;

static int width, height; //openGL window size.
static float Xangle = 0.0, Yangle = 40.0, Zangle = 0.0; //angles to rotate the scene
float moveLeft = -4.7, moveRight = 16.3, moveBottom = -4.5, moveTop = 2.4, zoomIn = -16, zoomOut = 0; //distance to move the camera by


vector<float> verticesVector; //vector to read in vertex x, y and z values fromt the OBJ file.
vector<int> facesVector; //vector to read in face vertex indices from the OBJ file.
static float* vertices = NULL;  //vertex array of the object x, y, z values.
static int* faces = NULL; //face (triangle) vertex indices.
static int numIndices; //number of face vertex indices.
vector<float*> allVertices;
vector<int> allNumIndices;
vector<int*> allFaces;
static unsigned int texture[1]; // Array of texture ids.
static float t = -15, t2 = -3;

static int call_train = 0;
static int leave_train = 0;
static int trainstop = 0;
static int addTrain = 0;
static int light = 1;


//function to load texture
void loadTextures()
{
	imageFile* image[1];
	image[0] = getBMP("image.bmp");
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

//function to populate arrays from obj file
void loadOBJ(std::string fileName)
{
	std::string line;
	int count, vertexIndex1, vertexIndex2, vertexIndex3;
	float coordinateValue;
	char currentCharacter, previousCharacter;

	//open the OBJ file.
	std::ifstream inFile(fileName.c_str(), std::ifstream::in);

	//read successive lines.
	while (getline(inFile, line))
	{
		//line has vertex data.
		if (line.substr(0, 2) == "v ")
		{
			//initialize a string from the character after "v " to the end.
			std::istringstream currentString(line.substr(2));

			//read x, y and z values. The (optional) w value is not read. 
			for (count = 1; count <= 3; count++)
			{
				currentString >> coordinateValue;
				verticesVector.push_back(coordinateValue);
			}
		}

		//line has face data.
		else if (line.substr(0, 2) == "f ")
		{
			//initialize a string from the character after "f " to the end.
			std::istringstream currentString(line.substr(2));
			previousCharacter = ' ';
			count = 0;
			while (currentString.get(currentCharacter))
			{
				//stop processing line at comment.
				if ((previousCharacter == '#') || (currentCharacter == '#')) break;

				//current character is the start of a vertex index.
				if ((previousCharacter == ' ') && (currentCharacter != ' '))
				{
					//move the string cursor back to just before the vertex index.
					currentString.unget();

					//read the first vertex index, decrement it so that the index range is from 0, increment vertex counter.
					if (count == 0)
					{
						currentString >> vertexIndex1;
						vertexIndex1--;
						count++;
					}

					//read the second vertex index, decrement it, increment vertex counter.
					else if (count == 1)
					{
						currentString >> vertexIndex2;
						vertexIndex2--;
						count++;
					}

					//read the third vertex index, decrement it, increment vertex counter AND output the first triangle.
					else if (count == 2)
					{
						currentString >> vertexIndex3;
						vertexIndex3--;
						count++;
						facesVector.push_back(vertexIndex1);
						facesVector.push_back(vertexIndex2);
						facesVector.push_back(vertexIndex3);
					}

					//from the fourth vertex and on output the next triangle of the fan.
					else
					{
						vertexIndex2 = vertexIndex3;
						currentString >> vertexIndex3;
						vertexIndex3--;
						facesVector.push_back(vertexIndex1);
						facesVector.push_back(vertexIndex2);
						facesVector.push_back(vertexIndex3);
					}

					//begin the process of detecting the next vertex index just after the vertex index just read.
					currentString.get(previousCharacter);
				}

				//current character is not the start of a vertex index. Move ahead one character.
				else previousCharacter = currentCharacter;
			}
		}

		//nothing other than vertex and face data is processed.
		else
		{
		}
	}

	//close the OBJ file.
	inFile.close();
}


//funtion to populate array for obj files
void populateOBJ(string filename)
{
	//read the external OBJ file into the internal vertex and face vectors.
	loadOBJ(filename);

	//size the vertex array and copy into it x, y, z values from the vertex vector.
	vertices = new float[verticesVector.size()];
	for (int i = 0; i < verticesVector.size(); i++) vertices[i] = verticesVector[i];

	//size the faces array and copy into it face index values from the face vector.
	faces = new int[facesVector.size()];
	for (int i = 0; i < facesVector.size(); i++) faces[i] = facesVector[i];
	numIndices = facesVector.size();

	facesVector.clear();
	verticesVector.clear();

	allVertices.push_back(vertices);
	allFaces.push_back(faces);
	allNumIndices.push_back(numIndices);
}

//function to draw an object with a specific color
void drawOBJ(int i, float r, float g, float b)
{
	glPushMatrix();
	glColor3f(r, g, b);
	glVertexPointer(3, GL_FLOAT, 0, allVertices[i]);
	glDrawElements(GL_TRIANGLES, allNumIndices[i], GL_UNSIGNED_INT, allFaces[i]);
	glPopMatrix();
}


// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightDifAndSpec0[] = { 1.0, 1.0, 1.0, 1.0 };
	float lightPos0[] = { 0.0, 0.0, 3.0, 0.0 };
	float lightDifAndSpec1[] = { 0.0, 1.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec0);
	glEnable(GL_LIGHT0);

	glDisable(GL_LIGHTING);
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	//cout << moveRight << " " << moveLeft << " " << moveBottom << " " << moveTop << " " << zoomOut << " " << zoomIn << " " << Xangle << " " << Yangle << " " << Zangle << endl;
	//move scene
	glTranslatef(moveRight + moveLeft, moveBottom + moveTop, zoomOut + zoomIn);

	//rotate scene.
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	//add texture
	glPushMatrix();
	glTranslatef(0, 0, -7.9);
	glScalef(1.0, 1.0, -1.0);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-8.282875, 1.257135, -3.272818);
	glTexCoord2f(1.0, 0.0); glVertex3f(-8.282875, 1.257135, -4.612810);
	glTexCoord2f(1.0, 1.0); glVertex3f(-8.282875, 1.985069, -4.612810);
	glTexCoord2f(0.0, 1.0); glVertex3f(-8.282875, 1.985069, -3.272818);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-8.282875, 1.257135, -3.272818);
	glTexCoord2f(1.0, 0.0); glVertex3f(-8.282875, 1.257135, -4.612810);
	glTexCoord2f(1.0, 1.0); glVertex3f(-8.282875, 1.985069, -4.612810);
	glTexCoord2f(0.0, 1.0); glVertex3f(-8.282875, 1.985069, -3.272818);
	glEnd();
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);
	drawOBJ(0, 0.82, 0.816, 0.808);
	drawOBJ(1, 0.447, 0.212, 0.153);
	drawOBJ(2, 0.729, 0.718, 0.698);
	drawOBJ(3, 0.894, 0.651, 0.447);
	drawOBJ(4, 0.918, 0.831, 0.667);
	drawOBJ(5, 0.525, 0.384, 0.29);
	drawOBJ(6, 0.184, 0.118, 0.102);
	drawOBJ(7, 0.094, 0.078, 0.145);

	drawOBJ(9, 0.667, 0.612, 0.541);
	if (light == 1)
	{
		drawOBJ(10, 1, 0, 0.267); // red
	}
	else
	{
		drawOBJ(8, 0.388, 0.78, 0.302); //green
	}

	drawOBJ(11, 0.584, 0.224, 0.173);
	drawOBJ(12, 0.784, 0.49, 0.251);
	drawOBJ(14, 0.149, 0.169, 0.267);
	drawOBJ(15, 0.149, 0.169, 0.267);
	drawOBJ(16, 0.149, 0.169, 0.267);

	//train
	if (call_train == 1 || trainstop == 1 || leave_train == 1)
	{
		glPushMatrix();
		glTranslatef(0, 0, t);
		drawOBJ(13, 0.894, 0.231, 0.267);
		drawOBJ(17, 0.95, 0.95, 0.95);
		drawOBJ(20, 0.149, 0.169, 0.267);
		drawOBJ(21, 0.173, 0.91, 0.961);
		drawOBJ(24, 0.8, 0.8, 0.8);
		glPopMatrix();
	}

	//train
	glPushMatrix();
	glTranslatef(-4.8, 0, t2);
	drawOBJ(13, 0.894, 0.231, 0.267);
	drawOBJ(17, 0.95, 0.95, 0.95);
	drawOBJ(20, 0.149, 0.169, 0.267);
	drawOBJ(21, 0.173, 0.91, 0.961);
	drawOBJ(24, 0.8, 0.8, 0.8);
	glPopMatrix();

	drawOBJ(18, 0.204, 0.196, 0.188);
	drawOBJ(19, 0.408, 0.392, 0.38);
	drawOBJ(22, 0.545, 0.608, 0.706);
	//drawOBJ(23, 0.8, 0.8, 0.8);
	glEnable(GL_TEXTURE_2D);
	glutSwapBuffers();
}

//animation function
void animate(int value)
{
	if (addTrain == 1)
	{
		t2 += 0.01;
		if (t2 >= 12)
		{
			t2 = -26;
		}
	}
	glutPostRedisplay();
	glutTimerFunc(1, animate, 1);
}

//idle function
void idle() {
	//glClearColor(1.0, 1.0, 1.0, 1.0);
	if (call_train == 1)
	{
		t += 0.01;
		if (t >= 2)
		{
			light = 1;
			trainstop = 1;
			call_train = 0;
		}
	}
	if (leave_train == 1 && trainstop == 1)
	{
		light = 0;
		t += 0.01;
		if (t >= 35)
		{
			trainstop = 0;
			call_train = 0;
			leave_train = 0;
			t = -26;
			light = 1;
		}
	}
	//cout << "call_train: " << call_train<<" train stop: "<<trainstop<<"leave_train: "<< leave_train<<endl;
	glutPostRedisplay();
}

// window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)w / (float)h, 1.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
	width = w;
	height = h;
}

// The right button menu callback function.
void rightMenu(int id)
{
	if (id == 1)
	{
		light = 1;
		call_train = 0;
		addTrain = 0;
		glutPostRedisplay();
	}
	if (id == 2)
	{
		light = 0;
		call_train = 1;
		addTrain = 1;
		if (trainstop == 1) leave_train = 1;
		animate(1);
		glutPostRedisplay();
	}
	if (id == 3) exit(0);
}

// Function to create menu.
void makeMenu(void)
{
	glutCreateMenu(rightMenu);
	glutAddMenuEntry("Red Light", 1);
	glutAddMenuEntry("Green Light", 2);
	glutAddMenuEntry("Quit", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//keyboard input processing routine
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'x':
		Xangle += 5.0;
		if (Xangle > 360.0) Xangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'X':
		Xangle -= 5.0;
		if (Xangle < 0.0) Xangle += 360.0;
		glutPostRedisplay();
		break;
	case 'y':
		Yangle += 5.0;
		if (Yangle > 360.0) Yangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Y':
		Yangle -= 5.0;
		if (Yangle < 0.0) Yangle += 360.0;
		glutPostRedisplay();
		break;
	case 'z':
		Zangle += 5.0;
		if (Zangle > 360.0) Zangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Z':
		Zangle -= 5.0;
		if (Zangle < 0.0) Zangle += 360.0;
		glutPostRedisplay();
		break;
	case '+':
		zoomIn += 0.1;
		glutPostRedisplay();
		break;
	case '-':
		zoomOut -= 0.1;
		glutPostRedisplay();
		break;
	case 'r': //to reset to original view
		Xangle = 0.0; Yangle = 40.0; Zangle = 0.0;
		moveLeft = -4.7; moveRight = 16.3; moveBottom = -4.5; moveTop = 2.4; zoomIn = -16; zoomOut = 0;
		glutPostRedisplay();
		break;
	case 's': //to call the train
		call_train = 1;
		light = 0;
		glutPostRedisplay();
		break;
	case 'S': //to make it leave
		leave_train = 1;
		glutPostRedisplay();
		break;
	case ' ': //start animation
		if (addTrain) addTrain = 0;
		else
		{
			addTrain = 1;
			animate(1);
		}
		glutPostRedisplay();
		break;
	case 'c':
		Xangle = 0.0; Yangle = 270.0; Zangle = 0.0;
		moveLeft = 1; moveRight = -3.6; moveBottom = -5.1; moveTop = 2.7; zoomIn = -2.7; zoomOut = 10;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

//callback function for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT) moveLeft += 0.1;
	if (key == GLUT_KEY_RIGHT) moveRight -= 0.1;
	if (key == GLUT_KEY_DOWN) moveTop += 0.1;
	if (key == GLUT_KEY_UP) moveBottom -= 0.1;
	glutPostRedisplay();
}

// Initialization routine.
void setup(void)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_LIGHTING);

	// Material property vectors.
	float matAmbAndDif[] = { 1.0, 1.0, 1.0, 0.3 };
	float matSpec[] = { 1.0, 1.0, 1, 0, 1.0 };
	float matShine[] = { 50.0 };

	// Material properties of ball.
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShine);

	// Cull back faces.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	loadTextures();

	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Turn on OpenGL texturing.
	glEnable(GL_TEXTURE_2D);

	populateOBJ("station.obj");
	populateOBJ("station_detail_brown.obj");
	populateOBJ("station_detail_grey.obj");
	populateOBJ("bench_dark.obj");
	populateOBJ("bench_light.obj");
	populateOBJ("bridge2.obj");
	populateOBJ("bridgedetail.obj");
	populateOBJ("fence.obj");
	populateOBJ("greenlight.obj");
	populateOBJ("nameonwall.obj");
	populateOBJ("redlight.obj");
	populateOBJ("shop_dark.obj");
	populateOBJ("shop_light.obj");
	populateOBJ("speedstripe.obj");
	populateOBJ("staircase_blue.obj");
	populateOBJ("stationnameboard_blue.obj");
	populateOBJ("trafficlight_body.obj");
	populateOBJ("trainbody.obj");
	populateOBJ("traintrack_line.obj");
	populateOBJ("traintrack_patri.obj");
	populateOBJ("wheels.obj");
	populateOBJ("windows.obj");
	populateOBJ("staircase.obj");
	populateOBJ("platformname.obj");
	populateOBJ("interior.obj");
	makeMenu();
}

//function to output interaction instructions to the C++ window.
void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Press left/right/up/down arrow keys to move the scene" << std::endl
		<< "Press +/- arrow keys to zoom in and out" << std::endl
		<< "Press x, X, y, Y, z, Z to change the angle" << std::endl
		<< "Press r to return to original position" << std::endl
		<< "Press s to call the train" << std::endl
		<< "Press S to make the train leave" << std::endl
		<< "Press space to start animation of second train" << std::endl
		<< "Press c to go to customer mode" << std::endl
		<< "Right-click for menu" << std::endl;

}

//main function
int main(int argc, char** argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA);

	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(10, 10);

	glutCreateWindow("trainScene.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutIdleFunc(idle);
	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}

/*
References:
Computer Graphics through OpenGl by Sumant Guha
*/