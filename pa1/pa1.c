//--------------------------------------------------------------
// 
//  4190.308 Computer Architecture (Fall 2018)
//
//  Project #1: 64-bit Arithmetic using 32-bit integers
//
//  September 18, 2018
//
//  Jin-Soo Kim (jinsoo.kim@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//--------------------------------------------------------------

#include <stdio.h>
#include "pa1.h"

// NOTE!!!!!
// You should use only 32-bit integer operations inside Uadd64(), Usub64(), 
// and Umul64() functions. 


int isOverflow(u32 a, u32 b)
{
    return ((a + b) < a) ? 1 : 0;
}

// Uadd64() implements the addition of two 64-bit unsigned integers.
// Assume that A and B are the 64-bit unsigned integer represented by
// a and b, respectively. In the following example, x.hi and x.lo should
// have the upper and lower 32 bits of (A + B), respectively.

HL64 Uadd64 (HL64 a, HL64 b)
{
    HL64 x;
    x.lo = a.lo + b.lo;
    x.hi = a.hi + b.hi;
    if (isOverflow(a.lo, b.lo)) {
        x.hi++;
    }
    return x;
}

// Usub64() implements the subtraction between two 64-bit unsigned integers.
// Assume that A and B are the 64-bit unsigned integer represented by
// a and b, respectively. In the following example, x.hi and x.lo should
// have the upper and lower 32 bits of (A - B), respectively.

HL64 Usub64 (HL64 a, HL64 b)
{
    HL64 compB;
    compB.lo = ~b.lo;
    compB.hi = ~b.hi;
    
    HL64 one;
    one.hi = (u32) 0;
    one.lo = (u32) 1;
    
    return Uadd64(a, Uadd64(compB, one));
}

// Usub64() implements the multiplication of two 64-bit unsigned integers.
// Assume that a' and b' are the 64-bit unsigned integer represented by
// a and b, respectively.  In the following example, x.hi and x.lo should
// have the upper and lower 32 bits of (A * B), respectively.

HL64 Umul64 (HL64 a, HL64 b)
{
    u32 ahi1 = a.hi>>16;
    u32 ahi2 = a.hi & 0x0000ffff;
    u32 alo1 = a.lo>>16;
    u32 alo2 = a.lo & 0x0000ffff;
    
    u32 bhi1 = b.hi>>16;
    u32 bhi2 = b.hi & 0x0000ffff;
    u32 blo1 = b.lo>>16;
    u32 blo2 = b.lo & 0x0000ffff;
    
    u32 low1 = alo2 * blo2;
    u32 low2 = (((alo2 * blo1) + (alo1 * blo2)) << 16);
    u32 high1 = (((alo2 * blo1) + (alo1 * blo2)) >> 16);
    u32 high2 = ((ahi2*blo2) + (alo1*blo1) + (alo2*bhi2));
    u32 high3 = (((ahi1*blo2) + (ahi2*blo1) + (alo1*bhi2) + (alo2*bhi1)) << 16);
    
    HL64 x;
    x.lo = low1 + low2;
    x.hi = high1 + high2 + high3;
    if (isOverflow(low1, low2)) {
        x.hi++;
    }
    if (isOverflow(alo2 * blo1, alo1 * blo2)) {
        x.hi += (((u32) 1) << 16);
    }
    return x;
}
