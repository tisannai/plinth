/**
 * @file   plinth.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Apr 19 10:45:54 EEST 2025
 *
 * @brief  Plinth - Base layer library.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "plinth.h"


/* ------------------------------------------------------------
 * Basic (heap) memory allocation:
 */

pl_t pl_alloc_memory( pl_size_t size )
{
    return calloc( 1, (size_t)size );
}

pl_none pl_free_memory( pl_t mem )
{
    free( (void*)mem );
}

pl_t pl_realloc_memory( pl_t mem, pl_size_t size )
{
    return realloc( (void*)mem, (size_t)size );
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
    va_list  coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );
    size = vsnprintf( NULL, 0, fmt, coap );
    va_end( coap );

    if ( size < 0 ) {
        // GCOV_EXCL_START
        va_end( ap );
        return NULL;
        // GCOV_EXCL_STOP
    }

    char* mem;

    mem = pl_alloc_memory( size + 1 );
    if ( mem == NULL ) {
        // GCOV_EXCL_START
        return NULL;
        // GCOV_EXCL_STOP
    }
    vsnprintf( mem, size + 1, fmt, ap );
    va_end( ap );

    return mem;
}



/* ------------------------------------------------------------
 * Arena Memory Allocator:
 */

static pl_none plam_node_init( plam_node_t node, pl_bool_t debt )
{
    node->prev = NULL;
    node->next = NULL;
    node->used = 0;
    node->debt = debt;
}


static plam_node_t plam_node_del( plam_node_t node )
{
    if ( node ) {

        plam_node_t left;
        plam_node_t right;
        plam_node_t cur;

        left = node->prev;
        right = node;

        while ( right ) {
            cur = right;
            right = right->next;
            if ( cur->debt ) {
                pl_free_memory( cur );
            }
        }

        while ( left ) {
            cur = left;
            left = left->prev;
            if ( cur->debt ) {
                pl_free_memory( cur );
            }
        }
    }

    return NULL;
}


pl_none plam_new( plam_t plam, pl_size_t size )
{
    if ( size > sizeof( plam_node_s ) ) {
        plam->node = pl_alloc_memory( size );
        if ( plam->node ) {
            plam_node_init( plam->node, pl_true );
            plam->size = size;
        } else {
            plam->size = 0; // GCOV_EXCL_LINE
        }
    } else {
        plam->node = NULL;
        plam->size = 0;
    }
}


pl_none plam_use( plam_t plam, pl_t node, pl_size_t size )
{
    plam->node = node;
    plam->size = size;
    plam_node_init( node, pl_false );
}


pl_none plam_use_plam( plam_t plam, plam_t base, pl_size_t size )
{
    plam_use( plam, plam_get( base, size ), size );
}


pl_none plam_empty( plam_t plam, pl_size_t size )
{
    plam->node = NULL;
    plam->size = size;
}


pl_none plam_del( plam_t plam )
{
    plam->node = plam_node_del( plam->node );
    plam->size = 0;
}


pl_t plam_get( plam_t plam, pl_size_t size )
{
    if ( size > plam->size - sizeof( plam_node_s ) ) {
        /* Too large allocation. */
        return NULL;
    }

    if ( plam->node == NULL ) {
        plam->node = pl_alloc_memory( plam->size );
        if ( plam->node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
        plam_node_init( plam->node, pl_true );
    }

    pl_size_t free;

    while ( 1 ) {

        free = plam->size - sizeof( plam_node_s ) - plam->node->used;

        if ( free < size ) {

            if ( plam->node->next ) {

                plam->node = plam->node->next;

            } else {

                /* Allocate new node. */
                plam_node_t node;
                node = pl_alloc_memory( plam->size );
                if ( node == NULL ) {
                    // GCOV_EXCL_START
                    return NULL;
                    // GCOV_EXCL_STOP
                }
                plam_node_init( node, pl_true );
                plam->node->next = node;
                node->prev = plam->node;
                plam->node = node;
                break;
            }
        } else {
            break;
        }
    }

    pl_t ret;
    ret = plam->node->data + plam->node->used;
    // ret = PLINTH_ADDR_ADD( plam->node->data, plam->node->used );
    plam->node->used += size;
    return ret;
}


pl_none plam_put( plam_t plam, pl_size_t size )
{
    while ( 1 ) {
        if ( plam->node->used > 0 ) {
            plam->node->used -= size;
            break;
        } else if ( plam->node->prev ) {
            plam->node = plam->node->prev;
        } else {
            break;
        }
    }
}


char* plam_strdup( plam_t plam, const char* str )
{
    if ( str ) {
        char*     dup;
        pl_size_t length;
        length = strlen( str ) + 1;
        dup = plam_get( plam, length );
        strncpy( dup, str, length );
        return dup;
    } else {
        return NULL;
    }
}


char* plam_format( plam_t plam, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    pl_pos_t size;
    va_list  coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );
    size = vsnprintf( NULL, 0, fmt, coap );
    va_end( coap );

    if ( size < 0 ) {
        // GCOV_EXCL_START
        va_end( ap );
        return NULL;
        // GCOV_EXCL_STOP
    }

    char* mem;

    mem = plam_get( plam, size + 1 );
    vsnprintf( mem, size + 1, fmt, ap );
    va_end( ap );

    return mem;
}


