#include <stdio.h>
#include <stdlib.h>

// first assignment for systems software
// virtual machine interpreter for PM/0 machine 

// TO DO

// some constants that may be important
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVEL 3

// the struct that holds the important parts of each instruction
// the opcode   OP
// the level    L
// the variable M
typedef struct instruction
{
	int OP;
	int L;
	int M;
}instr;

// our stack is just an array of integers
int *stack;

// stack pointer points to the top of the stack
int sp = 0;

// the base pointer points to the base of the current IR
int bp = 1;

// instruction register has some purpose but not in this program...
// int IR = 0;

// the program counter keeps track of our place in the code store
int pc = 0;

// halt is a flag telling the machine to stop executing
int halt = 0;

// keeps track of the length of the code store
int length = 0;

// keeps track of the current level
int level = 0;

// the code store, holds all the instructions to be used
instr *i;

// a pointer to the file that has all the output
FILE *output;

// reads in all the instructions from the file
void readInstructions()
{
    // allocate space for all the instructions up to the max code length
    i = malloc(sizeof(instr) * MAX_CODE_LENGTH);
    
    // if the allocation failed, print a simple error message and return to main
    if(i == NULL)
    {
        printf("THERE WAS AN ERROR ALLOCATING MEMORY\n");
        return;
    }
    
    // open the file with the code in it for reading, always be "mcode.txt"
    FILE *f = fopen("mcode.txt", "r");
    
    // these three integers represent the opcode(o), the level(l), and var(m)
    int o, l, m;
    
    // create a temporary instruction to keep track of the variables
    instr temp;
    
    // loop through the file, and take out the variables three at a time
    // fscanf(..) == 3 means while three variables were read in
    // and while we are less than our max length
    while(fscanf(f, "%d %d %d", &o, &l, &m) != EOF && length <= MAX_CODE_LENGTH)
    {
        // assign the op, l and m to the temp struct
        temp.OP = o;
        temp.L  = l;
        temp.M  = m;
        
        // put the instruction in the code store and move one more in the store
        i[length] = temp;
        length++;
    }
}

// finds the base l levels down of the AR
int base(l, base)
{
    // get the base of the current level
    int b = base;
    
    // while there are more levels to go down, set the base to be the base of 
    // the record below the current level then decrement the level
    while(l > 0)
    {
        b = stack[b + 1];
        l--;
    } 
    
    // return the base pointer of that level
    return b;
}

// the arithmetic unit that performs calculations
void ALU(int l, int m)
{
    // the operation is specified by the m, so if m is out of the bounds, then 
    // stop the instruction, since it will fail
    if(m < 0 || m > 13)
        return;
        
    // test different cases for m
    switch(m)
    {
        case 0: // return from the current level to the previous level
            // set the stack pointer to the one before the base pointer
            sp = bp -1;
            // set the program counter to the old pc before the subroutine
            pc = stack[sp + 4];
            // set the base pointer to the base pointer held in the stack
            bp = stack[sp + 3];
            break;
        case 1: // negate the top of the stack
            stack[sp] *= -1;
            break;
        case 2: // add the top of the stack to the second element and push stack
            stack[sp - 1] = stack[sp-1] + stack[sp];
            sp--;
            break;
        case 3: // subtract the top and second 
            stack[sp - 1] = stack[sp-1] - stack[sp];
            sp--;
            break;
        case 4: // multiply the top and second
            stack[sp - 1] = stack[sp-1] * stack[sp];
            sp--;
            break;
        case 5: // divide the top by the second
            stack[sp - 1] = stack[sp-1] / stack[sp];
            sp--;
            break;
        case 6: // check if the top of the stack is odd and push the value
            stack[sp] = stack[sp] % 2;
            sp--;
            break;
        case 7: // set as the modulus of the top by the second
            stack[sp - 1] = stack[sp] % stack[sp - 1];
            sp--;
            break;
        case 8: // set 1 if the top two are equal, 0 other wise
            stack[sp - 1] = (stack[sp] == stack[sp-1] ? 1 : 0);
            sp--;
            break;
        case 9: // unequal
            stack[sp - 1] = (stack[sp] != stack[sp-1] ? 1 : 0);
            sp--;
            break;
        case 10: // less than
            stack[sp - 1] = (stack[sp] < stack[sp - 1] ? 1: 0);
            sp--;
            break;
        case 11: // less than or equal
            stack[sp - 1] = (stack[sp] <= stack[sp - 1]? 1: 0);
            sp--;
            break;
        case 12: // greater than
            stack[sp - 1] = (stack[sp] > stack[sp - 1] ? 1: 0);
            sp--;
            break;
        case 13: // greater than or equal
            stack[sp - 1] = (stack[sp] >= stack[sp - 1]? 1: 0);
            sp--;
            break;
        default:
            return;
    }
}

