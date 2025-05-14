#include "unity.h"
#include "plinth.h"
#include <string.h>
#include <unistd.h>

void dbg_break( void ) {}


void test_basic( void )
{
    plam_s plam;
    plbm_s plbm;
    plcm_s plcm;

    char     mem[ 1024 ];
    char*    s1;
    char*    s2;
    char*    str;
    pl_pos_t item;
    pl_t     m;

    plcm_new( &plcm, 1024 );
    s1 = "testing...";
    item = plcm_get_pos( &plcm, 128 );
    TEST_ASSERT_EQUAL( 128, plcm_used( &plcm ) );
    plcm_set( &plcm, item, s1, strlen( s1 ) + 1 );
    TEST_ASSERT( strcmp( s1, plcm_ref( &plcm, item ) ) == 0 );
    plcm_del( &plcm );


    plcm_new( &plcm, 1024 );
    plcm_terminate( &plcm, sizeof( char ) );
    TEST_ASSERT_EQUAL( 0, plss_length( &plcm ) );
    plss_append( &plcm, plsr_from_string( s1 ) );
    TEST_ASSERT_EQUAL( strlen( s1 ), plss_length( &plcm ) );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_reformat_string( &plcm, "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", plss_string( &plcm ) ) == 0 );
    plcm_del( &plcm );

    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    TEST_ASSERT_EQUAL( 1024 - sizeof( plam_node_s ), plam_free( &plam ) );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    plam_put( &plam, 256 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_del( &plam );

    s2 = pl_alloc_string( NULL );
    TEST_ASSERT_EQUAL( NULL, s2 );
    s2 = pl_alloc_string( s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    pl_free_memory( s2 );

    s2 = pl_format_string( "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", s2 ) == 0 );
    pl_free_memory( s2 );

    plbm_use( &plbm, mem, 124, 8 );
    TEST_ASSERT_EQUAL( 124, plbm_node_size( &plbm ) );
    TEST_ASSERT_EQUAL( 8, plbm_block_size( &plbm ) );
    TEST_ASSERT_EQUAL( 124 - sizeof( plam_node_s ), plbm_node_capacity( &plbm ) );
    plbm_del( &plbm );
}


