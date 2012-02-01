
//-------------------------------------------------------------------
//-------------------------------------------------------------------

unsigned char subtest ( void );
int run_tea_test ( void );

extern void uart_init ( void );
extern void uart_send ( unsigned char );

void xhexstring ( unsigned int d )
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

void hexstring ( unsigned long d )
{
    //unsigned long ra;
    unsigned int rb;
    unsigned char rc;

    rb=32;
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
void hexstrings ( unsigned long d )
{
    //unsigned long ra;
    unsigned long rb;
    unsigned long rc;

    rb=32;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart_send(rc);
        if(rb==0) break;
    }
    uart_send(0x20);
}
void hexstringcr ( unsigned long d )
{
    //unsigned long ra;
    unsigned long rb;
    unsigned long rc;

    uart_send(0x0D);
    rb=32;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart_send(rc);
        if(rb==0) break;
    }
    uart_send(0x20);
}

int notmain ( void )
{
    subtest();
    //uart_init();
    //xhexstring(0x1234);
    //hexstring(0x12345678);
    //xhexstring(subtest());
    //run_tea_test();
    return(0);
}
