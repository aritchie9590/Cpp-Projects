// Mandelbrot Set GUI

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// threading variables
int nThreads = 16;
pthread_mutex_t startCountMutex;
pthread_mutex_t exitMutex;
pthread_cond_t exitCond;
int startCount;
pthread_barrier_t   barrier;

// Min and max complex plane values
Complex  minC(-2.0, -1.2);
Complex  maxC( 1.0, 1.8);
int      maxIt = 2000;     // Max iterations for the set computations
int      winDim = 512;

// Create vectors to store the history
int currentHistory = 0;
Complex minHistory[100];
Complex maxHistory[100];

Complex *raster;
int *numIters; 
// global variable to store number of iterations for each pixel in the complex plane to converge

double *Rs;
double *Gs;
double *Bs;

int xStart, xCurrent, xEnd, yStart, yCurrent, yEnd; // For mouse operation
bool motionFlag = false;


void display(void)
{ // Your OpenGL display code here
  glLoadIdentity();
  gluOrtho2D(0, winDim, winDim, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_POINTS);
    for (int row = 0; row < winDim; row++)
    {
      for (int col = 0; col < winDim; col++)
      {
          int val = numIters[col + row*winDim];
          if(val >= 2000){
            glColor3f(0.0, 0.0, 0.0);
          }
          else{
            glColor3f(Rs[val], Gs[val], Bs[val]);
          }
          glVertex2f(col, row);
      }
    }
    glEnd();
 
    // draw zoom square if aplicable 
    if (motionFlag){
      glColor3f(1.0, 0.0, 0.0);
      glBegin(GL_LINE_LOOP);
        glVertex2f(xStart,yStart);
        glVertex2f(xStart,yCurrent);
        glVertex2f(xCurrent,yCurrent);
        glVertex2f(xCurrent,yStart);
      glEnd();
    }
  glutSwapBuffers();    
}

void reshape(int w, int h)
{ // Your OpenGL window reshape code here
}

void init()
{ // Your OpenGL initialization code here
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize (winDim,winDim);
  glutCreateWindow("Mandelbrot");
  glMatrixMode(GL_MODELVIEW);
}

int calcMand(Complex c){
  int count = 0;
  Complex z = c;
  for(int i = 1; i <= maxIt; i++){
    if(z.Mag2() >= 4.0) break;
    z = z*z + c;
    count++; 
  }
  return count;
}


void * calcThread(void* v){
  unsigned long myId = (unsigned long)v;
  
  int rowsPerThread = winDim / nThreads;
  int startingRow = myId * rowsPerThread;

  // calculate the raster for the window
  double realdx = (maxC.real - minC.real)/(winDim - 1);
  double imagdx = (maxC.imag - minC.imag)/(winDim - 1);
  for(int row = startingRow; row < (startingRow + rowsPerThread); row++){
    for(int col = 0; col < winDim; col++){
      raster[col + row*winDim] = Complex(minC.real + col*realdx, maxC.imag - row*imagdx); 
      numIters[col + row*winDim] = calcMand(raster[col + row*winDim]);
      //cout << numIters[col + row*winDim] << endl;
    }
  }
  pthread_barrier_wait (&barrier);

  // end stage
  pthread_mutex_lock(&startCountMutex);
  startCount--;
  if (startCount == 0)
    { // Last to exit, notify main
      
      pthread_mutex_unlock(&startCountMutex);
      pthread_mutex_lock(&exitMutex);
      pthread_cond_signal(&exitCond);
      pthread_mutex_unlock(&exitMutex);
    }
  else
    {
      pthread_mutex_unlock(&startCountMutex);
    }

}


void calcRegion(){

  // calculate the raster for the window
  double realdx = (maxC.real - minC.real)/(winDim - 1);
  double imagdx = (maxC.imag - minC.imag)/(winDim - 1);
  for(int row = 0; row < winDim; row++){
    for(int col = 0; col < winDim; col++){
      raster[col + row*winDim] = Complex(minC.real + col*realdx, maxC.imag - row*imagdx); 
      numIters[col + row*winDim] = calcMand(raster[col + row*winDim]);
      //cout << numIters[col + row*winDim] << endl;
    }
  }
}



