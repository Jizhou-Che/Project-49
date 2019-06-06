#include <iostream>
#include <GLUT/glut.h> // macOS.
// #include <GL/glut.h> // Linux.
#include <cmath>
#include <limits>

// OpenGL global variables.
static int window;

static const int windowSize = 1280; // Size of window.

static const GLdouble displayRadius = 1.0; // Radius of the sphere for display.

static GLfloat rotateX = 0.0; // The pending rotation around x axis.
static GLfloat rotateY = 0.0; // The pending rotation around y axis.
static GLfloat rotateZ = 0.0; // The pending rotation around z axis.

static bool animation = false; // Whether animation is enabled.

// Calculation global variables.
static const double scale = 1.0 * pow(10, 0); // Scaling factor for improving calculation accuracy.
static const double radius = displayRadius * scale; // Radius of sphere.
static const GLint stacks = 20; // Numebr of latitude lines + 1.
static const GLint slices = 20; // Number of longtitude lines.
static const int numOfPoints = 3; // Number of points to place on the sphere.

static double crossPoints[(stacks - 1) * slices + 2][3]; // Set of all points at intersection.
static int currentPointsIndices[numOfPoints]; // The indices of the current placing of points.
static double currentPoints[numOfPoints][3]; // The current placing of points being measured.
static double optimalPoints[numOfPoints][3]; // The placing of points with optimal measurement value.
static double optimalMeasurement; // The measurement value for the optimal placing.

static bool processing = false; // Whether the measurement is processing.
static bool completed = false; // Whether the measurement is completed.

// Calculation of coordinates for points at intersection.
void set_cross_points(GLfloat radius, GLint slices, GLint stacks){
	int flag = 0;
	// Top.
	crossPoints[flag][0] = 0;
	crossPoints[flag][1] = 0;
	crossPoints[flag][2] = radius;
	for(int i = 0; i < stacks - 1; i++){
		// Stacks, from z+ to z-.
		for(int j = 0; j < slices; j++){
			// Slices, starting from x+.
			// Angle between OP and z+.
			double theta = (i + 1) * M_PI / stacks;
			// Angle between the projection of OP on x-y plane and x+.
			double phi = j * 2 * M_PI / slices;
			// Calculation of coordinates.
			flag++;
			crossPoints[flag][0] = sin(theta) * cos(phi) * radius;
			crossPoints[flag][1] = sin(theta) * sin(phi) * radius;
			crossPoints[flag][2] = cos(theta) * radius;
		}
	}
	// Bottom.
	flag++;
	crossPoints[flag][0] = 0;
	crossPoints[flag][1] = 0;
	crossPoints[flag][2] = -radius;
}

/*
// Measure the current placing of points.
// Update the optimal measurement value if applicable.
// Using measurement: maximise the minimum straight line distance between any two points.
// Without optimisation.
static double baseMeasurement = 0; // Initial value of measurement.
void measure(){
	// Calculate the minimum straight line distance between any two points.
	double minDistance = std::numeric_limits<double>::max();
	for(int i = 0; i < numOfPoints - 1; i++){
		for(int j = i + 1; j < numOfPoints; j++){
			double currentDistance = sqrt(pow(currentPoints[i][0] - currentPoints[j][0], 2) + pow(currentPoints[i][1] - currentPoints[j][1], 2) + pow(currentPoints[i][2] - currentPoints[j][2], 2));
			if(currentDistance <= minDistance){
				minDistance = currentDistance;
			}
		}
	}
	// Update the optimal measurement value if applicable.
	if(minDistance > baseMeasurement){
		optimalMeasurement = baseMeasurement = minDistance;
		for(int i = 0; i < numOfPoints; i++){
			optimalPoints[i][0] = currentPoints[i][0];
			optimalPoints[i][1] = currentPoints[i][1];
			optimalPoints[i][2] = currentPoints[i][2];
		}
		std::cout << "Current optimal measurement: " << optimalMeasurement << std::endl;
	}
}
*/

/*
// Measure the current placing of points.
// Update the optimal measurement value if applicable.
// Using measurement: maximise the minimum straight line distance between any two points.
// Optimisation: update the opsition of the second one of the two points that forms the minimum distance.
static double baseMeasurement = 0; // Initial value of measurement.
void measure(){
	// Calculate the minimum straight line distance between any two points.
	double minDistance = std::numeric_limits<double>::max();
	int worstIndex;
	for(int i = 0; i < numOfPoints - 1; i++){
		for(int j = i + 1; j < numOfPoints; j++){
			double currentDistance = sqrt(pow(currentPoints[i][0] - currentPoints[j][0], 2) + pow(currentPoints[i][1] - currentPoints[j][1], 2) + pow(currentPoints[i][2] - currentPoints[j][2], 2));
			if(currentDistance <= minDistance){
				minDistance = currentDistance;
				worstIndex = j;
			}
		}
	}
	// Update the optimal measurement value if applicable.
	if(minDistance > baseMeasurement){
		optimalMeasurement = baseMeasurement = minDistance;
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
*/

