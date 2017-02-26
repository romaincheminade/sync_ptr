# **sync_ptr** 

Instance management (and propagation) involves multiple combined patterns and requires us to keep track of the code managing creation and reclamation of such resources.

Chained references are complex to maintain, update, swap, steal, reclaim.
These operation often require costly traversals generating performance penalty. 

Providing an object managing these simplifies the design, 
ease the development process and guaranties execution safety. 

***
`sync_ptr` uses RAII technique by binding the life cycle and propagation of a managed object.

The object management is populated on all chained `sync_ptr` instances in a single call.

`sync_ptr` automatically release the resource once it is no more in use.

*** 

`sync_ptr` stands for **synchronized pointer**.

`sync_ptr` objects that are **copy constructed** or **copy assigned** point to the same underlying raw pointer.

**When the original `sync_ptr` or one of its copy underlying raw pointer changes, all `sync_ptr` and copies point to the updated raw pointer.**

`sync_ptr` and underlying raw pointer are **reference counted**.

The underlying pointer memory is returned when the reference count drops to zero or another raw pointer is assigned.

***

**`sync_ptr` come in 2 different flavors**
- **policy**, offers strong execution guarantee and thread safety using default policies, and is easily extensible using the provided policies or any desired one. 
- **atomic**, offers faster concurrent environment execution (lock free and wait free) but weaker execution guarantee, all operation return their success state leaving the programmer the choice in the response strategy.

***

### Policy sync_ptr
Header only implementation for easy integration.
~~~cpp
#include <mem/sync_ptr.h>
~~~
    
Call **copy assignment operator** or **copy constructor** to link objects to the same underlying raw pointer.

~~~cpp
struct Obj
{
    void do_something();
};

mem::sync_ptr<Obj> ptr1 = mem::make_sync<Obj>();
mem::sync_ptr<Obj> ptr2(ptr1);
mem::sync_ptr<Obj> ptr3 = ptr2;
ptr3->do_something(); // same as ptr1->do_something() and ptr2->do_something().
~~~

Call **reset()** to update raw pointer on all linked references.
~~~cpp
mem::sync_ptr<Obj> ptr1 = mem::make_sync<Obj>();
mem::sync_ptr<Obj> ptr2(ptr1);

// Update underlying raw pointer
Obj * obj = new Obj();
ptr1.reset(obj); // ptr1 and ptr2 point to obj.
~~~

Call **release()** to steal the raw pointer from a `sync_ptr` chain.
~~~cpp
mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();

// Stealing.
Obj * raw = ptr.release();

// Note that First underlying raw pointer is now null.
assert(ptr.get() == nullptr);
~~~

Call **exchange()** to steal the raw pointer from a `sync_ptr` chain and store a target one in the chain.
~~~cpp
mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();

// Stealing.
Obj * ptr_add   = ptr.get();
Obj * raw       = new Obj();
std::unique_ptr<Obj> ptr_xc(ptr.exchange(raw));

// "ptr_xc" now contains original underlying pointer from "ptr".
assert(ptr_xc);
assert(ptr_xc.get() == ptr_add);

// "ptr" now contains "raw".
assert(ptr);
assert(ptr.get() == raw);
~~~

Helpers.
~~~cpp
mem::make_sync()
mem::make_sync_with_allocator()
~~~

For convenience, relational operators are provided.

***

### Atomic sync_ptr

The same methods as the policy based one are provided.
The main difference is that they return their execution success state.

See `cc/sync_ptr.h` and `tests/cc_sync_ptr.h .cpp` for usage example.

***
Please note that `sync_ptr` behavior is different from `std::shared_ptr`.
~~~cpp
struct Obj
{};
std::shared_ptr<Obj> ptr1 = std::make_shared<Obj>();
std::shared_ptr<Obj> ptr2 = ptr1;
assert(ptr1 == ptr2); // same behavior
ptr1.reset(new Obj);
assert(ptr1 != ptr2); // different behavior

mem::sync_ptr<Obj> sptr1 = mem::make_sync<Obj>();
mem::sync_ptr<Obj> sptr2 = sptr1;
assert(sptr1 == sptr2); // same behavior
sptr1.reset(new Obj);
assert(sptr1 == sptr2); // different behavior
~~~ 
After calling **reset()** the two `sync_ptr` point to the **same** underlying raw pointer whereas the `std::shared_ptr` point to **different** ones.
