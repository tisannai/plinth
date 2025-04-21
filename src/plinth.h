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

/*
 * Common headers:
 */

// #ifndef PLINTH_NO_STD_INCLUDE
// #define PLINTH_STD_INCLUDE
// #include <errno.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// // #include <libgen.h>
// #include <ctype.h>
// #include <stdarg.h>
// #include <assert.h>
// #include <stdint.h>
// #endif

#include <stdint.h>
#include <stdarg.h>
// #include <ctype.h>

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
 *   Becomes:
 *       typedef struct point_struct_s point_s;
 *       typedef point_s* point_t;
 *       typedef point_s** point_p;
 *       struct point_struct_s {
 *           pl_int x;
 *           pl_int y;
 *       };
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
 *
 *   pl_enum(some_runtype) {
 *     RUN_NONE,
 *     RUN_TASK,
 *     RUN_FUNC
 *   };
 *
 *   // Becomes...
 *   typedef enum pl_runtype_e pl_runtype_t;
 *   enum pl_runtype_e {
 *     RUN_NONE,
 *     RUN_TASK,
 *     RUN_FUNC
 *   };
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



/* ------------------------------------------------------------
 * Low level memory.
 */

// #ifndef PLINTH_NO_MEM_API
//
// /* clang-format off */
// #define pl_new( type )                   calloc( 1, sizeof( type ) )
// #define pl_new_n( type, n )              calloc( ( n ), sizeof( type ) )
// #define pl_del( mem )                    free( mem )
// #define pl_alloc( size )                 calloc( 1, ( size ) )
// #define pl_realloc( mem, size )          realloc( ( mem ), ( size ) )
// #define pl_free( mem )                   free( mem )
// #define pl_strdup( str )                 strdup( str )
// #define pl_memcpy( src, dst, size )      memcpy( ( dst ), ( src ), ( size ) )
// #define pl_memcpy_type( src, dst, type ) memcpy( ( dst ), ( src ), sizeof( type ) )
// #define pl_memmov( src, dst, size )      memmove( ( dst ), ( src ), ( size ) )
// #define pl_memmov_type( src, dst, type ) memmove( ( dst ), ( src ), sizeof( type ) )
// #define pl_memclr( mem, size )           memset( ( mem ), 0, ( size ) )
// #define pl_memclr_type( mem, type )      memset( ( mem ), 0, sizeof( type ) )
// #define pl_memcmp( m1, m2, size )        memcmp( ( m1 ), ( m2 ), ( size ) )
// #define pl_memcmp_type( m1, m2, type)    memcmp( ( m1 ), ( m2 ), sizeof( type ) )
// /* clang-format on */
//
// #endif


/**
 * Default memory function callback.
 *
 * Params:
 * * obj: Container object.
 * * env: Program/library related environment.
 * * arg: Dynamic argument (transaction related context).
 */
typedef pl_t ( *pl_mem_cb_fn )( pl_t obj, pl_t env, pl_t arg );



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


typedef void* ( *ui_f )( void* env, void* arg );

/**
 * Universal Interface.
 */
pl_struct( ui )
{
    ui_f  fun; /**< Object method. */
    void* env; /**< Object. */
};

/* Align size (up) to multiple of alignment. */
#define PLINTH_ALIGN_TO( size, alignment ) \
    ( ( ( ( size ) + ( alignment ) - 1 ) / ( alignment ) ) * ( alignment ) )



/**
 * Segmented Memory Allocator Descriptor.
 */
pl_struct_type( plsm_node ) pl_struct_body( plsm_node )
{
    plsm_node_t prev;      /**< Previous node. */
    plsm_node_t next;      /**< Next node. */
    pl_size_t   used;      /**< Used count for data. */
    pl_bool_t   debt;      /**< Reservation debt? */
    char*       data[ 0 ]; /**< Data. */
};
pl_struct( plsm )
{
    plsm_node_t node; /**< Current node. */
    pl_size_t   size; /**< Node size. */
};



/**
 * Continuous Memory Allocator Descriptor.
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
    pl_size_t length;
    union
    {
        const char* string; /**< String content. */
        char*       str_m;  /**< For assignment. */
    };
};



pl_t    pl_alloc_memory( pl_size_t size );
pl_none pl_free_memory( pl_t mem );
pl_t    pl_realloc_memory( pl_t mem, pl_size_t size );
char*   pl_strdup( const char* str );
char*   pl_format( const char* fmt, ... );

pl_none   plsm_new( plsm_t plsm, pl_size_t size );
pl_none   plsm_use( plsm_t plsm, pl_t node, pl_size_t size );
pl_none   plsm_empty( plsm_t plsm, pl_size_t size );
pl_none   plsm_del( plsm_t plsm );
pl_t      plsm_get( plsm_t plsm, pl_size_t size );
char*     plsm_strdup( plsm_t plsm, const char* str );
char*     plsm_format( plsm_t plsm, const char* fmt, ... );
pl_size_t plsm_used( plsm_t plsm );
pl_size_t plsm_size( plsm_t plsm );
pl_bool_t plsm_is_empty( plsm_t plsm );

pl_none   plcm_new( plcm_t plcm, pl_size_t size );
pl_none   plcm_use( plcm_t plcm, pl_t mem, pl_size_t size );
pl_none   plcm_use_plsm( plcm_t plcm, plsm_t plsm, pl_size_t size );
pl_none   plcm_empty( plcm_t plcm, pl_size_t first_size );
pl_none   plcm_del( plcm_t plcm );
pl_none   plcm_resize( plcm_t plcm, pl_size_t size );
pl_pos_t  plcm_get_pos( plcm_t plcm, pl_size_t size );
pl_t      plcm_get_ref( plcm_t plcm, pl_size_t size );
pl_pos_t  plcm_store( plcm_t plcm, pl_t data, pl_size_t size );
pl_t      plcm_ref( plcm_t plcm, pl_pos_t pos );
pl_none   plcm_set( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size );
pl_none   plcm_reset( plcm_t plcm );
pl_size_t plcm_used( plcm_t plcm );
pl_size_t plcm_size( plcm_t plcm );
pl_t      plcm_data( plcm_t plcm );
pl_bool_t plcm_debt( plcm_t plcm );
pl_t      plcm_end( plcm_t plcm );
pl_bool_t plcm_is_empty( plcm_t plcm );

// pl_none plss_new( plcm_t plcm, pl_size_t size );
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
// plsr_s    plsr_duplicate( plsr_s plsr );
pl_bool_t plsr_compare( plsr_s p1, plsr_s p2 );
pl_bool_t plsr_compare_n( plsr_s p1, plsr_s p2, pl_size_t n );
plsr_s    plsr_invalid( pl_none );
pl_bool_t plsr_is_invalid( plsr_s plsr );
pl_bool_t plsr_is_valid( plsr_s plsr );


#endif