// execute the instruction being passed
void execute(instr i)
{
    // get the three variables to make processing easier
    int op = i.OP;
    int m = i.M;
    int l = i.L;
    
    // read holds the value if we are reading from the user 
    int read;
    
    // increment the program counter
    pc++;
    
    // if the op is out of the bounds of the instruction set
    if(op < 1 || op > 11)
        return;
    
    // if the stack pointer is above the max of the stack print error and return 
    if(sp >= MAX_STACK_HEIGHT)
    {
        printf("THE STACK HEIGHT IS TOO HIGH\n");
        return;
    }
    
    // check each case with the opcode
    switch(op)
    {
        case 1: // push literal m onto the stack
            // make the top value of the stack m
            stack[++sp] = m;
            break;
        case 2: // alu operation
            // call alu
            ALU(l, m);
            break;
        case 3: // load word from a previous stack frame
            // the top of the stack is set to the value stored in the base + m
            read = base(l, bp);
            stack[++sp] = stack[read + m];
            break;
        case 4: // store word
            // store in a stack frame at base the top of the stack
            read = base(l, bp);
            stack[read + m] = stack[sp--];
            break; 
        case 5: // call procedure at line m
            // set the value above the sp, now to be the bp to 0, return value
            stack[sp+1] = 0;
            // the value above that one is the static link, or the base l levels down
            stack[sp+2] = base(l, bp);
            // set the dynamic link to the current base pointer
            stack[sp+3] = bp;
            // set the return value to the old program counter that way we can return to where we were 
            stack[sp+4] = pc;
            // update the base pointer to be one above the stack pointer
            bp = sp+1;
            // set the program counter to the subroutine at line m
            pc = m;
            break;
        case 6: // allocate space for variables
            // do this by adding m variables to the top of the stack by moving
            // the stack pointer along m spaces, thus allocating m variables
            sp = sp + m;
            break;
        case 7: // jump to a line
            // move the program counter to instruction m in the code store
            pc = m;
            break;
        case 8: // branch if the top of the stack is 0
            if(stack[sp] == 0)
                pc = m;
            sp--;
            break;
        case 9: // pop from the stack and print to the console
            printf("%d\n", stack[sp--]);
            pc++;
            break;
        case 10: // read into the stack from the console
            scanf("%d", &read);
            stack[sp++] = read;
            pc++;
            break;
        case 11: // halt
            halt = 1;
            fprintf(output, "0   0   0");
            return;
        default:
            return;
    }
    fprintf(output, "%-2d\t%-2d\t%-2d\t", pc, bp, sp);
    int p = 1;
    for(; p <= sp; p++)
    {
        fprintf(output, "%d ", stack[p]);
        if(bp == p+1 || base(1, bp) == p+1) 
            fprintf(output, "| ");
    }
    fprintf(output, "\n");
}

void printInstructions()
{
    const char *codes[] = {"fet", "lit", "opr", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "sio", "sio"};
    if(i == NULL)
    {
        printf("UNABLE TO ACCESS INSTRUCTIONS\n");
        return;
    }
    int spot = 0;
    fprintf(output, "Line\tOP\tL\tM\n");
    while(spot < length)
    { 
        fprintf(output, "%2d\t%s\t%-2d\t%-2d\n", spot, codes[i[spot].OP], i[spot].L, i[spot].M);
        spot++;
    }
}

void printstack()
{
    const char *codes[] = {"fet", "lit", "opr", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "sio", "sio"};
    int spot = 0;
    fprintf(output, "%2d\t%s\t%d\t%-2d\t", pc, codes[i[pc].OP], i[pc].L, i[pc].M);
}

int main()
{
    // allocate space for the stack and open the output file
	stack = malloc(sizeof(int) * MAX_STACK_HEIGHT);
    output = fopen("output.txt", "w");
    
    // read in all the instructions 
    readInstructions();

    // if the the instruction list is null then terminate now
    if( i == NULL )
    {
        return 1;
    }    

    // print the instructions to the file before executing
    printInstructions();
    
    // print a header for the stack trace in the output file
    fprintf(output, "\n");
    fprintf(output, "\t\t\t\tpc  bp  sp  stack\n");
    fprintf(output, "Initial values\t0\t1\t0\n");

    // loop through the code store and keep going until halt is called and the
    // the halt flag is set to true    
    while(!halt)
    {
        // print the stack as it is now
        printstack();
        
        // execute the instruction at the program counter
        execute(i[pc]);
    }
    
    // free the code store, and the stack for memory management purposes
    free(i);
    free(stack);
    
    // close the output file
    fclose(output);
	return 0;
}