pl_size_t plam_used( plam_t plam )
{
    if ( plam->node ) {
        return plam->node->used;
    } else {
        return 0;
    }
}


pl_size_t plam_size( plam_t plam )
{
    return plam->size;
}


pl_bool_t plam_is_empty( plam_t plam )
{
    return ( plam->node == NULL );
}



/* ------------------------------------------------------------
 * Block Memory Allocator:
 */

static pl_bool_t plbm_is_valid( pl_size_t nsize, pl_size_t bsize )
{
    if ( ( nsize > sizeof( plam_node_s ) + bsize ) && bsize >= sizeof( pl_t ) ) {
        return pl_true;
    } else {
        return pl_false;
    }
}

static pl_none plbm_invalid( plbm_t plbm )
{
    plbm->node = NULL;
    plbm->head = NULL;
    plbm->nsize = 0;
    plbm->bsize = 0;
    plbm->itail = 0;
}

static pl_size_t plbm_itail( plbm_t plbm )
{
    return PLINTH_ALIGN_TO( plbm->nsize - sizeof( plam_node_s ), plbm->bsize ) / plbm->bsize;
}


pl_none plbm_new( plbm_t plbm, pl_size_t nsize, pl_size_t bsize )
{
    if ( plbm_is_valid( nsize, bsize ) ) {
        plbm->node = pl_alloc_memory( nsize );
        if ( plbm->node ) {
            plam_node_init( plbm->node, pl_true );
            plbm->head = NULL;
            plbm->nsize = nsize;
            plbm->bsize = bsize;
            plbm->itail = plbm_itail( plbm );
        } else {
            // GCOV_EXCL_START
            plbm_invalid( plbm );
            // GCOV_EXCL_STOP
        }
    } else {
        plbm_invalid( plbm );
    }
}


pl_none plbm_use( plbm_t plbm, pl_t node, pl_size_t nsize, pl_size_t bsize )
{
    if ( plbm_is_valid( nsize, bsize ) ) {
        plam_node_init( node, pl_false );
        plbm->node = node;
        plbm->head = NULL;
        plbm->nsize = nsize;
        plbm->bsize = bsize;
        plbm->itail = plbm_itail( plbm );
    } else {
        plbm_invalid( plbm );
    }
}


pl_none plbm_use_plam( plbm_t plbm, plam_t base, pl_size_t nsize, pl_size_t bsize )
{
    plbm_use( plbm, plam_get( base, nsize ), nsize, bsize );
}


pl_none plbm_empty( plbm_t plbm, pl_size_t nsize, pl_size_t bsize )
{
    if ( plbm_is_valid( nsize, bsize ) ) {
        plbm->node = NULL;
        plbm->head = NULL;
        plbm->nsize = nsize;
        plbm->bsize = bsize;
        plbm->itail = plbm_itail( plbm );
    } else {
        plbm_invalid( plbm );
    }
}


pl_none plbm_del( plbm_t plbm )
{
    plam_node_del( plbm->node );
    plbm_invalid( plbm );
}


