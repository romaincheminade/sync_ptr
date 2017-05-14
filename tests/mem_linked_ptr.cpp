
// Main header.
#include "mem_linked_ptr.h"

#include <cassert>
#include <memory>


void tests::mem_linked_ptr_synchro(void)
{
    class Obj
    {};
    mem::sync_ptr<Obj>		obj;

    mem::sync_ptr<Obj>		obj1 = mem::make_sync<Obj>();
    mem::sync_ptr<Obj>		obj2(obj1);
    mem::sync_ptr<Obj>		obj3 = mem::make_sync<Obj>();
    assert(obj1);
    assert(obj2);
    assert(obj3);

    assert(obj1.get() == obj2.get());
    assert(obj1.count() == obj2.count());
    assert(obj1 != obj3);

    obj1.reset(new Obj());
    assert(obj1 == obj2);
    assert(obj1.count() == obj2.count());

    obj1 = obj3;
    assert(obj1 == obj3);
    assert(obj1.count() == obj3.count());
    assert(obj1 != obj2);

    obj1.reset(new Obj());
    assert(obj1.get() != nullptr);
    assert(obj1 == obj3);
    assert(obj1.count() == obj3.count());
}
