// ECE4122/6122 RSA Encryption/Decryption assignment
// Fall Semester 2015

#include <iostream>
#include "RSA_Algorithm.h"

using namespace std;

int main()
{
  // Instantiate the one and only RSA_Algorithm object
  RSA_Algorithm RSA;
  
  // Loop from sz = 32 to 1024 inclusive in powers of 2
  // For each size choose 100 different key pairs
  // For each key pair choose 10 differnt plaintext messages making sure it is smaller than n.
  // For each message encrypt it using the public key (n,e).
  // After encryption, decrypt the ciphertext using the private key (n,d) and verify it matches the original message.
  
 
  // Loop from sz = 32 to 1024 inclusive in powers of 2
  for (size_t sz = 32; sz <=1024; sz*=2){
    // For each size choose 10 different key pairs
    for (int i = 0; i < 10; i++){
      // generate a key pair
      RSA.GenerateRandomKeyPair(sz);
      RSA.PrintNDE();
      
      // For each key pair choose 10 differnt plaintext messages making sure it is smaller than n.
      for (int j = 0; j < 10; j++){
        // choose a message and make sure it is smaller than n
        mpz_class MSG,ciphertext,decryptedMSG;
        bool validMessage = false;
        while(!validMessage){
          MSG = RSA.rng.get_z_bits(2*sz);
          if(MSG < RSA.n){
	    validMessage = true;
          }
        }  
        
	// print the message
        RSA.PrintM(MSG);
	
        // encrypt the message and print it
	ciphertext = RSA.Encrypt(MSG);
	RSA.PrintC(ciphertext);

	// decrypt the message and print whether it was successful
	// decryptedMSG = RSA.Decrypt(ciphertext);
        
	/*
        if(MSG == decryptedMSG){
	  cout << "Success" << endl;
	}
	else{
	  cout << "FAILED" << endl;
	}
	*/
      }
    }

  }
}
  
