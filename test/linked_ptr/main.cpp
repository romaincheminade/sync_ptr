
#include "mem_linked_ptr.h"


int main(
    int /*argc*/, char ** /*argv*/)
    try
{
    tests::mem_linked_ptr_synchro();

    return 0;
}
catch (...)
{
    return 1;
}
