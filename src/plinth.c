/**
 * file    plinth.c
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



static pl_none pl_node__init( pl_node_t node )
{
    if ( node ) {
        node->prev = NULL;
        node->next = NULL;
        node->used = 0;
    }
}

static pl_none plam__use_type( plam_t    plam,
                               pl_node_t node,
                               pl_size_t size,
                               pl_aa_t   type,
                               pl_t      ator )
{
    plam->node = node;
    plam->size = size;
    plam->type = type;
    plam->ator = ator;
    pl_node__init( node );
}

static pl_t pl_node__allocate( pl_size_t size, pl_aa_t type, pl_t ator )
{
    pl_node_t node;

    switch ( type ) {
        case PL_AA_SELF:
            node = pl_alloc_memory( size );
            break;
        case PL_AA_HEAP:
            node = pl_alloc_memory( size );
            break;
        case PL_AA_PLAM:
            node = plam_get( (plam_t)ator, size );
            break;
        case PL_AA_PLBM:
            node = plbm_get( (plbm_t)ator );
            break;
        // GCOV_EXCL_START
        default:
            node = NULL;
            break;
            // GCOV_EXCL_STOP
    }

    pl_node__init( node );

    return node;
}

static pl_node_t pl_node__del( pl_node_t node, pl_size_t size, pl_aa_t type, pl_t ator )
{
    if ( node ) {

        pl_node_t left;
        pl_node_t right;
        pl_node_t cur;

        left = node;
        right = node->next;

        switch ( type ) {
            case PL_AA_SELF:
                while ( right ) {
                    cur = right;
                    right = right->next;
                    pl_free_memory( cur );
                }
                while ( left->prev ) {
                    cur = left;
                    left = left->prev;
                    pl_free_memory( cur );
                }
                break;
            case PL_AA_HEAP:
                while ( right ) {
                    cur = right;
                    right = right->next;
                    pl_free_memory( cur );
                }
                while ( left ) {
                    cur = left;
                    left = left->prev;
                    pl_free_memory( cur );
                }
                break;
            case PL_AA_PLAM:
                while ( left->next ) {
                    left = left->next;
                }
                while ( left ) {
                    cur = left;
                    left = left->prev;
                    plam_put( (plam_t)ator, size );
                }
                break;
            case PL_AA_PLBM:
                while ( right ) {
                    cur = right;
                    right = right->next;
                    plbm_put( (plbm_t)ator, cur );
                }
                while ( left ) {
                    cur = left;
                    left = left->prev;
                    plbm_put( (plbm_t)ator, cur );
                }
                break;
            // GCOV_EXCL_START
            default:
                node = NULL;
                break;
                // GCOV_EXCL_STOP
        }
    }

    return NULL;
}



static pl_bool_t plbm__is_valid( pl_size_t nsize, pl_size_t bsize )
{
    if ( ( nsize > sizeof( pl_node_s ) + bsize ) && bsize >= sizeof( pl_t ) ) {
        return pl_true;
    } else {
        return pl_false;
    }
}

static pl_size_t plbm__itail( plbm_t plbm )
{
    return PLINTH_ALIGN_TO( plbm->nsize - sizeof( pl_node_s ), plbm->bsize ) / plbm->bsize;
}

static pl_none plbm__use_type( plbm_t    plbm,
                               pl_node_t node,
                               pl_size_t nsize,
                               pl_size_t bsize,
                               pl_aa_t   type,
                               pl_t      ator )
{
    plbm->node = node;
    plbm->head = NULL;
    plbm->nsize = nsize;
    plbm->bsize = bsize;
    plbm->type = type;
    plbm->ator = ator;
    pl_node__init( node );
    if ( plbm__is_valid( nsize, bsize ) ) {
        plbm->itail = plbm__itail( plbm );
    } else {
        plbm->node = NULL;
        plbm->nsize = nsize;
        plbm->bsize = bsize;
        plbm->itail = 0;
    }
}

static pl_none plbm__invalid( plbm_t plbm )
{
    plbm__use_type( plbm, NULL, 0, 0, PL_AA_SELF, NULL );
}



static pl_none plcm__init( plcm_t plcm )
{
    plcm->size = 0;
    plcm->used = 0;
    plcm->data = NULL;
    plcm->debt = pl_false;
}



static pl_none plss__terminate( plcm_t plcm )
{
    *( (char*)( plcm->data + plcm->used ) ) = 0;
}



/* ------------------------------------------------------------
   -------------------------- PUBLIC --------------------------
   ------------------------------------------------------------ */