pl_t plbm_get( plbm_t plbm )
{
    if ( plbm->node == NULL ) {
        plbm->node = pl_alloc_memory( plbm->nsize );
        if ( plbm->node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
        plam_node_init( plbm->node, pl_true );
    }

    pl_t ret;

    if ( plbm->head ) {
        ret = plbm->head;
        plbm->head = *( (pl_t*)plbm->head );
    } else if ( plbm->itail > 0 ) {
        ret = plbm->node->data + ( ( plbm_itail( plbm ) - plbm->itail ) * plbm->bsize );
        // ret = PLINTH_ADDR_ADD( plbm->node->data, ( ( plbm_itail( plbm ) - plbm->itail ) * plbm->bsize ) );
        plbm->itail--;
    } else {
        /* Allocate new node. */
        plam_node_t node;
        node = pl_alloc_memory( plbm->nsize );
        if ( node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
        plam_node_init( node, pl_true );
        plbm->node->next = node;
        node->prev = plbm->node;
        plbm->node = node;
        ret = node->data;
        plbm->itail = plbm_itail( plbm );
        plbm->itail--;
    }

    return ret;
}


pl_none plbm_put( plbm_t plbm, pl_t block )
{
    if ( plbm->head ) {
        pl_t prev;
        prev = plbm->head;
        plbm->head = block;
        *( (pl_t*)block ) = prev;
    } else {
        plbm->head = block;
        *( (pl_t*)block ) = NULL;
    }
}


pl_size_t plbm_nsize( plbm_t plbm )
{
    return plbm->nsize;
}


pl_size_t plbm_bsize( plbm_t plbm )
{
    return plbm->bsize;
}


pl_bool_t plbm_is_empty( plbm_t plbm )
{
    return ( plbm->node == NULL );
}



/* ------------------------------------------------------------
 * Continuous Memory Allocator:
 */

static pl_none plcm_init( plcm_t plcm )
{
    plcm->size = 0;
    plcm->used = 0;
    plcm->data = NULL;
    plcm->debt = pl_false;
}

pl_none plcm_new( plcm_t plcm, pl_size_t size )
{
    pl_t mem;

    mem = pl_alloc_memory( size );
    if ( mem ) {
        plcm->size = size;
        plcm->used = 0;
        plcm->data = mem;
        plcm->debt = pl_true;
    } else {
        // GCOV_EXCL_START
        plcm_init( plcm );
        // GCOV_EXCL_STOP
    }
}


pl_none plcm_use( plcm_t plcm, pl_t mem, pl_size_t size )
{
    plcm->size = size;
    plcm->used = 0;
    plcm->data = mem;
    plcm->debt = pl_false;
}


pl_none plcm_use_plam( plcm_t plcm, plam_t plam, pl_size_t size )
{
    plcm_use( plcm, plam_get( plam, size ), size );
}


pl_none plcm_empty( plcm_t plcm, pl_size_t size )
{
    plcm_init( plcm );
    plcm->size = size;
}


pl_none plcm_del( plcm_t plcm )
{
    if ( plcm->debt && !plcm_is_empty( plcm ) ) {
        pl_free_memory( plcm->data );
    }
    plcm_init( plcm );
}


pl_none plcm_resize( plcm_t plcm, pl_size_t size )
{
    if ( size > plcm->size || plcm_is_empty( plcm ) ) {

        pl_size_t new_size;

        if ( plcm_is_empty( plcm ) ) {

            /* Empty plcm. */
            if ( plcm->size == 0 || size > plcm->size ) {
                new_size = PLINTH_ALIGN_TO( size, 64 );
            } else {
                new_size = plcm->size;
            }

            plcm->data = pl_alloc_memory( new_size );
            if ( plcm->data ) {
                plcm->size = new_size;
                plcm->used = 0;
                plcm->debt = pl_true;
            } else {
                // GCOV_EXCL_START
                plcm_init( plcm );
                // GCOV_EXCL_STOP
            }

        } else if ( !plcm->debt ) {

            pl_t new_mem;

            if ( size > plcm->size * 2 ) {
                new_size = PLINTH_ALIGN_TO( size, plcm->size );
            } else {
                new_size = 2 * plcm->size;
            }

            new_mem = pl_alloc_memory( new_size );
            if ( new_mem ) {
                memcpy( new_mem, plcm->data, plcm->size );
                plcm->data = new_mem;
                plcm->size = new_size;
                plcm->debt = pl_true;
            } else {
                // GCOV_EXCL_START
                plcm_init( plcm );
                // GCOV_EXCL_STOP
            }

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


pl_none plcm_terminate( plcm_t plcm, pl_size_t size )
{
    memset( plcm->data + plcm->used, 0, size );
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


pl_bool_t plcm_debt( plcm_t plcm )
{
    return plcm->debt;
}


pl_t plcm_end( plcm_t plcm )
{
    return plcm->data + plcm->used;
}


pl_bool_t plcm_is_empty( plcm_t plcm )
{
    return ( plcm->data == NULL );
}



/* ------------------------------------------------------------
 * String Storage:
 */

static pl_none plss_terminate( plcm_t plcm )
{
    *( (char*)( plcm->data + plcm->used ) ) = 0;
}


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
    return plss_append( plcm, plsr_from_c( str ) );
}


plcm_t plss_append_ch( plcm_t plcm, char ch )
{
    char ch_null[ 2 ];
    ch_null[ 0 ] = ch;
    ch_null[ 1 ] = 0;
    return plss_append( plcm, plsr_from_c_length( ch_null, 1 ) );
}


plcm_t plss_set( plcm_t plcm, plsr_s str )
{
    plcm_resize( plcm, str.length + 1 );
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
    size = vsnprintf( plcm->data + plcm->used, size + 1, fmt, coap );
    va_end( coap );

    plcm->used += size;
}


const char* plss_string( plcm_t plcm )
{
    return (const char*)plcm_ref( plcm, 0 );
}


pl_size_t plss_length( plcm_t plcm )
{
    return plcm_used( plcm );
}


plsr_s plss_ref( plcm_t plcm )
{
    plsr_s ret;
    ret.string = (char*)plss_string( plcm );
    ret.length = plss_length( plcm );
    return ret;
}


/* ------------------------------------------------------------
 * String Referencing:
 */

plsr_s plsr_from_c( const char* str )
{
    plsr_s ret;
    ret.string = str;
    if ( str ) {
        ret.length = strlen( str );
    } else {
        ret.length = 0;
    }
    return ret;
}

plsr_s plsr_from_c_length( const char* str, pl_size_t length )
{
    plsr_s ret;
    ret.string = str;
    ret.length = length;
    return ret;
}


const char* plsr_string( plsr_s sr )
{
    return sr.string;
}


pl_size_t plsr_length( plsr_s sr )
{
    return sr.length;
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
