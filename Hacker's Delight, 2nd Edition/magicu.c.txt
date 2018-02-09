// Computes the magic number for unsigned division.
// Max line length is 57, to fit in hacker.book.

/* In December 2014 Colin Bartlett discovered that magicu gives an
incorrect result for divisor d = 0x80000001. This is the only value for
which it failed. The reason it failed is that when doubling q1 it
overflowed. The fix incorporated here is to check at the beginning of
the loop body if it's going to overflow when doubled, and if so to set a
variable gt = 1. This means that q1 > delta at the end of the loop body
(because q1 is a 33-bit value and hence must be greater than delta). The
program was working in other overflow cases by coincidence, I believe.
The program used to stop looping when p = 64 (if it gets that high), but
that test is no longer necessary. */

#include <stdlib.h>
#include <stdio.h>
// ------------------------------ cut ----------------------------------
struct mu {unsigned M;     // Magic number,
          int a;           // "add" indicator,
          int s;};         // and shift amount.
// ---------------------------- end cut --------------------------------

int main() {
   struct mu magicu(unsigned);
   struct mu magicu2(unsigned);
   struct mu mag;

   mag = magicu(1);
   printf("d = %08x, M = %08x a = %d s = %d\n", 1, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x00000000 + 1 + 0) return 1;

   mag = magicu(3);
   printf("d = %08x, M = %08x a = %d s = %d\n", 3, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0xaaaaaaab + 0 + 1) return 2;

   mag = magicu(7);
   printf("d = %08x, M = %08x a = %d s = %d\n", 7, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x24924925 + 1 + 3) return 3;

   mag = magicu(102807);
   printf("d = %08x, M = %08x a = %d s = %d\n", 102807, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0xa330fe27 + 0 + 16) return  4;

   mag = magicu(0x80000001);
   printf("d = %08x, M = %08x a = %d s = %d\n", 0x80000001, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0xffffffff + 0 + 31) return 5;

   mag = magicu(0xffffffff);
   printf("d = %08x, M = %08x a = %d s = %d\n", 0xffffffff, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x80000001 + 0 + 31) return 6;

   mag = magicu2(1);
   printf("\nd = %08x, M = %08x a = %d s = %d\n", 1, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x00000000 + 1 + 0) return 11;

   mag = magicu2(3);
   printf("d = %08x, M = %08x a = %d s = %d\n", 3, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0xaaaaaaab + 0 + 1) return 12;

   mag = magicu2(7);
   printf("d = %08x, M = %08x a = %d s = %d\n", 7, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x24924925 + 1 + 3) return 13;

   mag = magicu2(102807);
   printf("d = %08x, M = %08x a = %d s = %d\n", 102807, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x4661fc4d + 1 + 17) return 14;

   mag = magicu2(0x80000001);
   printf("d = %08x, M = %08x a = %d s = %d\n", 0x80000001, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0xffffffff + 0 + 31) return 15;

   mag = magicu2(0xffffffff);
   printf("d = %08x, M = %08x a = %d s = %d\n", 0xffffffff, mag.M, mag.a, mag.s);
   if (mag.M + mag.a + mag.s != 0x80000001 + 0 + 31) return 16;

   return 0;
}

// ------------------------------ cut ----------------------------------
struct mu magicu(unsigned d) {
                           // Must have 1 <= d <= 2**32-1.
   int p, gt = 0;
   unsigned nc, delta, q1, r1, q2, r2;
   struct mu magu;

   magu.a = 0;             // Initialize "add" indicator.
   nc = -1 - (-d)%d;       // Unsigned arithmetic here.
   p = 31;                 // Init. p.
   q1 = 0x80000000/nc;     // Init. q1 = 2**p/nc.
   r1 = 0x80000000 - q1*nc;// Init. r1 = rem(2**p, nc).
   q2 = 0x7FFFFFFF/d;      // Init. q2 = (2**p - 1)/d.
   r2 = 0x7FFFFFFF - q2*d; // Init. r2 = rem(2**p - 1, d).
   do {
      p = p + 1;
      if (q1 >= 0x80000000) gt = 1;  // Means q1 > delta.
      if (r1 >= nc - r1) {
         q1 = 2*q1 + 1;            // Update q1.
         r1 = 2*r1 - nc;}          // Update r1.
      else {
         q1 = 2*q1;
         r1 = 2*r1;}
      if (r2 + 1 >= d - r2) {
         if (q2 >= 0x7FFFFFFF) magu.a = 1;
         q2 = 2*q2 + 1;            // Update q2.
         r2 = 2*r2 + 1 - d;}       // Update r2.
      else {
         if (q2 >= 0x80000000) magu.a = 1;
         q2 = 2*q2;
         r2 = 2*r2 + 1;}
      delta = d - 1 - r2;
   } while (gt == 0 &&
           (q1 < delta || (q1 == delta && r1 == 0)));

   magu.M = q2 + 1;        // Magic number
   magu.s = p - 32;        // and shift amount to return
   return magu;            // (magu.a was set above).
}
// ---------------------------- end cut --------------------------------

// ------------------------------ cut ----------------------------------
struct mu magicu2(unsigned d) {
                           // Must have 1 <= d <= 2**32-1.
   int p;
   unsigned p32 = 1, q, r, delta;
   struct mu magu;
   magu.a = 0;             // Initialize "add" indicator.
   p = 31;                 // Initialize p.
   q = 0x7FFFFFFF/d;       // Initialize q = (2**p - 1)/d.
   r = 0x7FFFFFFF - q*d;   // Init. r = rem(2**p - 1, d).
   do {
      p = p + 1;
      if (p == 32) p32 = 1;     // Set p32 = 2**(p-32).
      else p32 = 2*p32;
      if (r + 1 >= d - r) {
         if (q >= 0x7FFFFFFF) magu.a = 1;
         q = 2*q + 1;           // Update q.
         r = 2*r + 1 - d;       // Update r.
      }
      else {
         if (q >= 0x80000000) magu.a = 1;
         q = 2*q;
         r = 2*r + 1;
      }
      delta = d - 1 - r;
   } while (p < 64 && p32 < delta);
   magu.M = q + 1;         // Magic number and
   magu.s = p - 32;        // shift amount to return
   return magu;            // (magu.a was set above).
}
// ---------------------------- end cut --------------------------------
