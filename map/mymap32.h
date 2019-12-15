#ifndef __MYMAP32_H__
#define __MYMAP32_H__

#include "mylist.h"
#include "pthread_mutex.hpp"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MAX_NAME 1024
#define MAX_OBJ_NAME 100

//24 bytes
class Node
{
    public:
        list_head link;
        list_head hash;
        intptr_t key;
        intptr_t val;
/*
        Node(){
            key = 0;
            val = 0;
        }
        ~Node();

    public:
*/
};

//88 bytes
class StrNode
{
    public:
        list_head link;
        list_head hash;
        char* key;
        intptr_t val;
        intptr_t len;
/*
        StrNode(){
            val = 0;
            len = 0;
            memset( key, 0, MAX_NAME );
        }
        ~StrNode();

    public:
*/
};


class MyMap32
{
    public:
        MyMap32();
        ~MyMap32();
        void    init( int hash_size, int max_size, const char* _obj_name );
	    bool	find( intptr_t key, intptr_t& val );
	    bool	set( intptr_t key, intptr_t val );
	    bool	del( intptr_t key );	
        void    clean(); 

        bool    isempty();
        int     count();

    public:
        Node*   first();
        Node*   next( Node* pre );

    public:
        int         num;
        int         hash_size; 
        int         pow2;
        int         max_size;
        char        obj_name[MAX_OBJ_NAME];

        list_head*  hash_table;
        list_head   link;
        list_head   free_list;
        Node*       mem;

};


class MyMapStr
{
    public:
        MyMapStr();
        ~MyMapStr();
        void    init( int hash_size, int max_size, const char* _obj_name );
	    bool	find( const char* key, intptr_t& val );
	    bool	set( const char* key, intptr_t val );
	    bool	del( const char* key );	
        void    clean();

        bool    isempty();
        int     count();

    public:
        StrNode*   first();
        StrNode*   next( StrNode* pre );

    public:
        int         num;
        int         hash_size; 
        int         pow2;
        int         max_size;
        char        obj_name[MAX_OBJ_NAME];

        list_head*  hash_table;
        list_head   link;
        list_head   free_list;
        StrNode*   mem;

};


#endif

