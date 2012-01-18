
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//#define SHOWREGS
//#define SHOWMEM
//#define SHOWROM
#define DISASSEMBLE
//#define BIGPC
//-------------------------------------------------------------------
// use data memory based address I/O address + 0x20
#define SPL  (0x3D + 0x20)
#define SPH  (0x3E + 0x20)
#define SREG (0x3F + 0x20)
//-------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------------------------------------
unsigned long cycles;
unsigned long fetch_count;
unsigned long read_count;
unsigned long write_count;
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
void set_nbit ( void ) { sreg|=NBIT; }
void set_zbit ( void ) { sreg|=ZBIT; }
void set_cbit ( void ) { sreg|=CBIT; }
void set_hbit ( void ) { sreg|=HBIT; }
void set_ibit ( void ) { sreg|=IBIT; }
//-------------------------------------------------------------------
void do_sflag ( void )
{
    switch(sreg&(NBIT|VBIT))
    {
        case NBIT:
        case VBIT:
            set_sbit();
            break;
        default:
            clr_sbit();
            break;
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
}
//-------------------------------------------------------------------
int is32bit ( unsigned short inst2 )
{
    if((inst2&0xFE0E)==0x940E) return(1); //CALL
    if((inst2&0xFE0E)==0x940C) return(1); //JMP
    if((inst2&0xFE0F)==0x9000) return(1); //LDS
    if((inst2&0xFE0F)==0x9200) return(1); //STS
    return(0);
}
//-------------------------------------------------------------------
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
    write_count++;
    address&=0xFFFF;
#ifdef SHOWMEM
    printf("write_memory [0x%04X] 0x%02X (%u)\n",address,data,data);
#endif

    if(address<0x20)
    {
        write_register(address,data);
    }
    else if(address<0x60)
    {
        switch(address)
        {
            case SREG:
            {
                sreg=data;
                break;
            }
            case SPH:
            {
                sp&=0x00FF;
                sp|=((unsigned short)data)<<8;
                break;
            }
            case SPL:
            {
                sp&=0xFF00;
                sp|=data;
                break;
            }
            default:
            {
                mem[address]=data;
                break;
            }
        }
    }
    else
    {
        mem[address]=data;
    }
}
//-------------------------------------------------------------------
unsigned char read_memory ( unsigned short address )
{
    unsigned char data;

    read_count++;
    address&=0xFFFF;
    if(address<0x20)
    {
        data=read_register(address);
    }
    else if(address<0x60)
    {
        switch(address)
        {
            case SREG:
            {
                data=sreg;
                break;
            }
            case SPH:
            {
                data=(sp>>8)&0xFF;
                break;
            }
            case SPL:
            {
                data=sp&0xFF;
                break;
            }
            default:
            {
                data=mem[address];
                break;
            }
        }
    }
    else
    {
        data=mem[address];
    }
#ifdef SHOWMEM
    printf("read_memory  [0x%04X] 0x%02X (%u)\n",address,data,data);
#endif
    return(data);
}
//-------------------------------------------------------------------
unsigned short fetch ( unsigned short address )
{
    unsigned short data;

    fetch_count++;
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
    fetch_count=0;
    read_count=0;
    write_count=0;
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
    unsigned int rx;


    pc&=ROMMASK;
    pc_base=pc;
    pc_next=pc+1;
    inst=fetch(pc);


    //ADC pattern 0001 11rd dddd rrrr
    //ROL pattern 0001 11dd dddd dddd
    if((inst&0xFC00)==0x1C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        if(rd==rr)
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    rol r%u\n",pc_base,inst,rd);
#endif
            ra=read_register(rd);
            rc=(ra<<1)&0xFF;
            if(sreg&CBIT) rc|=0x01;
            write_register(rd,rc);

            sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
            if(ra&0x08) set_hbit();
            if(ra&0x80) set_cbit();
            if(rc&0x80) set_nbit();
            if(rc==0x00) set_zbit();
            switch(ra&0xC0)
            {
                case 0x00: break;
                case 0x40: set_vbit(); break;
                case 0x80: set_vbit(); break;
                case 0xC0: break;
            }
            do_sflag();
        }
        else
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    adc r%u,r%u\n",pc_base,inst,rd,rr);
#endif
            ra=read_register(rd);
            rb=read_register(rr);
            if(sreg&CBIT) rk=1; else rk=0;
            rc=(ra+rb+rk)&0xFF;
            write_register(rd,rc);

            sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
            do_hflag(ra,rb,rk);
            do_cflag(ra,rb,rk);
            do_vflag(ra,rb,rk);
            if(rc&0x80) set_nbit();
            if(rc==0x00) set_zbit();
            do_sflag();
        }
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //ADD pattern 0000 11rd dddd rrrr
    //LSL pattern 0000 11dd dddd dddd
    if((inst&0xFC00)==0x0C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        if(rd==rr)
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    lsl r%u\n",pc_base,inst,rd);
#endif
            ra=read_register(rd);
            rc=(ra<<1)&0xFF;
            write_register(rd,rc);

            sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
            if(rd&0x08) set_hbit();
            if(rd&0x80) set_cbit();
            if(rc&0x80) set_nbit();
            if(rc==0x00) set_zbit();
            switch(rd&0xC0)
            {
                case 0x00: break;
                case 0x40: set_vbit(); break;
                case 0x80: set_vbit(); break;
                case 0xC0: break;
            }
            do_sflag();

        }
        else
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    add r%u,r%u\n",pc_base,inst,rd,rr);
#endif
            ra=read_register(rd);
            rb=read_register(rr);
            rc=(ra+rb)&0xFF;
            write_register(rd,rc);

            sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
            do_hflag(ra,rb,0);
            do_cflag(ra,rb,0);
            do_vflag(ra,rb,0);
            if(rc&0x80) set_nbit();
            if(rc==0x00) set_zbit();
            do_sflag();

        }

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //ADIW pattern 1001 0110 kkdd kkkk
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


    //AND pattern 0010 00rd dddd rrrr
    //TST pattern 0010 00dd dddd dddd
    if((inst&0xFC00)==0x2000)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        if(rd==rr)
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    tst r%u,r%u\n",pc_base,inst,rd,rr);
#endif
            ra=read_register(rd);
            //rb=read_register(rr);
            rc=(ra&ra)&0xFF;
            //write_register(rd,rc);

            sreg&=~(SBIT|VBIT|NBIT|ZBIT);
            //clr_vbit();
            if(rc&0x80)
            {
                set_nbit();
                set_sbit();
            }
            if(rc==0x00) set_zbit();
        }
        else
        {
#ifdef DISASSEMBLE
            printf("0x%04X: 0x%04X ......    and r%u,r%u\n",pc_base,inst,rd,rr);
#endif
            ra=read_register(rd);
            rb=read_register(rr);
            rc=(ra&rb)&0xFF;
            write_register(rd,rc);

            sreg&=~(SBIT|VBIT|NBIT|ZBIT);
            //clr_vbit();
            if(rc&0x80)
            {
                set_nbit();
                set_sbit();
            }
            if(rc==0x00) set_zbit();
        }

        pc=pc_next;
        cycles+=1;
        return(0);
    }



    //ANDI pattern 0111 kkkk dddd kkkk
    //CBR pattern 0111 kkkk dddd kkkk
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
        if(rc&0x80)
        {
            set_nbit();
            set_sbit();
        }
        if(rc==0x00) set_zbit();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //ASR pattern 1001 010d dddd 0101
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


    //BCLR pattern 1001 0100 1sss 1000
    //CLC  pattern 1001 0100 1000 1000
    //CLH  pattern 1001 0100 1101 1000
    //CLI  pattern 1001 0100 1111 1000
    //CLN  pattern 1001 0100 1010 1000
    //CLS  pattern 1001 0100 1100 1000
    //CLT  pattern 1001 0100 1110 1000
    //CLV  pattern 1001 0100 1011 1000
    //CLZ  pattern 1001 0100 1001 1000
    if((inst&0xFF8F)==0x9488)
    {
        rb=((inst>>4)&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bclr %u ; ",pc_base,inst,rb);
        switch(rb)
        {
            case  0: printf("clc"); break;
            case  1: printf("clz"); break;
            case  2: printf("cln"); break;
            case  3: printf("clv"); break;
            case  4: printf("cls"); break;
            case  5: printf("clh"); break;
            case  6: printf("clt"); break;
            default: printf("cli"); break;
        }
        printf("\n");
#endif
        sreg&=~(1<<rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BLD pattern 1111 100d dddd 0bbb
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


    //BRBC pattern 1111 01kk kkkk ksss
    //BRCC pattern 1111 01kk kkkk k000
    //BRGE pattern 1111 01kk kkkk k100
    //BRHC pattern 1111 01kk kkkk k101
    //BRID pattern 1111 01kk kkkk k111
    //BRNE pattern 1111 01kk kkkk k001
    //BRPL pattern 1111 01kk kkkk k010
    //BRSH pattern 1111 01kk kkkk k000
    //BRTC pattern 1111 01kk kkkk k110
    //BRVC pattern 1111 01kk kkkk k011
    if((inst&0xFC00)==0xF400)
    {
        rk=((inst>>3)&0x7F);
        if(rk&0x40) rk|=0xFFFF80;
        pc_cond=(pc_base+rk+1)&ROMMASK;
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

    //BRBS pattern 1111 00kk kkkk ksss
    //BRCS pattern 1111 00kk kkkk k000
    //BREQ pattern 1111 00kk kkkk k001
    //BRHC pattern 1111 00kk kkkk k101
    //BRIE pattern 1111 00kk kkkk k111
    //BRLO pattern 1111 00kk kkkk k000
    //BRLT pattern 1111 00kk kkkk k100
    //BRMI pattern 1111 00kk kkkk k010
    //BRTS pattern 1111 00kk kkkk k110
    //BRVS pattern 1111 00kk kkkk k011
    if((inst&0xFC00)==0xF000)
    {
        rk=((inst>>3)&0x7F);
        if(rk&0x40) rk|=0xFFFF80;
        pc_cond=(pc_base+rk+1)&ROMMASK;
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

    //BSET pattern 1001 0100 0sss 1000
    //SEC  pattern 1001 0100 0000 1000
    //SEH  pattern 1001 0100 0101 1000
    //SEI  pattern 1001 0100 0111 1000
    //SEN  pattern 1001 0100 0010 1000
    //SES  pattern 1001 0100 0100 1000
    //SET  pattern 1001 0100 0110 1000
    //SEV  pattern 1001 0100 0011 1000
    //SEZ  pattern 1001 0100 0001 1000
    if((inst&0xFF8F)==0x9408)
    {
        rb=((inst>>4)&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    bset %u ; \n",pc_base,inst,rb);
        switch(rb)
        {
            case  0: printf("sec"); break;
            case  1: printf("sez"); break;
            case  2: printf("sen"); break;
            case  3: printf("sev"); break;
            case  4: printf("ses"); break;
            case  5: printf("seh"); break;
            case  6: printf("set"); break;
            default: printf("sei"); break;
        }
        printf("\n");
#endif
        sreg|=(1<<rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //BST pattern 1111 101d dddd 0bbb
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

    //CALL pattern 1001 0100 0000 1110 kkkk kkkk kkkk kkkk 16 bit pc?
    //CALL pattern 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
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

    //CBI pattern 1001 1000 aaaa abbb
    if((inst&0xFF00)==0x9800)
    {
        ra=((inst>>3)&0x1F);
        rb=inst&0x7;

#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    cbi 0x%02X,%u\n",pc_base,inst,ra,rb);
#endif
        rc=read_memory(0x20+ra);
        rc&=~(1<<rb);
        write_memory(0x20+ra,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //COM  pattern 1001 010d dddd 0000
    if((inst&0xFE0F)==0x9400)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    com r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(~ra)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        //clr_cbit();
        set_cbit();
        if(rc&0x8000) set_nbit();
        if(rc==0x0000) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //CP   pattern 0001 01rd dddd rrrr
    if((inst&0xFC00)==0x1400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    cp r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra-rr)&0xFF;

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rb,1);
        do_cflag(ra,~rb,1);
        do_vflag(ra,~rb,1);
        if(rc&0x80) set_nbit();
        do_sflag();
        if(rc==0x00) set_zbit();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //CPC  pattern 0000 01rd dddd rrrr
    if((inst&0xFC00)==0x0400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    cpc r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        if(sreg&CBIT) rk=1; else rk=0;
        rc=(ra-rr-rk)&0xFF;

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rb,rk);
        do_cflag(ra,~rb,rk);
        do_vflag(ra,~rb,rk);
        if(rc&0x80) set_nbit();
        do_sflag();
        if(rc==0x00) set_zbit();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //CPI  pattern 0011 kkkk dddd kkkk
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

    //CPSE pattern 0001 00rd dddd rrrr
    if((inst&0xFC00)==0x1000)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        pc_cond=pc_base+2;
        inst2=read_memory(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    cpse r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);

        if(ra==rb)
        {
            pc=pc_cond;
            cycles+=1;
            if(is32bit(inst2)) cycles+=1;
        }
        else
        {
            pc=pc_next;
        }
        cycles+=1;
        return(0);
    }


    //DEC  pattern 1001 010d dddd 1010
    if((inst&0xFE0F)==0x940A)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    dec r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(ra-1)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        if(ra==0x80) set_vbit();
        if(ra&0x80) set_nbit();
        if(ra==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //DES  pattern 1001 0100 kkkk 1011
    if((inst&0xFF0F)==0x940B)
    {
        printf("DES NOT SUPPORTED\n");
        return(1);
    }

    //EICALL pattern 1001 0101 0001 1001
    if(inst==0x9519)
    {
        //looks like BIGPC only 22 bit pc
        printf("EICALL NOT SUPPORTED (todo)\n");
        return(1);
    }

    //EIJMP pattern 1001 0100 0001 1001
    if(inst==0x9419)
    {
        //looks like BIGPC only 22 bit pc
        printf("EIJMP NOT SUPPORTED (todo)\n");
        return(1);
    }

    //ELPM pattern 1001 0101 1101 1000
    //ELPM pattern 1001 000d dddd 0110
    //ELPM pattern 1001 000d dddd 0111
    if(inst==0x95D8)
    {
        //looks like xmega RAMP
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9006)
    {
        //looks like xmega RAMP
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9007)
    {
        //looks like xmega RAMP
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }

    //EOR  pattern 0010 01rd dddd rrrr
    //CLR  pattern 0010 01dd dddd dddd
    if((inst&0xFC00)==0x2400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        if(rd==rr)
        {
            printf("0x%04X: 0x%04X ......    clr r%u\n",pc_base,inst,rd);
        }
        else
        {
            printf("0x%04X: 0x%04X ......    eor r%u,r%u\n",pc_base,inst,rd,rr);
        }
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra^rb)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80) set_nbit();
        do_sflag();
        if(rc==0x00) set_zbit();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //FMUL pattern 0000 0011 0ddd 1rrr
    if((inst&0xFF88)==0x0308)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    fmul r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        //not supported right now, what chips use this?
        return(1);
    }

    //FMULS pattern 0000 0011 1ddd 0rrr
    if((inst&0xFF88)==0x0380)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    fmuls r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        //not supported right now, what chips use this?
        return(1);
    }

    //FMULSU pattern 0000 0011 1ddd 1rrr
    if((inst&0xFF88)==0x0388)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    fmulsu r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        //not supported right now, what chips use this?
        return(1);
    }

    //ICALL pattern 1001 0101 0000 1001
    if(inst==0x9509)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    icall\n",pc_base,inst);
#endif
        ra=read_register(30);
        rb=read_register(31);
        pc_cond=(rb<<8)|ra;

        //stack points at first unused
#ifdef BIGPC
        //upper bits zero
        write_memory(sp--,(pc_next>>16)&0xFF);
#endif
        write_memory(sp--,(pc_next>> 8)&0xFF);
        write_memory(sp--,(pc_next>> 0)&0xFF);

        pc=pc_cond;
#ifdef BIGPC
        cycles+=1;
#endif
        cycles+=3;
        return(0);
    }

    //IJMP pattern 1001 0100 0000 1001
    if(inst==0x9409)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ijmp\n",pc_base,inst);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;

        pc=rk;
        cycles+=2;
        return(0);
    }

    //IN pattern 1011 0aad dddd aaaa
    if((inst&0xF800)==0xB000)
    {
        rk=((inst&0x0600)>>5)|(inst&0x000F);
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    in 0x%02X,r%u\n",pc_base,inst,rk,rd);
#endif
        rb=read_memory(0x20+rk);
        write_register(rd,rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //INC pattern 1001 010d dddd 0011
    if((inst&0xFE0F)==0x9403)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    inc r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(ra+1)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        if(ra==0x7F) set_vbit();
        if(ra&0x80) set_nbit();
        if(ra==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //JMP pattern 1001 010k kkkk 110k kkkk kkkk kkkk kkkk
    if((inst&0xFE0E)==0x940C)
    {
        inst2=fetch(pc_base+1);

        rk=((inst>>3)&0x3E);
        rk|=inst&1;
        rk<<=16;
        rk|=inst2;

#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X 0x%04X    jmp 0x%06X\n",pc_base,inst,inst2,rk);
#endif

        pc=rk;
        cycles+=3;
        return(0);
    }


    //LAC  pattern 1001 001r rrrr 0110
    if((inst&0xFE0F)==0x9206)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lac z,r%u\n",pc_base,inst,rd);
#endif
        printf("lac not implemented\n");
        return(1);
    }

    //LAS  pattern 1001 001r rrrr 0101
    if((inst&0xFE0F)==0x9205)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    las z,r%u\n",pc_base,inst,rd);
#endif
        printf("las not implemented\n");
        return(1);
    }

    //LAT  pattern 1001 001r rrrr 0111
    if((inst&0xFE0F)==0x9207)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lat z,r%u\n",pc_base,inst,rd);
#endif
        printf("lat not implemented\n");
        return(1);
    }

    //LDx  pattern 1001 000d dddd 1100
    //LDx  pattern 1001 000d dddd 1101
    //LDx  pattern 1001 000d dddd 1110
    if((inst&0xFE0F)==0x900C)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,x\n",pc_base,inst,rd);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }
    if((inst&0xFE0F)==0x900D)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,x+\n",pc_base,inst,rd);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);
        rk=rk+1;
        write_register(26,(rk>>0)&0xFF);
        write_register(27,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x900E)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,-x\n",pc_base,inst,rd);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_memory(rk);
        write_register(rd,rc);
        write_register(26,(rk>>0)&0xFF);
        write_register(27,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=3;
        return(0);
    }


    //LDy  pattern 1000 000d dddd 1000
    //LDy  pattern 1001 000d dddd 1001
    //LDy  pattern 1001 000d dddd 1010
    //LDy  pattern 10q0 qq0d dddd 1qqq
    if((inst&0xFE0F)==0x8008)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,y\n",pc_base,inst,rd);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }
    if((inst&0xFE0F)==0x9009)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,y+\n",pc_base,inst,rd);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);
        rk=rk+1;
        write_register(28,(rk>>0)&0xFF);
        write_register(29,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x900A)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,-y\n",pc_base,inst,rd);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_memory(rk);
        write_register(rd,rc);
        write_register(28,(rk>>0)&0xFF);
        write_register(29,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=3;
        return(0);
    }

    //LDz  pattern 1000 000d dddd 0000
    //LDz  pattern 1001 000d dddd 0001
    //LDz  pattern 1001 000d dddd 0010
    //LDz  pattern 10q0 qq0d dddd 0qqq
    if((inst&0xFE0F)==0x8000)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,z\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }
    if((inst&0xFE0F)==0x9001)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,z+\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);
        rk=rk+1;
        write_register(30,(rk>>0)&0xFF);
        write_register(31,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x9002)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ld r%u,-z\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_memory(rk);
        write_register(rd,rc);
        write_register(30,(rk>>0)&0xFF);
        write_register(31,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=3;
        return(0);
    }

    //LDI  pattern 1110 kkkk dddd kkkk
    //SER  pattern 1110 1111 dddd 1111
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

    //LDS  pattern 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9000)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc+2;
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lds r%u,0x%04X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //LDS  pattern 1010 0kkk dddd kkkk
    if((inst&0xF800)==0xA000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0700)>>4)|(inst&0xF);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lds r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //LPM  pattern 1001 0101 1100 1000
    //LPM  pattern 1001 000d dddd 0100
    //LPM  pattern 1001 000d dddd 0101
    if(inst==0x95C8)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lpm\n",pc_base,inst);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(0,rc);

        pc=pc_next;
        cycles+=3;
        return(0);
    }
    if((inst&0xFE0F)==0x9004)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lpm r%u,z\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=3;
        return(0);
    }
    if((inst&0xFE0F)==0x9005)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lpm r%u,z+\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_memory(rk);
        write_register(rd,rc);
        rk=rk+1;
        write_register(30,(rk>>0)&0xFF);
        write_register(31,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=3;
        return(0);
    }

    //LSR  pattern 1001 010d dddd 0110
    if((inst&0xFE0F)==0x9406)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    lsr r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(ra>>1)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        if(ra&0x01)
        {
            set_cbit();
            set_vbit();
        }
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //MOV  pattern 0010 11rd dddd rrrr
    if((inst&0xFC00)==0x2C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    mov r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        rc=read_register(rr);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //MOVW pattern 0000 0001 dddd rrrr
    if((inst&0xFF00)==0x0100)
    {
        rd=((inst>>3)&0x1E);
        rr=((inst<<1)&0x1E);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    movw r%u:r%u,r%u:r%u\n",pc_base,inst,rd+1,rd,rr+1,rr);
#endif
        ra=read_register(rr+0);
        rb=read_register(rr+1);
        write_register(rd+0,ra);
        write_register(rd+1,rb);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //MUL  pattern 1001 11rd dddd rrrr
    if((inst&0xFC00)==0x9C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    mul r%u,r%u\n",pc_base,inst,rd,rr);
#endif

        printf("mul not implemented\n");

        pc=pc_next;
        cycles+=2;
        return(1);
    }

    //MULS pattern 0000 0010 dddd rrrr
    if((inst&0xFF00)==0x0200)
    {
        rd=16+((inst>>4)&0xF);
        rr=16+(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    muls r%u,r%u\n",pc_base,inst,rd,rr);
#endif

        printf("muls not implemented\n");

        pc=pc_next;
        cycles+=2;
        return(1);
    }

    //MULSU pattern 0000 0011 0ddd 0rrr
    if((inst&0xFF88)==0x0300)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x0007);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    mulsu r%u,r%u\n",pc_base,inst,rd,rr);
#endif

        printf("mulsu not implemented\n");

        pc=pc_next;
        cycles+=2;
        return(1);
    }

    //NEG  pattern 1001 010d dddd 0001
    if((inst&0xFE0F)==0x9401)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    neg r%u\n",pc_base,inst,rd);
#endif
        rb=read_register(rd);
        rc=(0-rb)&0xFF;
        write_register(rd,rc);

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(0,~rb,1);
        do_cflag(0,~rb,1);
        do_vflag(0,~rb,1);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //NOP  pattern 0000 0000 0000 0000
    if(inst==0x0000)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    nop r%u\n",pc_base,inst,rd);
#endif

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //OR  pattern 0010 10rd dddd rrrr
    if((inst&0xFC00)==0x2800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    or r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra|rb)&0xFF;
        write_register(rd,rc);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80)
        {
            set_nbit();
            set_sbit();
        }
        if(rc==0x00) set_zbit();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //ORI  pattern 0110 kkkk dddd kkkk
    //SBR  pattern 0110 kkkk dddd kkkk
    if((inst&0xF000)==0x6000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ori r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        ra=read_register(rd);
        rc=(ra|rk)&0xFF;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT);
        //clr_vbit();
        if(rc&0x80)
        {
            set_nbit();
            set_sbit();
        }
        if(rc==0x00) set_zbit();

        write_register(rd,rc);
        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //OUT  pattern 1011 1aar rrrr aaaa
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

    //POP  pattern 1001 000d dddd 1111
    if((inst&0xFE0F)==0x900F)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    pop r%u\n",pc_base,inst,rd);
