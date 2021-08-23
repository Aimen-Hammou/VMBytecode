#include <stdio.h>
#include <stdlib.h>


/*

    Programmer: Aiman Hammou (aimenhammou@gmail.com)

    Virtual machine base che può runnare codice macchina

    Il motivo per il quale ho deciso di scrivere una VM, o per essere più corretti
    una STACK-BASED VM, è per aver più familiarità con esse e così da riuscere ad implementarne
    una in modo più rapido per uno dei miei prossimi progetti: un linguaggio di programmazione che usa BYTECODE su VM
    per garantire performance e sopratutto PORTABILITA' ( ed eventuale garbage collector)

    Essendo che voglio creare una Virtual Stack si presenta il vantaggio di non operare su registri
    ma lo svantaggio di dover compiere più operazioni BYTECODE

    In questo caso simuliamo la VM ha il suo BYTECODE

    Supporta solo gli integer
*/


#define STACK_CAPACITY 100

typedef struct VIRTUALMACHINE
{
    int ProgramCounter; // Punta alla istruzione da eseguire
    int StackPointer  ; // Ci dice quanti elementi sono presenti NB: PUNTA SEMPRE IN ALTO.
    int FramePointer  ; // Per capire lo scope di una variabile

    int *Locals;
    int *Stack;
    int *SourceCodePtr;   // Puntatore all'array contente BYTECODE da eseguire

}VirtualMachine;


/* Ad ogni istruzione viene associato un OPCODE dall'enum, a partire da 1 */
enum INSTRUCTIONS
{
    ADD_INT = 1,
    SUB_INT,
    MUL_INT,
    DIV_INT,

    LT_INT,  // Less Than
    GT_INT,  // Greater Than
    EQ_INT,  // Equal To

    JUMP,    // Jump Branch
    JUMPT,   // Jump If True
    JUMPF,   // Jump If False

    CONST_INT, // Load Constant

    LOAD,    // Load local variable

    GLOAD,   // Load Global Variable

    STORE,   // Store in local

    GSTORE,  // Store in global

    PRINT,   // Print Value Pointer By Pc

    POP_V,     // Pop Top Element

    HALT,    // Arrest Program

    CALL,    // Call Procedure

    RET,     // Return From Procedure

};


VirtualMachine *InitVirtualMachine(int *_SourceCodePtr, int _ProgramCounter, int DataSize)
{
    VirtualMachine *VM = (VirtualMachine *) malloc ( sizeof (VirtualMachine) );

    VM->SourceCodePtr = _SourceCodePtr;
    VM->ProgramCounter = _ProgramCounter;
    VM->FramePointer = 0;

    VM->Locals = (int *) malloc (sizeof( int ) * DataSize);
    VM->Stack = (int *) malloc (sizeof( int ) * STACK_CAPACITY);

    return VM;
}

void FreeVirtualMachine(VirtualMachine *VM)
{
    free(VM->Locals);
    free(VM->Stack);

    free(VM);

    return;
}


/* Funzioni di Supporto */

int POP(VirtualMachine *VM)
{
    return VM->Stack[ VM->StackPointer-- ];
}

void PUSH(VirtualMachine *VM, int Value)
{
    return VM->Stack[ ++VM->StackPointer ] = Value;
}

int NextByteCode(VirtualMachine *VM)
{
    return VM->SourceCodePtr[ VM->ProgramCounter++ ];
}



