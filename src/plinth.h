#ifndef PLINTH_H
#define PLINTH_H

/**
 * @file   plinth.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Apr 19 10:45:54 EEST 2025
 *
 * @brief  Plinth - Arena style memory allocator.
 *
 */

#include <stdint.h>
#include <stdarg.h>


/* ------------------------------------------------------------
 * Simple type definition features:
 */

/** Re-define type and related pointers. */
#define pl_type( base, type )   \
    typedef base      type##_t; \
    typedef type##_t* type##_p;



/* ------------------------------------------------------------
 * Structure type definition.
 */

/**
 * Define struct and related typedefs for struct type ("_s"), pointer
 * type ("_t"), and reference type ("_p").
 *
 * Example:
 * @code
 *     Definition:
 *         pl_struct(point) {
 *             pl_int x;
 *             pl_int y;
 *         };
 *
 *     Becomes:
 *         typedef struct point_struct_s point_s;
 *         typedef point_s* point_t;
 *         typedef point_s** point_p;
 *         struct point_struct_s {
 *             pl_int x;
 *             pl_int y;
 *         };
 * @endcode
 */
#define pl_struct( name )                   \
    typedef struct name##struct_s name##_s; \
    typedef name##_s*             name##_t; \
    typedef name##_s**            name##_p; \
    struct name##struct_s

#define pl_struct_type( name )              \
    typedef struct name##struct_s name##_s; \
    typedef name##_s*             name##_t; \
    typedef name##_s**            name##_p;

#define pl_struct_body( name ) struct name##struct_s


/**
 * Define enumeration and corresponding type. An enum is created with
 * name+"_e", a typedef is created with name+"_t".
 *
 * Example:
 * @code
 *     Definition:
 *         pl_enum(some_runtype) {
 *           RUN_NONE,
 *           RUN_TASK,
 *           RUN_FUNC
 *         };
 *
 *     Becomes:
 *         typedef enum pl_runtype_e pl_runtype_t;
 *         enum pl_runtype_e {
 *           RUN_NONE,
 *           RUN_TASK,
 *           RUN_FUNC
 *         };
 * @endcode
 */
#define pl_enum( name )             \
    typedef enum name##_e name##_t; \
    enum name##_e


/* ------------------------------------------------------------
 * Basic data types.
 */

/** Boolean type. */
pl_enum( pl_bool ){ pl_false = 0, pl_true = 1 };

/** Void type. */
typedef void pl_none;

/** Generic pointer type. */
typedef void* pl_t;

/** Generic pointer type reference. */
typedef void** pl_p;


pl_type( int8_t, pl_i8 );    /**< Character type. */
pl_type( uint8_t, pl_u8 );   /**< Character type. */
pl_type( int64_t, pl_i64 );  /**< Int type (64-bits). */
pl_type( uint64_t, pl_u64 ); /**< Unsigned int type (64-bits). */

pl_type( char*, pl_str );  /**< String type. */
pl_type( double, pl_flt ); /**< 64-bit floating point. .*/

pl_type( uint64_t, pl_size ); /**< Size of allocation type. */
pl_type( int64_t, pl_ssize ); /**< Signed size of allocation type. */
pl_type( int64_t, pl_pos );   /**< Position in array. */
pl_type( uint64_t, pl_id );   /**< Identification number type. */

/** Nil pointer. */
#define pl_nil NULL


/* ------------------------------------------------------------
 * Standards loops.
 */

/** Forever loop. */
#define pl_loop for ( ;; )

/** For n-times loop. Loop counter is 'i'. */
#define pl_for_n( n ) for ( int i = 0; i < ( n ); i++ )

/** For n-times loop. Loop counter is 'x'. */
#define pl_for_n_x( n, x ) for ( int( x ) = 0; ( x ) < ( n ); ( x )++ )


// /**
//  * Default memory function callback.
//  *
//  * Params:
//  * * obj: Container object.
//  * * env: Program/library related environment.
//  * * arg: Dynamic argument (transaction related context).
//  */
// typedef pl_t ( *pl_mem_cb_fn )( pl_t obj, pl_t env, pl_t arg );