#endif
        rc=read_memory(++sp);
        write_register(rd,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }


    //PUSH pattern 1001 001d dddd 1111
    if((inst&0xFE0F)==0x920F)
    {
        rd=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    push r%u\n",pc_base,inst,rd);
#endif
        rc=read_register(rd);
        write_memory(sp--,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //RCALL pattern 1101 kkkk kkkk kkkk
    if((inst&0xF000)==0xD000)
    {
        rk=inst&0x0FFF;
        if(rk&0x800) rk|=0xFFF000;
        pc_cond=(pc_base+rk+1)&ROMMASK;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    rcall 0x%04X\n",pc_base,inst,pc_cond);
#endif

        //stack points at first unused
#ifdef BIGPC
        write_memory(sp--,(pc_next>>16)&0xFF);
#endif
        write_memory(sp--,(pc_next>> 8)&0xFF);
        write_memory(sp--,(pc_next>> 0)&0xFF);

        pc=pc_cond;
#ifdef BIGPC
        cycles+=1;
#endif
        cycles+=3;
        return(0);
    }

    //RET  pattern 1001 0101 0000 1000
    if(inst==0x9508)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ret\n",pc_base,inst);
#endif

        //stack points at first unused
        pc_next=0;
#ifdef BIGPC
        pc_next<<=8; pc_next|=read_memory(++sp);
#endif
        pc_next<<=8; pc_next|=read_memory(++sp);
        pc_next<<=8; pc_next|=read_memory(++sp);

        pc=pc_next;
#ifdef BIGPC
        cycles+=1;
#endif
        cycles+=4;
        return(0);
    }

    //RETI pattern 1001 0101 0001 1000
    if(inst==0x9518)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    reti\n",pc_base,inst);
#endif

        //stack points at first unused
        pc_next=0;
#ifdef BIGPC
        pc_next<<=8; pc_next|=read_memory(++sp);
#endif
        pc_next<<=8; pc_next|=read_memory(++sp);
        pc_next<<=8; pc_next|=read_memory(++sp);
        set_ibit();

        pc=pc_next;
#ifdef BIGPC
        cycles+=1;
#endif
        cycles+=4;
        return(0);
    }

    //RJMP 1100 kkkk kkkk kkkk
    if((inst&0xF000)==0xC000)
    {
        rk=inst&0xFFF;
        if(rk&0x800) rk|=0xFFF000;
        pc_next=(pc_base+rk+1)&ROMMASK;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    rjmp 0x%04X\n",pc_base,inst,pc_next);
#endif

        pc=pc_next;
        cycles+=2;
        return(0);
    }



    //ROR pattern 1001 010d dddd 0111
    if((inst&0xFE0F)==0x9407)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    ror r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=(ra>>1);
        if(sreg&CBIT) rc|=0x80;

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        if(ra&0x01) set_cbit();
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        switch(sreg&(NBIT|CBIT))
        {
            case NBIT:
            case CBIT:
                set_vbit();
                break;
            default:
                break;
        }
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SBC  pattern 0000 10rd dddd rrrr
    if((inst&0xFC00)==0x0800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbc r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        if(sreg&CBIT) { rk=1; rx=0; } else { rk=0; rx=1; }
        rc=(ra-rb-rk)&0xFF;
        write_register(rd,rc);

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rb,rx);
        do_cflag(ra,~rb,rx);
        do_vflag(ra,~rb,rx);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SBCI pattern 0100 kkkk dddd kkkk
    if((inst&0xF000)==0x4000)
    {
        rd=16+((inst>>4)&0xF);
        rb=((inst&0x0F00)>>4)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbci r%u,0x%02X ; %u\n",pc_base,inst,rd,rb,rb);
#endif
        ra=read_register(rd);
        if(sreg&CBIT) { rk=1; rx=0; } else { rk=0; rx=1; }
        rc=(ra-rb-rk)&0xFF;
        write_register(rd,rc);

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rb,rx);
        do_cflag(ra,~rb,rx);
        do_vflag(ra,~rb,rx);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SBI  pattern 1001 1010 aaaa abbb
    if((inst&0xFF00)==0x9A00)
    {
        rb=inst&7;
        ra=(inst>>3)&0x001F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbi 0x02X,%u\n",pc_base,inst,ra,rb);
#endif
        rc=read_memory(0x20+ra);
        rc|=(1<<rb);
        write_memory(0x20+ra,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }


    //SBIC pattern 1001 1001 aaaa abbb
    if((inst&0xFF00)==0x9900)
    {
        ra=((inst>>3)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=read_memory(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbic 0x%02X,%u\n",pc_base,inst,ra,rb);
#endif
        rc=read_memory(0x20+ra);
        if((rc&(1<<rb))==0)
        {
            pc=pc_cond;
            cycles+=1;
            if(is32bit(inst2)) cycles+=1;
        }
        else
        {
            pc=pc_next;
        }
        cycles+=1;
        return(0);
    }

    //SBIS pattern 1001 1011 aaaa abbb
    if((inst&0xFF00)==0x9B00)
    {
        ra=((inst>>3)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=read_memory(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbis 0x%02X,%u\n",pc_base,inst,ra,rb);
#endif
        rc=read_memory(0x20+ra);
        if(rc&(1<<rb))
        {
            pc=pc_cond;
            cycles+=1;
            if(is32bit(inst2)) cycles+=1;
        }
        else
        {
            pc=pc_next;
        }
        cycles+=1;
        return(0);
    }

    //SBIW pattern 1001 0111 kkdd kkkk
    if((inst&0xFF00)==0x9700)
    {
        rd=24+((inst>>3)&0x6);
        rk=((inst&0x00C0)>>2)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbiw r%u:r%u,0x%02X ; %u\n",pc_base,inst,rd+1,rd+0,rk,rk);
#endif
        ra=read_register(rd+0);
        rb=read_register(rd+1);
        ra|=rb<<8;
        rc=(ra-rk)&0xFFFF;
        write_register(rd+0,(rc>>0)&0xFF);
        write_register(rd+1,(rc>>8)&0xFF);

        sreg&=~(SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_cflag16(ra,~rk,1);
        do_vflag16(ra,~rk,1);
        if(rc&0x8000) set_nbit();
        if(rc==0x0000) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=2;
        return(0);
    }


    //SBRC pattern 1111 110r rrrr 0bbb
    if((inst&0xFE08)==0xFC00)
    {
        rr=((inst>>4)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=read_memory(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbrc r%u,%u\n",pc_base,inst,rr,rb);
#endif
        rc=read_register(rr);
        if((rr&(1<<rb))==0)
        {
            pc=pc_cond;
            cycles+=1;
            //another cycle if next one is two words. todo
        }
        else
        {
            pc=pc_next;
        }
        cycles+=1;
        return(0);
    }


    //SBRS pattern 1111 111r rrrr 0bbb
    if((inst&0xFE08)==0xFE00)
    {
        rr=((inst>>4)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=read_memory(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sbrs r%u,%u\n",pc_base,inst,rr,rb);
#endif
        rc=read_register(rr);
        if(rr&(1<<rb))
        {
            pc=pc_cond;
            cycles+=1;
            //another cycle if next one is two words. todo
        }
        else
        {
            pc=pc_next;
        }
        cycles+=1;
        return(0);
    }

    //SLEEP pattern 1001 0101 1000 1000
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

    //SPM  pattern 1001 0101 1110 1000
    //SPM  pattern 1001 0101 1111 1000
    if(inst==0x95E8)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    spm\n",pc_base,inst);
#endif
        printf("spm not implemented\n");
        return(1);
    }
    if(inst==0x95F8)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    spm\n",pc_base,inst);
#endif
        printf("spm not implemented\n");
        return(1);
    }

    //STx  pattern 1001 001r rrrr 1100
    //STx  pattern 1001 001r rrrr 1101
    //STx  pattern 1001 001r rrrr 1110
    if((inst&0xFE0F)==0x920C)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st x,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x920D)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st x+,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);
        rk=rk+1;
        write_register(26,(rk>>0)&0xFF);
        write_register(27,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x920E)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st -x,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(26);
        rb=read_register(27);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_register(rr);
        write_memory(rk,rc);
        write_register(26,(rk>>0)&0xFF);
        write_register(27,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //STy  pattern 1000 001r rrrr 1000
    //STy  pattern 1001 001r rrrr 1001
    //STy  pattern 1001 001r rrrr 1010
    //STy  pattern 10q0 qq1r rrrr 1qqq
    if((inst&0xFE0F)==0x8208)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st y,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }
    if((inst&0xFE0F)==0x9209)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st y+,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);
        rk=rk+1;
        write_register(28,(rk>>0)&0xFF);
        write_register(29,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=1;
        return(0);
    }
    if((inst&0xFE0F)==0x920A)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st -y,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(28);
        rb=read_register(29);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_register(rr);
        write_memory(rk,rc);
        write_register(28,(rk>>0)&0xFF);
        write_register(29,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //STz  pattern 1000 001r rrrr 0000
    //STz  pattern 1001 001r rrrr 0001
    //STz  pattern 1001 001r rrrr 0010
    //STz  pattern 10q0 qq1r rrrr 0qqq
    if((inst&0xFE0F)==0x8200)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st z,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x9201)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st z+,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rc=read_register(rr);
        write_memory(rk,rc);
        rk=rk+1;
        write_register(30,(rk>>0)&0xFF);
        write_register(31,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }
    if((inst&0xFE0F)==0x9202)
    {
        rr=(inst>>4)&0x1F;
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    st -z,r%u\n",pc_base,inst,rr);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rk=(rb<<8)|ra;
        rk=rk-1;
        rc=read_register(rr);
        write_memory(rk,rc);
        write_register(30,(rk>>0)&0xFF);
        write_register(31,(rk>>8)&0xFF);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //STS  pattern 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9200)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc+2;
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sts 0x%04x,r%u ; %u\n",pc_base,inst,rk,rd,rk);
#endif
        rc=read_register(rd);
        write_memory(rk,rc);

        pc=pc_next;
        cycles+=2;
        return(0);
    }

    //STS  pattern 1010 1kkk dddd kkkk
    if((inst&0xF800)==0xA800)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0700)>>4)|(inst&0xF);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sts 0x%02X,r%u ; %u\n",pc_base,inst,rk,rd,rk);
#endif
        rc=read_register(rd);
        write_memory(rk,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SUB  pattern 0001 10rd dddd rrrr
    if((inst&0xFC00)==0x1800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    sub r%u,r%u\n",pc_base,inst,rd,rr);
#endif
        ra=read_register(rd);
        rb=read_register(rr);
        rc=(ra-rb)&0xFF;
        write_register(rd,rc);

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rb,1);
        do_cflag(ra,~rb,1);
        do_vflag(ra,~rb,1);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SUBI pattern 0101 kkkk dddd kkkk
    if((inst&0xF000)==0x5000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    subi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
#endif
        ra=read_register(rd);
        rc=(ra-rk)&0xFF;
        write_register(rd,rc);

        sreg&=~(HBIT|SBIT|VBIT|NBIT|ZBIT|CBIT);
        do_hflag(ra,~rk,1);
        do_cflag(ra,~rk,1);
        do_vflag(ra,~rk,1);
        if(rc&0x80) set_nbit();
        if(rc==0x00) set_zbit();
        do_sflag();

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //SWAP pattern 1001 010d dddd 0010
    if((inst&0xFE0F)==0x9402)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    swap r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(rd);
        rc=((ra<<4)|(ra>>4))&0xFF;
        write_register(rd,rc);

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //WDR  pattern 1001 0101 1010 1000
    if(inst==0x95A8)
    {
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    wdr\n",pc_base,inst);
#endif
        printf("watchdog timer not implemented, todo\n");

        pc=pc_next;
        cycles+=1;
        return(0);
    }

    //XCH  pattern 1001 001r rrrr 0100
    if((inst&0xFE0F)==0x9204)
    {
        rd=((inst>>4)&0x1F);
#ifdef DISASSEMBLE
        printf("0x%04X: 0x%04X ......    xch z,r%u\n",pc_base,inst,rd);
#endif
        ra=read_register(30);
        rb=read_register(31);
        rc=read_register(rd);
        rk=(rb<<8)|ra;
        rb=read_memory(rk);
        write_memory(rk,rc);
        write_register(rd,rb);

        pc=pc_next;
        cycles+=1;
        return(0);
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
    memset(rom,0x55,sizeof(rom));

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
//unsigned int ra;

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

    printf("cycles %lu\n",cycles);
    printf("fetch_count %lu\n",fetch_count);
    printf("read_count %lu\n",read_count);
    printf("write_count %lu\n",write_count);
    printf("read/write %lu\n",read_count+write_count);

    return(0);
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