void test_plam( void )
{
    plam_s plam;
    plam_s plam2;
    char   mem[ 1024 ];
    plbm_s plbm;
    pl_t   m;
    char*  s1;
    char*  s2;
    pl_t   m1;
    pl_t   m2;

    plam_new( &plam, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 768, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam ) );


    /* Test plam_use_plam. */
    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_use_plam( &plam2, &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    m = plam_get( &plam2, 16 );
    TEST_ASSERT_EQUAL( 16, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam2, 16 );
    TEST_ASSERT_EQUAL( 32, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    plam_put( &plam, 256 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam ) );


    /* Test plam_use_plbm. */
    plbm_use( &plbm, mem, 1024, 384 );
    plam_use_plbm( &plam, &plbm );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_use_plam( &plam2, &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    m = plam_get( &plam2, 16 );
    TEST_ASSERT_EQUAL( 16, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam2, 16 );
    TEST_ASSERT_EQUAL( 32, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    plam_put( &plam, 256 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam ) );


    /* Test plam_into_plam. */
    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_into_plam( &plam2, &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    m = plam_get( &plam2, 128 );
    TEST_ASSERT_EQUAL( 128, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam2, 128 );
    TEST_ASSERT_EQUAL( 128, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    plam_del( &plam2 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam ) );


    /* Test plam_into_plbm. */
    plbm_use( &plbm, mem, 1024, 384 );
    plam_into_plbm( &plam2, &plbm );
    m = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    TEST_ASSERT( m != NULL );
    plam_del( &plam2 );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam2 ) );
    plbm_del( &plbm );


    /*
      display plam
      display *plam.node
     */

    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 768, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 0, plam_size( &plam ) );

    plam_empty( &plam, 2048 );
    TEST_ASSERT( plam_is_empty( &plam ) );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    TEST_ASSERT_EQUAL( 0, plam_free( &plam ) );

    plam_use( &plam, mem, 1024 );
    s1 = "testing...";
    s2 = plam_store_string( &plam, s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    s2 = plam_format_string( &plam, "hello %s\n", s1 );
    TEST_ASSERT( strcmp( "hello testing...\n", s2 ) == 0 );
    s2 = plam_store_string( &plam, NULL );
    TEST_ASSERT_EQUAL( NULL, s2 );
    plam_del( &plam );

    plam_new( &plam, 2 );
    TEST_ASSERT( plam_is_empty( &plam ) );

    plam_empty( &plam, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    m = plam_get( &plam, 1024 );
    TEST_ASSERT( m == NULL );
    plam_del( &plam );

    /* Test plam_get/plam_put. */
    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    for ( int i = 0; i < 6; i++ ) {
        m = plam_get( &plam, 256 );
        TEST_ASSERT( m != NULL );
    }
    TEST_ASSERT_EQUAL( 768, plam_used( &plam ) );
    for ( int i = 0; i < 6; i++ ) {
        plam_put( &plam, 256 );
        TEST_ASSERT( m != NULL );
    }
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_put( &plam, 256 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    for ( int i = 0; i < 6; i++ ) {
        m = plam_get( &plam, 256 );
        TEST_ASSERT( m != NULL );
    }
    TEST_ASSERT_EQUAL( 768, plam_used( &plam ) );
    plam_del( &plam );


    /* Cover plam__node_del. */
    plam_new( &plam, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m1 = plam_get( &plam, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    m2 = plam_get( &plam, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    plam_put( &plam, 512 );
    plam_put( &plam, 512 );
    plam_del( &plam );

    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m1 = plam_get( &plam, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    m2 = plam_get( &plam, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam ) );
    plam_put( &plam, 512 );
    plam_put( &plam, 512 );
    plam_del( &plam );

    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_into_plam( &plam2, &plam, 768 );
    TEST_ASSERT_EQUAL( 768, plam_used( &plam ) );
    m1 = plam_get( &plam2, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam2 ) );
    m2 = plam_get( &plam2, 512 );
    TEST_ASSERT_EQUAL( 512, plam_used( &plam2 ) );
    plam_put( &plam2, 512 );
    plam_put( &plam2, 512 );
    plam_del( &plam2 );
    plam_del( &plam );

    plbm_use( &plbm, mem, 1024, 384 );
    plam_into_plbm( &plam2, &plbm );
    m1 = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    m2 = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    m1 = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    m2 = plam_get( &plam2, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam2 ) );
    plam_put( &plam2, 256 );
    plam_put( &plam2, 256 );
    plam_put( &plam2, 256 );
    plam_put( &plam2, 256 );
    plam_del( &plam2 );
    plam_del( &plam );
}


void test_plbm( void )
{
    plbm_s plbm;
    plbm_s plbm2;
    char   mem[ 1024 ];
    plam_s plam;
    pl_t   m1, m2, m3;
    char*  s1;
    char*  s2;
    pl_t   mm[ 24 ];

    plbm_new( &plbm, sizeof( plam_node_s ) + 2 * 8, 8 );
    m1 = plbm_get( &plbm );
    TEST_ASSERT( m1 != NULL );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_continuous( &plbm ) );
    m2 = plbm_get( &plbm );
    TEST_ASSERT( m2 != NULL );
    m3 = plbm_get( &plbm );
    TEST_ASSERT( m3 != NULL );
    plbm_put( &plbm, m3 );
    plbm_put( &plbm, m1 );
    plbm_put( &plbm, m2 );
    for ( int i = 0; i < 4; i++ ) {
        m1 = plbm_get( &plbm );
        TEST_ASSERT( m1 != NULL );
    }
    TEST_ASSERT_EQUAL( pl_false, plbm_is_continuous( &plbm ) );
    plbm_del( &plbm );


    plbm_use( &plbm, mem, sizeof( plam_node_s ) + 2 * 8, 8 );
    m1 = plbm_get( &plbm );
    TEST_ASSERT( m1 != NULL );
    m2 = plbm_get( &plbm );
    TEST_ASSERT( m2 != NULL );
    m3 = plbm_get( &plbm );
    TEST_ASSERT( m3 != NULL );
    plbm_put( &plbm, m3 );
    plbm_put( &plbm, m1 );
    plbm_put( &plbm, m2 );
    for ( int i = 0; i < 4; i++ ) {
        m1 = plbm_get( &plbm );
        TEST_ASSERT( m1 != NULL );
    }
    plbm_del( &plbm );


    plbm_empty( &plbm, sizeof( plam_node_s ) + 2 * 8, 8 );
    m1 = plbm_get( &plbm );
    TEST_ASSERT( m1 != NULL );
    m2 = plbm_get( &plbm );
    TEST_ASSERT( m2 != NULL );
    m3 = plbm_get( &plbm );
    TEST_ASSERT( m3 != NULL );
    plbm_put( &plbm, m3 );
    plbm_put( &plbm, m1 );
    plbm_put( &plbm, m2 );
    for ( int i = 0; i < 4; i++ ) {
        m1 = plbm_get( &plbm );
        TEST_ASSERT( m1 != NULL );
    }
    plbm_del( &plbm );

    plbm_new( &plbm, 4, 4 );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_empty( &plbm ) );
    plbm_use( &plbm, mem, 4, 4 );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_empty( &plbm ) );
    plbm_empty( &plbm, 4, 4 );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_empty( &plbm ) );
    plbm_empty( &plbm, 1024, 4 );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_empty( &plbm ) );
    plbm_empty( &plbm, 4, 8 );
    TEST_ASSERT_EQUAL( pl_true, plbm_is_empty( &plbm ) );

    plam_use( &plam, mem, 1024 );
    plbm_use_plam( &plbm2, &plam, 256, 64 );
    for ( int i = 0; i < 6; i++ ) {
        mm[ i ] = plbm_get( &plbm2 );
        TEST_ASSERT( mm[ i ] != NULL );
    }
    for ( int i = 0; i < 6; i++ ) {
        plbm_put( &plbm2, mm[ i ] );
    }
    plbm_del( &plbm2 );
    plbm_del( &plbm );


    plbm_use( &plbm, mem, 1024, 256 );
    plbm_use_plbm( &plbm2, &plbm, 64 );
    for ( int i = 0; i < 6; i++ ) {
        mm[ i ] = plbm_get( &plbm2 );
        TEST_ASSERT( mm[ i ] != NULL );
    }
    for ( int i = 0; i < 6; i++ ) {
        plbm_put( &plbm2, mm[ i ] );
    }
    plbm_del( &plbm2 );
    plbm_del( &plbm );

    plam_use( &plam, mem, 1024 );
    plbm_into_plam( &plbm2, &plam, 256, 64 );
    for ( int i = 0; i < 6; i++ ) {
        mm[ i ] = plbm_get( &plbm2 );
        TEST_ASSERT( mm[ i ] != NULL );
    }
    for ( int i = 0; i < 6; i++ ) {
        plbm_put( &plbm2, mm[ i ] );
    }
    plbm_del( &plbm2 );
    plbm_del( &plbm );

    plbm_use( &plbm, mem, 1024, 256 );
    plbm_into_plbm( &plbm2, &plbm, 64 );
    for ( int i = 0; i < 6; i++ ) {
        mm[ i ] = plbm_get( &plbm2 );
        TEST_ASSERT( mm[ i ] != NULL );
    }
    for ( int i = 0; i < 6; i++ ) {
        plbm_put( &plbm2, mm[ i ] );
    }
    plbm_del( &plbm2 );
    plbm_del( &plbm );
}


