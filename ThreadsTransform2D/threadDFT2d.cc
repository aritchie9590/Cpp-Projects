// Threaded two-dimensional Discrete FFT transform
// YOUR NAME HERE
// ECE8893 Project 2

#include <sys/time.h>
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>

#include "Complex.h"
#include "InputImage.h"

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.

// create global variables
int nThreads = 16;
Complex* d;
int width;
int height;  
int N;
Complex* W;
int* reversedBits;

//containers to store the various stages of the transform
//
Complex* Trans1d;
Complex* Trans2d;

// The mutex and condition variables allow the main thread to
// know when all helper threads are completed.
pthread_mutex_t startCountMutex;
pthread_mutex_t exitMutex;
pthread_cond_t  exitCond;
int             startCount;

using namespace std;

class GoodBarrier {
public:
void MyBarrier_Init(int P0); // P is the total number of threads
void MyBarrier(int myId); // Enter the barrier, don’t exit till alll there
private:
int P;
int count; // Number of threads presently in the barrier
int FetchAndDecrementCount();
pthread_mutex_t countMutex;
bool* localSense; // We will create an array of bools, one per thread
bool globalSense; // Global sense
};


void GoodBarrier::MyBarrier_Init(int P0)
{
P = P0;
count = P0;
// Initialize the mutex used for FetchAndIncrement
pthread_mutex_init(&countMutex, 0);
// Create and initialize the localSense arrar, 1 entry per thread
localSense = new bool[P];
for (int i = 0; i < P; ++i) localSense[i] = true;
// Initialize global sense
globalSense = true;
}

void GoodBarrier::MyBarrier(int myId)
{ // This works. Why?
localSense[myId] = !localSense[myId]; // Toggle private sense variable
if (FetchAndDecrementCount() == 1)
{ // All threads here, reset count and toggle global sense
count = P;
globalSense = localSense[myId];
}
else
{
while (globalSense != localSense[myId]) { } // Spin
}
}


int GoodBarrier::FetchAndDecrementCount()
{ // We don’t have an atomic FetchAndDecrement, but we can get the
// same behavior by using a mutex
pthread_mutex_lock(&countMutex);
int myCount = count;
count--;
pthread_mutex_unlock(&countMutex);
return myCount;
}


GoodBarrier b1;
GoodBarrier bT1;
GoodBarrier b2;
GoodBarrier bT2;
GoodBarrier bT3;
GoodBarrier bI1;
GoodBarrier bT4;
GoodBarrier bI2;



// function to get elapsed time
int GetMillisecondClock()
{
  timeval tv;
  gettimeofday(&tv, 0);
  static bool first = true;
  static int startSec = 0;
  if (first)
    {
      startSec = tv.tv_sec;
      first = false;
    }
  //Time in milliseconds      
  return (tv.tv_sec - startSec) * 1000 + tv.tv_usec / 1000;
}





// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
int ReverseBits(unsigned v, int N)
{ //  Provided to students
  unsigned n = unsigned(N); // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return int(r);
}

                    





void Transform1D(Complex* h, int N, int* reversedBits, Complex* W)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  
  // reorder the array
  for (int i=0; i<N; i++){
    int j = reversedBits[i];
    if (i < j)
    {
        swap(h[i],h[j]);
    }
  }  

  for (int s = 1; s <= log2 (N); s++){
    // once for each size of transform
    int m = pow(2, s);
    //Complex Wp = Complex(cos(2*M_PI/m), -sin(2*M_PI/m));
    for (int k = 0; k<N; k+=m){
      // once for each transform of a give size
      //Complex W0 = Complex(1, 0);
      for(int j = 0; j<m/2; j++){
        // do each element of the given array
        //Complex temp1 = W0*h[k + j + m/2];
        Complex temp1 = W[j*N/m]*h[k + j + m/2];
        Complex temp2 = h[k+j];
        h[k+j] = temp2 + temp1;
        h[k + j + m/2] = temp2 - temp1;
        //W0 = W0 * Wp;
      } 
    }
  } 
}




void InverseTransform1D(Complex* h, int N, int* reversedBits, Complex* W)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  

  for (int s = log2(N); s >= 1; s--){
    // once for each size of transform
    int m = pow(2, s);
    //Complex Wp = Complex(cos(2*M_PI/m), -sin(2*M_PI/m));
    for (int k = N-m; k>=0; k-=m){
      // once for each transform of a give size
      //Complex W0 = Complex(1, 0);
      for(int j = 0; j<m/2; j++){
        // do each element of the given array
        //Complex temp1 = W0*h[k + j + m/2];
        Complex temp1 = h[k + j + m/2]; //remove w term
        Complex temp2 = h[k+j];
	h[k+j] = Complex(0.5, 0)*(temp2 + temp1);
        h[k + j + m/2] = Complex(0.5, 0)*(temp2 - temp1)/ W[j*N/m];
       // check this algorithm if errors 
      } 
    }
  }

  for (int i=0; i<N; i++){
  // reorder the array
    int j = reversedBits[i];
    if (i < j)
    {
        swap(h[i],h[j]);
    }
  }  
}







