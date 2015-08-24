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

void print_first_n_ket(ketama_t *ktma,
                      unsigned int key_num) {
    for (int i = 0; i < key_num; i++ ) {
        char k[10];
        sprintf( k, "%d", i );
        unsigned int kh = ketama_hashi( k );
        mcs* m = ketama_get_server(ktma, k);
        
        printf( "Key:%s hash:%u Continuum point:%u Host:%s\n", k, kh, m->point, m->ip );
    }
}

int main(int argc, char **argv) {
    //allocate ketama struct into stack (or heap)
    ketama_t ktma;
    
    //initialize the ketama struct
    ketama_init(&ktma);
    
    //add some server for test
    ketama_add_server(&ktma, "192.168.1.1:11211", 60);
    ketama_add_server(&ktma, "192.168.1.2:11211", 30);
    ketama_add_server(&ktma, "192.168.1.3:11211", 10);
    ketama_add_server(&ktma, "lowpriority.host:11211", 10);
    ketama_add_server(&ktma, "highpriority.host:11211", 80);
    
    //update continuum
    ketama_update_continuum(&ktma);
    
    //print continuum
    ketama_print_continuum(&ktma);
    
    //print key association up to string 100
    print_first_n_ket(&ktma, 100);
    
    //remove one server and update continuum
    ketama_remove_server(&ktma, "lowpriority.host:11211");
    
    //update continum
    ketama_update_continuum(&ktma);
    
    //print new continum
    ketama_print_continuum(&ktma);

    //print key association up to string 100
    print_first_n_ket(&ktma, 100);
    
    //destroy ketama struct
    ketama_destroy(&ktma);
    
    return 0;
}
