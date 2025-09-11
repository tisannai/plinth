#ifndef PLINTH_H
#define PLINTH_H


/**
 * @file   plinth.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Apr 19 10:45:54 EEST 2025
 *
 * @brief  Plinth - Base layer library.
 *
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>



/* ------------------------------------------------------------
 * Simple type definition features.
 */

/** Re-define type and related pointers. */
#define pl_type( base, type )   \
    typedef base      type##_t; \
    typedef type##_t* type##_p;


/** Function pointer type definition. */
#define pl_fn_type( name, rettype, ... ) typedef rettype ( *name##_fn_t )( __VA_ARGS__ );

/* ------------------------------------------------------------
 * Structure type definitions.
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

/** Forward-declaration of struct and friends. */
#define pl_struct_type( name )              \
    typedef struct name##struct_s name##_s; \
    typedef name##_s*             name##_t; \
    typedef name##_s**            name##_p;

/** Post-declaration of struct body. */
#define pl_struct_body( name ) struct name##struct_s


/**
 * Define enumeration and corresponding type. An enum is created as
 * {enum}_e, a typedef is created as {enum}_t.
 *
 * Example:
 * @code
 *     Definition:
 *         pl_enum(runtype) {
 *           RUN_NONE,
 *           RUN_TASK,
 *           RUN_FUNC
 *         };
 *
 *     Becomes:
 *         typedef enum runtype_e runtype_t;
 *         enum runtype_e {
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
pl_type( double, pl_flt );   /**< 64-bit floating point. .*/

pl_type( uint64_t, pl_size ); /**< Size of allocation type. */
pl_type( int64_t, pl_ssize ); /**< Signed size of allocation type. */
pl_type( int64_t, pl_pos );   /**< Position in array. */
pl_type( uint64_t, pl_id );   /**< Identification number type. */
pl_type( uint64_t, pl_hash ); /**< Identification number type. */


/** Nil pointer. */
#define pl_nil NULL



/* ------------------------------------------------------------
 * Miscellaneous.
 */

/**
 * Dummy function.
 */
extern pl_none pl_dummy( pl_none );


/**
 * Universal Interface method.
 *
 *     env:    Environment.
 *     argi:   Input to method.
 *     argo:   Output from method.
 *
 */
typedef pl_none ( *pl_ui_f )( pl_t env, pl_t argi, pl_t argo );


/**
 * Universal Interface.
 */
pl_struct( pl_ui )
{
    pl_t    env; /**< Environment. */
    pl_ui_f fun; /**< Method. */
};


/**
 * @brief Initialize ui structure.
 *
 * @param      ui   UI, Universal Interface.
 * @param      env  Environment for UI.
 * @param      fun  Method for UI.
 */
pl_none pl_ui_init( pl_ui_t ui, pl_t env, pl_ui_f fun );


/**
 * @brief Use Universal Interface.
 *
 * @param    ui    UI, Universal Interface.
 * @param    argi  Input argument, opaque pointer.
 * @param    argo  Output argument (response), opaque pointer.
 */
pl_none pl_ui_do( pl_ui_t ui, pl_t argi, pl_t argo );



/* ------------------------------------------------------------
 * Memory allocation.
 */

/** Align size (up) to multiple of alignment. */
#define PLINTH_ALIGN_TO( size, alignment ) \
    ( ( ( ( size ) + ( alignment ) - 1 ) / ( alignment ) ) * ( alignment ) )


/** Allocator affinity type. */
pl_enum( pl_aa ){ PL_AA_NONE = 0, PL_AA_SELF, PL_AA_HEAP, PL_AA_PLAM,
                  PL_AA_PLBM,     PL_AA_PLCM, PL_AA_DESC };


/**
 * Arena Memory Allocator Descriptor.
 *
 *        pl_node_s
 *       / used mem
 *      / /         ,unused mem
 *     #+++- <-> #+---
 *     '---'     '---'
 *       \         \
 *        node      current node
 *        size
 */
pl_struct_type( pl_node );
pl_struct_body( pl_node )
{
    pl_node_t prev;      /**< Previous node. */
    pl_node_t next;      /**< Next node. */
    pl_size_t used;      /**< Used count for data. */
    uint8_t   data[ 0 ]; /**< Data location. */
};
pl_struct( plam )
{
    pl_node_t node; /**< Current node. */
    pl_size_t size; /**< Node size. */
    pl_aa_t   type; /**< Reservation type. */
    pl_t      ator; /**< Allocator. */
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
    pl_node_t node;  /**< Current node. */
    pl_t      head;  /**< Block chain head. */
    pl_size_t nsize; /**< Node size. */
    pl_size_t bsize; /**< Block size. */
    pl_size_t itail; /**< Init tail count. */
    pl_aa_t   type;  /**< Reservation type. */
    pl_t      ator;  /**< Allocator. */
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
    pl_aa_t   type; /**< Reservation type. */
};

