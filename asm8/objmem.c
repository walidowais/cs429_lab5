/* 
   Assembler for PDP-8.  Memory and object file creation. 
*/

#include "asm8.h"


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* we want to assemble instructions.  We could assemble and output them
   all at once.  But we have a problem of forward references.  So we
   keep the (partially) assembled instructions in an array of them,
   essentially simulated memory.  That allows us to come back and 
   fix them up when the forward reference is resolved.

   We need to know which memory locations are from assembled
   instructions, and which are just empty; so each memory location
   has a bit (defined/not defined).
*/

char get_size(int i);

INST     memory[4096];
Boolean defined[4096];
Address entry_point = 0;


void Clear_Object_Code(void)
{
    int i;
    for (i = 0; i < 4096; i++)
        {
            defined[i] = FALSE;
        }
}

void Define_Object_Code(Address addr, INST inst, Boolean redefine)
{
    if (debug)
        fprintf(stderr, "object code: 0x%03X = 0x%03X\n", addr, inst);
    if (defined[addr] && !redefine)
        {
            fprintf(stderr, "redefined memory location: 0x%03X: was 0x%03X; new value 0x%03X\n",
                    addr, memory[addr], inst);
            number_of_errors += 1;
        }
                
    defined[addr] = TRUE;
    memory[addr] = inst;
}

INST Fetch_Object_Code(Address addr)
{
    INST inst;

    if (defined[addr])
        inst = memory[addr];
    else
        inst = 0;

    if (debug)
        fprintf(stderr, "read object code: 0x%03X = 0x%03X\n", addr, inst);
    return(inst);
}

void Output_Object_Code(void)
{
    int i, k;
    short ep1, ep2;
    int m1, m2;
    int size;
    short b1, b2;

    fputc('O', output);
    fputc('B', output);
    fputc('J', output);
    fputc('8', output);

    m1 = 0xFC0;
    m2 = 0x03F;

    ep1 = ((entry_point & m1) >> 6) & m2;
    fputc(ep1, output);
    ep2 = entry_point & m2;
    fputc(ep2, output);

    for(i = 0; i < 4096; i++){
        if(defined[i]){
            size = get_size(i);
            size = size & 0xFF;
            fputc(size, output);
            size -= 3;

            b1 = ((i & m1) >> 6) & m2;
            fputc(b1, output);
            b2 = i & m2;
            fputc(b2, output);

            k = 0;
            while((k < size) && (k < 256)){
                b1 = ((memory[i] & m1) >> 6) & m2;
                fputc(b1, output);
                b2 = memory[i] & m2;
                fputc(b2, output);

                if(k < 256){
                    i++;
                }
                k += 2;
            }
        }
    }

}

char get_size(int i){
    char count = 3;

    while(defined[i] && (count < 256)){
        if((count + 2) >= 256){
            return count;
        }

        count += 2;
        i++;
    }

    return count;
}