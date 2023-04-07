// Render an Icosahedron

#include <iostream>
#include <math.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

#define NFACE 20
#define NVERTEX 12

#define X .525731112119133606 
#define Z .850650808352039932

// These are the 12 vertices for the icosahedron
static GLfloat vdata[NVERTEX][3] = {    
   {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
   {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
   {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

// These are the 20 faces.  Each of the three entries for each 
// vertex gives the 3 vertices that make the face.
static GLint tindices[NFACE][3] = { 
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

int testNumber; // Global variable indicating which test number is desired
int depth; // Global variable indicating depth of subdivision for icosahedron
float* Rs; // R values
float* Gs; // G values
float* Bs; // B values


void Draw(GLfloat* v1, GLfloat* v2, GLfloat* v3, int face)
{
 
  glBegin(GL_TRIANGLES);
    glColor3f(Rs[face], Gs[face], Bs[face]);
    glVertex3fv(v1);
    glVertex3fv(v2);
    glVertex3fv(v3);
  glEnd();

  glBegin(GL_LINE_LOOP);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3fv(v1);
    glVertex3fv(v2);
    glVertex3fv(v3);
  glEnd();

}

void subDivide(GLfloat* v1, GLfloat* v2, GLfloat* v3, int iter, int face)
{
  int faceIncreaseFactor = pow(4,depth);

  GLfloat v12[3];
  GLfloat v13[3];
  GLfloat v23[3];
  
  // calculate midpoints
  for(int i = 0; i < 3; i++)
  {
    v12[i] = (GLfloat) (v1[i] + v2[i])/2.0;
    v13[i] = (GLfloat) (v1[i] + v3[i])/2.0;
    v23[i] = (GLfloat) (v2[i] + v3[i])/2.0;
  }
  // normalize midpoints
  for(int i = 0; i < 3; i++)
  {
    GLfloat mag12 = sqrt(pow(v12[0], 2) + pow(v12[1], 2) + pow(v12[2], 2));
    GLfloat mag13 = sqrt(pow(v13[0], 2) + pow(v13[1], 2) + pow(v13[2], 2));
    GLfloat mag23 = sqrt(pow(v23[0], 2) + pow(v23[1], 2) + pow(v23[2], 2));
    v12[i] = v12[i]/mag12;
    v13[i] = v13[i]/mag13;
    v23[i] = v23[i]/mag23;
  }

  if(iter == 1)
  {
    Draw(v1, v12, v13, faceIncreaseFactor*face);
    Draw(v12, v13, v23,faceIncreaseFactor*face+1);
    Draw(v12, v23, v2, faceIncreaseFactor*face+2);
    Draw(v13, v23, v3, faceIncreaseFactor*face+3);
  }
  else
  {
    // Recursive calls
    subDivide(v1, v12, v13, iter-1, face);
    subDivide(v12, v13, v23, iter-1, face);
    subDivide(v12, v23, v2, iter-1, face);
    subDivide(v13, v23, v3, iter-1, face);
  }
}

// Test cases.  Fill in your code for each test case
void Test1()
{
  for (int i = 0; i < NFACE; i++)
  {
    Draw(&vdata[tindices[i][0]][0], &vdata[tindices[i][1]][0], &vdata[tindices[i][2]][0], i);
  }
}

void Test2()
{
  // draw using test 1
  Test1();

  // rotate for the next iteration
  GLfloat deg = -1.0;
  glRotatef(deg, 1.0, 0.0, 0.0);
  glRotatef(deg, 0.0, 1.0, 0.0);
}

void Test3()
{
  for (int i = 0; i < NFACE; i++) {
    subDivide(&vdata[tindices[i][0]][0], &vdata[tindices[i][1]][0], &vdata[tindices[i][2]][0], 1, i);
  }
}

void Test4()
{
  // draw using test 5
  Test3();

  // rotate for the next iteration
  GLfloat deg = -1.0;
  glRotatef(deg, 1.0, 0.0, 0.0);
  glRotatef(deg, 0.0, 1.0, 0.0);
}

void Test5(int depth)
{
  for (int i = 0; i < NFACE; i++) {
    subDivide(&vdata[tindices[i][0]][0], &vdata[tindices[i][1]][0], &vdata[tindices[i][2]][0], depth, i);
  }
}

void Test6(int depth)
{
  // draw using test 5
  Test5(depth);

  // rotate for the next iteration
  GLfloat deg = -1.0;
  glRotatef(deg, 1.0, 0.0, 0.0);
  glRotatef(deg, 0.0, 1.0, 0.0);
}

// glut callbacks
void displayFunc(void)
{
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(0.5);

  // run the proper test, if user gives improper test number just run Test1()
  if (testNumber == 6) Test6(depth);
  else if (testNumber == 5) Test5(depth);
  else if (testNumber == 4) Test4();
  else if (testNumber == 3) Test3();
  else if (testNumber == 2) Test2();
  else Test1();
  glutSwapBuffers();
}

void timerFunc(int)
{
  // redisplay and set the timer again
  glutPostRedisplay();
  glutTimerFunc(100.0, timerFunc, NULL);
}

int main(int argc, char** argv)
{
  if (argc < 2)
    {
      std::cout << "Usage: icosahedron testnumber" << endl;
      exit(1);
    }
  
  // Set the global test number
  testNumber = atol(argv[1]);
  
  // set depth
  if (testNumber > 4)
  {
    depth = atol(argv[2]);
  }
  else if (testNumber > 2) 
  {
    depth = 1;
  }
  else depth = 0;

  // Initialize glut  and create your window here
  glutInit(&argc, argv);
  glutInitWindowSize(500,500);
  glutCreateWindow("Icos");
  gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  
  // Set your glut callbacks here
  glutDisplayFunc(displayFunc);
  glutTimerFunc(100.0, timerFunc, NULL);
  
  // create arrays to hold RGB values for faces
  int n = NFACE * pow(4, depth);
  Rs = new float[n];
  Gs = new float[n];
  Bs = new float[n];
  // set face colors
  srand (time(NULL));
  for(int i = 0; i < n; i++)
  {
    Rs[i] = (float) rand() / RAND_MAX;  
    Gs[i] = (float) rand() / RAND_MAX;  
    Bs[i] = (float) rand() / RAND_MAX;  
  }
  
  // Enter the glut main loop here
  glutMainLoop(); 

  return 0;
}

