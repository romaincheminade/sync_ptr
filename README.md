# **sync_ptr** 

## What ?

`sync_ptr` stands for **synchronized pointer**.

`sync_ptr` objects that are either **copy constructed** or **copy assigned** point to the same underlying raw pointer.

When the original `sync_ptr` or one of its copy underlying raw pointer changes, all `sync_ptr` and copies point to the updated raw pointer.

`sync_ptr` and underlying raw pointer are reference counted.

The underlying pointer memory is returned when the reference count drops to zero or another raw pointer is assigned.

Reference count and underlying pointer storage are thread safe, lock free, wait free.

Please note that `sync_ptr` behavior is different from `std::shared_ptr`.
~~~cpp
struct Obj
{};
std::shared_ptr<Obj> ptr1 = std::make_shared<Obj>();
std::shared_ptr<Obj> ptr2 = ptr1;
assert(ptr1 == ptr2);
ptr1.reset(new Obj);
assert(ptr1 != ptr2);

eve::mem::sync_ptr<Obj> sptr1 = eve::mem::make_sync<Obj>();
eve::mem::sync_ptr<Obj> sptr2 = sptr1;
assert(sptr1 == sptr2);
sptr1.reset(new Obj);
assert(sptr1 == sptr2);
~~~ 
After calling **reset()** the two `sync_ptr` point to the **same** underlying raw pointer whereas the `std::shared_ptr` point to **different** ones.
~~~cpp
assert(ptr1 != ptr2);
assert(sptr1 == sptr2);
~~~



## Why ?

I mostly use `sync_ptr` instead of instance management and propagation patterns (e.g. Singleton).

It has also proven very efficient in identifying cross cycle and layering issues in different implementations.

Please note that `sync_ptr` is heavier than raw pointer or `std::unique_ptr`,
therefore it is intended for use on specific objects (e.g. Manager(s)) 
and should not be spread all over the code base when not required.

## How ?

Single header implementation.
~~~cpp
#include <sync_ptr.h>
~~~
    
Call assignment operator or copy constructor to link objects to the same underlying raw pointer.

~~~cpp
eve::mem::sync_ptr<Foo> ptr1 = eve::mem::make_sync<Foo>();
eve::mem::sync_ptr<Foo> ptr2(ptr1);
eve::mem::sync_ptr<Foo> ptr3 = ptr2;
ptr3->Bar(); // same as ptr1->Bar() and ptr2->Bar().
~~~

Call reset() to update raw pointer on all linked instances.
~~~cpp
eve::mem::sync_ptr<Foo> ptr1 = eve::mem::make_sync<Foo>();
eve::mem::sync_ptr<Foo> ptr2(ptr1);

// Update underlying raw pointer
Foo * foo = new Foo();
ptr1.reset(foo); // ptr1 and ptr2 point to foo.
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