//
// Measure the current placing of points.
// Update the optimal measurement value if applicable.
// Using measurement: minimise 2 * arc(R) / arc(d).
// Where arc(R) is the arc radius of the smallest empty circle formed by any three points.
// Where arc(d) is the shortest arc distance between any two points.
// Without optimisation.
// Assume that there are more than two points.
//
double currentCircleCentre[3];
double optimalCircleCentre[3];
//
static double baseMeasurement = std::numeric_limits<double>::max(); // Initial value of measurement.
void measure(){
	// Calculate the arc radius of the smallest empty circle formed by any three points.
	double minRadius = std::numeric_limits<double>::max();
	for(int i = 0; i < numOfPoints - 2; i++){
		for(int j = i + 1; j < numOfPoints - 1; j++){
			for(int k = j + 1; k < numOfPoints; k++){
				// Calculate the ij vector and the ik vector.
				double vij[3] = {currentPoints[j][0] - currentPoints[i][0], currentPoints[j][1] - currentPoints[i][1], currentPoints[j][2] - currentPoints[i][2]};
				double vik[3] = {currentPoints[k][0] - currentPoints[i][0], currentPoints[k][1] - currentPoints[i][1], currentPoints[k][2] - currentPoints[i][2]};
				// Calculate a vector perpendicular to the plane defined by the three points using cross product.
				double vp[3] = {vij[1] * vik[2] - vik[1] * vij[2], vik[0] * vij[2] - vij[0] * vik[2], vij[0] * vik[1] - vik[0] * vij[1]};
				// Calculate a vector in the plane perpendicular to vij.
				double vpij[3] = {vij[1] * vp[2] - vp[1] * vij[2], vp[0] * vij[2] - vij[0] * vp[2], vij[0] * vp[1] - vp[0] * vij[1]};
				// Calculate midpoint of ij.
				double mij[3] = {(currentPoints[j][0] + currentPoints[i][0]) / 2, (currentPoints[j][1] + currentPoints[i][1]) / 2, (currentPoints[j][2] + currentPoints[i][2]) / 2};
				// Calculate a vector in the plane perpendicular to vik.
				double vpik[3] = {vp[1] * vik[2] - vik[1] * vp[2], vik[0] * vp[2] - vp[0] * vik[2], vp[0] * vik[1] - vik[0] * vp[1]};
				// Calculate midpoint of ik.
				double mik[3] = {(currentPoints[k][0] + currentPoints[i][0]) / 2, (currentPoints[k][1] + currentPoints[i][1]) / 2, (currentPoints[k][2] + currentPoints[i][2]) / 2};
				// Define which pair of parameters is used to solve tij.
				bool parameterPair[3] = {true, true, true}; // (x, y), (x, z), (y, z).
				if((vpij[0] == 0 && vpik[0] == 0) || (vpij[1] == 0 && vpik[1] == 0) || (vpij[0] == 0 && vpij[1] == 0) || (vpik[0] == 0 && vpik[1] == 0)){
					parameterPair[0] = false;
				}
				if((mij[0] == mik[0] && mij[1] == mik[1]) || (mij[0] == mik[0] && vpik[0] == 0) || (mij[1] == mik[1] && vpik[1] == 0) || (vpik[0] == 0 && vpik[1] == 0)){
					parameterPair[0] = false;
				}
				if((vpij[0] == 0 && vpik[0] == 0) || (vpij[2] == 0 && vpik[2] == 0) || (vpij[0] == 0 && vpij[2] == 0) || (vpik[0] == 0 && vpik[2] == 0)){
					parameterPair[1] = false;
				}
				if((mij[0] == mik[0] && mij[2] == mik[2]) || (mij[0] == mik[0] && vpik[0] == 0) || (mij[2] == mik[2] && vpik[2] == 0) || (vpik[0] == 0 && vpik[2] == 0)){
					parameterPair[1] = false;
				}
				if((vpij[1] == 0 && vpik[1] == 0) || (vpij[2] == 0 && vpik[2] == 0) || (vpij[1] == 0 && vpij[2] == 0) || (vpik[1] == 0 && vpik[2] == 0)){
					parameterPair[2] = false;
				}
				if((mij[1] == mik[1] && mij[2] == mik[2]) || (mij[1] == mik[1] && vpik[1] == 0) || (mij[2] == mik[2] && vpik[2] == 0) || (vpik[1] == 0 && vpik[2] == 0)){
					parameterPair[2] = false;
				}
				// Calculate the position of circle centre.
				double tij;
				if(parameterPair[0] == true){
					// Solve tij with x and y.
					tij = ((mik[1] - mij[1]) * vpik[0] + (mij[0] - mik[0]) * vpik[1]) / (vpij[1] * vpik[0] - vpij[0] * vpik[1]);
				}else if(parameterPair[1] == true){
					// Solve tij with x and z.
					tij = ((mik[2] - mij[2]) * vpik[0] + (mij[0] - mik[0]) * vpik[2]) / (vpij[2] * vpik[0] - vpij[0] * vpik[2]);
				}else{
					// Solve tij with y and z.
					tij = ((mik[2] - mij[2]) * vpik[1] + (mij[1] - mik[1]) * vpik[2]) / (vpij[2] * vpik[1] - vpij[1] * vpik[2]);
				}
				double circleCentre[3] = {mij[0] + tij * vpij[0], mij[1] + tij * vpij[1], mij[2] + tij * vpij[2]};
				double currentRadius = sqrt(pow(currentPoints[i][0] - circleCentre[0], 2) + pow(currentPoints[i][1] - circleCentre[1], 2) + pow(currentPoints[i][2] - circleCentre[2], 2));
				// Check if the circle is empty.
				bool circleEmpty = true;
				for(int l = 0; l < numOfPoints; l++){
					if(l != i && l != j && l != k){
						// A point other than the three points.
						// Distance between l and circle centre.
						double dlc = sqrt(pow(currentPoints[l][0] - circleCentre[0], 2) + pow(currentPoints[l][1] - circleCentre[1], 2) + pow(currentPoints[l][2] - circleCentre[2], 2));
						if(dlc < currentRadius){
							// l is in the circle.
							circleEmpty = false;
							break;
						}
					}
				}
				// Update the arc radius of the smallest empty circle formed by any three points if applicable.
				if(circleEmpty == true){
					if(currentRadius < minRadius){
						minRadius = currentRadius;
						//
						currentCircleCentre[0] = circleCentre[0];
						currentCircleCentre[1] = circleCentre[1];
						currentCircleCentre[2] = circleCentre[2];
						//
					}
				}
			}
		}
	}
	double minArcRadius = asin(minRadius / radius) * radius;
	// Calculate the shortest arc distance between any two points.
	double minDistance = std::numeric_limits<double>::max();
	for(int i = 0; i < numOfPoints - 1; i++){
		for(int j = i + 1; j < numOfPoints; j++){
			double currentDistance = sqrt(pow(currentPoints[i][0] - currentPoints[j][0], 2) + pow(currentPoints[i][1] - currentPoints[j][1], 2) + pow(currentPoints[i][2] - currentPoints[j][2], 2));
			if(currentDistance < minDistance){
				minDistance = currentDistance;
			}
		}
	}
	double minArcDistance = asin(minDistance / 2 / radius) * radius * 2;
	// Calculate measurement value.
	double measurement = 2 * minArcRadius / minArcDistance;
	// Update the optimal measurement value if applicable.
	if(measurement < baseMeasurement){
		optimalMeasurement = baseMeasurement = measurement;
		for(int i = 0; i < numOfPoints; i++){
			optimalPoints[i][0] = currentPoints[i][0];
			optimalPoints[i][1] = currentPoints[i][1];
			optimalPoints[i][2] = currentPoints[i][2];
		}
		//
		optimalCircleCentre[0] = currentCircleCentre[0];
		optimalCircleCentre[1] = currentCircleCentre[1];
		optimalCircleCentre[2] = currentCircleCentre[2];
		//
		std::cout << "Current optimal measurement: " << optimalMeasurement << std::endl;
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
			std::cout << "Optimal measurement: " << optimalMeasurement << std::endl;
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
	glutWireSphere(displayRadius, slices, stacks);
	// Display the optimal placing of points in green.
	for(int i = 0; i < numOfPoints; i++){
		glPushMatrix();
			glTranslated(GLdouble(optimalPoints[i][0] / scale), GLdouble(optimalPoints[i][1] / scale), GLdouble(optimalPoints[i][2] / scale));
			glColor3f(0, 1, 0);
			glutSolidSphere(0.01, 4, 4);
		glPopMatrix();
	}
	// Display the current placing of points being measured in red.
	for(int i = 0; i < numOfPoints; i++){
		glPushMatrix();
			glTranslated(GLdouble(currentPoints[i][0] / scale), GLdouble(currentPoints[i][1] / scale), GLdouble(currentPoints[i][2] / scale));
			glColor3f(1, 0, 0);
			glutWireSphere(0.01, 4, 4);
		glPopMatrix();
	}
	//
	glPushMatrix();
		glTranslated(GLdouble(currentCircleCentre[0]) / scale, GLdouble(currentCircleCentre[1] / scale), GLdouble(currentCircleCentre[2] / scale));
		glColor3f(0, 0, 1);
		glutWireSphere(0.01, 4, 4);
	glPopMatrix();
	//
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
	if(animation){
		glutPostRedisplay();
	}
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
		case ']':
			// Used in debug mode.
			processing = false;
			glutIdleFunc(NULL);
			place_points();
			glutPostRedisplay();
			break;
		case '/':
			// Used in no-animation mode.
			glutPostRedisplay();
			break;
		case 13:
			if(completed == false){
				if(processing == false){
					// Continue processing.
					processing = true;
					glutIdleFunc(idle);
				}else{
					// Pause processing.
					processing = false;
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