pl_none pl_dummy( pl_none ) {} // GCOV_EXCL_LINE


/* ------------------------------------------------------------
 * Universal Interface:
 */

pl_none pl_ui_init( pl_ui_t ui, pl_t env, pl_ui_f fun )
{
    ui->env = env;
    ui->fun = fun;
}


pl_none pl_ui_do( pl_ui_t ui, pl_t argi, pl_t argo )
{
    ui->fun( ui->env, argi, argo );
}




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


plsr_s pl_alloc_plsr( plsr_s plsr )
{
    char* str;
    str = pl_alloc_memory( plsr_length( plsr ) + 1 );
    if ( str ) {
        memcpy( str, plsr_string( plsr ), plsr_length( plsr ) + 1 );
        return plsr_from_string_and_length( str, plsr_length( plsr ) );
    } else {
        return plsr_null(); // GCOV_EXCL_LINE
    }
}


char* pl_alloc_string( const char* str )
{
    if ( str ) {
        plsr_s plsr;
        plsr = pl_alloc_plsr( plsr_from_string( str ) );
        if ( plsr_is_null( plsr ) ) {
            return NULL; // GCOV_EXCL_LINE
        } else {
            return (char*)plsr_string( plsr );
        }
    } else {
        return NULL;
    }
}


char* pl_format_string( const char* fmt, ... )
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

pl_none plam_new( plam_t plam, pl_size_t size )
{
    if ( size > sizeof( pl_node_s ) ) {

        plam->size = size;
        plam->type = PL_AA_HEAP;
        plam->ator = NULL;
        plam->node = pl_node__allocate( plam->size, plam->type, plam->ator );

        if ( plam->node == NULL ) {
            plam->size = 0; // GCOV_EXCL_LINE
        }

    } else {

        plam_empty( plam, 0 );
    }
}


pl_none plam_use( plam_t plam, pl_t node, pl_size_t size )
{
    plam__use_type( plam, node, size, PL_AA_SELF, NULL );
}


pl_none plam_use_plam( plam_t plam, plam_t host, pl_size_t size )
{
    plam__use_type( plam, plam_get( host, size ), size, PL_AA_SELF, NULL );
}


pl_none plam_use_plbm( plam_t plam, plbm_t host )
{
    plam__use_type( plam, plbm_get( host ), plbm_block_size( host ), PL_AA_SELF, NULL );
}


pl_none plam_into_plam( plam_t plam, plam_t host, pl_size_t size )
{
    plam__use_type( plam, plam_get( host, size ), size, PL_AA_PLAM, host );
}


pl_none plam_into_plbm( plam_t plam, plbm_t host )
{
    plam__use_type( plam, plbm_get( host ), plbm_block_size( host ), PL_AA_PLBM, host );
}


pl_none plam_empty( plam_t plam, pl_size_t size )
{
    plam->node = NULL;
    plam->size = size;
    plam->type = PL_AA_SELF;
    plam->ator = NULL;
}


pl_none plam_empty_into_plam( plam_t plam, plam_t host, pl_size_t size )
{
    plam->node = NULL;
    plam->size = size;
    plam->type = PL_AA_PLAM;
    plam->ator = host;
}