void* Transform2DThread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active count and signal main if all complete
  InputImage image("Tower.txt");  
  unsigned long myId = (unsigned long)v; // The parameter is actually the thread number
  myId = int(myId);
  // We can assume evenly divisible here. Would be a bit more complicated
  // if not.
  
  int rowsPerThread = height / nThreads;
  int startingRow = myId * rowsPerThread;
  
  // 1d transform 
  for (int i = 0; i < rowsPerThread; i++){
    Transform1D( ( d + ( (startingRow+ i) * width) ), N, reversedBits, W);
  }
  b1.MyBarrier(myId);

  //transpose with thread 0
  if (myId == 0){
    // save the 1d transformed data before transposing
    image.SaveImageData("MyAfter1D.txt", d, width, height); 
    for (int row=0; row<N; row++){
      for (int col = N - (N-row); col < N; col++){
        swap(d[(row*N) + col], d[(col*N) + row]);
      }
    }
    
  bT1.MyBarrier(myId);
  }
  else{
  bT1.MyBarrier(myId);
  }


  //do the columns
  // 1d transform 
  for (int i = 0; i < rowsPerThread; i++){
    Transform1D( ( d + ( (startingRow+ i) * width) ), N, reversedBits, W);
  }
  b2.MyBarrier(myId);
  
  


  //transpose back with thread 0
  if (myId == 0){
    for (int row=0; row<N; row++){
      for (int col = N - (N-row); col < N; col++){
        swap(d[(row*N) + col], d[(col*N) + row]);
      }
    }
    // Save the 2d transformed data after transposing back
    image.SaveImageData("MyAfter2D.txt", d, width, height); 
    bT2.MyBarrier(myId);
  }
  else{
    bT2.MyBarrier(myId);
  }


  //transpose back with thread 0
  if (myId == 0){
    for (int row=0; row<N; row++){
      for (int col = N - (N-row); col < N; col++){
        swap(d[(row*N) + col], d[(col*N) + row]);
      }
    }
    // Write the transformed datai
    bT3.MyBarrier(myId);
  }
  else{
    bT3.MyBarrier(myId);
  }

  // do inverse of columns 
  for (int i = 0; i < rowsPerThread; i++){
    InverseTransform1D( ( d + ( (startingRow+ i) * width) ), N, reversedBits, W);
  }
  bI1.MyBarrier(myId);

  
  //transpose back with thread 0
  if (myId == 0){
    for (int row=0; row<N; row++){
      for (int col = N - (N-row); col < N; col++){
        swap(d[(row*N) + col], d[(col*N) + row]);
      }
    }
    // Write the transformed datai
    bT4.MyBarrier(myId);
  }
  else{
    bT4.MyBarrier(myId);
  }


  // do inverse of rows and save
  for (int i = 0; i < rowsPerThread; i++){
    InverseTransform1D( ( d + ( (startingRow+ i) * width) ), N, reversedBits, W);
  }
  bI2.MyBarrier(myId);

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


  return 0;
}











void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Create the global pointer to the image array data
  d = image.GetImageData();
  width = image.GetWidth();
  height = image.GetHeight();  
  // create global variables
  N = width;
  W = new Complex[N];
  Trans1d = new Complex(height*width);
  Trans2d = new Complex(height*width);
  
  cout << " init time (seconds) " << GetMillisecondClock() / 1000.0 << endl;
  for (int n = 0; n < N/2; n++){
    W[n] = Complex(cos(2*M_PI*n/N), -sin(2*M_PI*n/N));
    W[n+(N/2)] = Complex(-1,0)*W[n];  
  }
  reversedBits = new int[N];
  for (int b = 0; b<N; b++){
    reversedBits[b] = ReverseBits(unsigned(b), N);
  }  
  cout << " init end time (seconds) " << GetMillisecondClock() / 1000.0 << endl;

  // Create 16 threads
  // All mutex and condition variables must be "initialized"
  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&startCountMutex,0);
  pthread_cond_init(&exitCond, 0);
  // Main holds the exit mutex until waiting for exitCond condition
  pthread_mutex_lock(&exitMutex);
  
  
  //barrier start
  b1.MyBarrier_Init(nThreads);
  bT1.MyBarrier_Init(nThreads);
  b2.MyBarrier_Init(nThreads);
  bT2.MyBarrier_Init(nThreads);
  bT3.MyBarrier_Init(nThreads);
  bI1.MyBarrier_Init(nThreads);
  bT4.MyBarrier_Init(nThreads);
  bI2.MyBarrier_Init(nThreads);

  // Get elapsed milliseconds (starting time after image loaded)
  GetMillisecondClock();
  startCount = nThreads;  // Total threads (to be) started 
  cout << " thread creation time (seconds) " << GetMillisecondClock() / 1000.0 << endl;
  
  for (int i = 0; i < nThreads; ++i)
    {
      // Now create the thread
      pthread_t pt; // pThread variable (output param from create)
      // Third param is the thread starting function
      // Fourth param is passed to the thread starting function
      pthread_create(&pt, 0, Transform2DThread, (void*)i);
    }
  cout << " thread creation end time (seconds) " << GetMillisecondClock() / 1000.0 << endl;
  // Wait for all threads complete
  pthread_cond_wait(&exitCond, &exitMutex);


  //save the inverse
  image.SaveImageDataReal("MyAfterInverse.txt", d, width, height); 
   
  // print how long the transform took 
  cout << "Elapsed time (seconds) " << GetMillisecondClock() / 1000.0 << endl;
}




int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
