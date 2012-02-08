
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void hexstring16 ( unsigned int d );

#define MAX 100
unsigned char pmap[MAX];

int run_primes_test ( void )
{
    unsigned char ra;
    unsigned char rb;


    for(ra=0;ra<MAX;ra++) pmap[ra]=0;
    pmap[0]=1;
    for(ra=0,rb=0;rb<MAX;rb++) if(pmap[rb]==0) ra++;
    hexstring16(ra);

    ra=0;
    while(1)
    {
        for(rb=0;rb<MAX;rb++)
        {
            if(pmap[rb]==0) break;
        }
        if(rb<MAX)
        {
            pmap[rb]=1;
            ra++;
        }
        else
        {
            break;
        }
    }
    hexstring16(ra);




    for(ra=0;ra<MAX;ra++) pmap[ra]=0;
    pmap[0]=1;
    pmap[1]=1;

    while(1)
    {
        for(rb=0;rb<MAX;rb++)
        {
            if(pmap[rb]==0)
            {
                pmap[rb]=2;
                break;
            }
        }
        if(rb<MAX)
        {
            for(ra=rb+rb;ra<MAX;ra+=rb) pmap[ra]=1;
        }
        else
        {
            break;
        }
    }


    //for(ra=0,rb=0;ra<MAX;ra++)
    //{
        ////hexstring16(pmap[ra]);
        //if(pmap[ra]==2)
        //{
            //rb++;
            //hexstring16(ra);
        //}
    //}
    //hexstring16(rb);


    return(0);
}