/**
 * Plcm find, compare function type. Compare function should return 1
 * for match.
 */
pl_fn_type( plcm_compare, int, pl_size_t size, const pl_t a, const pl_t b );


/**
 * Unified Memory Allocator Descriptor.
 *
 */
pl_struct( plum )
{
    pl_aa_t type; /**< Host allocator type. */
    pl_t    host; /**< Host handle. */
};


/**
 * String Reference.
 *
 *        string
 *       /
 *     .---.
 *     text_---
 *     '--'
 *       \
 *        length
 */
pl_struct( plsr )
{
    pl_size_t   length; /**< String length. */
    const char* string; /**< String content. */
};


/* ------------------------------------------------------------
 * Access macros with type abstraction.
 */

/** \cond */

#define pl_alloc_memory_for_type( type ) pl_alloc_memory( sizeof( type ) )

#define plam_get_for_type( plam, type ) plam_get( ( plam ), sizeof( type ) )
#define plam_put_for_type( plam, type ) plam_put( ( plam ), sizeof( type ) )

#define plcm_get_pos_for_type( plcm, type ) plcm_get_pos( ( plcm ), sizeof( type ) )
#define plcm_get_ref_for_type( plcm, type ) plcm_get_ref( ( plcm ), sizeof( type ) )
#define plcm_put_for_type( plcm, type ) plcm_put( ( plcm ), sizeof( type ) )
#define plcm_store_for_type( plcm, data, type ) plcm_store( ( plcm ), ( data ), sizeof( type ) )
#define plcm_ref_for_type( plcm, pos, type ) plcm_ref( ( plcm ), ( pos ) * sizeof( type ) )
#define plcm_set_for_type( plcm, pos, data, type ) \
    plcm_set( ( plcm ), ( pos ), ( data ), sizeof( type ) )
#define plcm_terminate_for_type( plcm, type ) plcm_terminate( ( plcm ), sizeof( type ) )
#define plcm_used_for_type( plcm, type ) ( plcm_used( ( plcm ) ) / sizeof( type ) )

#define plcm_null {0,0,NULL,PL_AA_SELF}

/** \endcond */

/**
 * Declare stack local plcm with "name" and reserve array, with
 * "size", for storage.
 */
