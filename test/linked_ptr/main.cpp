
#include "mem_linked_ptr.h"


int main(
    int /*argc*/, char ** /*argv*/)
    try
{
    tests::mem_linked_ptr_synchro();
    tests::mem_linked_ptr_orphan();

    return 0;
}
catch (...)
{
    return 1;
}
