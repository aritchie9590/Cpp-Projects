// Distributed two-dimensional Discrete FFT transform
// YOUR NAME HERE
// ECE8893 Project 1


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;
// make W a const global since it will be used a lot

// done //
void Transform1D(Complex* h, int w, Complex* H)
{
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array.
  
  // more readable notation
  int N = w;
	
  // double for loop DFT algorithm  
  for(int n=0; n<N; n++)
  {
    Complex temp = Complex(0,0); 
    for(int k=0; k<N; k++)
    {
     temp = temp + Complex(cos(2*M_PI*n*k/N), -1*sin(2*M_PI*n*k/N)) * h[k];	
    }
  H[n] = temp;
  }           
}

void I_Transform1D(Complex* H, int w, Complex* h)
{
  // Implement a simple 1-d inverse DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array.
  
  // more readable notation
  int N = w;
	
  // double for loop DFT algorithm  
  for(int n=0; n<N; n++)
  {
    Complex temp = Complex(0,0); 
    for(int k=0; k<N; k++)
    {
     temp = temp + Complex(cos(2*M_PI*n*k/N), sin(2*M_PI*n*k/N)) * H[k];
    }
  h[n] = Complex((1/double(N)), 0)*temp;
  }           
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  // 4) Obtain a pointer to the Complex 1d array of input data
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  // 6) Send the resultant transformed values to the appropriate
  //    other processors for the next phase.
  // 6a) To send and receive columns, you might need a separate
  //     Complex array of the correct size.
  // 7) Receive messages from other processes to collect your columns
  // 8) When all columns received, do the 1D transforms on the columns
  // 9) Send final answers to CPU 0 (unless you are CPU 0)
  //   9a) If you are CPU 0, collect all values from other processors
  //       and print out with SaveImageData().

  
  
  
  //create mpi datatype for complex
  Complex dummy[1] = Complex(1.2, 1.3);
  MPI_Datatype complex;   // required variable
  MPI_Type_contiguous(sizeof(dummy), MPI_CHAR, &complex);
  MPI_Type_commit(&complex);


  
  // Step (1)
  InputImage image(inputFN);  // Create the helper object for reading the image
  int width = image.GetWidth();
  int height = image.GetHeight();  
  
  // Step (2)
  int  numtasks, rank, rc; 
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Status status;
 
  // Step (3)
  Complex* rowTransform = new  Complex [width*height];
  Complex* fullTransform = new  Complex [width*height];
  Complex* fullTransform_T = new  Complex [width*height];
  Complex* I_colTransform_T = new  Complex [width*height];
  Complex* I_colTransform = new  Complex [width*height]; 
  Complex* I_fullTransform = new  Complex [width*height];
  // Step (4)
  Complex* inputPtr = image.GetImageData();

  // Step (5)
  if(rank==0)
  {
    for(int i=0; i<height/numtasks; i++)
    {
      // do the transforms
      Complex H[width];
      Transform1D(inputPtr+(width*i), width, H);
      for(int k=0; k<width; k++)
      {
        // since we are already at rank 0, go ahead and put the results in the row
        // transform array 
        rowTransform[k+(width*i)] = H[k];
      }
    }
    
    
    // check if this is the final rank and if it is do any remaining rows
    // i dont think this is necessary on the rank zero since h/t = h if t=1    
    if (numtasks != 1){
	    // receive from all the various sources and compile the results
	    
	    for (int i=0; i<numtasks-1; i++){
		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		int len; 
		MPI_Get_count(&status, complex, &len);
		Complex buf[len];
		int src = status.MPI_SOURCE;
		//std::fill_n(buf, 2*(height*width/numtasks), Complex(0,0));
	 
		rc = MPI_Recv(buf, len, complex, src,
			0, MPI_COMM_WORLD, &status);
		if (rc != MPI_SUCCESS)
		{
		  cout << "Rank " << rank
		       << " recv failed, rc " << rc << endl;
		  MPI_Finalize();
		  exit(1);
		}
	    
		int count = 0;
		MPI_Get_count(&status, complex, &count);
		int source = status.MPI_SOURCE;
                for(int i=0; i<count; i++)
		{
		  rowTransform[width*source*(height/numtasks) + i] = buf[i];
		}
	      
	      }
    }
    
    // finished sending and receiving row transforms
    cout << "saving 1d image" << endl;
    image.SaveImageData("MyAfter1D.txt", rowTransform, width, height); 
	    

  } 
  else
  {
    // do the computation in the case of any other rank
    if (rank == numtasks-1)
    {
      Complex buf[width*(height - rank*height/numtasks)];
      for(int i=rank*height/numtasks; i<height; i++)
      {
        // do the transforms
        Complex H[width];
        Transform1D(inputPtr+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
    else
    {
      Complex buf[width*(height/numtasks)];
      for(int i=rank*height/numtasks; i<((rank+1)*height/numtasks); i++)
      {
        // do the transforms
        Complex H[width];
        Transform1D(inputPtr+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      
      }
   
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
      if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }    
  }


  //do the column transforms and save the data
  if(rank==0)
  {
    //compute the transpose of the row transform matrix and send to all of the other ranks
    Complex* rowTransform_T = new Complex [width*height];
    int i, j;
    for (j = 0; j < width; j++) 
    {
        for (i = 0; i < height; i++) 
        {
          rowTransform_T[i + (j*height)] = rowTransform[j + i*width]; 
        }
    }
    rc = MPI_Bcast(rowTransform_T, width*height, complex, 0, MPI_COMM_WORLD);	
    //interchange the width and height for all columnwise computations
    int temp = height;
    height = width;
    width = temp;
    
    for(int i=0; i<height/numtasks; i++)
    {
      // do the transforms
      Complex H[width];
      Transform1D(rowTransform_T+(width*i), width, H);
      for(int k=0; k<width; k++)
      {
        // since we are already at rank 0, go ahead and put the results in the row
        // transform array 
        fullTransform_T[k+(width*i)] = H[k];
      }
    }
    
    
    // check if this is the final rank and if it is do any remaining rows
    // i dont think this is necessary on the rank zero since h/t = h if t=1    
    if (numtasks != 1){
    // receive from all the various sources and compile the results
    
    for (int i=0; i<numtasks-1; i++){
        // deleted the 2*
        //
        //
        //
        //
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int len; 
        MPI_Get_count(&status, complex, &len);
        Complex buf[len];
        int src = status.MPI_SOURCE;
        //std::fill_n(buf, 2*(height*width/numtasks), Complex(0,0));
 
        rc = MPI_Recv(buf, len, complex, src,
	       	0, MPI_COMM_WORLD, &status);
        if (rc != MPI_SUCCESS)
        {
          cout << "Rank " << rank
	       << " recv failed, rc " << rc << endl;
          MPI_Finalize();
          exit(1);
        }
    
        int count = 0;
        MPI_Get_count(&status, complex, &count);
        int source = status.MPI_SOURCE;

        for(int i=0; i<count; i++)
        {
          fullTransform_T[width*source*(height/numtasks) + i] = buf[i];
        }
      
      }
    }
    
    //we computed the column wise transform in the transpose space so transpose everything again
    for (j = 0; j < width; j++) 
    {
        for (i = 0; i < height; i++) 
        {
          fullTransform[i + (j*height)] = fullTransform_T[j + i*width]; 
        }
    }
    // switch the width and height again
    temp = width;
    width = height;
    height = temp;
    // finished sending and receiving row transforms
    cout << "saving 2d image" << endl;
    image.SaveImageData("MyAfter2D.txt", fullTransform, width, height); 
  }
  else
  {
      Complex* T1DT = new Complex [height*width];
      // receive the broadcast
      rc = MPI_Bcast(T1DT, width*height, complex, 0, MPI_COMM_WORLD);	
      if (rc != MPI_SUCCESS)
      {
        cout << "Rank " << rank
        << " recv failed, rc " << rc << endl;
        MPI_Finalize();
        exit(1);
      }

    //interchange the width and height for all columnwise computations
    int temp = height;
    height = width;
    width = temp;
    
    // do the computation in the case of any other rank
    if (rank == numtasks-1)
    {
      Complex buf[width*(height - rank*height/numtasks)];
      for(int i=rank*height/numtasks; i<height; i++)
      {
        // do the transforms
        Complex H[width];
        Transform1D(T1DT+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
    else
    {
      Complex buf[width*(height/numtasks)];
      for(int i=rank*height/numtasks; i<((rank+1)*height/numtasks); i++)
      {
        // do the transforms
        Complex H[width];
        Transform1D(T1DT+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
   
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
      if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }    
    temp = height;
    height = width;
    width = height;
  }     


// DO THE INVERSE
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
// Do the inverse on the columns.
  if(rank==0)
  {
    

    //compute the transpose of the full transform matrix and send to all of the other ranks
    rc = MPI_Bcast(fullTransform_T, width*height, complex, 0, MPI_COMM_WORLD);	
    //interchange the width and height for all columnwise computations
    int temp = height;
    height = width;
    width = temp;
    
    for(int i=0; i<height/numtasks; i++)
    {
      // do the transforms
      Complex H[width];
      I_Transform1D(fullTransform_T+(width*i), width, H);
      for(int k=0; k<width; k++)
      {
        // since we are already at rank 0, go ahead and put the results in the row
        // transform array 
        I_colTransform_T[k+(width*i)] = H[k];
      }
    }
    
    
    // check if this is the final rank and if it is do any remaining rows
    // i dont think this is necessary on the rank zero since h/t = h if t=1    
    if (numtasks != 1){
    // receive from all the various sources and compile the results
    
    for (int i=0; i<numtasks-1; i++){
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int len; 
        MPI_Get_count(&status, complex, &len);
        Complex buf[len];
        int src = status.MPI_SOURCE;
 
        rc = MPI_Recv(buf, len, complex, src,
	       	0, MPI_COMM_WORLD, &status);
        if (rc != MPI_SUCCESS)
        {
          cout << "Rank " << rank
	       << " recv failed, rc " << rc << endl;
          MPI_Finalize();
          exit(1);
        }
    
        int count = 0;
        MPI_Get_count(&status, complex, &count);
        int source = status.MPI_SOURCE;

        for(int i=0; i<count; i++)
        {
          I_colTransform_T[width*source*(height/numtasks) + i] = buf[i];
        }
      
      }
    }
    
    //we computed the column wise transform in the transpose space so transpose everything again
    int i, j;
    for (j = 0; j < width; j++) 
    {
        for (i = 0; i < height; i++) 
        {
          I_colTransform[i + (j*height)] = I_colTransform_T[j + i*width]; 
        }
    }
    // switch the width and height again
    temp = width;
    width = height;
    height = temp;
  }
  else
  {
      Complex* T2DT = new Complex [height*width];
      // receive the broadcast
      rc = MPI_Bcast(T2DT, width*height, complex, 0, MPI_COMM_WORLD);	
      
      if (rc != MPI_SUCCESS)
      {
        cout << "Rank " << rank
        << " recv failed, rc " << rc << endl;
        MPI_Finalize();
        exit(1);
      }

    //interchange the width and height for all columnwise computations
    int temp = height;
    height = width;
    width = temp;
    
    // do the computation in the case of any other rank
    if (rank == numtasks-1)
    {
      Complex buf[width*(height - rank*height/numtasks)];
      for(int i=rank*height/numtasks; i<height; i++)
      {
        // do the transforms
        Complex H[width];
        I_Transform1D(T2DT+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
    else
    {
      Complex buf[width*(height/numtasks)];
      for(int i=rank*height/numtasks; i<((rank+1)*height/numtasks); i++)
      {
        // do the transforms
        Complex H[width];
        I_Transform1D(T2DT+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
   
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
      if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
    // change the dimensions back    
    temp = height;
    height = width;
    width = height;
  }     

//
//
//
//
//
//
//
//
//
//
// Do the inverse on the rows.
  if(rank==0)
  {
    

    //compute the transpose of the full transform matrix and send to all of the other ranks
    rc = MPI_Bcast(I_colTransform, width*height, complex, 0, MPI_COMM_WORLD);	
    
    for(int i=0; i<height/numtasks; i++)
    {
      // do the transforms
      Complex H[width];
      I_Transform1D(I_colTransform+(width*i), width, H);
      for(int k=0; k<width; k++)
      {
        // since we are already at rank 0, go ahead and put the results in the row
        // transform array 
        I_fullTransform[k+(width*i)] = H[k];
      }
    }
    
    
    // check if this is the final rank and if it is do any remaining rows
    // i dont think this is necessary on the rank zero since h/t = h if t=1    
    if (numtasks != 1){
    // receive from all the various sources and compile the results
    
    for (int i=0; i<numtasks-1; i++){
        MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int len; 
        MPI_Get_count(&status, complex, &len);
        Complex buf[len];
        int src = status.MPI_SOURCE;
        rc = MPI_Recv(buf, len, complex, src,
	       	0, MPI_COMM_WORLD, &status);
        if (rc != MPI_SUCCESS)
        {
          cout << "Rank " << rank
	       << " recv failed, rc " << rc << endl;
          MPI_Finalize();
          exit(1);
        }
    
        int count = 0;
        MPI_Get_count(&status, complex, &count);
        int source = status.MPI_SOURCE;

        for(int i=0; i<count; i++)
        {
          I_fullTransform[width*source*(height/numtasks) + i] = buf[i];
        }
      
      }
    }
    cout << "saving inverse" << endl; 
    image.SaveImageDataReal("MyAfterInverse.txt", I_fullTransform, width, height); 
  
  }
  else
  {
      Complex* T1D = new Complex [height*width];
      // receive the broadcast
      rc = MPI_Bcast(T1D, width*height, complex, 0, MPI_COMM_WORLD);	
      if (rc != MPI_SUCCESS)
      {
        cout << "Rank " << rank
        << " recv failed, rc " << rc << endl;
        MPI_Finalize();
        exit(1);
      }

    // do the computation in the case of any other rank
    if (rank == numtasks-1)
    {
      Complex buf[width*(height - rank*height/numtasks)];
      for(int i=rank*height/numtasks; i<height; i++)
      {
        // do the transforms
        Complex H[width];
        I_Transform1D(T1D+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
        if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
    else
    {
      Complex buf[width*(height/numtasks)];
      for(int i=rank*height/numtasks; i<((rank+1)*height/numtasks); i++)
      {
        // do the transforms
        Complex H[width];
        I_Transform1D(T1D+(width*i), width, H);
        for(int k=0; k<width; k++)
        {
          // put the results in a buffer to send back 
          buf[k+(width*(i-rank*height/numtasks))] = H[k];
        }
      }
   
      // send the buffer
      rc = MPI_Send(buf, sizeof(buf)/sizeof(dummy), complex, 0,
                        0, MPI_COMM_WORLD);
      if (rc != MPI_SUCCESS)
          {
            cout << "Rank " << rank
                 << " send failed, rc " << rc << endl;
            MPI_Finalize();
            exit(1);
          }
    }
  }     


}





int main(int argc, char** argv)
{
  



  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  
  // MPI initialization here
  int rc; 
  rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS) {
    printf ("Error starting MPI program. Terminating.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
  }
  
  Transform2D(fn.c_str()); // Perform the transform.
  
  // Finalize MPI here
  MPI_Finalize();
}  
  

  
