# sync_ptr

    Reference counted synchronized pointer.
    
    Requires C++11 or higher.
    
    When the original sync_ptr or one of its copy underlying raw pointer changes, all sync_ptr and copies point to the updated raw pointer.
    
    Call assignment operator or copy constructor to link objects to the same underlying raw pointer.
       
    eve::mem::sync_ptr<Foo> ptr1(new Foo());
    eve::mem::sync_ptr<Foo> ptr2(ptr1);
    eve::mem::sync_ptr<Foo> ptr3 = ptr1;
    
    Call reset(ptr) to update raw pointer on all linked instances.
    eve::mem::sync_ptr<Foo> ptr1(new Foo());
    eve::mem::sync_ptr<Foo> ptr2(ptr1);
    ptr1.reset(new Foo());
    
    Helpers.
    eve::mem::make_sync()
    eve::mem::make_sync_with_deleter()
    eve::mem::make_sync_with_allocator()
    eve::mem::make_sync_with_allocator_and_deleter()
    
    Deleter must provide "void operator(TPtr * ptr)".
    Allocator must provide "TPtr * operator(...)".