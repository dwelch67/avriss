
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
    maxadd+=1;
    maxadd>>=1;
    for(ra=0;ra<maxadd;ra++)
    {
        printf("0x%04X : 0x%04X\n",ra,rom[ra]);
    }

    memset(hit,0,sizeof(hit));
    newhits=0;
    add_hit(0); //reset vector

    while(1)
    {
        newhits=0;
        for(ra=0;ra<maxadd;ra++)
        {
            if(hit[ra]) find_hits(ra);
        }
        if(newhits==0) break;
    }

    for(ra=0;ra<maxadd;ra++)
    {
        printf("0x%04X : 0x%04X %u\n",ra,rom[ra],hit[ra]);
    }


    return(0);
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

