
//-------------------------------------------------------------------
//-------------------------------------------------------------------

unsigned char subtest ( unsigned char, unsigned char );

int run_primes_test ( void );

extern void uart_init ( void );
extern void uart_send ( unsigned char );

void hexstring16 ( unsigned int d )
{
    //unsigned long ra;
    unsigned int rb;
    unsigned char rc;

    rb=16;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart_send(rc);
        if(rb==0) break;
    }
    uart_send(0x0D);
    uart_send(0x0A);
}

int notmain ( void )
{
    unsigned int ra,rb,rc;

    uart_init();
    hexstring16(0x1234);

    //rc=0;
    //for(ra=0;ra<0x100;ra++)
    //{
        //for(rb=0;rb<0x100;rb++)
        //{
            //rc+=subtest(ra,rb);
        //}
    //}
    //hexstring16(rc);


    run_primes_test();
    return(0);
}
