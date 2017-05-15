
// Main header.
#include "mem_linked_ptr.h"

#include <mem/linked_ptr.h>
#include <test_assert.h>


void tests::mem_linked_ptr_synchro(void)
{
    class Obj
    {};

    mem::sync_ptr<Obj>		obj1 = mem::make_sync<Obj>();
    mem::linked_ptr<Obj>	obj2 = obj1;
    test_assert(obj1);
    test_assert(obj2);

    test_assert(obj1 == obj2);
    test_assert(obj1.count() == obj2.count());

    obj1.reset(new Obj());
    test_assert(obj1 == obj2);
    test_assert(obj1.count() == obj2.count());
}