void RunVirtualMachine(VirtualMachine *VM)
{

    int OperationCode = 0;

    int Address = 0;

    int FirstValue = 0;
    int SecondValue = 0;

    int ArgumentCounter = 0;
    int ReturnValue = 0;

    int TemporaryValue = 0; // ValueToLoad

    /* Per ogni ciclo guardiamo l'operazione da eseguire */

    while(1)
    {
        OperationCode = NextByteCode(VM);

        //printf("Opcode: %d\n", OperationCode);

        switch(OperationCode)
        {
            /* Essendo che è LIFO gli elementi sono pushati in ordine contrario */

            case ADD_INT:
                SecondValue = POP(VM);
                FirstValue = POP(VM);

                PUSH(VM, FirstValue + SecondValue);
                break;

            case SUB_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                PUSH(VM, FirstValue - SecondValue);

                break;

            case MUL_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                PUSH(VM, FirstValue * SecondValue);

                break;

            case DIV_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                PUSH(VM, ( int ) ( FirstValue / SecondValue) );

                break;

            case LT_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                TemporaryValue = (FirstValue < SecondValue ) ? 1 : 0;

                PUSH(VM, TemporaryValue);

                break;

            case GT_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                TemporaryValue = (FirstValue < SecondValue ) ? 1 : 0;

                PUSH(VM, TemporaryValue);

                break;

            case EQ_INT:

                SecondValue = POP(VM);
                FirstValue = POP(VM);

                TemporaryValue = (FirstValue == SecondValue ) ? 1 : 0;

                PUSH(VM, TemporaryValue);

                break;

            /* Per capire meglio il Jump è consigliato Leggere il ByteCode prima */
            case JUMP:

                VM->ProgramCounter = NextByteCode(VM); // JUMP 20 --- NextByteCode() ritorna 20 quindi salta alla ventesima operazione

                break;

            case JUMPT:

                Address = NextByteCode(VM);

                if (POP(VM))
                    VM->ProgramCounter = Address;

                break;

            case JUMPF:

                Address = NextByteCode(VM);

                if (!POP(VM))
                    VM->ProgramCounter = Address;

                break;

            case CONST_INT:

                TemporaryValue = NextByteCode(VM);

                PUSH(VM, TemporaryValue);

                break;

            case LOAD:

                TemporaryValue = NextByteCode(VM); // Offset

                PUSH(VM, VM->Stack[VM->FramePointer + TemporaryValue]);

                break;

            case GLOAD:

                Address = POP(VM);

                TemporaryValue = VM->Locals[ Address ];

                PUSH(VM, TemporaryValue);

                break;

            case STORE:

                TemporaryValue = NextByteCode(VM); // Offset

                VM->Locals[ VM->FramePointer + TemporaryValue] = POP(VM);

                break;

            case GSTORE:

                TemporaryValue = POP(VM);

                Address = NextByteCode(VM);

                VM->Locals[ Address ] = TemporaryValue;
                break;

            case PRINT:

                printf("%d\n", POP(VM));
                break;

            case POP_V:

                VM->StackPointer--;

                break;

            case HALT:

                return;

            case CALL:

                Address = NextByteCode(VM);
                ArgumentCounter = NextByteCode(VM);

                PUSH(VM, ArgumentCounter);
                PUSH(VM, VM->FramePointer);    // Salviamo il FramePointer
                PUSH(VM, VM->ProgramCounter);  // Salviamo il ProgramCounter attuale

                VM->FramePointer = VM->StackPointer;

                VM->ProgramCounter = Address;

                break;

            case RET:

                ReturnValue = POP(VM);

                VM->StackPointer = VM->FramePointer;
                VM->ProgramCounter = POP(VM);
                VM->FramePointer = POP(VM);

                ArgumentCounter = POP(VM);

                VM->StackPointer -= ArgumentCounter;

                PUSH(VM, ReturnValue);

                break;

            default:

                break;
        }
    }
}




int main()
{

    const int fib = 0; // Entry point della funzione
    int sourceCode[] =
    {

        // int fib(n) {
        //     if(n == 0) return 0;
        LOAD, -3,       // 0 - Carico il parametro della funzione di fibonacci
        CONST_INT, 0,   // 2 - Metto 0 come costante
        EQ_INT,         // 4 - Verifico: che il parametro sia uguale ad 0: n == 0
        JUMPF, 10,       // 5 - se NON uguali salta istruzione 10
        CONST_INT, 0,   // 7 - sennò metti 0
        RET,            // 9 - ritorno della funzione
        //     if(n < 3) return 1;
        LOAD, -3,       // 10 - carico il parametro
        CONST_INT, 3,   // 12 - constante = 3
        LT_INT,         // 14 - controllo se N < 3
        JUMPF, 20,       // 15 - ise vero allora salta istruzione 20
        CONST_INT, 1,   // 17 - altrimenti metti 1
        RET,            // 19 - Ritorna
        //     else return fib(n-1) + fib(n-2);
        LOAD, -3,       // 20 - carico il parametro
        CONST_INT, 1,   // 22 - constante = 1
        SUB_INT,        // 24 - calcolo N-1
        CALL, fib, 1,   // 25 - chiamo la funzione con un argomento ( appena calcolato )
        LOAD, -3,       // 28 - Ricarico il parametro
        CONST_INT, 2,   // 30 - costante = 2
        SUB_INT,        // 32 - calcolo N-2
        CALL, fib, 1,   // 33 - chiamo la funzione con un argomento ( appena calcolato )
        ADD_INT,        // 36 - essendo che abbiamo due valori di ritorno in stack li sommiamo
        RET,            // 37 - ritorna dalla funzion
        // entrypoint - main function
        CONST_INT, 6,   // 38 - metti 6
        CALL, fib, 1,   // 40 - richiamo funzione: fib(6) 
        PRINT,          // 43 - stampa risultato
        HALT            // 44 - fine programma


    };


    VirtualMachine *VM = InitVirtualMachine(sourceCode, 38, 0);


    RunVirtualMachine(VM);

    return 0;
}
