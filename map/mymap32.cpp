#include "mymap32.h"

/*! member function of MyMap32 */ 
MyMap32::MyMap32()
{
    num = 0;
    hash_table = 0;
    hash_size = 0;
    max_size = 0;
    pow2 = 0;
    mem = 0;
    memset( obj_name, 0, MAX_OBJ_NAME );
    INIT_LIST_HEAD( &free_list );
    INIT_LIST_HEAD( &link );
}

void MyMap32::clean()
{
    while(!list_empty(&link)) {
        list_head* pos = link.next;
        list_del(pos);
        Node* node = list_entry(pos, Node, link);
        list_del(&node->hash);
        list_del(&node->link);
        int index = (node - mem) / sizeof(Node);
        if(index >= 0 && index < max_size) {
            list_add( &(node->link), &free_list );
        } else {
            delete node;
        }
    }

    int i;
    for(i = 0; i < hash_size; ++i) {
        INIT_LIST_HEAD(&(hash_table[i]));
    }

    num = 0;
    INIT_LIST_HEAD(&link);
}

MyMap32::~MyMap32()
{
    clean();
    if( hash_table ) {
        delete []hash_table;
        hash_table = NULL;
    }

    if( mem ) {
        delete []mem;
        mem = NULL;
    }
}

void MyMap32::init( int _hash_size, int _max_size, const char* _obj_name )
{
    INIT_LIST_HEAD( &link );

    int i = 0;
    for( i = 1; i < 32; ++i ) {
        if( _hash_size <= ( 1 << i ) ) break;
    }
    
    num = 0;
    pow2 = i;
    hash_size = ( 1 << i );
    hash_table = new list_head[ hash_size ];
    for(i = 0; i < hash_size; ++i ) {
        INIT_LIST_HEAD( &hash_table[i] );
    }

    mem = new Node[ _max_size ];
    INIT_LIST_HEAD( &free_list );
    for(i = 0; i < _max_size; ++i){
        list_add_tail(&mem[i].link, &free_list);
    }
    max_size = _max_size;
    strncpy( obj_name, _obj_name, MAX_OBJ_NAME );
}

bool MyMap32::find( intptr_t key, intptr_t& val )
{
    int index = key & (hash_size-1); 
    list_head* pos;
    list_for_each_safe( pos, &(hash_table[index]) ) {
        Node* node = list_entry( pos, Node, hash );
        if( node->key == key ) {
            val = node->val;
            return true;
        }
    } 
    return false;
}

bool MyMap32::set( intptr_t key, intptr_t val )
{
    Node* node = 0;
    list_head* pos;
    int index = key & (hash_size-1); 
    list_for_each( pos, &(hash_table[index]) ) {
        node = list_entry( pos, Node, hash );
        if( node->key == key ) {
            node->val = val;
            return false;
        }
    } 
     
    if( list_empty(&free_list) ) {
        node = new Node;
    } else {
        pos = free_list.next;
        list_del(pos);
        node = list_entry( pos, Node, link ); 
    }

    if( node ) {
        node->key = key;
        node->val = val;
        list_add( &(node->hash), &(hash_table[index]) );
        list_add( &(node->link), &link );
        num++;
    }
    return true;
}

bool MyMap32::del( intptr_t key )
{
    int index = key & (hash_size-1); 
    list_head* pos;
    list_for_each_safe( pos, &(hash_table[index]) ) {
        Node* node = list_entry( pos, Node, hash );
        if( node->key == key ) {
            list_del( &(node->hash) );
            list_del( &(node->link) );

            int index = (node - mem) / sizeof(Node);
            if(index >= 0 && index < max_size) {
                list_add( &(node->link), &free_list );
            } else {
                delete node; 
            }

            num--;
            return true;
        }
    } 
    return false;
}


Node* MyMap32::first()
{
    if( !list_empty( &link ) ) {
        Node* node = list_entry( link.next, Node, link );
        return node;
    }
    return 0;
}

Node* MyMap32::next( Node* pre )
{
    list_head* pos = pre->link.next;
    if( pos != &link ) {
        Node* node = list_entry( pos, Node, link );
        return node;
    }
    return 0;
}

bool MyMap32::isempty()
{
    return list_empty( &link );
}

int MyMap32::count()
{
    return num;
}


static int str2id( const char* str, int len, int hash_size )
{
    if( len < 1 ) return 0;
	int hash = 5381;
	int c;
	while ((c = *str++) && len--) {
		hash = ((hash << 5) + hash) + c;
    }
	return (hash & (hash_size -1) );
}