/* ------------------------------------------------------------
 * Standard streams.
 */

/* clang-format off */

#ifndef pl_stdin
#    define pl_stdin stdin
#endif

#ifndef pl_stdout
#    define pl_stdout stdout
#endif

#ifndef pl_stderr
#    define pl_stderr stderr
#endif

/* clang-format on */




/* ------------------------------------------------------------
 * Miscellaneous.
 */

inline pl_none pl_dummy( pl_none ) {}

typedef pl_t ( *pl_ui_f )( pl_t env, pl_t arg );

/**
 * Universal Interface.
 */
pl_struct( pl_ui )
{
    pl_ui_f fun; /**< Object method. */
    pl_t    env; /**< Object. */
};

/* Align size (up) to multiple of alignment. */
#define PLINTH_ALIGN_TO( size, alignment ) \
    ( ( ( ( size ) + ( alignment ) - 1 ) / ( alignment ) ) * ( alignment ) )

// #define PLINTH_ADDR_ADD(addr,offset) (((char*)addr)+offset)
// #define PLINTH_ADDR_SUB(addr,offset) (((char*)addr)-offset)

// bool is_power_of_two(unsigned int n) {
//     return n != 0 && (n & (n - 1)) == 0;
// }


/**
 * Arena Memory Allocator Descriptor.
 *
 *        plam_node_s
 *       / used mem
 *      / /         ,unused mem
 *     #+++- <-> #+---
 *     '---'     '---'
 *       \         \
 *        node      current node
 *        size
 */
pl_struct_type( plam_node );
pl_struct_body( plam_node )
{
    plam_node_t prev;      /**< Previous node. */
    plam_node_t next;      /**< Next node. */
    pl_size_t   used;      /**< Used count for data. */
    pl_bool_t   debt;      /**< Reservation debt? */
    uint8_t     data[ 0 ]; /**< Data. */
};
pl_struct( plam )
{
    plam_node_t node; /**< Current node. */
    pl_size_t   size; /**< Node size. */
};



/**
 * Block Memory Allocator Descriptor.
 *
 *        ..       .-.
 *        v|       | v
 *     #++-- <-> #+-+-
 *        |^       ^ |
 *        o'-------|-'
 *                 head
 */
pl_struct( plbm )
{
    plam_node_t node;  /**< Current node. */
    pl_t        head;  /**< Block chain head. */
    pl_size_t   nsize; /**< Node size. */
    pl_size_t   bsize; /**< Block size. */
    pl_size_t   itail; /**< Init tail count. */
};



/**
 * Continuous Memory Allocator Descriptor.
 *
 *        plcm_s    data
 *       /         /    used
 *      /         /    /   ,size
 *     #    =>   +++++-----
 */
pl_struct( plcm )
{
    pl_size_t size; /**< Reservation size for data. */
    pl_size_t used; /**< Used count for data. */
    pl_t      data; /**< Pointer to data. */
    pl_bool_t debt; /**< Reservation debt? */
};


/**
 * String Reference.
 */
pl_struct( plsr )
{
    pl_size_t   length;
    const char* string; /**< String content. */
};


#define plam_get_with_type( plam, type ) plam_get( ( plam ), sizeof( type ) );
#define plam_put_with_type( plam, type ) plam_put( ( plam ), sizeof( type ) );

#define plcm_get_pos_with_type( plam, type ) plcm_get_pos( ( plam ), sizeof( type ) );
#define plcm_get_ref_with_type( plam, type ) plcm_get_ref( ( plam ), sizeof( type ) );
#define plcm_store_with_type( plam, data, type ) plcm_store( ( plam ), ( data ), sizeof( type ) );
#define plcm_set_with_type( plam, pos, data, type ) \
    plcm_set( ( plcm ), ( pos ), ( data ), sizeof( type ) );
#define plcm_terminate_with_type( plam, type ) plcm_terminate( ( plam ), sizeof( type ) );


pl_t    pl_alloc_memory( pl_size_t size );
pl_none pl_free_memory( pl_t mem );
pl_t    pl_realloc_memory( pl_t mem, pl_size_t size );
char*   pl_strdup( const char* str );
char*   pl_format( const char* fmt, ... );

