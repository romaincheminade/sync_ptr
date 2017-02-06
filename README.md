# **sync_ptr** 

## What ?

`sync_ptr` stands for **synchronized pointer**.

`sync_ptr` objects that are either **copy constructed** or **copy assigned** point to the same underlying raw pointer.

When the original `sync_ptr` or one of its copy underlying raw pointer changes, all `sync_ptr` and copies point to the updated raw pointer.

`sync_ptr` and underlying raw pointer are reference counted and released when the reference count drops to zero.

Reference count and underlying pointer storage are thread safe.

## Why ?

I mostly use `sync_ptr` instead of instance management and propagation patterns (e.g. Singleton).

It has also proven very efficient in identifying cross cycle and layering issues in different implementations.

Please note that `sync_ptr` is heavier than raw pointer or `std::unique_ptr`,
therefore it is intended for use on specific objects (e.g. Manager(s)) 
and should not be spread all over the code base when not required.

## How ?

Simgle header implementation.
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

// Release underlying raw pointer.
ptr2.reset(); // ptr1 and ptr2 point to nullptr.
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
