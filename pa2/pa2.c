//--------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2018)
//
//  Project #2: TinyFP (8-bit floating point) Representation
//
//  October 16, 2018
//
//  Jin-Soo Kim (jinsoo.kim@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//--------------------------------------------------------------

#include <stdio.h>
#include "pa2.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
(byte & 0x80 ? '1' : '0'), \
(byte & 0x40 ? '1' : '0'), \
(byte & 0x20 ? '1' : '0'), \
(byte & 0x10 ? '1' : '0'), \
(byte & 0x08 ? '1' : '0'), \
(byte & 0x04 ? '1' : '0'), \
(byte & 0x02 ? '1' : '0'), \
(byte & 0x01 ? '1' : '0')


#define mask_all 0b11111111
#define mask_zero 0b00000000

#define mask_GRS 0b00011100
#define mask_GRS_2 0b00111000

#define mask_MSB 0b10000000
#define mask_E 0b01111000
#define mask_M 0b00000111

#define posNAN 0b01111001
#define negNAN 0b11111001
#define posInf 0b01111000
#define negInf 0b11111000

#define posZero 0b00000000
#define negZero 0b10000000

#define float_mask_E 0b01111111100000000000000000000000
#define float_mask_M 0b00000000011111111111111111111111

#define float_posInf 0b01111111100000000000000000000000
#define float_negInf 0b11111111100000000000000000000000
#define float_posNAN 0b01111111100000000000000000000001
#define float_negNAN 0b11111111100000000000000000000001

#define float_posZero 0b00000000000000000000000000000000
#define float_negZero 0b10000000000000000000000000000000

// range: -240 ~ 240

union bw_float {
    unsigned int i;
    float f;
};

int isNegNAN(tinyfp x){
    if((x&mask_E) == mask_E && (x&mask_M) != mask_zero && (x>>7) == 1){
        return 1;
    } else {
        return 0;
    }
}

int isPosNAN(tinyfp x){
    if((x&mask_E) == mask_E && (x&mask_M) != mask_zero && (x>>7) == 0){
        return 1;
    } else {
        return 0;
    }
}

int float_isNegNAN(union bw_float x){
    if((x.i & float_mask_E) == float_mask_E && (x.i & float_mask_M) != float_posZero && (x.i>>31) == 1){
        return 1;
    } else {
        return 0;
    }
}

int float_isPosNAN(union bw_float x){
    if((x.i & float_mask_E) == float_mask_E && (x.i & float_mask_M) != float_posZero && (x.i>>31) == 0){
        return 1;
    } else {
        return 0;
    }
}



tinyfp int2tinyfp(int x)
{
    if(x==0){
        return 0b00000000; // positive 0
    }
    if(x==0b10000000000000000000000000000000){
        return negInf;
    }
    
    if(x>= 249){ // rounding을 감안하더라도 overflow
        return posInf;
    } else if (x<= -249) {
        return negInf;
    }
    
    
    tinyfp sign = 0;
    
    if(x<0){
        x = -x; // 양수로 만들어준다!
        sign = 1<<7;
    }
    int exp = 7;
    
    tinyfp frac = (x&0b00000000000000000000000011111111); // int value의 하위 8bit
    
    if(frac==0b00000000){ // 1이 하나도 없다면... ex 011110110 ... 00000000 : 이외의 case는 위에서 자동으로 걸러지게 된다
        return sign == 0 ? posInf : negInf;
    }
    
    while((frac&0b10000000) != 0b10000000){ // 이 루프 이후에 10010100의 꼴의, 첫 자리가 반드시 1로 시작하는 비트를 얻게 된다
        frac = frac << 1;
        exp--;
    }
    
    tinyfp S = ((frac&0b00000111) != 0b00000000) ? 1 : 0;         // 1
    
    frac = frac<<1; // 맨 처음의 정수 부분 1은 생략
    frac = frac>>3;
    
    
    tinyfp first = (frac&(1<<4))>>4; // 1
    tinyfp second = (frac&(1<<3))>>3; // 1
    
    tinyfp G = (frac&(1<<2))>>2; // 1
    tinyfp R = (frac&(1<<1))>>1; // 0
    
    if(R==1 && S==1){ // 올려야 함
        frac = frac + 0b00000100; // 반올림
        if(first==1 && second==1 && G==1){ // 10.000으로 바뀐 경우
            frac = frac >> 1;
            exp++;
        }
    } else if (G==1 && R==1 && S==0){ // round to even: G가 1일 경우
        frac = frac + 0b00000100; // 반올림
        if(first==1 && second==1 && G==1){ // 10.000으로 바뀐 경우 -> 0010 0001이 됨 : 0001 0000 으로 변환해준다
            frac = frac >> 1;
            exp++;
        }
    }
    
    frac = frac >> 2;
    if(exp > 7){
        return sign == 0 ? posInf : negInf;
    }
    tinyfp new_exp = (exp+7)<<3;
    return sign|new_exp|frac;
}


