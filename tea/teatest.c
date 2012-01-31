
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#include "testdata.h"

unsigned long edata[TESTDATALEN];

void hexstring ( unsigned long d ); //printf("%08X\n");
void hexstrings ( unsigned long d ); //printf("%08X ");
void hexstringcr ( unsigned long d ); //printf("%08X\r");

void encrypt (unsigned long* v, unsigned long* k) {
    unsigned long v0=v[0], v1=v[1], sum=0, i;           /* set up */
    unsigned long delta=0x9e3779b9;                     /* a key schedule constant */
    unsigned long k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i < 32; i++) {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

void decrypt (unsigned long* v, unsigned long* k) {
    unsigned long v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up */
    unsigned long delta=0x9e3779b9;                     /* a key schedule constant */
    unsigned long k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}


int run_tea_test ( void )
{
    unsigned long ra;
    unsigned long errors;
    unsigned long data[2];
    unsigned long key[4];

    errors=0;

    key[0]=0x11111111;
    key[1]=0x22222222;
    key[2]=0x33333333;
    key[3]=0x44444444;

    hexstring(0x12345678);

    if((TESTDATALEN&1))
    {
        hexstring(0xBADBAD00);
        return(1);
    }

    for(ra=0;ra<TESTDATALEN;ra+=2)
    {
        data[0]=testdata[ra+0];
        data[1]=testdata[ra+1];
        encrypt(data,key);
        edata[ra+0]=data[0];
        edata[ra+1]=data[1];
    }

    for(ra=0;ra<TESTDATALEN;ra+=2)
    {
        data[0]=edata[ra+0];
        data[1]=edata[ra+1];
        decrypt(data,key);
        if(data[0]!=testdata[ra+0])
        {
            errors++;
            hexstrings(ra); hexstrings(data[0]); hexstring(testdata[ra+0]);
        }
        if(data[1]!=testdata[ra+1])
        {
            errors++;
            hexstrings(ra); hexstrings(data[1]); hexstring(testdata[ra+1]);
        }
        if(errors>20) break;
    }
    hexstring(errors);
    if(errors)
    {
        hexstring(0xBADBAD99);
        return(1);
    }
    hexstring(0x12345678);
    return(0);
}

