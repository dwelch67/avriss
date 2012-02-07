
//-------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------------------------------------
unsigned int newhits;

unsigned int maxadd;
#define ROMMASK 0x00FFFF
unsigned short rom[ROMMASK+1];
unsigned int hit[ROMMASK+1];
unsigned char newline[1024];
unsigned char gstring[80];
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned short fetch ( unsigned short address )
{
    return(rom[address&ROMMASK]);
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
//-----------------------------------------------------------------------------
void add_hit ( unsigned int add )
{
    if((add&ROMMASK)!=add)
    {
        //error
        return;
    }
    if(hit[add]) return; //not an error, expected often
    hit[add]=1;
    newhits++;
}
//-----------------------------------------------------------------------------
unsigned int find_hits ( unsigned int pc_base )
{
    unsigned int pc_next;
    unsigned int pc_cond;
    unsigned int inst;
    unsigned int inst2;
    unsigned int rk;

    pc_next=pc_base+1;
    inst=rom[pc_base];

    //ADC pattern 0001 11rd dddd rrrr
    //ROL pattern 0001 11dd dddd dddd
    if((inst&0xFC00)==0x1C00)
    {
        add_hit(pc_next);
        return(0);
    }

    //ADD pattern 0000 11rd dddd rrrr
    //LSL pattern 0000 11dd dddd dddd
    if((inst&0xFC00)==0x0C00)
    {
        add_hit(pc_next);
        return(0);
    }

    //ADIW pattern 1001 0110 kkdd kkkk
    if((inst&0xFF00)==0x9600)
    {
        add_hit(pc_next);
        return(0);
    }

    //AND pattern 0010 00rd dddd rrrr
    //TST pattern 0010 00dd dddd dddd
    if((inst&0xFC00)==0x2000)
    {
        add_hit(pc_next);
        return(0);
    }

    //ANDI pattern 0111 kkkk dddd kkkk
    //CBR pattern 0111 kkkk dddd kkkk
    if((inst&0xF000)==0x7000)
    {
        add_hit(pc_next);
        return(0);
    }

    //ASR pattern 1001 010d dddd 0101
    if((inst&0xFE0F)==0x9405)
    {
        add_hit(pc_next);
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
        add_hit(pc_next);
        return(0);
    }

    //BLD pattern 1111 100d dddd 0bbb
    if((inst&0xFE08)==0xF800)
    {
        add_hit(pc_next);
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
        if(rk&0x40) rk|=~0x7F;
        pc_cond=(pc_base+rk+1)&ROMMASK;
        add_hit(pc_cond);
        add_hit(pc_next);
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
        if(rk&0x40) rk|=~0x7F;
        pc_cond=(pc_base+rk+1)&ROMMASK;
        add_hit(pc_cond);
        add_hit(pc_next);
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
        add_hit(pc_next);
        return(0);
    }

    //BST pattern 1111 101d dddd 0bbb
    if((inst&0xFE08)==0xFA00)
    {
        add_hit(pc_next);
        return(0);
    }

    //CALL pattern 1001 0100 0000 1110 kkkk kkkk kkkk kkkk 16 bit pc?
    //CALL pattern 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
//    if((inst&0xFE0E)==0x940E)
    if(inst==0x940E)
    {
        if(hit[(pc_next&ROMMASK)])
        {
            printf("Instruction Collision 0x%04X\n",pc_next);
        }
        inst2=rom[((pc_base+1)&ROMMASK)];
        pc_next=pc_base+2;
        //rk=((inst>>3)&0x3E);
        //rk|=inst&1;
        //rk<<=16;
        //rk|=inst2;
        rk=inst2;
        pc_cond=rk;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //CBI pattern 1001 1000 aaaa abbb
    if((inst&0xFF00)==0x9800)
    {
        add_hit(pc_next);
        return(0);
    }

    //COM  pattern 1001 010d dddd 0000
    if((inst&0xFE0F)==0x9400)
    {
        add_hit(pc_next);
        return(0);
    }

    //CP   pattern 0001 01rd dddd rrrr
    if((inst&0xFC00)==0x1400)
    {
        add_hit(pc_next);
        return(0);
    }

    //CPC  pattern 0000 01rd dddd rrrr
    if((inst&0xFC00)==0x0400)
    {
        add_hit(pc_next);
        return(0);
    }

    //CPI  pattern 0011 kkkk dddd kkkk
    if((inst&0xF000)==0x3000)
    {
        add_hit(pc_next);
        return(0);
    }

    //CPSE pattern 0001 00rd dddd rrrr
    if((inst&0xFC00)==0x1000)
    {
        pc_cond=pc_base+2;
        inst2=rom[(pc_next&ROMMASK)];
        if(is32bit(inst2)) pc_cond+=1;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //DEC  pattern 1001 010d dddd 1010
    if((inst&0xFE0F)==0x940A)
    {
        add_hit(pc_next);
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
        printf("EICALL NOT SUPPORTED\n");
        return(1);
    }

    //EIJMP pattern 1001 0100 0001 1001
    if(inst==0x9419)
    {
        printf("EIJMP NOT SUPPORTED\n");
        return(1);
    }

    //ELPM pattern 1001 0101 1101 1000
    //ELPM pattern 1001 000d dddd 0110
    //ELPM pattern 1001 000d dddd 0111
    if(inst==0x95D8)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9006)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9007)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }

    //EOR  pattern 0010 01rd dddd rrrr
    //CLR  pattern 0010 01dd dddd dddd
    if((inst&0xFC00)==0x2400)
    {
        add_hit(pc_next);
        return(0);
    }

    //FMUL pattern 0000 0011 0ddd 1rrr
    if((inst&0xFF88)==0x0308)
    {
        add_hit(pc_next);
        return(0);
    }

    //FMULS pattern 0000 0011 1ddd 0rrr
    if((inst&0xFF88)==0x0380)
    {
        add_hit(pc_next);
        return(0);
    }

    //FMULSU pattern 0000 0011 1ddd 1rrr
    if((inst&0xFF88)==0x0388)
    {
        add_hit(pc_next);
        return(0);
    }

    //ICALL pattern 1001 0101 0000 1001
    if(inst==0x9509)
    {
        add_hit(pc_next);
        return(0);
    }

    //IJMP pattern 1001 0100 0000 1001
    if(inst==0x9409)
    {
        return(1);
    }

    //IN pattern 1011 0aad dddd aaaa
    if((inst&0xF800)==0xB000)
    {
        add_hit(pc_next);
        return(0);
    }

    //INC pattern 1001 010d dddd 0011
    if((inst&0xFE0F)==0x9403)
    {
        add_hit(pc_next);
        return(0);
    }

    //JMP pattern 1001 010k kkkk 110k kkkk kkkk kkkk kkkk
    if((inst&0xFE0E)==0x940C)
    {
        inst2=rom[(pc_base+1)&ROMMASK];
        //rk=((inst>>3)&0x3E);
        //rk|=inst&1;
        //rk<<=16;
        //rk|=inst2;
        rk=inst2;
        add_hit(rk);
        return(1);
    }

    //LAC  pattern 1001 001r rrrr 0110
    if((inst&0xFE0F)==0x9206)
    {
        add_hit(pc_next);
        return(0);
    }

    //LAS  pattern 1001 001r rrrr 0101
    if((inst&0xFE0F)==0x9205)
    {
        add_hit(pc_next);
        return(0);
    }

    //LAT  pattern 1001 001r rrrr 0111
    if((inst&0xFE0F)==0x9207)
    {
        add_hit(pc_next);
        return(0);
    }

    //LDx  pattern 1001 000d dddd 1100
    //LDx  pattern 1001 000d dddd 1101
    //LDx  pattern 1001 000d dddd 1110
    if((inst&0xFE0F)==0x900C)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x900D)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x900E)
    {
        add_hit(pc_next);
        return(0);
    }


    //LDy  pattern 1000 000d dddd 1000
    //LDy  pattern 1001 000d dddd 1001
    //LDy  pattern 1001 000d dddd 1010
    //LDy  pattern 10q0 qq0d dddd 1qqq
    if((inst&0xFE0F)==0x8008)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9009)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x900A)
    {
        add_hit(pc_next);
        return(0);
    }

    //LDz  pattern 1000 000d dddd 0000
    //LDz  pattern 1001 000d dddd 0001
    //LDz  pattern 1001 000d dddd 0010
    //LDz  pattern 10q0 qq0d dddd 0qqq
    if((inst&0xFE0F)==0x8000)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9001)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9002)
    {
        add_hit(pc_next);
        return(0);
    }

    //LDI  pattern 1110 kkkk dddd kkkk
    //SER  pattern 1110 1111 dddd 1111
    if((inst&0xF000)==0xE000)
    {
        add_hit(pc_next);
        return(0);
    }

    //LDS  pattern 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9000)
    {
        //inst2=fetch(pc_base+1);
        pc_next=pc_base+2;
        add_hit(pc_next);
        return(0);
    }

    //LDS  pattern 1010 0kkk dddd kkkk
    if((inst&0xF800)==0xA000)
    {
        add_hit(pc_next);
        return(0);
    }

    //LPM  pattern 1001 0101 1100 1000
    //LPM  pattern 1001 000d dddd 0100
    //LPM  pattern 1001 000d dddd 0101
    if(inst==0x95C8)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9004)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9005)
    {
        add_hit(pc_next);
        return(0);
    }

    //LSR  pattern 1001 010d dddd 0110
    if((inst&0xFE0F)==0x9406)
    {
        add_hit(pc_next);
        return(0);
    }

    //MOV  pattern 0010 11rd dddd rrrr
    if((inst&0xFC00)==0x2C00)
    {
        add_hit(pc_next);
        return(0);
    }

    //MOVW pattern 0000 0001 dddd rrrr
    if((inst&0xFF00)==0x0100)
    {
        add_hit(pc_next);
        return(0);
    }

    //MUL  pattern 1001 11rd dddd rrrr
    if((inst&0xFC00)==0x9C00)
    {
        add_hit(pc_next);
        return(1);
    }

    //MULS pattern 0000 0010 dddd rrrr
    if((inst&0xFF00)==0x0200)
    {
        add_hit(pc_next);
        return(1);
    }

    //MULSU pattern 0000 0011 0ddd 0rrr
    if((inst&0xFF88)==0x0300)
    {
        add_hit(pc_next);
        return(1);
    }

    //NEG  pattern 1001 010d dddd 0001
    if((inst&0xFE0F)==0x9401)
    {
        add_hit(pc_next);
        return(0);
    }

    //NOP  pattern 0000 0000 0000 0000
    if(inst==0x0000)
    {
        add_hit(pc_next);
        return(0);
    }

    //OR  pattern 0010 10rd dddd rrrr
    if((inst&0xFC00)==0x2800)
    {
        add_hit(pc_next);
        return(0);
    }

    //ORI  pattern 0110 kkkk dddd kkkk
    //SBR  pattern 0110 kkkk dddd kkkk
    if((inst&0xF000)==0x6000)
    {
        add_hit(pc_next);
        return(0);
    }

    //OUT  pattern 1011 1aar rrrr aaaa
    if((inst&0xF800)==0xB800)
    {
        add_hit(pc_next);
        return(0);
    }

    //POP  pattern 1001 000d dddd 1111
    if((inst&0xFE0F)==0x900F)
    {
        add_hit(pc_next);
        return(0);
    }


    //PUSH pattern 1001 001d dddd 1111
    if((inst&0xFE0F)==0x920F)
    {
        add_hit(pc_next);
        return(0);
    }

    //RCALL pattern 1101 kkkk kkkk kkkk
    if((inst&0xF000)==0xD000)
    {
        rk=inst&0x0FFF;
        if(rk&0x800) rk|=~0xFFF;
        pc_cond=(pc_base+rk+1)&ROMMASK;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //RET  pattern 1001 0101 0000 1000
    if(inst==0x9508)
    {
        return(0);
    }

    //RETI pattern 1001 0101 0001 1000
    if(inst==0x9518)
    {
        return(0);
    }

    //RJMP 1100 kkkk kkkk kkkk
    if((inst&0xF000)==0xC000)
    {
        rk=inst&0xFFF;
        if(rk&0x800) rk|=~0xFFF;
        pc_next=(pc_base+rk+1)&ROMMASK;
        add_hit(pc_next);
        return(1);
    }

    //ROR pattern 1001 010d dddd 0111
    if((inst&0xFE0F)==0x9407)
    {
        add_hit(pc_next);
        return(0);
    }

    //SBC  pattern 0000 10rd dddd rrrr
    if((inst&0xFC00)==0x0800)
    {
        add_hit(pc_next);
        return(0);
    }

    //SBCI pattern 0100 kkkk dddd kkkk
    if((inst&0xF000)==0x4000)
    {
        add_hit(pc_next);
        return(0);
    }

    //SBI  pattern 1001 1010 aaaa abbb
    if((inst&0xFF00)==0x9A00)
    {
        add_hit(pc_next);
        return(0);
    }

    //SBIC pattern 1001 1001 aaaa abbb
    if((inst&0xFF00)==0x9900)
    {
        pc_cond=pc_base+2;
        inst2=rom[pc_next&ROMMASK];
        if(is32bit(inst2)) pc_cond+=1;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //SBIS pattern 1001 1011 aaaa abbb
    if((inst&0xFF00)==0x9B00)
    {
        pc_cond=pc_base+2;
        inst2=rom[pc_next&ROMMASK];
        if(is32bit(inst2)) pc_cond+=1;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //SBIW pattern 1001 0111 kkdd kkkk
    if((inst&0xFF00)==0x9700)
    {
        add_hit(pc_next);
        return(0);
    }


    //SBRC pattern 1111 110r rrrr 0bbb
    if((inst&0xFE08)==0xFC00)
    {
        pc_cond=pc_base+2;
        inst2=rom[pc_next&ROMMASK];
        if(is32bit(inst2)) pc_cond+=1;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }


    //SBRS pattern 1111 111r rrrr 0bbb
    if((inst&0xFE08)==0xFE00)
    {
        pc_cond=pc_base+2;
        inst2=rom[pc_next&ROMMASK];
        if(is32bit(inst2)) pc_cond+=1;
        add_hit(pc_cond);
        add_hit(pc_next);
        return(0);
    }

    //SLEEP pattern 1001 0101 1000 1000
    if(inst==0x9588)
    {
        add_hit(pc_next);
        return(1);
    }

    //SPM  pattern 1001 0101 1110 1000
    //SPM  pattern 1001 0101 1111 1000
    if(inst==0x95E8)
    {
        add_hit(pc_next);
        return(1);
    }
    if(inst==0x95F8)
    {
        add_hit(pc_next);
        return(1);
    }

    //STx  pattern 1001 001r rrrr 1100
    //STx  pattern 1001 001r rrrr 1101
    //STx  pattern 1001 001r rrrr 1110
    if((inst&0xFE0F)==0x920C)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x920D)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x920E)
    {
        add_hit(pc_next);
        return(0);
    }

    //STy  pattern 1000 001r rrrr 1000
    //STy  pattern 1001 001r rrrr 1001
    //STy  pattern 1001 001r rrrr 1010
    //STy  pattern 10q0 qq1r rrrr 1qqq
    if((inst&0xFE0F)==0x8208)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9209)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x920A)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xD208)==0x8208)
    {
        add_hit(pc_next);
        return(0);
    }

    //STz  pattern 1000 001r rrrr 0000
    //STz  pattern 1001 001r rrrr 0001
    //STz  pattern 1001 001r rrrr 0010
    //STz  pattern 10q0 qq1r rrrr 0qqq
    if((inst&0xFE0F)==0x8200)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9201)
    {
        add_hit(pc_next);
        return(0);
    }
    if((inst&0xFE0F)==0x9202)
    {
        add_hit(pc_next);
        return(0);
    }

    //STS  pattern 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9200)
    {
        pc_next=pc_base+2;
        add_hit(pc_next);
        return(0);
    }

    //STS  pattern 1010 1kkk dddd kkkk
    if((inst&0xF800)==0xA800)
    {
        add_hit(pc_next);
        return(0);
    }

    //SUB  pattern 0001 10rd dddd rrrr
    if((inst&0xFC00)==0x1800)
    {
        add_hit(pc_next);
        return(0);
    }

    //SUBI pattern 0101 kkkk dddd kkkk
    if((inst&0xF000)==0x5000)
    {
        add_hit(pc_next);
        return(0);
    }

    //SWAP pattern 1001 010d dddd 0010
    if((inst&0xFE0F)==0x9402)
    {
        add_hit(pc_next);
        return(0);
    }

    //WDR  pattern 1001 0101 1010 1000
    if(inst==0x95A8)
    {
        add_hit(pc_next);
        return(0);
    }

    //XCH  pattern 1001 001r rrrr 0100
    if((inst&0xFE0F)==0x9204)
    {
        add_hit(pc_next);
        return(0);
    }




    //BREAK
    if(inst==0x9598)
    {
        //add_hit(pc_next);
        return(1);
    }
    printf("0x%04X: 0x%04X 0x%04X    UNDEFINED\n",pc_base,inst,rom[(pc_base+1)&ROMMASK]);
    return(1);


}
//-----------------------------------------------------------------------------
//-------------------------------------------------------------------
int diss  ( unsigned int pc_base )
{
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


    pc_next=(pc_base+1)&ROMMASK;
    inst=rom[pc_base&ROMMASK];

    //ADC pattern 0001 11rd dddd rrrr
    //ROL pattern 0001 11dd dddd dddd
    if((inst&0xFC00)==0x1C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        if(rd==rr)
        {
            printf("0x%04X: 0x%04X ......    rol r%u\n",pc_base,inst,rd);
        }
        else
        {
            printf("0x%04X: 0x%04X ......    adc r%u,r%u\n",pc_base,inst,rd,rr);
        }
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
            printf("0x%04X: 0x%04X ......    lsl r%u\n",pc_base,inst,rd);
        }
        else
        {
            printf("0x%04X: 0x%04X ......    add r%u,r%u\n",pc_base,inst,rd,rr);
        }
        return(0);
    }

    //ADIW pattern 1001 0110 kkdd kkkk
    if((inst&0xFF00)==0x9600)
    {
        rd=24+((inst>>3)&0x6);
        rk=((inst&0x00C0)>>2)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    adiw r%u:r%u,0x%02X ; %u\n",pc_base,inst,rd+1,rd,rk,rk);
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
            printf("0x%04X: 0x%04X ......    tst r%u,r%u\n",pc_base,inst,rd,rr);
        }
        else
        {
            printf("0x%04X: 0x%04X ......    and r%u,r%u\n",pc_base,inst,rd,rr);
        }
        return(0);
    }



    //ANDI pattern 0111 kkkk dddd kkkk
    //CBR pattern 0111 kkkk dddd kkkk
    if((inst&0xF000)==0x7000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    andi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //ASR pattern 1001 010d dddd 0101
    if((inst&0xFE0F)==0x9405)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    asr r%u\n",pc_base,inst,rd);
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
        return(0);
    }

    //BLD pattern 1111 100d dddd 0bbb
    if((inst&0xFE08)==0xF800)
    {
        rd=((inst>>4)&0x1F);
        rb=(inst&0x7);
        printf("0x%04X: 0x%04X ......    bld r%u,%u\n",pc_base,inst,rd,rb);
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
        return(0);
    }

    //BST pattern 1111 101d dddd 0bbb
    if((inst&0xFE08)==0xFA00)
    {
        rd=((inst>>4)&0x1F);
        rb=(inst&0x7);
        printf("0x%04X: 0x%04X ......    bst r%u,%u\n",pc_base,inst,rd,rb);
        return(0);
    }

    //CALL pattern 1001 0100 0000 1110 kkkk kkkk kkkk kkkk 16 bit pc?
    //CALL pattern 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
