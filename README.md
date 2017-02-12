# **sync_ptr** 

Instance management and propagation is challenging.
It involves multiple combined patterns and requires to keep track of the code managing creation and reclamation of such resources.

Updating the resource on all references is not possible using standard smart pointers `std::shared_ptr`.

Chained references are complex to maintain, update, swap, steal, reclame.
These operation often requires costly traversals. 

Providing an object managing these simplifies the design, 
ease the developement process and guaranties execution safety. 

***

`sync_ptr` uses RAII technique by binding the life cycle and propagation of a unique instance.

**It updates that instance for all its references by a single safe call.**

`sync_ptr` automatically release the resource once it is no more in use.

It offer stealing for faster rebase.

`sync_ptr` is thread safe, lock free, wait free.

*** 

`sync_ptr` stands for **synchronized pointer**.

`sync_ptr` objects that are either **copy constructed** or **copy assigned** point to the same underlying raw pointer.

**When the original `sync_ptr` or one of its copy underlying raw pointer changes, all `sync_ptr` and copies point to the updated raw pointer.**

`sync_ptr` and underlying raw pointer are reference counted.

The underlying pointer memory is returned when the reference count drops to zero or another raw pointer is assigned.

***

Single header implementation.
~~~cpp
#include <sync_ptr.h>
~~~
    
Call **copy assignment operator** or **copy constructor** to link objects to the same underlying raw pointer.

~~~cpp
eve::mem::sync_ptr<Foo> ptr1 = eve::mem::make_sync<Foo>();
eve::mem::sync_ptr<Foo> ptr2(ptr1);
eve::mem::sync_ptr<Foo> ptr3 = ptr2;
ptr3->Bar(); // same as ptr1->Bar() and ptr2->Bar().
~~~

Call **reset()** to update raw pointer on all linked references.
~~~cpp
eve::mem::sync_ptr<Foo> ptr1 = eve::mem::make_sync<Foo>();
eve::mem::sync_ptr<Foo> ptr2(ptr1);

// Update underlying raw pointer
Foo * foo = new Foo();
ptr1.reset(foo); // ptr1 and ptr2 point to foo.
~~~

Call **steal()** to steal the raw pointer from a `sync_ptr` chain.
~~~cpp
struct Obj
{};

// First Chain.
eve::mem::sync_ptr<Obj> ptr1 = eve::mem::make_sync<Obj>();
eve::mem::sync_ptr<Obj> ptr2 = ptr1;
// First Chain raw pointer.
auto raw = ptr1.get();

// Second Chain.
eve::mem::sync_ptr<Obj> ptr3 = eve::mem::make_sync<Obj>();
eve::mem::sync_ptr<Obj> ptr4 = ptr3;

// Stealing.
ptr3.steal(ptr1);
assert(ptr3.get() == raw);
assert(ptr3 == ptr4);

// Note that First Chain underlying raw pointer is now null.
assert(ptr1.get() == nullptr);
assert(ptr1 == ptr2);
~~~

Helpers.
~~~cpp
eve::mem::make_sync()
eve::mem::make_sync_with_deleter()
eve::mem::make_sync_with_allocator()
eve::mem::make_sync_with_allocator_and_deleter()
~~~

Deleter must provide "void operator(T * ptr)".
Allocator must provide "T * operator(...)".

For convenience, all relational operators are provided.

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

eve::mem::sync_ptr<Obj> sptr1 = eve::mem::make_sync<Obj>();
eve::mem::sync_ptr<Obj> sptr2 = sptr1;
assert(sptr1 == sptr2); // same behavior
sptr1.reset(new Obj);
assert(sptr1 == sptr2); // different behavior
~~~ 
After calling **reset()** the two `sync_ptr` point to the **same** underlying raw pointer whereas the `std::shared_ptr` point to **different** ones.
