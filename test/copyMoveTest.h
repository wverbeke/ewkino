//function to test copy and move constructors, copy and move assignment operators, and destructor of different classes 
//the function will execute each of these operations, so they can then be scanned by valgrind for memory leaks

//include c++ library classes
#include <utility>


template< typename T > void copyMoveTest( const T& object ){

    //copy constructor
    T copied_object( object );

    //move constructor
    T moved_object( std::move( copied_object ) );

    //copy assignment
    T object_to_copy( object );
    moved_object = object_to_copy;
    
    //copy self-assignment
    moved_object = moved_object;

    //move assignment
    moved_object = T( object );

    //move self-assignment
    //WARNING : This should actually never occur in user code! This test checks the robustness of the code even under BUGGY CONDITIONS THAT SHOULD NOT OCCUR.
    moved_object = std::move( moved_object );

    //destructor wil be called now on copied_object, object_to_copy and moved_object
}