pl_none plam_empty_into_plbm( plam_t plam, plbm_t host )
{
    plam->node = NULL;
    plam->size = plbm_block_size( host );
    plam->type = PL_AA_PLBM;
    plam->ator = host;
}


pl_none plam_del( plam_t plam )
{
    plam->node = pl_node__del( plam->node, plam->size, plam->type, plam->ator );
    plam_empty( plam, 0 );
}


pl_t plam_get( plam_t plam, pl_size_t size )
{
    if ( size > plam_node_capacity( plam ) ) {
        /* Too large allocation. */
        return NULL;
    }

    if ( plam->node == NULL ) {
        plam->node = pl_node__allocate( plam->size, plam->type, plam->ator );
        if ( plam->node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
    }

    while ( 1 ) {

        if ( plam_free( plam ) < size ) {

            if ( plam->node->next ) {

                /* Advance to next node and retry. */
                plam->node = plam->node->next;

            } else {

                /* Allocate new node. */
                pl_node_t node;
                node = pl_node__allocate( plam->size, plam->type, plam->ator );
                if ( node == NULL ) {
                    // GCOV_EXCL_START
                    return NULL;
                    // GCOV_EXCL_STOP
                }
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


pl_t plam_store( plam_t plam, const pl_t data, pl_size_t size )
{
    pl_t mem;
    mem = plam_get( plam, size );
    memcpy( mem, data, size );
    return mem;
}


plsr_s plam_store_plsr( plam_t plam, plsr_s plsr )
{
    char* str;
    str = plam_store( plam, (const pl_t)plsr_string( plsr ), plsr_length( plsr ) + 1 );
    return plsr_from_string_and_length( str, plsr_length( plsr ) );
}


char* plam_store_string( plam_t plam, const char* str )
{
    if ( str ) {
        plsr_s plsr;
        plsr = plam_store_plsr( plam, plsr_from_string( str ) );
        return (char*)plsr_string( plsr );
    } else {
        return NULL;
    }
}


char* plam_format_string( plam_t plam, const char* fmt, ... )
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


pl_size_t plam_free( plam_t plam )
{
    if ( plam->node ) {
        return plam_node_capacity( plam ) - plam->node->used;
    } else {
        return 0;
    }
}


pl_size_t plam_size( plam_t plam )
{
    return plam->size;
}


pl_size_t plam_node_capacity( plam_t plam )
{
    return plam->size - sizeof( pl_node_s );
}


pl_bool_t plam_is_empty( plam_t plam )
{
    return ( plam->node == NULL );
}



/* ------------------------------------------------------------
 * Block Memory Allocator:
 */

pl_none plbm_new( plbm_t plbm, pl_size_t nsize, pl_size_t bsize )
{
    if ( plbm__is_valid( nsize, bsize ) ) {
        plbm__use_type(
            plbm, pl_node__allocate( nsize, PL_AA_HEAP, NULL ), nsize, bsize, PL_AA_HEAP, NULL );
    } else {
        plbm__invalid( plbm );
    }
}


pl_none plbm_use( plbm_t plbm, pl_t node, pl_size_t nsize, pl_size_t bsize )
{
    if ( plbm__is_valid( nsize, bsize ) ) {
        plbm__use_type( plbm, node, nsize, bsize, PL_AA_SELF, NULL );
    } else {
        plbm__invalid( plbm );
    }
}


pl_none plbm_use_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize )
{
    plbm__use_type( plbm, plam_get( host, nsize ), nsize, bsize, PL_AA_SELF, NULL );
}


pl_none plbm_use_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize )
{
    plbm__use_type( plbm, plbm_get( host ), plbm_block_size( host ), bsize, PL_AA_SELF, NULL );
}


pl_none plbm_into_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize )
{
    plbm__use_type( plbm, plam_get( host, nsize ), nsize, bsize, PL_AA_PLAM, host );
}


pl_none plbm_into_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize )
{
    plbm__use_type( plbm, plbm_get( host ), plbm_block_size( host ), bsize, PL_AA_PLBM, host );
}


pl_none plbm_empty( plbm_t plbm, pl_size_t nsize, pl_size_t bsize )
{
    plbm__use_type( plbm, NULL, nsize, bsize, PL_AA_SELF, NULL );
}


pl_none plbm_empty_into_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize )
{
    plbm__use_type( plbm, NULL, nsize, bsize, PL_AA_PLAM, host );
}


