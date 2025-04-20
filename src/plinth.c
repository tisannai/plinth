/**
 * @file   plinth.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Apr 19 10:45:54 EEST 2025
 *
 * @brief  Plinth - Arena style memory allocator.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "plinth.h"


/* ------------------------------------------------------------
 * Low level memory allocation:
 */

pl_t pl_alloc_memory( pl_size_t size )
{
    return calloc( 1, (size_t) size );
}

pl_none pl_free_memory( pl_t mem )
{
    free( (void*) mem );
}

pl_t pl_realloc_memory( pl_t mem, pl_size_t size )
{
    return realloc( (void*) mem, (size_t) size );
}

char* pl_strdup( const char* str )
{
    if ( str ) {
        return strdup( str );
    } else {
        return NULL;
    }
}

char* pl_format( const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    pl_pos_t size;
    va_list coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );
    size = vsnprintf( NULL, 0, fmt, coap );
    va_end( coap );

    if ( size < 0 ) {
        va_end( ap );
        return NULL; // GCOV_EXCL_LINE
    }

    char* mem;

    mem = pl_alloc_memory( size+1 );
    vsnprintf( mem, size+1, fmt, ap );
    va_end( ap );

    return mem;
}



/* ------------------------------------------------------------
 * Segmented Memory Allocator:
 */

static pl_none plsm_node_init( plsm_node_t node, pl_bool_t heap )
{
    node->prev = NULL;
    node->next = NULL;
    node->used = 0;
    node->heap = heap;
}


pl_none plsm_new( plsm_t plsm, pl_size_t size )
{
    plsm->node = pl_alloc_memory( size );
    if ( plsm->node ) {
        plsm_node_init( plsm->node, pl_true );
        plsm->size = size;
    } else {
        plsm->size = 0;
    }
}


pl_none plsm_use( plsm_t plsm, pl_t node, pl_size_t size )
{
    plsm->node = node;
    plsm_node_init( node, pl_false );
}


pl_none plsm_empty( plsm_t plsm, pl_size_t size )
{
    plsm->node = NULL;
    plsm->size = size;
}


pl_none plsm_del( plsm_t plsm )
{
    plsm_node_t left;
    plsm_node_t right;
    plsm_node_t cur;
    
    if ( plsm->node ) {

        left = plsm->node->prev;
        right = plsm->node;

        while ( right ) {
            cur = right;
            right = right->next;
            if ( cur->heap ) {
                pl_free_memory( cur );
            }
        }

        while ( left ) {
            cur = left;
            left = left->prev;
            if ( cur->heap ) {
                pl_free_memory( cur );
            }
        }

    }
}


pl_t plsm_get( plsm_t plsm, pl_size_t size )
{
    if ( size > plsm->size - sizeof( plsm_node_s ) ) {
        /* Too large allocation. */
        return NULL;
    }

    if ( plsm->node == NULL ) {
        plsm->node = pl_alloc_memory( plsm->size );
        plsm_node_init( plsm->node, pl_true );
    }

    pl_size_t free;
    free = plsm->size - sizeof( plsm_node_s ) - plsm->node->used;
    if ( free < size ) {
        /* Allocate new node. */
        plsm_node_t node;
        node = pl_alloc_memory( plsm->size );
        plsm_node_init( node, pl_true );
        plsm->node->next = node;
        node->prev = plsm->node;
        plsm->node = node;
    }

    pl_t ret;
    ret = plsm->node->data + plsm->node->used;
    plsm->node->used += size;
    return ret;
}


char* plsm_strdup( plsm_t plsm, char* str )
{
    if ( str ) {
        char* dup;
        pl_size_t length;
        length = strlen( str ) + 1;
        dup = plsm_get( plsm, length );
        strncpy( dup, str, length );
        return dup;
    } else {
        return NULL;
    }
}


char* plsm_format( plsm_t plsm, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    pl_pos_t size;
    va_list coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );
    size = vsnprintf( NULL, 0, fmt, coap );
    va_end( coap );

    if ( size < 0 ) {
        va_end( ap );
        return NULL; // GCOV_EXCL_LINE
    }

    char* mem;

    mem = plsm_get( plsm, size + 1 );
    vsnprintf( mem, size+1, fmt, ap );
    va_end( ap );

    return mem;
}


pl_bool_t plsm_is_empty( plsm_t plsm )
{
    return ( plsm->node == NULL );
}



/* ------------------------------------------------------------
 * Continuous Memory Allocator:
 */

