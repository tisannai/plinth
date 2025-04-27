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
    plss_append( &plcm, plsr_from_c( s1 ) );
    TEST_ASSERT_EQUAL( strlen( s1 ), plss_length( &plcm ) );
    TEST_ASSERT( strcmp( s1, plss_string( &plcm ) ) == 0 );
    plss_reformat( &plcm, "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", plss_string( &plcm ) ) == 0 );
    plcm_del( &plcm );

    plam_use( &plam, mem, 1024 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    m = plam_get( &plam, 256 );
    TEST_ASSERT_EQUAL( 256, plam_used( &plam ) );
    TEST_ASSERT( m != NULL );
    plam_put( &plam, 256 );
    TEST_ASSERT_EQUAL( 0, plam_used( &plam ) );
    plam_del( &plam );

    s2 = pl_strdup( NULL );
    TEST_ASSERT_EQUAL( NULL, s2 );
    s2 = pl_strdup( s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    pl_free_memory( s2 );

    s2 = pl_format( "%s_%s", s1, s1 );
    TEST_ASSERT( strcmp( "testing..._testing...", s2 ) == 0 );
    pl_free_memory( s2 );

    plbm_use( &plbm, mem, 124, 8 );
    TEST_ASSERT_EQUAL( 124, plbm_nsize( &plbm ) );
    TEST_ASSERT_EQUAL( 8, plbm_bsize( &plbm ) );
    plbm_del( &plbm );
}


void test_plam( void )
{
    plam_s plam;
    plam_s plam2;
    char   mem[ 1024 ];
    pl_t   m;
    char*  s1;
    char*  s2;

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

    plam_use( &plam, mem, 1024 );
    s1 = "testing...";
    s2 = plam_strdup( &plam, s1 );
    TEST_ASSERT( strcmp( s1, s2 ) == 0 );
    s2 = plam_format( &plam, "hello %s\n", s1 );
    TEST_ASSERT( strcmp( "hello testing...\n", s2 ) == 0 );
    s2 = plam_strdup( &plam, NULL );
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
}


void test_plbm( void )
{
    plbm_s plbm;
    char   mem[ 1024 ];
    pl_t   m1, m2, m3;
    char*  s1;
    char*  s2;

    plbm_new( &plbm, sizeof( plam_node_s ) + 2 * 8, 8 );
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
}


void test_plcm( void )
{
    plcm_s plcm;
    plam_s plam;
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

    plam_use( &plam, mem, 1024 );
    plcm_use_plam( &plcm, &plam, 4 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
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
    plss_reformat( &plcm, "%s", s1 );

    plam_use( &plam, mem, 1024 );
    plcm_use_plam( &plcm, &plam, 8 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 0 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_reformat( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plcm_del( &plcm );

    plcm_empty( &plcm, 16 );
    TEST_ASSERT_EQUAL( 0, plcm_debt( &plcm ) );
    plss_format( &plcm, "%s", s1 );
    TEST_ASSERT_EQUAL( 1, plcm_debt( &plcm ) );
    TEST_ASSERT_EQUAL( 10, plss_length( &plcm ) );
    plss_format( &plcm, "%s %s %s %s", s1, s1, s1, s1 );
    TEST_ASSERT_EQUAL( 53, plss_length( &plcm ) );
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
