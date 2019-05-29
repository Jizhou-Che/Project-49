#include <iostream>
#include <GLUT/glut.h> // macOS.
// #include <GL/glut.h> // Linux.
#include <cmath>

// Calculation global variables.
static const GLfloat radius = 1.0; // Radius of sphere.
static const GLint stacks = 20; // Numebr of latitude lines + 1.
static const GLint slices = 20; // Number of longtitude lines.
static const int numOfPoints = 8; // Number of points to place on the sphere.

static GLdouble crossPoints[(stacks - 1) * slices + 2][3]; // Set of all points at intersection.
static int currentPointsIndices[numOfPoints]; // The indices of the current placing of points.
static GLdouble currentPoints[numOfPoints][3]; // The current placing of points being measured.
static GLdouble optimalPoints[numOfPoints][3]; // The placing of points with optimal measurement value.
static double optimalMeasurement; // The measurement value for the optimal placing.

static bool processing = false;
static bool completed = false;

// OpenGL global variables.
static int window;

static const int windowSize = 1280;

static GLfloat rotateX = 0.0;
static GLfloat rotateY = 0.0;
static GLfloat rotateZ = 0.0;

// Calculation of coordinates for points at intersection.
void set_cross_points(GLfloat radius, GLint slices, GLint stacks){
	int flag = 0;
	// Top.
	crossPoints[flag][0] = 0.0;
	crossPoints[flag][1] = 0.0;
	crossPoints[flag][2] = 1.0;
	for(int i = 0; i < stacks - 1; i++){
		// Stacks, from z+ to z-.
		for(int j = 0; j < slices; j++){
			// Slices, starting from x+.
			// Angle between the projection of OP on x-y plane and x+.
			double theta = j * 2 * M_PI / slices;
			// Angle between OP and z+.
			double fi = (i + 1) * M_PI / stacks;
			// Calculation of coordinates.
			flag++;
			crossPoints[flag][0] = cos(theta) * sin(fi) * radius;
			crossPoints[flag][1] = sin(theta) * sin(fi) * radius;
			crossPoints[flag][2] = cos(fi) * radius;
		}
	}
	// Bottom.
	flag++;
	crossPoints[flag][0] = 0.0;
	crossPoints[flag][1] = 0.0;
	crossPoints[flag][2] = -1.0;
}

/*
// Measure the current placing of points.
// Update the optimal measurement value if applicable.
// Using measurement: maximise the minimum straight line distance between any two points.
// Without optimisation.
static double minMeasurement = 0;
void measure(){
	// Calculate the minimum straight line distance between any two points.
	double minDistance = 2 * radius;
	for(int i = 0; i < numOfPoints - 1; i++){
		for(int j = i + 1; j < numOfPoints; j++){
			double distance = sqrt(pow(currentPoints[i][0] - currentPoints[j][0], 2) + pow(currentPoints[i][1] - currentPoints[j][1], 2) + pow(currentPoints[i][2] - currentPoints[j][2], 2));
			if(distance <= minDistance){
				minDistance = distance;
			}
		}
	}
	// Update the optimal measurement value if applicable.
	if(minDistance > minMeasurement){
		optimalMeasurement = minMeasurement = minDistance;
		for(int i = 0; i < numOfPoints; i++){
			optimalPoints[i][0] = currentPoints[i][0];
			optimalPoints[i][1] = currentPoints[i][1];
			optimalPoints[i][2] = currentPoints[i][2];
		}
		std::cout << "Current optimal measurement: " << optimalMeasurement << std::endl;
	}
}
*/

//
// Measure the current placing of points.
// Update the optimal measurement value if applicable.
// Using measurement: maximise the minimum straight line distance between any two points.
// Optimisation: update the opsition of the second one of the two points that forms the minimum distance.
static double minMeasurement = 0;
void measure(){
	// Calculate the minimum straight line distance between any two points.
	double minDistance = 2 * radius;
	int worstIndex;
	for(int i = 0; i < numOfPoints - 1; i++){
		for(int j = i + 1; j < numOfPoints; j++){
			double distance = sqrt(pow(currentPoints[i][0] - currentPoints[j][0], 2) + pow(currentPoints[i][1] - currentPoints[j][1], 2) + pow(currentPoints[i][2] - currentPoints[j][2], 2));
			if(distance <= minDistance){
				minDistance = distance;
				worstIndex = j;
			}
		}
	}
	// Update the optimal measurement value if applicable.
	if(minDistance > minMeasurement){
		optimalMeasurement = minMeasurement = minDistance;
		for(int i = 0; i < numOfPoints; i++){
			optimalPoints[i][0] = currentPoints[i][0];
			optimalPoints[i][1] = currentPoints[i][1];
			optimalPoints[i][2] = currentPoints[i][2];
		}
		std::cout << "Current optimal measurement: " << optimalMeasurement << std::endl;
	}
	// Optimise the worst index.
	if(worstIndex != numOfPoints - 1){
		currentPointsIndices[worstIndex]++;
		currentPoints[worstIndex][0] = crossPoints[currentPointsIndices[worstIndex]][0];
		currentPoints[worstIndex][1] = crossPoints[currentPointsIndices[worstIndex]][1];
		currentPoints[worstIndex][2] = crossPoints[currentPointsIndices[worstIndex]][2];
	}
	for(int i = worstIndex + 1; i < numOfPoints; i++){
		currentPointsIndices[i] = currentPointsIndices[i - 1] + 1;
		if(i == numOfPoints - 1){
			currentPointsIndices[i]--;
		}
		currentPoints[i][0] = crossPoints[currentPointsIndices[i]][0];
		currentPoints[i][1] = crossPoints[currentPointsIndices[i]][1];
		currentPoints[i][2] = crossPoints[currentPointsIndices[i]][2];
	}
}
//

