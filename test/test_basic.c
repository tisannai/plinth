#include "unity.h"
#include "plinth.h"
#include <string.h>
#include <unistd.h>

void dbg_break( void ) {}


void test_basic( void )
{
    plcm_s plcm;
    plsm_s plsm;

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
    plss_append( &plcm, plsr_from_c( s1 ) );
    TEST_ASSERT_EQUAL( strlen( s1 ), plss_length( &plcm ) );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_reformat( &plcm, "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", plss_string( &plcm ) ) == 0 );
    plcm_del( &plcm );

    plsm_use( &plsm, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plsm_used( &plsm ) );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    plsm_del( &plsm );

    s2 = pl_strdup( NULL );
    TEST_ASSERT_EQUAL( NULL, s2 );
    s2 = pl_strdup( s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    pl_free_memory( s2 );

    s2 = pl_format( "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", s2 ) == 0 );
    pl_free_memory( s2 );
}


void test_plsm( void )
{
    plsm_s plsm;
    char   mem[ 1024 ];
    pl_t   m;
    char*  s1;
    char*  s2;

    plsm_new( &plsm, 1024 );
    TEST_ASSERT_EQUAL( 0, plsm_used( &plsm ) );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 512, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 768, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    plsm_del( &plsm );
    TEST_ASSERT_EQUAL( 0, plsm_size( &plsm ) );

    /*
      display plsm
      display *plsm.node
     */

    plsm_use( &plsm, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plsm_used( &plsm ) );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 512, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 768, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    plsm_del( &plsm );
    TEST_ASSERT_EQUAL( 0, plsm_size( &plsm ) );

    plsm_empty( &plsm, 2048 );
    TEST_ASSERT( plsm_is_empty( &plsm ) );

    plsm_use( &plsm, mem, 1024 );
    s1 = "testing...";
    s2 = plsm_strdup( &plsm, s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    s2 = plsm_format( &plsm, "hello %s\n", s1 );
    TEST_ASSERT( strcmp( "hello testing...\n", s2 ) == 0 );
    s2 = plsm_strdup( &plsm, NULL );
    TEST_ASSERT_EQUAL( NULL, s2 );
    plsm_del( &plsm );

    plsm_new( &plsm, 2 );
    TEST_ASSERT( plsm_is_empty( &plsm ) );

    plsm_empty( &plsm, 1024 );
    TEST_ASSERT_EQUAL( 0, plsm_used( &plsm ) );
    m = plsm_get( &plsm, 256 );
    TEST_ASSERT_EQUAL( 256, plsm_used( &plsm ) );
    TEST_ASSERT( m != NULL );
    m = plsm_get( &plsm, 1024 );
    TEST_ASSERT( m == NULL );
    plsm_del( &plsm );
}


void test_plcm( void )
{
    plcm_s plcm;
    plsm_s plsm;
    char   mem[ 1024 ];
    pl_t   m;
    char*  s1;
    char*  s2;
    plsr_s sr;

    s1 = "testing...";

    plcm_new( &plcm, 16 );
    plss_append_c( &plcm, s1 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_append_ch( &plcm, 'a' );
    TEST_ASSERT( strcmp( "testing...a", plss_string( &plcm ) ) == 0 );
    plss_set( &plcm, plsr_from_c( s1 ) );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_format( &plcm, " %s", s1 );
    TEST_ASSERT( strcmp( "testing... testing...", plss_string( &plcm ) ) == 0 );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    sr = plss_ref( &plcm );
    TEST_ASSERT_EQUAL( 10, sr.length );
    TEST_ASSERT( strcmp( s1, sr.string ) == 0 );
    plcm_del( &plcm );

    plsm_use( &plsm, mem, 1024 );
    plcm_use_plsm( &plcm, &plsm, 4 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_heap( &plcm ) );
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
    plsm_del( &plsm );
    TEST_ASSERT_EQUAL( 1, plsm_is_empty( &plsm ) );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 1, plcm_is_empty( &plcm ) );

    plsm_use( &plsm, mem, 1024 );
    plcm_use_plsm( &plcm, &plsm, 4 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );

    plsm_use( &plsm, mem, 1024 );
    plcm_use_plsm( &plcm, &plsm, 8 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_heap( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 0 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_heap( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_heap( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_heap( &plcm ) );
    plss_format( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_heap( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plss_format( &plcm, "%s %s %s %s", s1, s1, s1, s1 );
    TEST_ASSERT_EQUAL( 53, plss_length( &plcm ) );
    plcm_del( &plcm );
}


void test_plsr( void )
{
    //     plcm_s plcm;
    //     plsm_s plsm;
    //     char mem[ 1024 ];
    //     pl_t m;
    char*  s1;
    char*  s2;
    plsr_s sr;

    s1 = "testing...";
    sr = plsr_from_c( s1 );
    TEST_ASSERT( strcmp( s1, plsr_string( sr ) ) == 0 );
    TEST_ASSERT_EQUAL( 10, plsr_length( sr ) );

    sr = plsr_from_c( NULL );
    TEST_ASSERT_EQUAL( 0, plsr_length( sr ) );

    s1 = "testing...";
    sr = plsr_from_c( s1 );
    TEST_ASSERT_EQUAL( pl_true, plsr_compare( sr, plsr_from_c( s1 ) ) );

    s2 = "testing";
    sr = plsr_from_c( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare( sr, plsr_from_c( s1 ) ) );

    s2 = "...testing";
    sr = plsr_from_c( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare( sr, plsr_from_c( s1 ) ) );

    s1 = "testing...";
    sr = plsr_from_c( s1 );
    TEST_ASSERT_EQUAL( pl_true, plsr_compare_n( sr, plsr_from_c( s1 ), 8 ) );

    s2 = "testing";
    sr = plsr_from_c( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare_n( sr, plsr_from_c( s1 ), 8 ) );

    s2 = "...testing";
    sr = plsr_from_c( s2 );
    TEST_ASSERT_EQUAL( pl_false, plsr_compare_n( sr, plsr_from_c( s1 ), 8 ) );

    sr = plsr_invalid();
    TEST_ASSERT_EQUAL( pl_true, plsr_is_invalid( sr ) );
    TEST_ASSERT_EQUAL( pl_false, plsr_is_valid( sr ) );
}