pl_none plcm_new( plcm_t plcm, pl_size_t size )
{
    pl_t mem;

    mem = pl_alloc_memory( size );
    if ( mem ) {
        plcm->size = size;
        plcm->used = 0;
        plcm->data = mem;
        plcm->heap = pl_true;
    } else {
        plcm->size = 0;
        plcm->used = 0;
        plcm->data = NULL;
        plcm->heap = pl_false;
    }
}


pl_none plcm_use( plcm_t plcm, pl_t mem, pl_size_t size )
{
    plcm->size = size;
    plcm->used = 0;
    plcm->data = mem;
    plcm->heap = pl_false;
}


pl_none plcm_use_plsm( plcm_t plcm, plsm_t plsm, pl_size_t size )
{
    plcm_use( plcm, plsm_get( plsm, size ), size );
}


pl_none plcm_empty( plcm_t plcm, pl_size_t first_size )
{
    plcm->size = 0;
    plcm->used = first_size;
    plcm->data = NULL;
    plcm->heap = pl_false;
}


pl_none plcm_del( plcm_t plcm )
{
    if ( plcm->heap && !plcm_is_empty( plcm ) ) {
        pl_free_memory( plcm->data );
    }
}


pl_none plcm_resize( plcm_t plcm, pl_size_t size )
{
    if ( size > plcm->size ) {

        pl_size_t new_size;

        if ( !plcm->heap ) {

            pl_t new_mem;

            if ( size > plcm->size * 2 ) {
                new_size = PLINTH_ALIGN_TO( size, plcm->size );
            } else {
                new_size = 2 * plcm->size;
            }

            new_mem = pl_alloc_memory( new_size );
            memcpy( new_mem, plcm->data, plcm->size );
            plcm->data = new_mem;
            plcm->size = new_size;
            plcm->heap = pl_true;

        } else if ( plcm_is_empty( plcm ) ) {

            /* Empty plcm. */
            if ( plcm->used == 0 || size > plcm->used ) {
                new_size = PLINTH_ALIGN_TO( size, 64 );
            } else {
                new_size = plcm->used;
            }

            plcm->data = pl_alloc_memory( new_size );
            plcm->size = new_size;
            plcm->used = 0;
            plcm->heap = pl_true;

        } else {

            /*
              By default, we double the allocation, but if that is not
              enough, we resize to the next sufficient multiple of
              plcm->size.
             */
            if ( size > plcm->size * 2 ) {
                new_size = PLINTH_ALIGN_TO( size, plcm->size );
            } else {
                new_size = 2 * plcm->size;
            }

            plcm->data = pl_realloc_memory( plcm->data, new_size );
            memset( plcm->data + plcm->size, 0, ( size - plcm->size ) );
            plcm->size = new_size;

        }
    }
}


pl_pos_t plcm_get_pos( plcm_t plcm, pl_size_t size )
{
    pl_pos_t ret;
    plcm_resize( plcm, plcm->used + size );
    ret = plcm->used;
    plcm->used += size;
    return ret;
}


pl_t plcm_get_ref( plcm_t plcm, pl_size_t size )
{
    pl_t ret;
    plcm_resize( plcm, plcm->used + size );
    ret = plcm_ref( plcm, plcm->used );
    plcm->used += size;
    return ret;
}


pl_pos_t plcm_store( plcm_t plcm, pl_t data, pl_size_t size )
{
    pl_pos_t ret;
    ret = plcm_get_pos( plcm, size );
    plcm_set( plcm, ret, data, size );
    return ret;
}


pl_t plcm_ref( plcm_t plcm, pl_pos_t pos )
{
    return plcm->data + pos;
}


pl_none plcm_set( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size )
{
    memcpy( plcm_ref( plcm, pos ), data, size );
}


pl_none plcm_reset( plcm_t plcm )
{
    plcm->used = 0;
}


pl_size_t plcm_used( plcm_t plcm )
{
    return plcm->used;
}


pl_size_t plcm_size( plcm_t plcm )
{
    return plcm->size;
}


pl_t plcm_data( plcm_t plcm )
{
    return plcm->data;
}


pl_t plcm_end( plcm_t plcm )
{
    return plcm->data + plcm->used;
}


pl_bool_t plcm_is_empty( plcm_t plcm )
{
    return ( plcm->data == NULL && plcm->size == 0 );
}



/* ------------------------------------------------------------
 * String Storage:
 */

static pl_none plss_terminate( plcm_t plcm )
{
    *((char*)(plcm->data + plcm->used)) = 0;
}


// static pl_none plss_setup( plcm_t plcm )
// {
//     if ( plcm->used == 0 ) {
//         plcm->used = 1;
//         plss_terminate( plcm );
//     }
// }

// pl_none plcm_new( plcm_t plcm, pl_size_t size )

