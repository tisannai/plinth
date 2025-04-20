#include "unity.h"
#include "plinth.h"
#include <string.h>
#include <unistd.h>

void dbg_break( void ) {}


void test_basic( void )
{
    plcm_s plcm_res;
    plcm_t plcm;
    
    char* str;
    pl_pos_t item;

    plcm = &plcm_res;


    plcm_new( plcm, 1024 );
    str = "testing...";
    item = plcm_get_pos( plcm, 128 );
    TEST_ASSERT_EQUAL( 128, plcm_used( plcm ) );
    plcm_set( plcm, item, str, strlen( str ) + 1 );
    TEST_ASSERT( !strcmp( str, plcm_ref( plcm, item ) ) );
    plcm_del( plcm );


    plcm_new( plcm, 1024 );
    plss_append( plcm, plsr_from_c( str ) );
    TEST_ASSERT_EQUAL( strlen( str), plss_length( plcm ) );
    TEST_ASSERT( !strcmp( str, plss_string( plcm ) ) );
    plss_reformat( plcm, "%s_%s", str, str );
    TEST_ASSERT( !strcmp( "testing..._testing...", plss_string( plcm ) ) );
    plcm_del( plcm );
}
