#ifndef PhysicsObjectCollection_H
#define PhysicsObjectCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>



template< typename ObjectType > class PhysicsObjectCollection {

    public:
        using collection_type = std::vector< std::shared_ptr< ObjectType > >;
        using iterator = typename collection_type::iterator;
        using const_iterator = typename collection_type::const_iterator;
        using value_type = typename collection_type::value_type;
        using size_type = typename collection_type::size_type;

        PhysicsObjectCollection() {}
        PhysicsObjectCollection( const collection_type& col ) : collection( col ) {}

        void push_back( const ObjectType& );
        void push_back( ObjectType&& );

        iterator begin(){ return collection.begin(); }
        const_iterator begin() const{ return collection.cbegin(); }
        const_iterator cbegin() const{ return collection.cbegin(); }
        iterator end(){ return collection.end(); }
        const_iterator end() const{ return collection.cend(); }
        const_iterator cend() const{ return collection.cend(); }

        size_type size() const{ return collection.size(); }
        
        template< typename func > void sortByAttribute( const func& f );
        void sortByPt(){ return sortByAttribute( [](const std::shared_ptr< ObjectType >& lhs, const std::shared_ptr< ObjectType >& rhs){ return lhs->pt() > rhs->pt(); } ); }

        ~PhysicsObjectCollection() = default;

    protected:
        void selectObjects( bool (ObjectType::*passSelection)() const );

    private:
        collection_type collection;
};



template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::push_back( const ObjectType& physicsObject ){
    collection.push_back( std::shared_ptr< ObjectType >( physicsObject.clone() ) );
}


template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::push_back( ObjectType&& physicsObject ){
    collection.push_back( std::shared_ptr< ObjectType >( std::move( physicsObject ).clone() ) );
}


template< typename ObjectType > template< typename func > void PhysicsObjectCollection< ObjectType >::sortByAttribute( const func& f ){
    std::sort( begin(), end(), f );
}


template< typename ObjectType > void PhysicsObjectCollection< ObjectType >::selectObjects( bool (ObjectType::*passSelection)() const ){
    std::vector< const_iterator > objectsToDelete;
    for( const_iterator it = cbegin(); it != cend(); ++it ){
        if( ( (**it).*passSelection)() ){
            objectsToDelete.push_back( it );
        }
    }
    for( auto& it : objectsToDelete ){
        collection.erase( it );
    }
}
#endif