void MyMapStr::init( int _hash_size, int _max_size, const char* _obj_name )
{
    INIT_LIST_HEAD( &link );

    int i = 0;
    for( i = 1; i < 32; ++i ) {
        if( _hash_size <= ( 1 << i ) ) break;
    }
    
    num = 0;
    pow2 = i;
    hash_size = ( 1 << i );
    hash_table = new list_head[ hash_size ];
    for(i = 0; i < hash_size; ++i ) {
        INIT_LIST_HEAD( &hash_table[i] );
    }

    mem = new StrNode[ _max_size ];
    INIT_LIST_HEAD( &free_list );
    for(i = 0; i < _max_size; ++i){
        list_add_tail(&mem[i].link, &free_list);
    }
    max_size = _max_size;
    strncpy( obj_name, _obj_name, MAX_OBJ_NAME );
}


bool MyMapStr::find( const char* key, intptr_t& val )
{
    int len = strlen( key );
    if( len < 1 ) return false;
    int index = str2id( key, len, hash_size ); 
    list_head* pos;
    list_for_each_safe( pos, &(hash_table[index]) ) {
        StrNode* node = list_entry( pos, StrNode, hash );
        if( node->len == len ) {
            if( strncmp( node->key, key, len ) == 0 ) {
                val = node->val;
                return true;
            }
        } 
    }
    return false;
}


bool MyMapStr::set( const char* key, intptr_t val )
{
    StrNode* node = 0;
    int len = strlen( key );
    if( len < 1 ) return false;

    int index = str2id( key, len, hash_size );
    list_head* pos;
    list_for_each_safe( pos, &(hash_table[index]) ) {
        StrNode* node = list_entry( pos, StrNode, hash );
        if( node->len == len ) {
            if( strncmp( node->key, key, len ) == 0 ) {
            node->val = val;
            return true;
            }
        }
    }
     
    if( list_empty(&free_list) ) {
        node = new StrNode;
    } else {
        pos = free_list.next;
        list_del(pos);
        node = list_entry( pos, StrNode, link ); 
    }

    if( node ) {
        node->key = (char*)malloc( len + 1 );
        if( !node->key )
        {
            MY_ERROR( "[MyMapStr](set)malloc error, key: %s", key );
        }
        strncpy( node->key, key, len );
		node->key[len] = '\0';
        node->val = val;
        node->len = len;
        list_add( &(node->hash), &(hash_table[index]) );
        list_add( &(node->link), &link );
        num++;
    }
    return true;
}

bool MyMapStr::del( const char* key )
{
    int len = strlen( key );
    if( len < 1 ) return false;
    int index = str2id( key, len, hash_size );

    list_head* pos;
    list_for_each_safe( pos, &(hash_table[index]) ) {
        StrNode* node = list_entry( pos, StrNode, hash );
        if( node->len == len ) {
            if( strncmp( node->key, key, len ) == 0 ) {
                list_del( &(node->hash) );
                list_del( &(node->link) );
                free( node->key );

                int index = (node - mem) / sizeof(Node);
                if(index >= 0 && index < max_size) {
                    list_add( &(node->link), &free_list );
                } else {
                    delete node;
                }
                num--;
                return true;
            }
        }
    } 
    return false;
}

void MyMapStr::clean()
{
    while(!list_empty(&link)) {
        list_head* pos = link.next;
        list_del(pos);
        StrNode* node = list_entry(pos, StrNode, link);
        list_del(&node->hash);
        list_del(&node->link);
        free( node->key );
        int index = (node - mem) / sizeof(Node);
        if(index >= 0 && index < max_size) {
            list_add( &(node->link), &free_list );
        } else {
            delete node;
        }
    }

    int i;
    for(i = 0; i < hash_size; ++i) {
        INIT_LIST_HEAD(&(hash_table[i]));
    }

    num = 0;
    INIT_LIST_HEAD(&link);
}

MyMapStr::~MyMapStr()
{
    clean();
    if( hash_table ) {
        delete []hash_table;
        hash_table = NULL;
    }

    if( mem ) {
        delete []mem;
        mem = NULL;
    }
}

 
StrNode* MyMapStr::first()
{
    if( !list_empty( &link ) ) {
        StrNode* node = list_entry( link.next, StrNode, link );
        return node;
    }
    return 0;
}

StrNode* MyMapStr::next( StrNode* pre )
{
    list_head* pos = pre->link.next;
    if( pos != &link ) {
        StrNode* node = list_entry( pos, StrNode, link );
        return node;
    }
    return 0;
}

bool MyMapStr::isempty()
{
    return list_empty( &link );
}

int MyMapStr::count()
{
    return num;
}

MyMapStr::MyMapStr()
{
    num = 0;
    hash_table = 0;
    hash_size = 0;
    max_size = 0;
    pow2 = 0;
    mem = 0;
    INIT_LIST_HEAD( &free_list );
    INIT_LIST_HEAD( &link );
}