void mouse(int button, int state, int x, int y)
{ // Your mouse click processing here
  // state == 0 means pressed, state != 0 means released
  // Note that the x and y coordinates passed in are in
  // PIXELS, with y = 0 at the top.
 

  // this corresponds to pressing the mouse button
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    xStart = x;
    yStart = y;
  }
  

  // this corresponds to releasing the mouse button
  if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
  {
    motionFlag = false; // make sure box doesn't get redrawn after zoom

    if( xStart != x && yStart != y){
      xEnd = xCurrent;
      yEnd = yCurrent; 

      Complex start = raster[xStart + yStart*winDim];
      Complex end = raster[xEnd + yEnd*winDim];
      if (start.real < end.real)
      {
        minC.real = start.real;
        maxC.real = end.real;
      }
      else
      {
        minC.real = end.real;
        maxC.real = start.real;
      }

      if (start.imag < end.imag){
        minC.imag = start.imag;
        maxC.imag = end.imag;
      }
      else{
        minC.imag = end.imag;
        maxC.imag = start.imag;
      }

      // update history
      currentHistory++;
      minHistory[currentHistory] = minC;
      maxHistory[currentHistory] = maxC;

      // recalculate display
      //calcRegion();
  // Create 16 threads
  // All mutex and condition variables must be "initialized"
  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&startCountMutex,0);
  pthread_cond_init(&exitCond, 0);
  // Main holds the exit mutex until waiting for exitCond condition
  pthread_mutex_lock(&exitMutex);
  pthread_barrier_init (&barrier, NULL, nThreads);
  startCount = nThreads;
  for (int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt, 0, calcThread, (void*)i);
  }
  pthread_cond_wait(&exitCond, &exitMutex);
    }
  }


}

void motion(int x, int y)
{ // Your mouse motion here, x and y coordinates are as above
    // plot a square to highlight the selected region
    motionFlag = true;
    
    // figure out how to trucate the rectangle to a square by calculating the minimum side distance
    int xdist = x - xStart;
    int ydist = y - yStart;
    int sideLen = min(abs(xdist), abs(ydist));
      if (xdist > 0 & ydist > 0) // Q1
      {
        xCurrent = xStart + sideLen;
	yCurrent = yStart + sideLen;    
      }
      else if (xdist < 0 & ydist > 0) // Q2
      {
        xCurrent = xStart - sideLen;
	yCurrent = yStart + sideLen;    
      
      }
      else if (xdist < 0 & ydist < 0) // Q3
      {
        xCurrent = xStart - sideLen;
	yCurrent = yStart - sideLen;    
      }
      else // Q4
      {
        xCurrent = xStart + sideLen;
	yCurrent = yStart - sideLen;    
      }
    glutPostRedisplay();
}

void keyboard(unsigned char c, int x, int y)
{ // Your keyboard processing here
  if( (c == 'b'|| c == 'B') && currentHistory > 0){
    currentHistory--;
    minC = minHistory[currentHistory];
    maxC = maxHistory[currentHistory];
    //calcRegion();
  // Create 16 threads
  // All mutex and condition variables must be "initialized"
  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&startCountMutex,0);
  pthread_cond_init(&exitCond, 0);
  // Main holds the exit mutex until waiting for exitCond condition
  pthread_mutex_lock(&exitMutex);
  pthread_barrier_init (&barrier, NULL, nThreads);
  startCount = nThreads;
  for (int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt, 0, calcThread, (void*)i);
  }
  pthread_cond_wait(&exitCond, &exitMutex);
  }

}

int main(int argc, char** argv)
{
  // initialize the history   
  minHistory[0] = Complex(minC);
  maxHistory[0] = Complex(maxC);
  
  // allocate memory for global variables
  raster = new Complex[winDim*winDim];
  numIters = new int[winDim*winDim];


  // Create 16 threads
  // All mutex and condition variables must be "initialized"
  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&startCountMutex,0);
  pthread_cond_init(&exitCond, 0);
  // Main holds the exit mutex until waiting for exitCond condition
  pthread_mutex_lock(&exitMutex);
  pthread_barrier_init (&barrier, NULL, nThreads);
  startCount = nThreads;
  for (int i = 0; i < nThreads; ++i)
  {
    pthread_t pt;
    pthread_create(&pt, 0, calcThread, (void*)i);
  }
  pthread_cond_wait(&exitCond, &exitMutex);

  //calcRegion();
  
  // these are arrays of length max iter, they will map every iteration value
  // in the range [0, 2000] to unique RGB values
  Rs = new double[maxIt+1];
  Gs = new double[maxIt+1];
  Bs = new double[maxIt+1];
  
  for(int i = 0; i < maxIt; i++){
    Rs[i] = (double)rand() / (double)RAND_MAX;
    Gs[i] = (double)rand() / (double)RAND_MAX;
    Bs[i] = (double)rand() / (double)RAND_MAX;
  }

  // Initialize OpenGL, but only on the "master" thread or process.
  // See the assignment writeup to determine which is "master" 
  // and which is slave. 
  glutInit(&argc, argv);
  init();
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutMainLoop();
   return 0;
}

