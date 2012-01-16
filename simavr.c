
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//#define SHOWREGS
//#define SHOWMEM
//#define SHOWROM
#define DISASSEMBLE
//#define BIGPC
//-------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------------------------------------
unsigned int cycles;
unsigned int pc;
unsigned int sp;
unsigned int sreg;
#define CBIT (1<<0)
#define ZBIT (1<<1)
#define NBIT (1<<2)
#define VBIT (1<<3)
#define SBIT (1<<4)
#define HBIT (1<<5)
#define TBIT (1<<6)
#define IBIT (1<<7)
unsigned char reg[32];
unsigned char mem[0x10000];
#ifdef BIGPC
#define ROMMASK 0x3FFFFF
#else
#define ROMMASK 0x00FFFF
#endif
unsigned short rom[ROMMASK+1];
void set_tbit ( void ) { sreg|=TBIT; }
void clr_tbit ( void ) { sreg&=~TBIT; }
void set_sbit ( void ) { sreg|=SBIT; }
void clr_sbit ( void ) { sreg&=~SBIT; }
void set_vbit ( void ) { sreg|=VBIT; }
//void clr_vbit ( void ) { sreg&=~VBIT; }
void set_nbit ( void ) { sreg|=NBIT; }
//void clr_nbit ( void ) { sreg&=~NBIT; }
void set_zbit ( void ) { sreg|=ZBIT; }
//void clr_zbit ( void ) { sreg&=~ZBIT; }
void set_cbit ( void ) { sreg|=CBIT; }

void do_sflag ( void )
{
    if(sreg&NBIT)
    {
        if(sreg&VBIT) clr_sbit();
        else          set_sbit();
    }
    else
    {
        if(sreg&VBIT) set_sbit();
        else          clr_sbit();
    }
}

//-------------------------------------------------------------------
void do_cflag ( unsigned int a, unsigned int b, unsigned int c )
{
    unsigned int rc;

    a&=0xFF;
    b&=0xFF;
    sreg&=~CBIT;
    rc=(a&0x7F)+(b&0x7F)+c; //carry in
    rc = (rc>>7)+(a>>7)+(b>>7);  //carry out
    if(rc&2) sreg|=CBIT;
}
//-------------------------------------------------------------------
void do_hflag ( unsigned int a, unsigned int b, unsigned int c )
{
    unsigned int rc;

    a&=0xF;
    b&=0xF;
    sreg&=~HBIT;
    rc=(a&0x7)+(b&0x7)+c; //carry in
    rc = (rc>>3)+(a>>3)+(b>>3);  //carry out
    if(rc&2) sreg|=HBIT;
}
//-------------------------------------------------------------------
void do_vflag ( unsigned int a, unsigned int b, unsigned int c )
{
    unsigned int rc;
    unsigned int rd;

    a&=0xFF;
    b&=0xFF;
    sreg&=~VBIT;
    rc=(a&0x7F)+(b&0x7F)+c; //carry in
    rc>>=7; //carry in in lsbit
    rd=(rc&1)+((a>>7)&1)+((b>>7)&1); //carry out
    rd>>=1; //carry out in lsbit
    rc=(rc^rd)&1; //if carry in != carry out then signed overflow
    if(rc) sreg|=VBIT;
}

