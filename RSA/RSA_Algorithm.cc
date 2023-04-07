// RSA Assignment for ECE4122/6122 Fall 2015

#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "RSA_Algorithm.h"

using namespace std;

// Implement the RSA_Algorithm methods here

// Constructor
RSA_Algorithm::RSA_Algorithm()
  : rng(gmp_randinit_default)
{
  // get a random seed for the random number generator
  int dr = open("/dev/random", O_RDONLY);
  if (dr < 0)
    {
      cout << "Can't open /dev/random, exiting" << endl;
      exit(0);
    }
  unsigned long drValue;
  read(dr, (char*)&drValue, sizeof(drValue));
  //cout << "drValue " << drValue << endl;
  rng.seed(drValue);
// No need to init n, d, or e.
}

// Fill in the remainder of the RSA_Algorithm methods

// helper function to compute prime numbers
void getPrime(mpz_class& x, size_t sz, gmp_randclass& rng)
{
  bool foundPrime = false;
  while(!foundPrime)
  {
    // set x to a random value of size sz
    x = rng.get_z_bits(sz);
    // run algorithm 100 times to determine if x is 'probably' prime
    if(mpz_probab_prime_p(x.get_mpz_t(),100) != 0) 
    {  
      foundPrime = true;
    }
  }
}


// Generate key
void RSA_Algorithm::GenerateRandomKeyPair(size_t sz)
{
  // generate p, q, n such that n = pq
  mpz_class p, q;
  getPrime(p, sz, rng);
  getPrime(q, sz, rng);
  n = p*q;
  
  // generate phi(n) = (p-1)(q-1)
  mpz_class phi = (p-1)*(q-1);
  
  // compute the pulic / private key pair
  mpz_class gcd;

  // compute public key: d by finding a random int d < phi, of size 2*sz, where GCD(d, phi) = 1
  bool computed = false;
  while(!computed){
    d = rng.get_z_bits(2*sz);
    if(d < phi){
      mpz_gcd(gcd.get_mpz_t(), d.get_mpz_t(), phi.get_mpz_t());
      if ((gcd == 1)){
        computed = true;
      }
    }
  }

  // compute private key: e which is the multiplicative inverse of d mod phi
  mpz_invert(e.get_mpz_t(), d.get_mpz_t(), phi.get_mpz_t());
}



// Encrypt plaintext message M with key  pair n/e
// By convention, we will make the encryption key e the private key
// and the decryption key d the public key.
mpz_class RSA_Algorithm::Encrypt(mpz_class M)
{
  // ciphertext = (message^e) mod n
  mpz_class ciphertext;
  mpz_powm(ciphertext.get_mpz_t(), M.get_mpz_t(), e.get_mpz_t(), n.get_mpz_t());
  return ciphertext;
}

// Decrypt ciphertext message C with key pair n/d
mpz_class RSA_Algorithm::Decrypt(mpz_class C)
{
  // decripted = (ciphertext^d) mod n
  mpz_class decryptedText;
  mpz_powm(decryptedText.get_mpz_t(), C.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());
  return decryptedText;
}


void RSA_Algorithm::PrintND()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << endl;
}

void RSA_Algorithm::PrintNE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " e " << e << endl;
}

void RSA_Algorithm::PrintNDE()
{ // Do not change this, right format for the grading script
  cout << "n " << n << " d " << d << " e " << e << endl;
}

void RSA_Algorithm::PrintM(mpz_class M)
{ // Do not change this, right format for the grading script
  cout << "M " << M << endl;
}

void RSA_Algorithm::PrintC(mpz_class C)
{ // Do not change this, right format for the grading script
  cout << "C " << C << endl;
}