void test_plcm( void )
{
    plcm_s    plcm;
    plam_s    plam;
    char      mem[ 1024 ];
    pl_t      m;
    char*     s1;
    char*     s2;
    char**    s3;
    plsr_s    sr;
    pl_bool_t ret;

    s1 = "testing...";

    plcm_new( &plcm, 16 );
    TEST_ASSERT_EQUAL( pl_true, plss_is_empty( &plcm ) );
    plss_append_string( &plcm, s1 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_append_char( &plcm, 'a' );
    TEST_ASSERT( strcmp( "testing...a", plss_string( &plcm ) ) == 0 );
    plss_set( &plcm, plsr_from_string( s1 ) );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_format_string( &plcm, " %s", s1 );
    TEST_ASSERT( strcmp( "testing... testing...", plss_string( &plcm ) ) == 0 );
    plss_reformat_string( &plcm, "%s", s1 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );

    plss_remove( &plcm, 2, 2 );
    TEST_ASSERT( strcmp( "teing...", plss_string( &plcm ) ) == 0 );
    plss_insert( &plcm, 2, "st", 2 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_insert( &plcm, 10, "st", 2 );
    TEST_ASSERT( strcmp( "testing...st", plss_string( &plcm ) ) == 0 );
    TEST_ASSERT_EQUAL( pl_false, plss_is_empty( &plcm ) );
    plss_remove( &plcm, 10, 2 );
    sr = plss_ref( &plcm );
    TEST_ASSERT_EQUAL( 10, sr.length );
    TEST_ASSERT( strcmp( s1, sr.string ) == 0 );
    plcm_del( &plcm );

    plam_use( &plam, mem, 1024 );
    plcm_use_plam( &plcm, &plam, 4 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat_string( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_reset( &plcm );
    s2 = plcm_get_ref( &plcm, 10 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plcm_reset( &plcm );
    plcm_store( &plcm, s1, 10 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    TEST_ASSERT_EQUAL( 12, plcm_size( &plcm ) );
    TEST_ASSERT( strcmp( s1, plcm_data( &plcm ) ) == 0 );
    TEST_ASSERT_EQUAL( plcm_data( &plcm ) + plcm_used( &plcm ), plcm_end( &plcm ) );

    plcm_del( &plcm );
    TEST_ASSERT_EQUAL( 1, plcm_is_empty( &plcm ) );
    plam_del( &plam );
    TEST_ASSERT_EQUAL( 1, plam_is_empty( &plam ) );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 1, plcm_is_empty( &plcm ) );

    plcm_use_plam( &plcm, &plam, 4 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat_string( &plcm, "%s", s1 );

    plam_use( &plam, mem, 1024 );
    plcm_use_plam( &plcm, &plam, 8 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat_string( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 0 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat_string( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat_string( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_format_string( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plss_format_string( &plcm, "%s %s %s %s", s1, s1, s1, s1 );
    TEST_ASSERT_EQUAL( 53, plss_length( &plcm ) );
    plcm_del( &plcm );


    s2 = "again";
    plcm_empty( &plcm, 4 * sizeof( char* ) );

    plcm_store_ptr( &plcm, &s1 );
    plcm_store_ptr( &plcm, &s2 );
    plcm_store_null( &plcm );

    s3 = plcm_ref_ptr( &plcm, 0 );
    TEST_ASSERT( strcmp( s1, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 1 );
    TEST_ASSERT( strcmp( s2, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 2 );
    TEST_ASSERT_EQUAL( 0, *s3 );
    TEST_ASSERT_EQUAL( 2, plcm_used_ptr( &plcm ) );

    plcm_remove( &plcm, 0, sizeof( void* ) );
    s3 = plcm_ref_ptr( &plcm, 0 );
    TEST_ASSERT( strcmp( s2, *s3 ) == 0 );
    plcm_store_null( &plcm );
    s3 = plcm_ref_ptr( &plcm, 1 );
    TEST_ASSERT_EQUAL( 0, *s3 );
    TEST_ASSERT_EQUAL( 1, plcm_used_ptr( &plcm ) );

    plcm_insert( &plcm, 0, &s1, sizeof( char* ) );
    s3 = plcm_ref_ptr( &plcm, 0 );
    TEST_ASSERT( strcmp( s1, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 1 );
    TEST_ASSERT( strcmp( s2, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 2 );
    TEST_ASSERT_EQUAL( 0, *s3 );

    plcm_insert( &plcm, 2 * sizeof( char* ), &s1, sizeof( char* ) );
    plcm_insert( &plcm, 3 * sizeof( char* ), &s2, sizeof( char* ) );
    s3 = plcm_ref_ptr( &plcm, 0 );
    TEST_ASSERT( strcmp( s1, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 1 );
    TEST_ASSERT( strcmp( s2, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 2 );
    TEST_ASSERT( strcmp( s1, *s3 ) == 0 );
    s3 = plcm_ref_ptr( &plcm, 3 );
    TEST_ASSERT( strcmp( s2, *s3 ) == 0 );

    ret = plcm_terminate_ptr( &plcm );
    TEST_ASSERT_EQUAL( pl_false, ret );

    plcm_del( &plcm );
}


void test_plsr( void )
{
    //     plcm_s plcm;
    //     plam_s plam;
    //     char mem[ 1024 ];
    //     pl_t m;
    char*  s1;
    char*  s2;
    plsr_s sr;

    s1 = "testing...";
    sr = plsr_from_string( s1 );
    TEST_ASSERT( strcmp( s1, plsr_string( sr ) ) == 0 );
    TEST_ASSERT_EQUAL( 10, plsr_length( sr ) );

    sr = plsr_from_string( NULL );
    TEST_ASSERT_EQUAL( 0, plsr_length( sr ) );

    s1 = "testing...";
    sr = plsr_from_string( s1 );
    TEST_ASSERT_EQUAL( pl_true, plsr_compare( sr, plsr_from_string( s1 ) ) );

    s2 = "testing";
    sr = plsr_from_string( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare( sr, plsr_from_string( s1 ) ) );

    s2 = "...testing";
    sr = plsr_from_string( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare( sr, plsr_from_string( s1 ) ) );

    s1 = "testing...";
    sr = plsr_from_string( s1 );
    TEST_ASSERT_EQUAL( pl_true, plsr_compare_n( sr, plsr_from_string( s1 ), 8 ) );

    s2 = "testing";
    sr = plsr_from_string( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare_n( sr, plsr_from_string( s1 ), 8 ) );

    s2 = "...testing";
    sr = plsr_from_string( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare_n( sr, plsr_from_string( s1 ), 8 ) );

    sr = plsr_null();
    TEST_ASSERT_EQUAL( pl_true, plsr_is_null( sr ) );
}



pl_none ui_echo( pl_t env, pl_t argi, pl_t argo )
{
    *( (char**)argo ) = (char*)argi;
}


void test_ui( void )
{
    pl_ui_s ui;
    char*   msg_out;
    char*   msg_in;

    pl_ui_init( &ui, NULL, ui_echo );
    msg_out = "hello";
    pl_ui_do( &ui, msg_out, &msg_in );
    TEST_ASSERT( strcmp( msg_out, msg_in ) == 0 );
}
