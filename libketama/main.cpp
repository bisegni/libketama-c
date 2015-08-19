/*
 * Using a known ketama.servers file, and a fixed set of keys
 * print and hash the output of this program using your modified
 * libketama, compare the hash of the output to the known correct
 * hash in the test harness.
 *
 */

#include "ketama.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    
    ketama_t ktma;
    //ketama_roll( &c, "/Users/bisegni/Desktop/libketama/ketama.two.servers" );
    ketama_init(&ktma);
    ketama_add_server(&ktma, "192.168.1.1:11211", 60);
    ketama_add_server(&ktma, "192.168.1.2:11211", 30);
    ketama_add_server(&ktma, "192.168.1.3:11211", 10);
    ketama_add_server(&ktma, "macbisegni.lnf.infn.it:11211", 10);
    ketama_add_server(&ktma, "chaost-cds1.chaos.lnf.infn.it:11211", 10);
    
    ketama_update_continuum(&ktma);
    
    ketama_print_continuum(&ktma);
    
    int i;
    for ( i = 0; i < 1000000; i++ ) {
        char k[10];
        sprintf( k, "%d", i );
        unsigned int kh = ketama_hashi( k );
        mcs* m = ketama_get_server( k, &ktma);
        
        printf( "hash:%u Continuum point:%u Host:%s\n", kh, m->point, m->ip );
    }
    ketama_destroy(&ktma);
    return 0;
}