#define plss_use( name, size )              \
    plcm_s name;                            \
    char   name##_plss_declare[ ( size ) ]; \
    plcm_use( &name, name##_plss_declare, ( size ) )


/** Iterate over all items. */
#define plcm_each_ptr( plcm, iter, cast )                             \
    for ( pl_size_t plcm_each_ptr_index = 0;                          \
          ( plcm_each_ptr_index < plcm_used_ptr( plcm ) ) &&          \
          ( iter = (cast)plcm_ref_ptr( plcm, plcm_each_ptr_index ) ); \
          plcm_each_ptr_index++ )


/* ------------------------------------------------------------
 * Basic (heap) memory allocation:
 */

/**
 * @brief Allocate memory from heap (zeroed).
 *
 * @param   size   Allocation size in bytes.
 *
 * @return  Pointer to allocation, or NULL.
 */
pl_t pl_alloc_memory( pl_size_t size );


/**
 * @brief Allocate memory from heap (non-zeroed).
 *
 * @param   size   Allocation size in bytes.
 *
 * @return  Pointer to allocation, or NULL.
 */
pl_t pl_alloc_only( pl_size_t size );


/**
 * @brief Deallocate heap memory.
 *
 * @param   mem    Pointer to allocation.
 */
pl_none pl_free_memory( pl_t mem );


/**
 * @brief Reallocate memory from heap.
 *
 * @param   mem    Pointer to allocation.
 * @param   size   Allocation size in bytes.
 *
 * @return  Pointer to reallocation, or NULL.
 */
pl_t pl_realloc_memory( pl_t mem, pl_size_t size );


/**
 * @brief Duplicate plsr string as heap memory with null termination.
 *
 * @param   plsr    Plsr string to duplicate.
 *
 * @return  Duplicate, or null plsr.
 */
plsr_s pl_alloc_plsr( plsr_s plsr );


/**
 * @brief Duplicate string as heap memory with null termination.
 *
 * @param   str    String to duplicate.
 *
 * @return  Duplicated string, or NULL.
 */
char* pl_alloc_string( const char* str );


/**
 * @brief Format string to heap memory with null termination.
 *
 * @param   fmt    Format specifier.
 *
 * @return  Formatted string in heap, or NULL.
 */
char* pl_format_string( const char* fmt, ... );


/**
 * @brief Clear memory area.
 *
 * @param   mem    Pointer to allocation.
 * @param   size   Allocation size in bytes.
 *
 * @return  Pointer to allocation, or NULL.
 */
pl_t pl_clear_memory( pl_t mem, pl_size_t size );



/* ------------------------------------------------------------
 * Arena Memory Allocator:
 */

/**
 * @brief Create plam in heap (with debt).
 *
 * @param    plam   Plam handle.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_new( plam_t plam, pl_size_t size );


/**
 * @brief Initiate plam to node (no debt).
 *
 * @param    plam   Plam handle.
 * @param    node   Node.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_use( plam_t plam, pl_t node, pl_size_t size );


/**
 * @brief Initiate nested plam from plam (no debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plam handle.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_use_plam( plam_t plam, plam_t host, pl_size_t size );


/**
 * @brief Initiate nested plam from plbm (no debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plbm handle.
 *
 * @return None.
 */
pl_none plam_use_plbm( plam_t plam, plbm_t host );


/**
 * @brief Deploy plam inside plam (debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plam handle.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_into_plam( plam_t plam, plam_t host, pl_size_t size );


/**
 * @brief Deploy plam inside plbm (debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plbm handle.
 *
 * @return None.
 */
pl_none plam_into_plbm( plam_t plam, plbm_t host );


/**
 * @brief Create empty plam for heap allocations.
 *
 * Empty plam is a placeholder with handle setup for allocations.
 * However, no heap allocations are made at creation, i.e. this allows
 * lazy behavior.
 *
 * @param    plam   Plam handle.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_empty( plam_t plam, pl_size_t size );


/**
 * @brief Create empty nested plam for plam allocations.
 *
 * Empty plam is a placeholder with handle setup for allocations.
 * However, no host allocations are made at creation, i.e. this allows
 * lazy behavior.
 *
 * @param    plam   Plam handle.
 * @param    host   Plam handle.
 * @param    size   Node size.
 *
 * @return None.
 */
pl_none plam_empty_into_plam( plam_t plam, plam_t host, pl_size_t size );


/**
 * @brief Create empty nested plam for plbm allocations.
 *
 * Empty plam is a placeholder with handle setup for allocations.
 * However, no host allocations are made at creation, i.e. this allows
 * lazy behavior.
 *
 * @param    plam   Plam handle.
 * @param    host   Plbm handle.
 *
 * @return None.
 */
pl_none plam_empty_into_plbm( plam_t plam, plbm_t host );


/**
 * @brief Delete plam.
 *
 * If plam has debt, then the heap memory is deallocated. If plam has
 * no debt, deletion is mute.
 *
 * @param    plam   Plam handle.
 *
 * @return None.
 */
pl_none plam_del( plam_t plam );


/**
 * @brief Get allocation from plam.
 *
 * @param    plam   Plam handle.
 * @param    size   Allocation size.
 *
 * @return Allocation.
 */
pl_t plam_get( plam_t plam, pl_size_t size );


/**
 * @brief Put allocation back to plam.
 *
 * User is responsible in making the puts in reserver order with
 * correct sizes.
 *
 * @param    plam   Plam handle.
 * @param    size   Allocation size.
 *
 * @return None.
 */
pl_none plam_put( plam_t plam, pl_size_t size );


/**
 * @brief Get allocation from plam and store the data.
 *
 * @param    plam   Plam handle.
 * @param    data   Data to store.
 * @param    size   Allocation size.
 *
 * @return Pointer to stored data.
 */
pl_t plam_store( plam_t plam, const pl_t data, pl_size_t size );


/**
 * @brief Get allocation from plam and store the plsr content.
 *
 * @param    plam   Plam handle.
 * @param    plsr   String reference.
 *
 * @return Plsr handle to stored content.
 */
plsr_s plam_store_plsr( plam_t plam, plsr_s plsr );


/**
 * @brief Get allocation from plam and store the string.
 *
 * @param    plam   Plam handle.
 * @param    str    String.
 *
 * @return Stored string.
 */
char* plam_store_string( plam_t plam, const char* str );


/**
 * @brief Format string to plam.
 *
 * @param    plam   Plam handle.
 * @param    fmt    Format specifier.
 *
 * @return  Formatted string from plam.
 */
char* plam_format_string( plam_t plam, const char* fmt, ... );


/**
 * @brief Used memory of current node.
 *
 * @param    plam   Plam handle.
 *
 * @return Used memory.
 */
pl_size_t plam_used( plam_t plam );


/**
 * @brief Free memory in current node.
 *
 * @param    plam   Plam handle.
 *
 * @return Free memory.
 */
pl_size_t plam_free( plam_t plam );


/**
 * @brief Return node size.
 *
 * @param    plam   Plam handle.
 *
 * @return Node size.
 */
pl_size_t plam_size( plam_t plam );


/**
 * @brief Return node capacity.
 *
 * @param    plam   Plam handle.
 *
 * @return Node capasity.
 */
pl_size_t plam_node_capacity( plam_t plam );


/**
 * @brief Is plam empty?
 *
 * @param    plam   Plam handle.
 *
 * @return True, if plam is empty.
 */
pl_bool_t plam_is_empty( plam_t plam );



/* ------------------------------------------------------------
 * Block Memory Allocator:
 */

/**
 * @brief Create plbm in heap (with debt).
 *
 * @param    plbm   Plbm handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_new( plbm_t plbm, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Initiate plbm to node (no debt).
 *
 * @param    plbm   Plbm handle.
 * @param    node   Node.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_use( plbm_t plbm, pl_t node, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Initiate nested plbm from plam (no debt).
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plam handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_use_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Initiate nested plbm from plbm (no debt).
 *
 * Node size is inherited from host block size.
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plbm handle.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_use_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize );


/**
 * @brief Deploy plbm inside plam (debt).
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plam handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_into_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Deploy plbm inside plbm (debt).
 *
 * Node size is inherited from host block size.
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plbm handle.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_into_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize );


/**
 * @brief Create empty plbm.
 *
 * Empty plbm is a placeholder with handle setup for allocations.
 * However, no heap allocations are made at creation.
 *
 * @param    plbm   Plbm handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_empty( plbm_t plbm, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Create empty nested plbm for plam allocations.
 *
 * Empty plbm is a placeholder with handle setup for allocations.
 * However, no host allocations are made at creation, i.e. this allows
 * lazy behavior.
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plam handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_empty_into_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Create empty nested plbm for plbm allocations.
 *
 * Empty plbm is a placeholder with handle setup for allocations.
 * However, no host allocations are made at creation, i.e. this allows
 * lazy behavior.
 *
 * Node size is inherited from host block size.
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plbm handle.
 * @param    bsize  Block size.
 *
 * @return None.
 */
pl_none plbm_empty_into_plbm( plbm_t plbm, plbm_t host, pl_size_t bsize );


/**
 * @brief Delete plbm.
 *
 * If plbm has debt, then the heap memory is deallocated. If plbm has
 * no debt, deletion is mute.
 *
 * @param    plbm   Plbm handle.
 *
 * @return None.
 */
pl_none plbm_del( plbm_t plbm );


/**
 * @brief Get allocation from plbm.
 *
 * @param    plbm   Plbm handle.
 *
 * @return Allocation.
 */
pl_t plbm_get( plbm_t plbm );


/**
 * @brief Put allocation back to plbm.
 *
 * @param    plbm   Plbm handle.
 * @param    block  Block to return.
 *
 * @return None.
 */
pl_none plbm_put( plbm_t plbm, pl_t block );


/**
 * @brief Allocate storage for pointer and store its value.
 *
 * @param    plbm   Plbm handle.
 * @param    ptr    Pointer to store.
 *
 * @return Storage address of the pointer.
 */
pl_t plbm_store_ptr( plbm_t plbm, pl_t ptr );


/**
 * @brief Reference pointer value from the given storage address.
 *
 * @param    plbm    Plbm handle.
 * @param    storage Storage address of the pointer.
 *
 * @return Pointer value.
 */
pl_t plbm_ref_ptr( plbm_t plbm, pl_t storage );


/**
 * @brief Return node size.
 *
 * @param    plbm   Plbm handle.
 *
 * @return Node size.
 */
pl_size_t plbm_node_size( plbm_t plbm );


/**
 * @brief Return node capacity.
 *
 * @param    plbm   Plbm handle.
 *
 * @return Storage capacity.
 */
pl_size_t plbm_node_capacity( plbm_t plbm );


/**
 * @brief Return block size.
 *
 * @param    plbm   Plbm handle.
 *
 * @return Block size.
 */
pl_size_t plbm_block_size( plbm_t plbm );


/**
 * @brief Is plbm continuous?
 *
 * Plbm is continuous when all the allocation are in single node.
 *
 * @param    plbm   Plbm handle.
 *
 * @return True, if plbm is continuous.
 */
pl_bool_t plbm_is_continuous( plbm_t plbm );


/**
 * @brief Is plbm empty?
 *
 * @param    plbm   Plbm handle.
 *
 * @return True, if plbm is empty.
 */
pl_bool_t plbm_is_empty( plbm_t plbm );



/* ------------------------------------------------------------
 * Continuous Memory Allocator:
 */

/**
 * @brief Create plcm in heap (with debt).
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size.
 *
 * @return Plcm handle.
 */
plcm_t plcm_new( plcm_t plcm, pl_size_t size );


/**
 * @brief Create plcm in heap (with debt) for pointers.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size for number pointers.
 *
 * @return Plcm handle.
 */
plcm_t plcm_new_ptr( plcm_t plcm, pl_size_t size );


/**
 * @brief Create plcm to pre-existing allocation (no debt).
 *
 * @param    plcm   Plcm handle.
 * @param    mem    Allocation.
 * @param    size   Size of allocation.
 *
 * @return Plcm handle.
 */
plcm_t plcm_use( plcm_t plcm, pl_t mem, pl_size_t size );


/**
 * @brief Initiate nested plcm from plam (no debt).
 *
 * @param    plcm   Nested plcm handle.
 * @param    host   Plam handle.
 * @param    size   Allocation size.
 *
 * @return Plcm handle.
 */
plcm_t plcm_use_plam( plcm_t plcm, plam_t host, pl_size_t size );


/**
 * @brief Initiate nested plcm from plbm (no debt).
 *
 * @param    plcm   Nested plcm handle.
 * @param    host   Plbm handle.
 *
 * @return Plcm handle.
 */
plcm_t plcm_use_plbm( plcm_t plcm, plbm_t host );


/**
 * @brief Create empty plcm.
 *
 * Empty plcm is a placeholder with handle setup for allocations.
 * However, no heap allocations are made at creation.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size (for future).
 *
 * @return Plcm handle.
 */
plcm_t plcm_empty( plcm_t plcm, pl_size_t size );


/**
 * @brief Create empty plcm for pointers.
 *
 * Empty plcm is a placeholder with handle setup for allocations.
 * However, no heap allocations are made at creation.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size (for future) for pointers.
 *
 * @return Plcm handle.
 */
plcm_t plcm_empty_ptr( plcm_t plcm, pl_size_t size );


/**
 * Convert Plcm to Plcm Shadow.
 *
 * @param plcm Plcm handle.
 *
 * @return Shadow, referencing the host.
 */
plcm_s plcm_shadow( plcm_t plcm );


pl_none plcm_copy_to( plcm_t plcm, plcm_t target, pl_t mem, pl_size_t size );


/**
 * Copy Plcm content and dimensions.
 *
 * @param plcm Plcm handle.
 *
 * @return Copy.
 */
plcm_s plcm_copy( plcm_t plcm );


/**
 * @brief Delete plcm.
 *
 * If plcm has debt, then the heap memory is deallocated. If plcm has
 * no debt, deletion is mute.
 *
 * @param    plcm   Plcm handle.
 *
 * @return NULL.
 */
plcm_t plcm_del( plcm_t plcm );


/**
 * @brief Resize plcm allocation.
 *
 * @param    plcm   Plcm handle.
 * @param    size   New allocation size.
 *
 * @return None.
 */
pl_none plcm_resize( plcm_t plcm, pl_size_t size );


/**
 * @brief Compact plcm allocation to minimum size.
 *
 * @param    plcm   Plcm handle.
 *
 * @return None.
 */
pl_none plcm_compact( plcm_t plcm );


/**
 * @brief Get allocation from plcm as position.
 *
 * Position is an offset to data start.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size.
 *
 * @return Allocation position.
 */
pl_pos_t plcm_get_pos( plcm_t plcm, pl_size_t size );


/**
 * @brief Get allocation from plcm as pointer reference.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size.
 *
 * @return Allocation reference.
 */
pl_t plcm_get_ref( plcm_t plcm, pl_size_t size );


/**
 * @brief Put allocation back to plcm.
 *
 * User is responsible in making the puts in reserver order with
 * correct sizes.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size.
 *
 * @return None.
 */
pl_none plcm_put( plcm_t plcm, pl_size_t size );


/**
 * @brief Get allocation from plcm and store value to it.
 *
 * @param    plcm   Plcm handle.
 * @param    data   Data to store.
 * @param    size   Allocation and data size.
 *
 * @return Allocation position.
 */
pl_pos_t plcm_store( plcm_t plcm, pl_t data, pl_size_t size );


/**
 * @brief Get allocation from plcm and store pointer value to it.
 *
 * @param    plcm     Plcm handle.
 * @param    ptr      Pointer value.
 *
 * @return Allocation position.
 */
pl_pos_t plcm_store_ptr( plcm_t plcm, pl_t ptr );


/**
 * @brief Get allocation from plcm and store null pointer value to it.
 *
 * Storage is resized, but the used count is not changed.
 *
 * @param    plcm     Plcm handle.
 */
pl_none plcm_store_null( plcm_t plcm );


/**
 * @brief Reference allocation from plcm.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Allocation position.
 *
 * @return Allocation reference.
 */
pl_t plcm_ref( plcm_t plcm, pl_pos_t pos );


/**
 * @brief Reference pointer allocation from plcm.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Pointer position.
 *
 * @return Pointer value.
 */
pl_t plcm_ref_ptr( plcm_t plcm, pl_pos_t pos );


/**
 * @brief Set value for data.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Allocation position.
 * @param    data   Address of data to store.
 * @param    size   Data size.
 *
 * @return None.
 */
pl_none plcm_set( plcm_t plcm, pl_pos_t pos, const pl_t data, pl_size_t size );


/**
 * @brief Set pointer value to pointer position.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Pointer position.
 * @param    ptr    Pointer value.
 *
 * @return None.
 */
pl_none plcm_set_ptr( plcm_t plcm, pl_pos_t pos, const pl_t ptr );


/**
 * @brief Remove data.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Position of removal.
 * @param    size   Size of removal.
 *
 * @return None.
 */
pl_none plcm_remove( plcm_t plcm, pl_pos_t pos, pl_size_t size );


/**
 * @brief Remove pointer.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Pointer position of removal.
 *
 * @return None.
 */
pl_none plcm_remove_ptr( plcm_t plcm, pl_pos_t pos );


/**
 * @brief Insert data.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Position of insertion.
 * @param    data   Data to store.
 * @param    size   Data size.
 *
 * @return None.
 */
pl_none plcm_insert( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size );


/**
 * @brief Insert pointer.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Pointer position of insertion.
 * @param    ptr    Pointer value.
 *
 * @return None.
 */
pl_none plcm_insert_ptr( plcm_t plcm, pl_pos_t pos, pl_t ptr );


/**
 * @brief Set terminating value after used data.
 *
 * Use this for any type of NULL termination of continuous data.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Size of termination.
 *
 * @return True, if termination done and fitted.
 */
pl_bool_t plcm_terminate( plcm_t plcm, pl_size_t size );


/**
 * @brief Set terminating value after used data for pointers.
 *
 * @param    plcm   Plcm handle.
 *
 * @return True, if termination done and fitted.
 */
pl_bool_t plcm_terminate_ptr( plcm_t plcm );


/**
 * @brief Reset used data to zero.
 *
 * @param    plcm   Plcm handle.
 *
 * @return None.
 */
pl_none plcm_reset( plcm_t plcm );


/**
 * @brief Reset used data to zero and clear data.
 *
 * @param    plcm   Plcm handle.
 *
 * @return None.
 */
pl_none plcm_clear( plcm_t plcm );


/**
 * @brief Used memory size.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Used memory.
 */
pl_size_t plcm_used( plcm_t plcm );


/**
 * @brief Used memory size as pointer count.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Used memory as number of pointers.
 */
pl_size_t plcm_used_ptr( plcm_t plcm );


/**
 * @brief Allocated memory size.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Allocation size.
 */
pl_size_t plcm_size( plcm_t plcm );


/**
 * @brief Allocated memory size as pointers.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Allocation size as pointers.
 */
pl_size_t plcm_size_ptr( plcm_t plcm );


/**
 * @brief Return reference to data.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Data reference.
 */
pl_t plcm_data( plcm_t plcm );


/**
 * @brief Is plcm using debt?
 *
 * @param    plcm   Plcm handle.
 *
 * @return True, if plcm has debt.
 */
pl_bool_t plcm_debt( plcm_t plcm );


/**
 * @brief Reference to end of data (after used).
 *
 * @param    plcm   Plcm handle.
 *
 * @return Reference to end.
 */
pl_t plcm_end( plcm_t plcm );


/**
 * @brief Is plcm empty?
 *
 * @param    plcm   Plcm handle.
 *
 * @return True, if plcm is empty.
 */
pl_bool_t plcm_is_empty( plcm_t plcm );


/**
 * @brief Find pointer from plcm.
 *
 * @param    plcm   Plcm handle.
 * @param    ref    Pointer to find.
 *
 * @return Index, or -1 for not found.
 */
pl_pos_t plcm_find_ptr( plcm_t plcm, pl_t ref );


/**
 * @brief Find object from plcm.
 *
 * @param    plcm    Plcm handle.
 * @param    compare Comparison function.
 * @param    size    Item size in bytes.
 * @param    ref     Pointer for reference.
 *
 * @return Index, or -1 for not found.
 */
pl_pos_t plcm_find_with( plcm_t plcm, plcm_compare_fn_t compare, pl_size_t size, pl_t ref );


/* ------------------------------------------------------------
 * Unified Memory Allocator:
 */

/**
 * @brief Initiate plum with allocator.
 *
 * @param    plum   Plum handle.
 * @param    type   Allocator type.
 * @param    host   Allocator host (the actual allocator).
 *
 * @return None.
 */
pl_none plum_use( plum_t plum, pl_aa_t type, pl_t host );


/**
 * @brief Get allocation from plum.
 *
 * @param    plum   Plum handle.
 * @param    size   Allocation size.
 *
 * @return Allocation.
 */
pl_t plum_get( plum_t plum, pl_size_t size );


/**
 * @brief Put allocation back to plum.
 *
 * @param    plum   Plum handle.
 * @param    mem    Allocation pointer.
 * @param    size   Allocation size.
 *
 * @return Address or NULL if no memory was reclaimed.
 */
pl_t plum_put( plum_t plum, pl_t mem, pl_size_t size );


/**
 * @brief Update allocation size in plum.
 *
 * Current allocation content is copied to updated allocation.
 *
 * @param    plum   Plum handle.
 * @param    mem    Allocation pointer.
 * @param    osize  Allocation size of current.
 * @param    nsize  Allocation size of new.
 *
 * @return Updated allocation.
 */
pl_t plum_update( plum_t plum, pl_t mem, pl_size_t osize, pl_size_t nsize );


/**
 * @brief Return plum allocator (host) type.
 *
 * @param    plum   Plum handle.
 *
 * @return Type.
 */
pl_aa_t plum_type( plum_t plum );


/**
 * @brief Return plum allocator (host).
 *
 * @param    plum   Plum handle.
 *
 * @return Allocator.
 */
pl_t plum_host( plum_t plum );



/* ------------------------------------------------------------
 * String Storage:
 */

/**
 * @brief Create plss from plsr.
 *
 * @param    plcm   Plcm handle.
 * @param    plsr   Plsr handle.
 *
 * @return None.
 */
plcm_t plss_from_plsr( plcm_t plcm, plsr_s plsr );


/**
 * @brief Append plsr to plcm.
 *
 * @param   plcm   Plcm handle.
 * @param   str    Plsr handle.
 *
 * @return  Plcm handle.
 */
plcm_t plss_append( plcm_t plcm, plsr_s str );


/**
 * @brief Append c-string to plcm.
 *
 * @param   plcm   Plcm handle.
 * @param   str    C-string handle.
 *
 * @return  Plcm handle.
 */
plcm_t plss_append_string( plcm_t plcm, const char* str );


/**
 * @brief Append char to plcm.
 *
 * @param   plcm   Plcm handle.
 * @param   ch     Char.
 *
 * @return  Plcm handle.
 */
plcm_t plss_append_char( plcm_t plcm, char ch );


/**
 * @brief Remove sub-string.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Position of removal.
 * @param    size   Size of removal.
 *
 * @return None.
 */
pl_none plss_remove( plcm_t plcm, pl_pos_t pos, pl_size_t size );


/**
 * @brief Insert sub-string.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Position of insertion.
 * @param    data   Data to store.
 * @param    size   Data size.
 *
 * @return None.
 */
pl_none plss_insert( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size );


/**
 * @brief Set (overwrite) plcm content.
 *
 * @param   plcm   Plcm handle.
 * @param   str    Plsr handle.
 *
 * @return  Plcm handle.
 */
plcm_t plss_set( plcm_t plcm, plsr_s str );


/**
 * Refresh string length of plcm.
 *
 * Useful when some function has filled string content and it should
 * be in sync with the Plcm itself.
 *
 * @param plcm Plcm handle.
 *
 * @return Plcm.
 */
plcm_t plss_refresh( plcm_t plcm );


/**
 * @brief Compact plss (pclm) allocation to minimum size.
 *
 * @param    plcm   Plcm handle.
 *
 * @return None.
 */
plcm_t plss_compact( plcm_t plcm );


/**
 * @brief Format string to plcm, append.
 *
 * @param   plcm   Plcm handle.
 * @param   fmt    Format specifier.
 *
 * @return  Plcm handle.
 */
plcm_t plss_format_string( plcm_t plcm, const char* fmt, ... );


/**
 * @brief Format string to plcm, overwrite.
 *
 * @param   plcm   Plcm handle.
 * @param   fmt    Format specifier.
 *
 * @return  Plcm handle.
 */
plcm_t plss_reformat_string( plcm_t plcm, const char* fmt, ... );


/**
 * @brief Format string to plcm, append.
 *
 * @param   plcm   Plcm handle.
 * @param   fmt    Format specifier.
 * @param   ap     Variable arguments.
 *
 * @return  None.
 */
pl_none plss_va_format_string( plcm_t plcm, const char* fmt, va_list ap );


/**
 * Read file into an existing plcm and return plcm for file
 * content.
 *
 * @param plcm      Plcm handle.
 * @param filename  Filename.
 *
 * @return Plcm, NULL with failure.
 */
plcm_t plss_read_file( plcm_t plcm, const char* filename );


/**
 * Read file into an existing plcm and return plcm for file content
 * with null padding in the start and end.
 *
 * @param plcm      Plcm handle.
 * @param filename  Filename.
 * @param left      Start pad.
 * @param right     End pad.
 *
 * @return plcm, NULL with failure.
 */
plcm_t plss_read_file_with_pad( plcm_t      plcm,
                                const char* filename,
                                pl_size_t   left,
                                pl_size_t   right );


/**
 * Write plcm content to file.
 *
 * @param plcm      Plcm handle.
 * @param filename  Filename.
 *
 * @return plcm, NULL for failure.
 */
plcm_t plss_write_file( plcm_t plcm, const char* filename );


/**
 * @brief String in plcm.
 *
 * @param   plcm   Plcm handle.
 *
 * @return  String reference.
 */
const char* plss_string( plcm_t plcm );


/**
 * @brief String length in plcm.
 *
 * @param   plcm   Plcm handle.
 *
 * @return  String length.
 */
pl_size_t plss_length( plcm_t plcm );


/**
 * @brief String in plcm.
 *
 * @param   plcm   Plcm handle.
 *
 * @return  Plsr handle.
 */
plsr_s plss_ref( plcm_t plcm );


/**
 * @brief Is plss string empty?
 *
 * @param    plcm   Plcm handle.
 *
 * @return True, if plss string is empty.
 */
pl_bool_t plss_is_empty( plcm_t plcm );



/* ------------------------------------------------------------
 * String Referencing:
 */

/**
 * @brief Create plsr from plcm.
 *
 * @param    plcm   Plcm handle.
 *
 * @return Plsr.
 */
plsr_s plsr_from_plcm( plcm_t plcm );


/**
 * @brief Create plsr from c-string.
 *
 * @param    str   C-string.
 *
 * @return  Plsr handle.
 */
plsr_s plsr_from_string( const char* str );


/**
 * @brief Create plsr from c-string and length.
 *
 * @param    str     C-string.
 * @param    length  C-string length.
 *
 * @return  Plsr handle.
 */
plsr_s plsr_from_string_and_length( const char* str, pl_size_t length );


/**
 * @brief String in plsr.
 *
 * @param   plsr   Plsr handle.
 *
 * @return  String reference.
 */
const char* plsr_string( plsr_s plsr );


/**
 * @brief String length in plsr.
 *
 * @param   plsr   Plsr handle.
 *
 * @return  String length.
 */
pl_size_t plsr_length( plsr_s plsr );


/**
 * @brief Compare two plsr.
 *
 * @param   p1     Plsr handle.
 * @param   p2     Plsr handle.
 *
 * @return  True, if length and content match.
 */
pl_bool_t plsr_compare( plsr_s p1, plsr_s p2 );


/**
 * @brief Compare two plsr for n characters.
 *
 * @param   p1     Plsr handle.
 * @param   p2     Plsr handle.
 * @param   n      Comparison length.
 *
 * @return  True, content match for the given length.
 */
pl_bool_t plsr_compare_n( plsr_s p1, plsr_s p2, pl_size_t n );


/**
 * @brief Create a null plsr.
 *
 * @return  Plsr handle.
 */
plsr_s plsr_null( pl_none );


/**
 * @brief Is plsr null?
 *
 * @param    plsr   Plsr handle.
 *
 * @return True, if plsr is null.
 */
pl_bool_t plsr_is_null( plsr_s plsr );


/**
 * @brief Is plsr an empty string?
 *
 * Empty means that there is a string associated, but length is zero.
 *
 * @param    plsr   Plsr handle.
 *
 * @return True, if plsr is empty.
 */
pl_bool_t plsr_is_empty( plsr_s plsr );


/**
 * @brief Return next line content, without the terminating newline.
 *
 * The next line is a string reference starting from current offset to
 * the newline or eof. Offset is updated to be after next newline or
 * at eof, if eof was encountered.
 *
 * @param          plsr   Plsr handle.
 * @param[in,out]  offset Current offset.
 *
 * @return  Plsr to current line, or null for eof.
 */
plsr_s plsr_next_line( plsr_s plsr, pl_size_p offset );


/**
 * @brief Return indeced char.
 *
 * Note: return 0, if index is out-of-range.
 *
 * @param   plsr   Plsr handle.
 * @param   index  Char index.
 *
 * @return  Indexed char.
 */
char plsr_index( plsr_s plsr, pl_pos_t index );


#endif
