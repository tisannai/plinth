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


#include <stdint.h>
#include <stdarg.h>



/* ------------------------------------------------------------
 * Simple type definition features.
 */

/** Re-define type and related pointers. */
#define pl_type( base, type )   \
    typedef base      type##_t; \
    typedef type##_t* type##_p;



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

#define pl_struct_type( name )              \
    typedef struct name##struct_s name##_s; \
    typedef name##_s*             name##_t; \
    typedef name##_s**            name##_p;

#define pl_struct_body( name ) struct name##struct_s


/**
 * Define enumeration and corresponding type. An enum is created as
 * <enum>_e, a typedef is created as <enum>_t.
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

/* Align size (up) to multiple of alignment. */
#define PLINTH_ALIGN_TO( size, alignment ) \
    ( ( ( ( size ) + ( alignment ) - 1 ) / ( alignment ) ) * ( alignment ) )


/** Allocator affinity type. */
pl_enum( pl_aa ){ PL_AA_SELF = 0, PL_AA_HEAP, PL_AA_PLAM, PL_AA_PLBM, PL_AA_PLCM };


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
    uint8_t     data[ 0 ]; /**< Data location. */
};
pl_struct( plam )
{
    plam_node_t node; /**< Current node. */
    pl_size_t   size; /**< Node size. */
    pl_aa_t     type; /**< Reservation type. */
    pl_t        ator; /**< Allocator. */
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
    pl_aa_t     type;  /**< Reservation type. */
    pl_t        ator;  /**< Allocator. */
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

#define pl_alloc_memory_for_type( type ) pl_alloc_memory( sizeof( type ) )

#define plam_get_for_type( plam, type ) plam_get( ( plam ), sizeof( type ) )
#define plam_put_for_type( plam, type ) plam_put( ( plam ), sizeof( type ) )

#define plcm_get_pos_for_type( plam, type ) plcm_get_pos( ( plam ), sizeof( type ) )
#define plcm_get_ref_for_type( plam, type ) plcm_get_ref( ( plam ), sizeof( type ) )
#define plcm_store_for_type( plam, data, type ) plcm_store( ( plam ), ( data ), sizeof( type ) )
#define plcm_ref_for_type( plam, pos, type ) plcm_ref( ( plam ), ( pos ) * sizeof( type ) )
#define plcm_set_for_type( plam, pos, data, type ) \
    plcm_set( ( plcm ), ( pos ), ( data ), sizeof( type ) )
#define plcm_terminate_for_type( plam, type ) plcm_terminate( ( plam ), sizeof( type ) )
#define plcm_used_for_type( plam, type ) ( plcm_used( ( plam ) ) / sizeof( type ) )

/*
  Declare stack local plcm with "name" and reserve array, with
  "size", for storage.
 */
#define plss_use( name, size )              \
    plcm_s name;                            \
    char   name##_plss_declare[ ( size ) ]; \
    plcm_use( &name, name##_plss_declare, ( size ) )



/* ------------------------------------------------------------
 * Basic (heap) memory allocation:
 */

/**
 * @brief Allocate memory from heap.
 *
 * @param   size   Allocation size in bytes.
 *
 * @return  Pointer to allocation.
 */
pl_t pl_alloc_memory( pl_size_t size );


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
 * @return  Pointer to reallocation.
 */
pl_t pl_realloc_memory( pl_t mem, pl_size_t size );


/**
 * @brief Duplicate plsr string as heap memory.
 *
 * @param   plsr    Plsr string to duplicate.
 *
 * @return  Duplicate.
 */
plsr_s pl_alloc_plsr( plsr_s plsr );


/**
 * @brief Duplicate string as heap memory.
 *
 * @param   str    String to duplicate.
 *
 * @return  Duplicated string.
 */
char* pl_alloc_string( const char* str );


/**
 * @brief Format string to heap memory.
 *
 * @param   fmt    Format specifier.
 *
 * @return  Formatted string in heap.
 */
char* pl_format_string( const char* fmt, ... );



/* ------------------------------------------------------------
 * Arena Memory Allocator:
 */

/**
 * @brief Create plam in heap (with debt).
 *
 * @param    plam   Plam handle.
 * @param    size   Node size.
 *
 * @return None
 */
pl_none plam_new( plam_t plam, pl_size_t size );


/**
 * @brief Initiate plam to node (no debt).
 *
 * @param    plam   Plam handle.
 * @param    node   Node.
 * @param    size   Node size.
 *
 * @return None
 */
pl_none plam_use( plam_t plam, pl_t node, pl_size_t size );


/**
 * @brief Initiate nested plam from plam (no debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plam handle.
 * @param    size   Node size.
 *
 * @return None
 */
pl_none plam_use_plam( plam_t plam, plam_t host, pl_size_t size );


/**
 * @brief Initiate nested plam from plbm (no debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plbm handle.
 *
 * @return None
 */
pl_none plam_use_plbm( plam_t plam, plbm_t host );


/**
 * @brief Deploy plam inside plam (debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plam handle.
 * @param    size   Node size.
 *
 * @return None
 */
pl_none plam_into_plam( plam_t plam, plam_t host, pl_size_t size );


/**
 * @brief Deploy plam inside plbm (debt).
 *
 * @param    plam   Nested plam handle.
 * @param    host   Plbm handle.
 *
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
 */
pl_none plbm_use_plam( plbm_t plbm, plam_t host, pl_size_t nsize, pl_size_t bsize );


/**
 * @brief Initiate nested plbm from plbm (no debt).
 *
 * Node size is inherited from host block size.
 *
 * @param    plbm   Nested plbm handle.
 * @param    host   Plbm handle.
 * @param    nsize  Node size.
 * @param    bsize  Block size.
 *
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
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
 * @return None
 */
pl_none plbm_del( plbm_t plbm );


/**
 * @brief Get allocation from plbm.
 *
 * @param    plbm   Plbm handle.
 * @param    size   Allocation size.
 *
 * @return Allocation.
 */
pl_t plbm_get( plbm_t plbm );


/**
 * @brief Put allocation back to plbm.
 *
 * User is responsible in making the puts in reserver order with
 * correct sizes.
 *
 * @param    plbm   Plbm handle.
 * @param    size   Allocation size.
 *
 * @return None
 */
pl_none plbm_put( plbm_t plbm, pl_t block );


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
 * @return None
 */
pl_none plcm_new( plcm_t plcm, pl_size_t size );


/**
 * @brief Create plcm to pre-existing allocation (no debt).
 *
 * @param    plcm   Plcm handle.
 * @param    mem    Allocation.
 * @param    size   Size of allocation.
 *
 * @return None
 */
pl_none plcm_use( plcm_t plcm, pl_t mem, pl_size_t size );


/**
 * @brief Initiate nested plcm from plam (no debt).
 *
 * @param    plcm   Nested plcm handle.
 * @param    host   Plam handle.
 * @param    size   Allocation size.
 *
 * @return None
 */
pl_none plcm_use_plam( plcm_t plcm, plam_t host, pl_size_t size );


/**
 * @brief Initiate nested plcm from plbm (no debt).
 *
 * @param    plcm   Nested plcm handle.
 * @param    host   Plbm handle.
 *
 * @return None
 */
pl_none plcm_use_plbm( plcm_t plcm, plbm_t host );


/**
 * @brief Create empty plcm.
 *
 * Empty plcm is a placeholder with handle setup for allocations.
 * However, no heap allocations are made at creation.
 *
 * @param    plcm   Plcm handle.
 * @param    size   Allocation size (for future).
 *
 * @return None
 */
pl_none plcm_empty( plcm_t plcm, pl_size_t size );


/**
 * @brief Delete plcm.
 *
 * If plcm has debt, then the heap memory is deallocated. If plcm has
 * no debt, deletion is mute.
 *
 * @param    plcm   Plcm handle.
 *
 * @return None
 */
pl_none plcm_del( plcm_t plcm );


/**
 * @brief Resize plcm allocation.
 *
 * @param    plcm   Plcm handle.
 * @param    size   New allocation size.
 *
 * @return None
 */
pl_none plcm_resize( plcm_t plcm, pl_size_t size );


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
 * @param    ptraddr  Address of the pointer.
 *
 * @return Allocation position.
 */
pl_pos_t plcm_store_ptr( plcm_t plcm, pl_t ptraddr );


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
 * @return Allocation reference.
 */
pl_t plcm_ref_ptr( plcm_t plcm, pl_pos_t pos );


/**
 * @brief Set value for data.
 *
 * @param    plcm   Plcm handle.
 * @param    pos    Allocation position.
 * @param    data   Data to store.
 * @param    size   Data size.
 *
 * @return None.
 */
pl_none plcm_set( plcm_t plcm, pl_pos_t pos, pl_t data, pl_size_t size );


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



/* ------------------------------------------------------------
 * String Storage:
 */

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
const char* plsr_string( plsr_s sr );


/**
 * @brief String length in plsr.
 *
 * @param   plsr   Plsr handle.
 *
 * @return  String length.
 */
pl_size_t plsr_length( plsr_s sr );


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


#endif