// pl_none plss_new( plcm_t plcm, pl_size_t size )
// {
//     char ch;
//     ch = 0;
//     plcm_new( plcm, size );
//     plcm_store( plcm, &ch, 1 );
// }



plcm_t plss_append( plcm_t plcm, plsr_s str )
{
    plcm_resize( plcm, plcm->used + str.length + 1 );
    memcpy( plcm->data + plcm->used, str.string, str.length );
    plcm->used += str.length;
    plss_terminate( plcm );
    return plcm;
}

plcm_t plss_append_c( plcm_t plcm, char* str )
{
    pl_size_t length;
    length = strlen( str );
    plcm_resize( plcm, plcm->used + length + 1 );
    memcpy( plcm->data + plcm->used, str, length );
    plcm->used += length;
    plss_terminate( plcm );
    return plcm;
}

plcm_t plss_append_ch( plcm_t plcm, char ch )
{
    char* ref;
    
    plcm_resize( plcm, plcm->used + 2 );
    ref = plcm->data + plcm->used;
    ref[ 0 ] = ch;
    ref[ 1 ] = 0;
    plcm->used += 1;
    return plcm;
}


plcm_t plss_set( plcm_t plcm, plsr_s str )
{
    plcm_resize( plcm, str.length+1 );
    memcpy( plcm->data, str.string, str.length );
    plcm->used = str.length;
    plss_terminate( plcm );
    return plcm;
}


plcm_t plss_format( plcm_t plcm, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    plss_va_format( plcm, fmt, ap );
    va_end( ap );

    return plcm;
}


plcm_t plss_reformat( plcm_t plcm, const char* fmt, ... )
{
    va_list ap;

    plcm_reset( plcm );
    va_start( ap, fmt );
    plss_va_format( plcm, fmt, ap );
    va_end( ap );

    return plcm;
}


pl_none plss_va_format( plcm_t plcm, const char* fmt, va_list ap )
{
    va_list coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );

    pl_pos_t size;
    size = vsnprintf( NULL, 0, fmt, ap );

    if ( size < 0 ) {
        return; // GCOV_EXCL_LINE
    }

    plcm_resize( plcm, plcm->used + size + 1 );
    size = vsnprintf( plcm->data + plcm->used, size+1, fmt, coap );
    va_end( coap );

    plcm->used += size;
}


pl_size_t plss_length( plcm_t plcm )
{
    return plcm_used( plcm );
}


const pl_str_t plss_string( plcm_t plcm )
{
    return (const pl_str_t) plcm_ref( plcm, 0 );
}


plsr_s plss_ref( plcm_t plcm )
{
    plsr_s ret;
    //ret.str_m = (pl_str_t) plss_string( plcm );
    ret.string = (pl_str_t) plss_string( plcm );
    ret.length = plss_length( plcm );
    return ret;
}


/* ------------------------------------------------------------
 * String Referencing:
 */

plsr_s plsr_from_c( const char* c_string )
{
    plsr_s ret;
    ret.string = c_string;
    if ( c_string ) {
        ret.length = strlen( c_string );
    } else {
        ret.length = 0;
    }
    return ret;
}

plsr_s plsr_from_c_length( const char* c_string, pl_size_t length )
{
    plsr_s ret;
    ret.string = c_string;
    ret.length = length;
    return ret;
}


plsr_s plsr_duplicate( plsr_s plsr )
{
    if ( plsr_is_invalid( plsr ) ) {
        return plsr;
    } else {
        plsr_s ret;
        ret.str_m = pl_alloc_memory( plsr.length + 1 );
        memcpy( ret.str_m, plsr.string, plsr.length + 1 );
        ret.length = plsr.length;
        return ret;
    }
}


pl_bool_t plsr_compare( plsr_s p1, plsr_s p2 )
{
    if ( p1.length != p2.length ) {
        return pl_false;
    } else {
        if ( !strncmp( p1.string, p2.string, p1.length ) ) {
            return pl_true;
        } else {
            return pl_false;
        }
    }
}


pl_bool_t plsr_compare_n( plsr_s p1, plsr_s p2, pl_size_t n )
{
    if ( ( p1.length < n ) || ( p2.length < n ) ) {
        return pl_false;
    } else {
        if ( !strncmp( p1.string, p2.string, n ) ) {
            return pl_true;
        } else {
            return pl_false;
        }
    }
}


plsr_s plsr_invalid( pl_none )
{
    plsr_s ret;
    ret.string = NULL;
    ret.length = 0;
    return ret;
}


pl_bool_t plsr_is_invalid( plsr_s plsr )
{
    return ( plsr.string == NULL );
}


pl_bool_t plsr_is_valid( plsr_s plsr )
{
    return ( plsr.string != NULL );
}