int tinyfp2int(tinyfp x)
{
    float a = tinyfp2float(x); // 이 함수가 완벽하게 동작한다는 가정 하에 가능
    return (int) a;
}


tinyfp float2tinyfp(float x)
{
    union bw_float a;
    a.f = x;
    
    tinyfp sign = (a.i&(1<<31))>>24; // 1000 0000 의 형태. 바로 or하면 됨
    int exp = (a.i&float_mask_E)>>23; // 정수 형태로 반환된 exp 값
    exp = exp-127; // 8 비트 용으로 변환. 범위를 넘어설 수 있음에 유의. 이는 후에 처리해 주어야 함.

    if((a.i&float_mask_E) == 0){ //denormalized & zero
        return 0|sign;
    } else if ((a.i&float_mask_E) == float_mask_E) { // Inf or NAN
        if((a.i&float_mask_M) == 0){
            return sign == 0 ? posInf : negInf;
        } else {
            return sign == 0 ? posNAN : negNAN;
        }
    }
    
    if(x>= 249.0){ // rounding을 감안하더라도 overflow
        return posInf;
    } else if (x<= -249.0) {
        return negInf;
    } else if (x<(1/1024.0) && x>-(1/1024.0)){
        return sign|0;
    }
    
    
    unsigned int org_frac = (a.i&float_mask_M);
    
    tinyfp S = ((org_frac&0b00000000000001111111111111111111) != 0) ? 1 : 0;
    
    tinyfp frac = (a.i&float_mask_M)>>18; // 0001 1101 의 형태로 상위 5비트를 얻게 됨 GRS는 순서대로 101
    
    tinyfp first = (frac&(1<<4))>>4; // 1
    tinyfp second = (frac&(1<<3))>>3; // 1
    
    tinyfp G = (frac&(1<<2))>>2; // 1
    tinyfp R = (frac&(1<<1))>>1; // 0
    
    if(R==1 && S==1){ // 올려야 함
        frac = frac + 0b00000100; // 반올림
        if(first==1 && second==1 && G==1){ // 10.000으로 바뀐 경우
            frac = frac >> 1;
            exp++;
        }
    } else if (G==1 && R==1 && S==0){ // round to even
        frac = frac + 0b00000100; // 반올림
        if(first==1 && second==1 && G==1){ // 10.000으로 바뀐 경우 -> 0010 0001이 됨 : 0001 0000 으로 변환해준다
            frac = frac >> 1;
            exp++;
        }
    }

    frac = frac >> 2;
    
    // exp 처리를 해준다. exp의 범위는 -6~7
    if(exp>7){ // 더 클 경우 표현 불가능함
        return sign == 0 ? posInf : negInf;
    }
    if(exp <= -10){
        return sign|0b00000000;
    }
    if(exp <= -7){ // 표현 가능 범위보다 작을 경우. denormalized form으로 변환하여 출력해준다
        frac = (frac >> 1) | (1<<2);
        exp++;
        while(exp <= -7){
            frac = frac >> 1;
            exp++;
        }
        return sign|0|frac;
    }
    
    tinyfp new_exp = (exp+7) << 3;
    return sign|new_exp|frac;
}


float tinyfp2float(tinyfp x)
{
    union bw_float a;
    
    if(x==negZero){
        return -0.0;
    }
    if(x==posZero){
        return +0.0;
    }
    if(x==negInf){
        a.i = float_negInf;
        return a.f;
    }
    if(x==posInf){
        a.i = float_posInf;
        return a.f;
    }
    if(isNegNAN(x)){
        a.i = float_negNAN;
        return a.f;
    }
    if(isPosNAN(x)){
        a.i = float_posNAN;
        return a.f;
    }
    
    unsigned int sign = (x&mask_MSB)>>7;
    sign = sign<<31;
    
    unsigned int exp = (x&mask_E)>>3;
    unsigned int frac = (x&mask_M);
    
    if(exp==0){ //denormalized form. 예외처리 가능
        switch (x&mask_M) {
            case 0b0000001: // 1/512
                return sign == 0? (1.0/512): -(1.0/512);
                break;
            case 0b0000010: // 2/512
                return sign == 0? (2.0/512): -(2.0/512);
                break;
            case 0b0000011: // 3/512
                return sign == 0? (3.0/512): -(3.0/512);
                break;
            case 0b0000100: // 4/512
                return sign == 0? (4.0/512): -(4.0/512);
                break;
            case 0b0000101: // 5/512
                return sign == 0? (5.0/512): -(5.0/512);
                break;
            case 0b0000110: // 6/512
                return sign == 0? (6.0/512): -(6.0/512);
                break;
            case 0b0000111: // 7/512
                return sign == 0? (7.0/512): -(7.0/512);
                break;
            default:
                break;
        }
    } else {
        exp = (exp-7+127)<<23;
    }
    
    frac = frac <<20;
    a.i = sign|exp|frac;
    return a.f;
}