//-------------------------------------------------------------------
void do_cflag16 ( unsigned int a, unsigned int b, unsigned int c )
{
    unsigned int rc;

    a&=0xFFFF;
    b&=0xFFFF;
    sreg&=~CBIT;
    rc=(a&0x7FFF)+(b&0x7FFF)+c; //carry in
    rc = (rc>>15)+(a>>15)+(b>>15);  //carry out
    if(rc&2) sreg|=CBIT;
}
//-------------------------------------------------------------------
void do_vflag16 ( unsigned int a, unsigned int b, unsigned int c )
{
    unsigned int rc;
    unsigned int rd;

    a&=0xFFFF;
    b&=0xFFFF;
    sreg&=~VBIT;
    rc=(a&0x7FFF)+(b&0x7FFF)+c; //carry in
    rc>>=15; //carry in in lsbit
    rd=(rc&1)+((a>>15)&1)+((b>>15)&1); //carry out
    rd>>=1; //carry out in lsbit
    rc=(rc^rd)&1; //if carry in != carry out then signed overflow
    if(rc) sreg|=VBIT;
}//-------------------------------------------------------------------
void write_register ( unsigned char r, unsigned char data )
{
    r&=31;
#ifdef SHOWREGS
    printf("write_register r%u 0x%02X (%u)\n",r,data,data);
#endif
    reg[r]=data;
}
//-------------------------------------------------------------------
unsigned char read_register ( unsigned char r )
{
    unsigned char data;
    r&=31;
    data=reg[r];
#ifdef SHOWREGS
    printf("read_register  r%u 0x%02X (%u)\n",r,data,data);
#endif
    return(data);
}
//-------------------------------------------------------------------
void write_memory ( unsigned short address, unsigned char data )
{
    //address&=0xFFFF;
#ifdef SHOWMEM
    printf("write_memory [0x%04X] 0x%02X (%u)\n",address,data,data);
#endif
    mem[address]=data;
}
//-------------------------------------------------------------------
unsigned char read_memory ( unsigned short address )
{
    unsigned char data;
    //address&=0xFFFF;
    data=mem[address];
#ifdef SHOWMEM
    printf("read_memory  [0x%04X] 0x%02X (%u)\n",address,data,data);
#endif
    return(data);
}
//-------------------------------------------------------------------
unsigned short fetch ( unsigned short address )
{
    unsigned short data;

    address&=ROMMASK;
    data=rom[address];
#ifdef SHOWROM
    printf("fetch [0x%04X] 0x%02X (%u)\n",address,data,data);
#endif
    return(data);
}
//-------------------------------------------------------------------
void reset ( void )
{
    pc=0;
    sp=0xFFFF;
    sreg=0;
    cycles=0;
    memset(reg,0,sizeof(reg));

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------
int run_one ( void )
{
    unsigned int pc_base;
    unsigned int pc_next;
    unsigned int pc_cond;
    unsigned int inst;
    unsigned int inst2;
    unsigned int ra;
    unsigned int rb;
    unsigned int rc;
    unsigned int rd;
    unsigned int rk;
    unsigned int rr;


    pc&=ROMMASK;
    pc_base=pc;
    pc_next=pc+1;
    inst=rom[pc];






    //ADC
    if((inst&0xFC00)==0x1C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    adc r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        if(sreg&CBIT) rk=1; else rk=0;
        rc=(ra+rb+rk)&0xFF;

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,rb,rk);
        do_cflag(ra,rb,rk);
        do_vflag(ra,rb,rk);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //ADD
    if((inst&0xFC00)==0x0C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    add r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra+rb)&0xFF;

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,rb,0);
        do_cflag(ra,rb,0);
        do_vflag(ra,rb,0);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //ADIW
    if((inst&0xFF00)==0x9600)
    {
        rd=24+((inst>>3)&0x6);
        rk=((inst&0x00C0)>>2)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    adiw r%u:r%u,0x%02X ; %u\n",pc_base,inst,rd+1,rd,rk,rk);
#endif
        ra=read_register(rd+1);
        ra<<=1;
        ra|=read_register(rd+0);
        rb=rk;
        rc=(ra+rb)&0xFFFF;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_cflag16(ra,rb,0);
        do_vflag16(ra,rb,0);
        if(rc&0x8000) set_nbit();
        if(rc==0x0000) set_zbit();
        do_sflag();

        write_register(rd+1,(rc>>8)&0xFF);
        write_register(rd+0,(rc>>0)&0xFF);
        pc=pc_next;
        cycles+=2;
        return(0);
    }


    //AND
    if((inst&0xFC00)==0x2000)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    and r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra&rb)&0xFF;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //ANDI
    if((inst&0xF000)==0x7000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    andi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        ra=read_register(rd);
        rc=(ra&rk)&0xFF;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //ASR
    if((inst&0xFE0F)==0x9405)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    asr r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(ra|0x80)|(ra>>1);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        if(ra&1) set_cbit();
        switch(ra&0x81) // VBIT = NBIT xor CBIT
        {
            case 0x00: break;
            case 0x01: set_vbit(); break;
            case 0x80: set_vbit(); break;
            case 0x81: break;
        }
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }


    //BCLR
    if((inst&0xFF8F)==0x9488)
    {
        rb=((inst>>4)&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bclr %u\n",pc_base,inst,rb);
#endif
        sreg&=~(1<<rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BLD
    if((inst&0xFE08)==0xF800)
    {
        rd=((inst>>4)&0x1F);
        rb=(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bld r%u,%u\n",pc_base,inst,rd,rb);
#endif
        ra=read_register(rd);
        if(sreg&TBIT)
        {
            rc=ra|(1<<rb);
        }
        else
        {
            rc=ra&~(1<<rb);
        }
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }


    //BRBC
    if((inst&0xFC00)==0xF400)
    {
        rk=((inst>>3)&0x7F);
        if(rk&0x40) rk|=0xFF80;
        pc_cond=(pc_next+rk)&0xFFFF;
        rb=(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    brbc %u,0x%04X ; ",pc_base,inst,rb,pc_cond);
        switch(rb)
        {
            case  0: printf("CC brcc/brsh"); break;
            case  1: printf("ZC brne"); break;
            case  2: printf("NC brpl"); break;
            case  3: printf("VC brvc"); break;
            case  4: printf("SC brsc/brge"); break;
            case  5: printf("HC brhc"); break;
            case  6: printf("TC brtc"); break;
            default: printf("IC brid"); break;
        }
        printf("\n");
#endif
        if(sreg&(1<<rb))
        {
        }
        else
        {
            pc_next=pc_cond;
            cycles+=1;
        }

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BRBS
    if((inst&0xFC00)==0xF000)
    {
        rk=((inst>>3)&0x7F);
        if(rk&0x40) rk|=0xFF80;
        pc_cond=(pc_next+rk)&0xFFFF;
        rb=(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    brbs %u,0x%04X ; ",pc_base,inst,rb,pc_cond);
        switch(rb)
        {
            case  0: printf("CS brcs/brlo"); break;
            case  1: printf("ZS breq"); break;
            case  2: printf("NS brmi"); break;
            case  3: printf("VS brvs"); break;
            case  4: printf("SS brss/brlt"); break;
            case  5: printf("HS brhs"); break;
            case  6: printf("TS brts"); break;
            default: printf("IS brie"); break;
        }
        printf("\n");

#endif
        if(sreg&(1<<rb))
        {
            pc_next=pc_cond;
            cycles+=1;
        }
        else
        {
        }

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BSET
    if((inst&0xFF8F)==0x9408)
    {
        rb=((inst>>4)&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bset %u\n",pc_base,inst,rb);
#endif
        sreg|=(1<<rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BST
    if((inst&0xFE08)==0xFA00)
    {
        rd=((inst>>4)&0x1F);
        rb=(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bst r%u,%u\n",pc_base,inst,rd,rb);
#endif
        ra=read_register(rd);
        if(ra&(1<<rb))
        {
            set_tbit();
        }
        else
        {
            clr_tbit();
        }

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //CALL
    if((inst&0xFE0E)==0x940E)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc+2;

        rk=((inst>>3)&0x3E);
        rk|=inst&1;
        rk<<=16;
        rk|=inst2;

        //address 0x1234 is on the stack as
        //sp->0x12
        //sp+1->0x34

#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X 0x%04X    call 0x%06X\n",pc_base,inst,inst2,rk);
#endif

        //stack points at first unused
#ifdef BIGPC
        write_memory(sp--,(pc_next>>16)&0xFF);
#endif
        write_memory(sp--,(pc_next>> 8)&0xFF);
        write_memory(sp--,(pc_next>> 0)&0xFF);

        pc=rk;
#ifdef BIGPC
        cycles+=1;
#endif
        cycles+=4;
        return(0);
    }






















    //CPI
    if((inst&0xF000)==0x3000)
    {
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        rd=16+((inst>>4)&0xF);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    cpi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        ra=read_register(rd);
        rc=(ra-rk)&0xFF;

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rk,1);
        do_cflag(ra,~rk,1);
        do_vflag(ra,~rk,1);
        if(rc&0x80) set_nbit();
        do_sflag();
        if(rc==0x00) set_zbit();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //EOR
    if((inst&0xFC00)==0x2400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    eor r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra^rb)&0xFF;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80) set_nbit();
        do_sflag();
        if(rc==0x00) set_zbit();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //INC
    if((inst&0xFE0F)==0x9403)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    inc r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        if(ra==0x7F) set_vbit();
        ra=ra+1;
        ra&=0xFF;
        if(ra&0x80) set_nbit();
        if(ra==0x00) set_zbit();
        do_sflag();

        write_register(rd,ra);
        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //LDI
    if((inst&0xF000)==0xE000)
    {
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        rd=16+((inst>>4)&0xF);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ldi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        write_register(rd,rk);
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //OUT
    if((inst&0xF800)==0xB800)
    {
        rk=((inst&0x0600)>>5)|(inst&0x000F);
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    out 0x%02X,r%u\n",pc_base,inst,rk,rd);
#endif
        ra=read_register(rd);
        write_memory(rk+0x20,ra);
        pc=pc_next;
        cycles+=1;
        return(0);
    }


    //RJMP
    if((inst&0xF000)==0xC000)
    {
        rk=inst&0xFFF;
        if(rk&0x800) rk|=0xF000;
        pc_next=(pc_base+rk+1)&0x7FFF;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    rjmp 0x%04X\n",pc_base,inst,pc_next);
#endif
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SLEEP
    if(inst==0x9588)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sleep\n",pc_base,inst);
#endif
        printf("going to sleep\n");
        cycles+=1;
        pc=pc_next;
        return(1);
    }












    //BREAK
    if(inst==0x9598)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    break\n",pc_base,inst);
#endif
        cycles+=1;
        pc=pc_next;
        return(1);
    }








    printf("0x%04X: 0x%04X 0x%04X    UNDEFINED\n",pc_base,inst,fetch(pc_base+1));
    return(1);
}
//-------------------------------------------------------------------


























unsigned char gstring[80];
unsigned char newline[1024];


//-----------------------------------------------------------------------------
int readhex ( FILE *fp )
{
//unsigned int errors;

unsigned int addhigh;
unsigned int add;

unsigned int ra;
//unsigned int rb;

//unsigned int pages;
//unsigned int page;

unsigned int line;

unsigned char checksum;

unsigned int len;
unsigned int maxadd;

unsigned char t;

    addhigh=0;
    memset(rom,0x0,sizeof(rom));

    line=0;
    while(fgets(newline,sizeof(newline)-1,fp))
    {
        line++;
        //printf("%s",newline);
        if(newline[0]!=':')
        {
            printf("Syntax error <%u> no colon\n",line);
            continue;
        }
        gstring[0]=newline[1];
        gstring[1]=newline[2];
        gstring[2]=0;

        len=strtoul(gstring,NULL,16);
        checksum=0;
        for(ra=0;ra<(len+5);ra++)
        {
            gstring[0]=newline[(ra<<1)+1];
            gstring[1]=newline[(ra<<1)+2];
            gstring[2]=0;
            checksum+=(unsigned char)strtoul(gstring,NULL,16);
        }
        if(checksum)
        {
            printf("checksum error <%u>\n",line);
        }
        gstring[0]=newline[3];
        gstring[1]=newline[4];
        gstring[2]=newline[5];
        gstring[3]=newline[6];
        gstring[4]=0;
        add=strtoul(gstring,NULL,16);
        add|=addhigh;


        if(add>0x80000)
        {
            printf("address too big 0x%04X\n",add);
            return(1);
        }
        if(len&1)
        {
            printf("length odd\n");
            return(1);
        }



        gstring[0]=newline[7];
        gstring[1]=newline[8];
        gstring[2]=0;
        t=(unsigned char)strtoul(gstring,NULL,16);

        //;llaaaattdddddd
        //01234567890

        switch(t)
        {
            default:
                printf("UNKOWN type %02X <%u>\n",t,line);
                break;
            case 0x00:
                len>>=1;
                for(ra=0;ra<len;ra++)
                {

                    if(add>maxadd) maxadd=add;

                    gstring[0]=newline[(ra<<2)+9+2];
                    gstring[1]=newline[(ra<<2)+9+3];
                    gstring[2]=newline[(ra<<2)+9+0];
                    gstring[3]=newline[(ra<<2)+9+1];
                    gstring[4]=0;

                    rom[add>>1]=(unsigned short)strtoul(gstring,NULL,16);
//                    printf("%08X: %02X\n",add,t);
                    add+=2;
                }
                break;
            case 0x01:
                printf("End of data\n");
                break;
            case 0x02:
                gstring[0]=newline[9];
                gstring[1]=newline[10];
                gstring[2]=newline[11];
                gstring[3]=newline[12];
                gstring[4]=0;
                addhigh=strtoul(gstring,NULL,16);
                addhigh<<=16;
                printf("addhigh %08X\n",addhigh);
                break;

        }
    }

    //printf("%u lines processed\n",line);
    //printf("%08X\n",maxadd);

    //if(maxadd&0x7F)
    //{
        //maxadd+=0x80;
        //maxadd&=0xFFFFFF80;
        //printf("%08X\n",maxadd);
    //}

    //for(ra=0;ra<maxadd;ra+=2)
    //{
        //printf("0x%04X: 0x%04X\n",ra,rom[ra>>1]);
    //}

    return(0);


}
//-------------------------------------------------------------------
int main ( int argc, char *argv[] )
{
    FILE *fp;
unsigned int ra;

    if(argc<2)
    {
        printf("hex file not specified\n");
        return(1);
    }
    fp=fopen(argv[1],"rt");
    if(fp==NULL)
    {
        printf("error opening file [%s]\n",argv[1]);
        return(1);
    }
    if(readhex(fp)) return(1);


    reset();

    //for(ra=0;ra<20;ra++)
    while(1)
    {
        if(run_one()) break;
    }






    return(0);
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