pl_none plbm_empty_into_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize )
{
    plbm__use_type( plbm, NULL, plbm_block_size( host ), bsize, PL_AA_PLBM, host );
}


pl_none plbm_del( plbm_t plbm )
{
    pl_node__del( plbm->node, plbm->nsize, plbm->type, plbm->ator );
    plbm__invalid( plbm );
}


pl_t plbm_get( plbm_t plbm )
{
    if ( plbm->node == NULL ) {
        plbm->node = pl_node__allocate( plbm->nsize, plbm->type, plbm->ator );
        if ( plbm->node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
    }

    pl_t ret;

    if ( plbm->head ) {
        ret = plbm->head;
        plbm->head = *( (pl_t*)plbm->head );
    } else if ( plbm->itail > 0 ) {
        ret = plbm->node->data + ( ( plbm__itail( plbm ) - plbm->itail ) * plbm->bsize );
        plbm->itail--;
    } else {
        /* Allocate new node. */
        pl_node_t node;
        node = pl_node__allocate( plbm->nsize, plbm->type, plbm->ator );
        if ( node == NULL ) {
            // GCOV_EXCL_START
            return NULL;
            // GCOV_EXCL_STOP
        }
        plbm->node->next = node;
        node->prev = plbm->node;
        plbm->node = node;
        ret = node->data;
        plbm->itail = plbm__itail( plbm );
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


pl_size_t plbm_node_size( plbm_t plbm )
{
    return plbm->nsize;
}


pl_size_t plbm_node_capacity( plbm_t plbm )
{
    return plbm->nsize - sizeof( pl_node_s );
}


pl_size_t plbm_block_size( plbm_t plbm )
{
    return plbm->bsize;
}


pl_bool_t plbm_is_continuous( plbm_t plbm )
{
    return ( plbm->node->prev == NULL && plbm->node->next == NULL );
}


pl_bool_t plbm_is_empty( plbm_t plbm )
{
    return ( plbm->node == NULL );
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
        plcm->debt = pl_true;
    } else {
        // GCOV_EXCL_START
        plcm__init( plcm );
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


pl_none plcm_use_plam( plcm_t plcm, plam_t host, pl_size_t size )
{
    plcm_use( plcm, plam_get( host, size ), size );
}


pl_none plcm_use_plbm( plcm_t plcm, plbm_t host )
{
    plcm_use( plcm, plbm_get( host ), plbm_block_size( host ) );
}


pl_none plcm_empty( plcm_t plcm, pl_size_t size )
{
    plcm__init( plcm );
    plcm->size = size;
}


pl_none plcm_del( plcm_t plcm )
{
    if ( plcm->debt && !plcm_is_empty( plcm ) ) {
        pl_free_memory( plcm->data );
    }
    plcm__init( plcm );
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
                plcm__init( plcm );
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
                plcm__init( plcm );
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


pl_none plcm_put( plcm_t plcm, pl_size_t size )
{
    if ( size <= plcm->used ) {
        plcm->used -= size;
    }
}


pl_pos_t plcm_store( plcm_t plcm, pl_t data, pl_size_t size )
{
    pl_pos_t ret;
    ret = plcm_get_pos( plcm, size );
    plcm_set( plcm, ret, data, size );
    return ret;
}


pl_pos_t plcm_store_ptr( plcm_t plcm, pl_t ptraddr )
{
    return plcm_store( plcm, ptraddr, sizeof( void* ) );
}


pl_none plcm_store_null( plcm_t plcm )
{
    plcm_resize( plcm, plcm->used + sizeof( pl_t ) );
    plcm_terminate_ptr( plcm );
}


pl_t plcm_ref( plcm_t plcm, pl_pos_t pos )
{
    return plcm->data + pos;
}


pl_t plcm_ref_ptr( plcm_t plcm, pl_pos_t pos )
{
    return plcm->data + pos * sizeof( void* );
}


pl_none plcm_set( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size )
{
    memcpy( plcm_ref( plcm, pos ), data, size );
}


pl_none plcm_remove( plcm_t plcm, pl_pos_t pos, pl_size_t size )
{
    memmove( plcm->data + pos, plcm->data + pos + size, plcm->used - pos - size );
    plcm->used -= size;
}


pl_none plcm_insert( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size )
{
    memmove( plcm->data + pos + size, plcm->data + pos, plcm->used - pos );
    plcm->used += size;
    plcm_set( plcm, pos, data, size );
}


pl_bool_t plcm_terminate( plcm_t plcm, pl_size_t size )
{
    if ( plcm->used + size <= plcm->size ) {
        memset( plcm->data + plcm->used, 0, size );
        return pl_true;
    } else {
        return pl_false;
    }
}


pl_bool_t plcm_terminate_ptr( plcm_t plcm )
{
    return plcm_terminate( plcm, sizeof( void* ) );
}


pl_none plcm_reset( plcm_t plcm )
{
    plcm->used = 0;
}


pl_size_t plcm_used( plcm_t plcm )
{
    return plcm->used;
}


pl_size_t plcm_used_ptr( plcm_t plcm )
{
    return ( plcm->used / sizeof( void* ) );
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
 * Unified Memory Allocator:
 */

pl_none plum_use( plum_t plum, pl_aa_t type, pl_t host )
{
    plum->type = type;
    plum->host = host;
}


pl_t plum_get( plum_t plum, pl_size_t size )
{
    switch ( plum->type ) {
        case PL_AA_HEAP: {
            return pl_alloc_memory( size );
        }
        case PL_AA_PLAM: {
            return plam_get( (plam_t)plum->host, size );
        }
        case PL_AA_PLBM: {
            plbm_t plbm;
            plbm = (plbm_t)plum->host;
            if ( size <= plbm_block_size( plbm ) ) {
                return plbm_get( plbm );
            } else {
                return NULL;
            }
        }
        case PL_AA_PLCM: {
            return plcm_get_ref( (plcm_t)plum->host, size );
        }
        // GCOV_EXCL_START
        default:
            return NULL;
            // GCOV_EXCL_STOP
    }
}


pl_t plum_put( plum_t plum, pl_t mem, pl_size_t size )
{
    switch ( plum->type ) {

        case PL_AA_HEAP: {
            pl_free_memory( mem );
            return NULL;
        }

        case PL_AA_PLAM: {

            plam_t    plam;
            pl_size_t used_ref;
            pl_t      mem_ref;

            plam = (plam_t)plum->host;

            while ( 1 ) {
                if ( plam->node->used > 0 ) {
                    used_ref = plam->node->used - size;
                    mem_ref = plam->node->data + used_ref;
                    if ( mem == mem_ref ) {
                        plam->node->used -= size;
                        return mem;
                    } else {
                        return NULL;
                    }
                } else if ( plam->node->prev ) {
                    plam->node = plam->node->prev;
                } else {
                    return NULL;
                }
            }
        }

        case PL_AA_PLBM: {
            plbm_put( (plbm_t)plum->host, mem );
            return mem;
        }

        case PL_AA_PLCM: {

            plcm_t    plcm;
            pl_size_t used_ref;
            pl_t      mem_ref;

            plcm = (plcm_t)plum->host;

            used_ref = plcm->used - size;
            mem_ref = plcm->data + used_ref;

            if ( mem == mem_ref ) {
                plcm_put( plcm, size );
                return mem;
            } else {
                return NULL;
            }
        }
        // GCOV_EXCL_START
        default:
            return NULL;
            // GCOV_EXCL_STOP
    }
}


pl_t plum_update( plum_t plum, pl_t mem, pl_size_t osize, pl_size_t nsize )
{
    switch ( plum->type ) {

        case PL_AA_HEAP: {
            return pl_realloc_memory( mem, nsize );
        }

        case PL_AA_PLAM:
        case PL_AA_PLBM:
        case PL_AA_PLCM: {

            pl_t      omem;
            pl_t      nmem;
            pl_size_t size;

            omem = plum_put( plum, mem, osize );
            nmem = plum_get( plum, nsize );

            if ( omem != nmem ) {
                if ( nsize > osize ) {
                    size = osize;
                } else {
                    size = nsize;
                }
                memcpy( nmem, mem, size );
            }

            return nmem;
        }

        // GCOV_EXCL_START
        default:
            return NULL;
            // GCOV_EXCL_STOP
    }
}


pl_aa_t plum_type( plum_t plum )
{
    return plum->type;
}


pl_t plum_host( plum_t plum )
{
    return plum->host;
}


/* ------------------------------------------------------------
 * String Storage:
 */

plcm_t plss_append( plcm_t plcm, plsr_s str )
{
    plcm_resize( plcm, plcm->used + str.length + 1 );
    memcpy( plcm->data + plcm->used, str.string, str.length );
    plcm->used += str.length;
    plss__terminate( plcm );
    return plcm;
}


plcm_t plss_append_string( plcm_t plcm, const char* str )
{
    return plss_append( plcm, plsr_from_string( str ) );
}


plcm_t plss_append_char( plcm_t plcm, char ch )
{
    char ch_null[ 2 ];
    ch_null[ 0 ] = ch;
    ch_null[ 1 ] = 0;
    return plss_append( plcm, plsr_from_string_and_length( ch_null, 1 ) );
}


pl_none plss_remove( plcm_t plcm, pl_pos_t pos, pl_size_t size )
{
    memmove( plcm->data + pos, plcm->data + pos + size, plcm->used - pos - size + 1 );
    plcm->used -= size;
}


pl_none plss_insert( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size )
{
    if ( pos >= plcm->used ) {
        plss_append( plcm, plsr_from_string_and_length( data, size ) );
    } else {
        memmove( plcm->data + pos + size, plcm->data + pos, plcm->used - pos + 1 );
        plcm->used += size;
        plcm_set( plcm, pos, data, size );
    }
}


plcm_t plss_set( plcm_t plcm, plsr_s str )
{
    plcm_resize( plcm, str.length + 1 );
    memcpy( plcm->data, str.string, str.length );
    plcm->used = str.length;
    plss__terminate( plcm );
    return plcm;
}


plcm_t plss_format_string( plcm_t plcm, const char* fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );
    plss_va_format_string( plcm, fmt, ap );
    va_end( ap );

    return plcm;
}


plcm_t plss_reformat_string( plcm_t plcm, const char* fmt, ... )
{
    va_list ap;

    plcm_reset( plcm );
    va_start( ap, fmt );
    plss_va_format_string( plcm, fmt, ap );
    va_end( ap );

    return plcm;
}


pl_none plss_va_format_string( plcm_t plcm, const char* fmt, va_list ap )
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


pl_bool_t plss_is_empty( plcm_t plcm )
{
    return ( plcm_used( plcm ) == 0 );
}



/* ------------------------------------------------------------
 * String Referencing:
 */

plsr_s plsr_from_string( const char* str )
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


plsr_s plsr_from_string_and_length( const char* str, pl_size_t length )
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


plsr_s plsr_null( pl_none )
{
    plsr_s ret;
    ret.string = NULL;
    ret.length = 0;
    return ret;
}


pl_bool_t plsr_is_null( plsr_s plsr )
{
    return ( plsr.string == NULL );
}