// Place points onto the sphere.
void place_points(){
	if(currentPointsIndices[numOfPoints - 1] == 0){
		// Initialise the indices for current placing of points.
		for(int i = 0; i < numOfPoints; i++){
			currentPointsIndices[i] = i;
			currentPoints[i][0] = crossPoints[i][0];
			currentPoints[i][1] = crossPoints[i][1];
			currentPoints[i][2] = crossPoints[i][2];
		}
		measure();
	}else{
		bool updated = false;
		for(int i = numOfPoints; i > 0; i--){
			if(currentPointsIndices[i - 1] < (stacks - 1) * slices + 2 - numOfPoints + i - 1){
				currentPointsIndices[i - 1]++;
				currentPoints[i - 1][0] = crossPoints[currentPointsIndices[i - 1]][0];
				currentPoints[i - 1][1] = crossPoints[currentPointsIndices[i - 1]][1];
				currentPoints[i - 1][2] = crossPoints[currentPointsIndices[i - 1]][2];
				for(int j = i; j < numOfPoints; j++){
					currentPointsIndices[j] = currentPointsIndices[j - 1] + 1;
					currentPoints[j][0] = crossPoints[currentPointsIndices[j]][0];
					currentPoints[j][1] = crossPoints[currentPointsIndices[j]][1];
					currentPoints[j][2] = crossPoints[currentPointsIndices[j]][2];
				}
				updated = true;
				break;
			}
		}
		if(updated == true){
			measure();
		}else{
			completed = true;
			glutIdleFunc(NULL);
			std::cout << "Completed." << std::endl;
			std::cout << "Optimal placing of points:" << std::endl;
			for(int i = 0; i < numOfPoints; i++){
				std::cout << optimalPoints[i][0] << "\t";
				std::cout << optimalPoints[i][1] << "\t";
				std::cout << optimalPoints[i][2] << std::endl;
			}
			std::cout << "Optimal measurement:" << std::endl;
			std::cout << optimalMeasurement << std::endl;
		}
	}
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT);
	// Axes.
	glBegin(GL_LINES);
		// Red x-axis.
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(5, 0, 0);
		// Green y-axis.
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 5, 0);
		// Blue z-axis.
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 5);
	glEnd();
	// Sphere.
	glColor3f(1, 1, 1);
	glutWireSphere(radius, slices, stacks);
	// Display the optimal placing of points in green.
	for(int i = 0; i < numOfPoints; i++){
		glPushMatrix();
			glTranslated(optimalPoints[i][0], optimalPoints[i][1], optimalPoints[i][2]);
			glColor3f(0, 1, 0);
			glutSolidSphere(0.01, 4, 4);
		glPopMatrix();
	}
	// Display the current placing of points being measured in red.
	for(int i = 0; i < numOfPoints; i++){
		glPushMatrix();
			glTranslated(currentPoints[i][0], currentPoints[i][1], currentPoints[i][2]);
			glColor3f(1, 0, 0);
			glutWireSphere(0.01, 4, 4);
		glPopMatrix();
	}
	// Rotation.
	glRotatef(rotateX, 1.0, 0.0, 0.0);
	glRotatef(rotateY, 0.0, 1.0, 0.0);
	glRotatef(rotateZ, 0.0, 0.0, 1.0);
	rotateX = 0.0;
	rotateY = 0.0;
	rotateZ = 0.0;
	glutSwapBuffers();
}

void idle(){
	place_points();
//	glutPostRedisplay(); // Comment out this line to disable animation.
}

void reshape(GLint w, GLint h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, (GLfloat)w/(GLfloat)h, 1.0, 80.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(1.0, 2.0, 2.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 'w':
			rotateX = -1.0;
			glutPostRedisplay();
			break;
		case 's':
			rotateX = 1.0;
			glutPostRedisplay();
			break;
		case 'q':
			rotateY = -1.0;
			glutPostRedisplay();
			break;
		case 'e':
			rotateY = 1.0;
			glutPostRedisplay();
			break;
		case 'a':
			rotateZ = -1.0;
			glutPostRedisplay();
			break;
		case 'd':
			rotateZ = 1.0;
			glutPostRedisplay();
			break;
		//
		// Used in no-animation mode.
		case '/':
			glutPostRedisplay();
			break;
		//
		case 13:
			if(completed == false){
				if(processing == false){
					// Continue processing.
					processing = !processing;
					glutIdleFunc(idle);
				}else{
					// Pause processing.
					processing = !processing;
					glutIdleFunc(NULL);
				}
			}
			break;
		case 27:
			glutDestroyWindow(window);
			exit(0);
	}
}

int main(int argc, char ** argv){
	set_cross_points(radius, slices, stacks);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(windowSize, windowSize);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