pl_none   plam_new( plam_t plam, pl_size_t size );
pl_none   plam_use( plam_t plam, pl_t node, pl_size_t size );
pl_none   plam_use_plam( plam_t plam, plam_t base, pl_size_t size );
pl_none   plam_empty( plam_t plam, pl_size_t size );
pl_none   plam_del( plam_t plam );
pl_t      plam_get( plam_t plam, pl_size_t size );
pl_none   plam_put( plam_t plam, pl_size_t size );
char*     plam_strdup( plam_t plam, const char* str );
char*     plam_format( plam_t plam, const char* fmt, ... );
pl_size_t plam_used( plam_t plam );
pl_size_t plam_size( plam_t plam );
pl_bool_t plam_is_empty( plam_t plam );

pl_none   plbm_new( plbm_t plbm, pl_size_t nsize, pl_size_t bsize );
pl_none   plbm_use( plbm_t plbm, pl_t node, pl_size_t nsize, pl_size_t bsize );
pl_none   plbm_empty( plbm_t plbm, pl_size_t nsize, pl_size_t bsize );
pl_none   plbm_del( plbm_t plbm );
pl_t      plbm_get( plbm_t plbm );
pl_none   plbm_put( plbm_t plbm, pl_t block );
pl_size_t plbm_nsize( plbm_t plbm );
pl_size_t plbm_bsize( plbm_t plbm );
pl_bool_t plbm_is_empty( plbm_t plbm );

pl_none   plcm_new( plcm_t plcm, pl_size_t size );
pl_none   plcm_use( plcm_t plcm, pl_t mem, pl_size_t size );
pl_none   plcm_use_plam( plcm_t plcm, plam_t plam, pl_size_t size );
pl_none   plcm_empty( plcm_t plcm, pl_size_t first_size );
pl_none   plcm_del( plcm_t plcm );
pl_none   plcm_resize( plcm_t plcm, pl_size_t size );
pl_pos_t  plcm_get_pos( plcm_t plcm, pl_size_t size );
pl_t      plcm_get_ref( plcm_t plcm, pl_size_t size );
pl_pos_t  plcm_store( plcm_t plcm, pl_t data, pl_size_t size );
pl_t      plcm_ref( plcm_t plcm, pl_pos_t pos );
pl_none   plcm_set( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size );
pl_none   plcm_terminate( plcm_t plcm, pl_size_t size );
pl_none   plcm_reset( plcm_t plcm );
pl_size_t plcm_used( plcm_t plcm );
pl_size_t plcm_size( plcm_t plcm );
pl_t      plcm_data( plcm_t plcm );
pl_bool_t plcm_debt( plcm_t plcm );
pl_t      plcm_end( plcm_t plcm );
pl_bool_t plcm_is_empty( plcm_t plcm );

plcm_t         plss_append( plcm_t plcm, plsr_s str );
plcm_t         plss_append_c( plcm_t plcm, char* str );
plcm_t         plss_append_ch( plcm_t plcm, char ch );
plcm_t         plss_set( plcm_t plcm, plsr_s str );
plcm_t         plss_format( plcm_t plcm, const char* fmt, ... );
plcm_t         plss_reformat( plcm_t plcm, const char* fmt, ... );
pl_none        plss_va_format( plcm_t plcm, const char* fmt, va_list ap );
pl_size_t      plss_length( plcm_t plcm );
const pl_str_t plss_string( plcm_t plcm );
plsr_s         plss_ref( plcm_t plcm );

plsr_s      plsr_from_c( const char* c_string );
plsr_s      plsr_from_c_length( const char* c_string, pl_size_t length );
const char* plsr_string( plsr_s sr );
pl_size_t   plsr_length( plsr_s sr );
pl_bool_t   plsr_compare( plsr_s p1, plsr_s p2 );
pl_bool_t   plsr_compare_n( plsr_s p1, plsr_s p2, pl_size_t n );
plsr_s      plsr_invalid( pl_none );
pl_bool_t   plsr_is_invalid( plsr_s plsr );
pl_bool_t   plsr_is_valid( plsr_s plsr );


#endif
