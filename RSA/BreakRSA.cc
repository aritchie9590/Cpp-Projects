// Grad student portion of the RSA assignment
// Fall 2015 ECE6122

#include <iostream>

#include "RSA_Algorithm.h"

using namespace std;

mpz_class factor(mpz_class n)
{
  // use Pollard's rho algorithm for simplicity
  // can update with a quadratic sieve later if time allows
  // info and pseudocode for this algorithm can be found at:
  // https://en.wikipedia.org/wiki/Pollard%27s_rho_algorithm
  mpz_class x, y, yprime, diff, d;
  x = 2;
  y = 2;
  d = 1;

  while(d==1){
    x = (x*x + 1) % n;
    yprime = (y*y + 1) % n;
    y = (yprime*yprime + 1) % n;
    diff = abs(x - y);
    mpz_gcd( d.get_mpz_t() , diff.get_mpz_t() , n.get_mpz_t());
  }

  // instead of implementing the if d == n failure condition we will
  // allow this algorithm to fail silently
  return d;

}

int main(int argc, char** argv)
{ // Arguments are as follows:
  //argv[1] = n;
  //argv[2] = e;  // n and e are the public key
  //argv[3] = first 6 characters of encrypted message
  //argv[4] = next 6 characters .. up to argv[12] are the lsat 6 characters
  // The number of arguments will always be exacty 12, and each argument past the
  // public key contain 6 ascii characters of the encrypted message.
  // Each of the 32bit values in the argv[] array are right justified in the
  // low order 48 bits of each unsigned long.  The upper 16 bits are always
  // zero, which insures the each unsigned long is < n (64 bits) and therefore
  // the RSA encryption will work.

  // Below is an example of the BreakRSA and command line arguments:

// ./BreakRSA  2966772883822367927 2642027824495698257  817537070500556663 1328829247235192134 
// 1451942276855579785 2150743175814047358 72488230455769594 1989174916172335943 962538406513796755 
// 1069665942590443121 72678741742252898 1379869649761557209

//   The corect output from the above is:
//   HelloTest  riley CekwkABRIZFlqmWTanyXLogFgBUENvzwHpEHRCZIKRZ
//
//   The broken (computed) private key for the above is 4105243553

  mpz_class p, q, phi, dtemp, etemp;

  // Our one and only RSA_Algorithm object
  RSA_Algorithm rsa;
  
  // set the values we already know i.e. n and e (the public key pair)
  rsa.n = mpz_class(argv[1]);
  rsa.e = mpz_class(argv[2]);
  etemp = rsa.e;
  
  // factor n to get p
  p = factor(rsa.n);
  
  // calculate q and phi
  q = rsa.n/p;
  phi = (p-1)*(q-1);
  
  // compute d as the multiplicative inverse of e mod phi
  mpz_invert(dtemp.get_mpz_t() , etemp.get_mpz_t() , phi.get_mpz_t());

  // Set rsa.d to the calculated private key d
  rsa.d = dtemp;
 
  for (int i = 3; i < 13; ++i)
    { // Decrypt each set of 6 characters
      mpz_class c(argv[i]);
      mpz_class m = rsa.Decrypt(c);
      //  use the get_ui() method in mpz_class to get the lower 48 bits of the m
      unsigned long ul = m.get_ui();
     // Now print the 6 ascii values in variable ul.  
      // As stated above the 6 characters
      // are in the low order 48 bits of ui.
      for (int j = 0; j < 6; ++j)
        {
          unsigned char ch = ul >> (48 - j * 8 - 8);
          cout << ch;
        }
    }
  cout << endl;
}