//    if((inst&0xFE0E)==0x940E)
    if(inst==0x940E)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc_base+2;

        //rk=((inst>>3)&0x3E);
        //rk|=inst&1;
        //rk<<=16;
        //rk|=inst2;
        rk=inst2;

        //address 0x1234 is on the stack as
        //sp->0x12
        //sp+1->0x34

        printf("0x%04X: 0x%04X 0x%04X    call 0x%04X\n",pc_base,inst,inst2,rk);
        return(0);
    }

    //CBI pattern 1001 1000 aaaa abbb
    if((inst&0xFF00)==0x9800)
    {
        ra=((inst>>3)&0x1F);
        rb=inst&0x7;

        printf("0x%04X: 0x%04X ......    cbi 0x%02X,%u\n",pc_base,inst,ra,rb);
        return(0);
    }

    //COM  pattern 1001 010d dddd 0000
    if((inst&0xFE0F)==0x9400)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    com r%u\n",pc_base,inst,rd);
        return(0);
    }

    //CP   pattern 0001 01rd dddd rrrr
    if((inst&0xFC00)==0x1400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    cp r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //CPC  pattern 0000 01rd dddd rrrr
    if((inst&0xFC00)==0x0400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    cpc r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //CPI  pattern 0011 kkkk dddd kkkk
    if((inst&0xF000)==0x3000)
    {
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        rd=16+((inst>>4)&0xF);
        printf("0x%04X: 0x%04X ......    cpi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //CPSE pattern 0001 00rd dddd rrrr
    if((inst&0xFC00)==0x1000)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        pc_cond=pc_base+2;
        inst2=fetch(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
        printf("0x%04X: 0x%04X ......    cpse r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }


    //DEC  pattern 1001 010d dddd 1010
    if((inst&0xFE0F)==0x940A)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    dec r%u\n",pc_base,inst,rd);
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
        printf("EICALL NOT SUPPORTED\n");
        return(1);
    }

    //EIJMP pattern 1001 0100 0001 1001
    if(inst==0x9419)
    {
        printf("EIJMP NOT SUPPORTED\n");
        return(1);
    }

    //ELPM pattern 1001 0101 1101 1000
    //ELPM pattern 1001 000d dddd 0110
    //ELPM pattern 1001 000d dddd 0111
    if(inst==0x95D8)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9006)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }
    if((inst&0xFE0F)==0x9007)
    {
        printf("ELPM NOT SUPPORTED\n");
        return(1);
    }

    //EOR  pattern 0010 01rd dddd rrrr
    //CLR  pattern 0010 01dd dddd dddd
    if((inst&0xFC00)==0x2400)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        if(rd==rr)
        {
            printf("0x%04X: 0x%04X ......    clr r%u\n",pc_base,inst,rd);
        }
        else
        {
            printf("0x%04X: 0x%04X ......    eor r%u,r%u\n",pc_base,inst,rd,rr);
        }
        return(0);
    }

    //FMUL pattern 0000 0011 0ddd 1rrr
    if((inst&0xFF88)==0x0308)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
        printf("0x%04X: 0x%04X ......    fmul r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //FMULS pattern 0000 0011 1ddd 0rrr
    if((inst&0xFF88)==0x0380)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
        printf("0x%04X: 0x%04X ......    fmuls r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //FMULSU pattern 0000 0011 1ddd 1rrr
    if((inst&0xFF88)==0x0388)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x7);
        printf("0x%04X: 0x%04X ......    fmulsu r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //ICALL pattern 1001 0101 0000 1001
    if(inst==0x9509)
    {
        printf("0x%04X: 0x%04X ......    icall\n",pc_base,inst);
        return(0);
    }

    //IJMP pattern 1001 0100 0000 1001
    if(inst==0x9409)
    {
        printf("0x%04X: 0x%04X ......    ijmp\n",pc_base,inst);
        return(0);
    }

    //IN pattern 1011 0aad dddd aaaa
    if((inst&0xF800)==0xB000)
    {
        rk=((inst&0x0600)>>5)|(inst&0x000F);
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    in 0x%02X,r%u\n",pc_base,inst,rk,rd);
        return(0);
    }

    //INC pattern 1001 010d dddd 0011
    if((inst&0xFE0F)==0x9403)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    inc r%u\n",pc_base,inst,rd);
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

        printf("0x%04X: 0x%04X 0x%04X    jmp 0x%06X\n",pc_base,inst,inst2,rk);
        return(0);
    }


    //LAC  pattern 1001 001r rrrr 0110
    if((inst&0xFE0F)==0x9206)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    lac z,r%u\n",pc_base,inst,rd);
        return(1);
    }

    //LAS  pattern 1001 001r rrrr 0101
    if((inst&0xFE0F)==0x9205)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    las z,r%u\n",pc_base,inst,rd);
        return(1);
    }

    //LAT  pattern 1001 001r rrrr 0111
    if((inst&0xFE0F)==0x9207)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    lat z,r%u\n",pc_base,inst,rd);
        return(1);
    }

    //LDx  pattern 1001 000d dddd 1100
    //LDx  pattern 1001 000d dddd 1101
    //LDx  pattern 1001 000d dddd 1110
    if((inst&0xFE0F)==0x900C)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,x\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x900D)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,x+\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x900E)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,-x\n",pc_base,inst,rd);
        return(0);
    }


    //LDy  pattern 1000 000d dddd 1000
    //LDy  pattern 1001 000d dddd 1001
    //LDy  pattern 1001 000d dddd 1010
    //LDy  pattern 10q0 qq0d dddd 1qqq
    if((inst&0xFE0F)==0x8008)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,y\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x9009)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,y+\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x900A)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,-y\n",pc_base,inst,rd);
        return(0);
    }

    //LDz  pattern 1000 000d dddd 0000
    //LDz  pattern 1001 000d dddd 0001
    //LDz  pattern 1001 000d dddd 0010
    //LDz  pattern 10q0 qq0d dddd 0qqq
    if((inst&0xFE0F)==0x8000)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,z\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x9001)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,z+\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x9002)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    ld r%u,-z\n",pc_base,inst,rd);
        return(0);
    }

    //LDI  pattern 1110 kkkk dddd kkkk
    //SER  pattern 1110 1111 dddd 1111
    if((inst&0xF000)==0xE000)
    {
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        rd=16+((inst>>4)&0xF);
        printf("0x%04X: 0x%04X ......    ldi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //LDS  pattern 1001 000d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9000)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc_base+2;
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    lds r%u,0x%04X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //LDS  pattern 1010 0kkk dddd kkkk
    if((inst&0xF800)==0xA000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0700)>>4)|(inst&0xF);
        printf("0x%04X: 0x%04X ......    lds r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //LPM  pattern 1001 0101 1100 1000
    //LPM  pattern 1001 000d dddd 0100
    //LPM  pattern 1001 000d dddd 0101
    if(inst==0x95C8)
    {
        printf("0x%04X: 0x%04X ......    lpm\n",pc_base,inst);
        return(0);
    }
    if((inst&0xFE0F)==0x9004)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    lpm r%u,z\n",pc_base,inst,rd);
        return(0);
    }
    if((inst&0xFE0F)==0x9005)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    lpm r%u,z+\n",pc_base,inst,rd);
        return(0);
    }

    //LSR  pattern 1001 010d dddd 0110
    if((inst&0xFE0F)==0x9406)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    lsr r%u\n",pc_base,inst,rd);
        return(0);
    }

    //MOV  pattern 0010 11rd dddd rrrr
    if((inst&0xFC00)==0x2C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    mov r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //MOVW pattern 0000 0001 dddd rrrr
    if((inst&0xFF00)==0x0100)
    {
        rd=((inst>>3)&0x1E);
        rr=((inst<<1)&0x1E);
        printf("0x%04X: 0x%04X ......    movw r%u:r%u,r%u:r%u\n",pc_base,inst,rd+1,rd,rr+1,rr);
        return(0);
    }

    //MUL  pattern 1001 11rd dddd rrrr
    if((inst&0xFC00)==0x9C00)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    mul r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //MULS pattern 0000 0010 dddd rrrr
    if((inst&0xFF00)==0x0200)
    {
        rd=16+((inst>>4)&0xF);
        rr=16+(inst&0x000F);
        printf("0x%04X: 0x%04X ......    muls r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //MULSU pattern 0000 0011 0ddd 0rrr
    if((inst&0xFF88)==0x0300)
    {
        rd=16+((inst>>4)&0x7);
        rr=16+(inst&0x0007);
        printf("0x%04X: 0x%04X ......    mulsu r%u,r%u\n",pc_base,inst,rd,rr);
        return(1);
    }

    //NEG  pattern 1001 010d dddd 0001
    if((inst&0xFE0F)==0x9401)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    neg r%u\n",pc_base,inst,rd);
        return(0);
    }

    //NOP  pattern 0000 0000 0000 0000
    if(inst==0x0000)
    {
        printf("0x%04X: 0x%04X ......    nop\n",pc_base,inst);
        return(0);
    }

    //OR  pattern 0010 10rd dddd rrrr
    if((inst&0xFC00)==0x2800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    or r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //ORI  pattern 0110 kkkk dddd kkkk
    //SBR  pattern 0110 kkkk dddd kkkk
    if((inst&0xF000)==0x6000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    ori r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //OUT  pattern 1011 1aar rrrr aaaa
    if((inst&0xF800)==0xB800)
    {
        rk=((inst&0x0600)>>5)|(inst&0x000F);
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    out 0x%02X,r%u\n",pc_base,inst,rk,rd);
        return(0);
    }

    //POP  pattern 1001 000d dddd 1111
    if((inst&0xFE0F)==0x900F)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    pop r%u\n",pc_base,inst,rd);
        return(0);
    }

    //PUSH pattern 1001 001d dddd 1111
    if((inst&0xFE0F)==0x920F)
    {
        rd=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    push r%u\n",pc_base,inst,rd);
        return(0);
    }

    //RCALL pattern 1101 kkkk kkkk kkkk
    if((inst&0xF000)==0xD000)
    {
        rk=inst&0x0FFF;
        if(rk&0x800) rk|=0xFFF000;
        pc_cond=(pc_base+rk+1)&ROMMASK;
        printf("0x%04X: 0x%04X ......    rcall 0x%04X\n",pc_base,inst,pc_cond);
        return(0);
    }

    //RET  pattern 1001 0101 0000 1000
    if(inst==0x9508)
    {
        printf("0x%04X: 0x%04X ......    ret\n",pc_base,inst);
        return(0);
    }

    //RETI pattern 1001 0101 0001 1000
    if(inst==0x9518)
    {
        printf("0x%04X: 0x%04X ......    reti\n",pc_base,inst);
        return(0);
    }

    //RJMP 1100 kkkk kkkk kkkk
    if((inst&0xF000)==0xC000)
    {
        rk=inst&0xFFF;
        if(rk&0x800) rk|=~0xFFF;
        pc_next=(pc_base+rk+1)&ROMMASK;
        printf("0x%04X: 0x%04X ......    rjmp 0x%04X\n",pc_base,inst,pc_next);
        return(0);
    }

    //ROR pattern 1001 010d dddd 0111
    if((inst&0xFE0F)==0x9407)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    ror r%u\n",pc_base,inst,rd);
        return(0);
    }

    //SBC  pattern 0000 10rd dddd rrrr
    if((inst&0xFC00)==0x0800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    sbc r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //SBCI pattern 0100 kkkk dddd kkkk
    if((inst&0xF000)==0x4000)
    {
        rd=16+((inst>>4)&0xF);
        rb=((inst&0x0F00)>>4)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    sbci r%u,0x%02X ; %u\n",pc_base,inst,rd,rb,rb);
        return(0);
    }

    //SBI  pattern 1001 1010 aaaa abbb
    if((inst&0xFF00)==0x9A00)
    {
        rb=inst&7;
        ra=(inst>>3)&0x001F;
        printf("0x%04X: 0x%04X ......    sbi 0x02X,%u\n",pc_base,inst,ra,rb);
        return(0);
    }


    //SBIC pattern 1001 1001 aaaa abbb
    if((inst&0xFF00)==0x9900)
    {
        ra=((inst>>3)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=fetch(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
        printf("0x%04X: 0x%04X ......    sbic 0x%02X,%u\n",pc_base,inst,ra,rb);
        return(0);
    }

    //SBIS pattern 1001 1011 aaaa abbb
    if((inst&0xFF00)==0x9B00)
    {
        ra=((inst>>3)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=fetch(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
        printf("0x%04X: 0x%04X ......    sbis 0x%02X,%u\n",pc_base,inst,ra,rb);
        return(0);
    }

    //SBIW pattern 1001 0111 kkdd kkkk
    if((inst&0xFF00)==0x9700)
    {
        rd=24+((inst>>3)&0x6);
        rk=((inst&0x00C0)>>2)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    sbiw r%u:r%u,0x%02X ; %u\n",pc_base,inst,rd+1,rd+0,rk,rk);
        return(0);
    }


    //SBRC pattern 1111 110r rrrr 0bbb
    if((inst&0xFE08)==0xFC00)
    {
        rr=((inst>>4)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=fetch(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
        printf("0x%04X: 0x%04X ......    sbrc r%u,%u\n",pc_base,inst,rr,rb);
        return(0);
    }


    //SBRS pattern 1111 111r rrrr 0bbb
    if((inst&0xFE08)==0xFE00)
    {
        rr=((inst>>4)&0x1F);
        rb=(inst&0x0007);
        pc_cond=pc_base+2;
        inst2=fetch(pc_next);
        if(is32bit(inst2)) pc_cond+=1;
        printf("0x%04X: 0x%04X ......    sbrs r%u,%u\n",pc_base,inst,rr,rb);
        return(0);
    }

    //SLEEP pattern 1001 0101 1000 1000
    if(inst==0x9588)
    {
        printf("0x%04X: 0x%04X ......    sleep\n",pc_base,inst);
        return(1);
    }

    //SPM  pattern 1001 0101 1110 1000
    //SPM  pattern 1001 0101 1111 1000
    if(inst==0x95E8)
    {
        printf("0x%04X: 0x%04X ......    spm\n",pc_base,inst);
        return(1);
    }
    if(inst==0x95F8)
    {
        printf("0x%04X: 0x%04X ......    spm\n",pc_base,inst);
        return(1);
    }

    //STx  pattern 1001 001r rrrr 1100
    //STx  pattern 1001 001r rrrr 1101
    //STx  pattern 1001 001r rrrr 1110
    if((inst&0xFE0F)==0x920C)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st x,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x920D)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st x+,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x920E)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st -x,r%u\n",pc_base,inst,rr);
        return(0);
    }

    //STy  pattern 1000 001r rrrr 1000
    //STy  pattern 1001 001r rrrr 1001
    //STy  pattern 1001 001r rrrr 1010
    //STy  pattern 10q0 qq1r rrrr 1qqq
    if((inst&0xFE0F)==0x8208)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st y,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x9209)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st y+,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x920A)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st -y,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xD208)==0x8208)
    {
        rr=(inst>>4)&0x1F;
        rb=((inst&0x2000)>>8)|((inst&0x0C00)>>7)|(inst&0x0007);
        printf("0x%04X: 0x%04X ......    st y+%u,r%u ; 0x%02X\n",pc_base,inst,rb,rr,rb);
        return(0);
    }

    //STz  pattern 1000 001r rrrr 0000
    //STz  pattern 1001 001r rrrr 0001
    //STz  pattern 1001 001r rrrr 0010
    //STz  pattern 10q0 qq1r rrrr 0qqq
    if((inst&0xFE0F)==0x8200)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st z,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x9201)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st z+,r%u\n",pc_base,inst,rr);
        return(0);
    }
    if((inst&0xFE0F)==0x9202)
    {
        rr=(inst>>4)&0x1F;
        printf("0x%04X: 0x%04X ......    st -z,r%u\n",pc_base,inst,rr);
        return(0);
    }

    //STS  pattern 1001 001d dddd 0000 kkkk kkkk kkkk kkkk
    if((inst&0xFE0F)==0x9200)
    {
        inst2=fetch(pc_base+1);
        pc_next=pc_base+2;
        rd=((inst>>4)&0x1F);
        rk=inst2;
        printf("0x%04X: 0x%04X 0x%04X    sts 0x%04x,r%u ; %u\n",pc_base,inst,inst2,rk,rd,rk);
        return(0);
    }

    //STS  pattern 1010 1kkk dddd kkkk
    if((inst&0xF800)==0xA800)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0700)>>4)|(inst&0xF);
        printf("0x%04X: 0x%04X ......    sts 0x%02X,r%u ; %u\n",pc_base,inst,rk,rd,rk);
        return(0);
    }

    //SUB  pattern 0001 10rd dddd rrrr
    if((inst&0xFC00)==0x1800)
    {
        rd=((inst>>4)&0x1F);
        rr=((inst&0x0200)>>5)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    sub r%u,r%u\n",pc_base,inst,rd,rr);
        return(0);
    }

    //SUBI pattern 0101 kkkk dddd kkkk
    if((inst&0xF000)==0x5000)
    {
        rd=16+((inst>>4)&0xF);
        rk=((inst&0x0F00)>>4)|(inst&0x000F);
        printf("0x%04X: 0x%04X ......    subi r%u,0x%02X ; %u\n",pc_base,inst,rd,rk,rk);
        return(0);
    }

    //SWAP pattern 1001 010d dddd 0010
    if((inst&0xFE0F)==0x9402)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    swap r%u\n",pc_base,inst,rd);
        return(0);
    }

    //WDR  pattern 1001 0101 1010 1000
    if(inst==0x95A8)
    {
        printf("0x%04X: 0x%04X ......    wdr\n",pc_base,inst);
        return(0);
    }

    //XCH  pattern 1001 001r rrrr 0100
    if((inst&0xFE0F)==0x9204)
    {
        rd=((inst>>4)&0x1F);
        printf("0x%04X: 0x%04X ......    xch z,r%u\n",pc_base,inst,rd);
        return(0);
    }

    //BREAK
    if(inst==0x9598)
    {
        printf("0x%04X: 0x%04X ......    break\n",pc_base,inst);
        return(0);
    }








    printf("0x%04X: 0x%04X 0x%04X    UNDEFINED\n",pc_base,inst,rom[pc_next]);
    return(1);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int readhex ( FILE *fp )
{
    unsigned int addhigh;
    unsigned int add;
    unsigned int ra;
    unsigned int line;
    unsigned char checksum;
    unsigned int len;
    unsigned char t;

    addhigh=0;
    memset(rom,0xFF,sizeof(rom));

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


        if((add&ROMMASK)!=add)
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
                //printf("End of data\n");
                break;
            case 0x02:
                gstring[0]=newline[9];
                gstring[1]=newline[10];
                gstring[2]=newline[11];
                gstring[3]=newline[12];
                gstring[4]=0;
                addhigh=strtoul(gstring,NULL,16);
                addhigh<<=16;
                //printf("addhigh %08X\n",addhigh);
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
    maxadd+=1;
    maxadd>>=1;
    for(ra=0;ra<maxadd;ra++)
    {
//        printf("0x%04X : 0x%04X\n",ra,rom[ra]);
    }

    memset(hit,0,sizeof(hit));
    newhits=0;
    add_hit(0); //reset vector

    while(1)
    {
        newhits=0;
        for(ra=0;ra<=maxadd;ra++)
        {
            if(hit[ra]) find_hits(ra);
        }
        if(newhits==0) break;
    }

    for(ra=0;ra<=maxadd;ra++)
    {
//        printf("0x%04X : 0x%04X %u\n",ra,rom[ra],hit[ra]);
    }

    for(ra=0;ra<=maxadd;ra++)
    {
        if(hit[ra]) diss(ra);
        else printf("0x%04X: 0x%04X ...... \n",ra,rom[ra]);

    }



    return(0);
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

