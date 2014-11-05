#ifdef GPU_KENEL
#include "ds.hpp"
struct Node{
    Ray ray;
    Node* parent;
    float3 intersectPoint;
    float3 diffuseColor;

    float3 refractColor;
    float3 reflectColor;

    float3 refractratio;
    float3 reflectratio;

    RayType raytype;
    Material m;
    ProgramLabels pc;
};

typedef struct
{
    __local Node* stackData;
    size_t stackSize;
    size_t maxStackSize;
} Stack;

void stackInit(Stack* pStack, __local Node* pStackData, size_t maxStackSize)
{
    pStack->stackData = pStackData;
    pStack->stackSize = 0;
    pStack->maxStackSize = maxStackSize;
}

void stackPush( Stack* pStack, Node frame)
{
    if(pStack->stackSize < pStack->maxStackSize)
    {
        pStack->stackData[pStack->stackSize] = frame;
        pStack->stackSize += 1;
    }
}

void stackPop( Stack* pStack, int numValuesToPop )
{
    if( pStack->stackSize > (numValuesToPop - 1) )
    {
        pStack->stackSize -= numValuesToPop;
    }
}

Node* stackTop( Stack* pStack, int offset )
{
    if( pStack->stackSize > (0 + offset) )
    {
        return pStack->stackData + (pStack->stackSize - 1 - offset);
    }
    return NULL;
}


#endif

