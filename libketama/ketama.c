/*
* Copyright (c) 2007, Last.fm, All rights reserved.
* Richard Jones <rj@last.fm>
* Christian Muehlhaeuser <muesli@gmail.com>
*
* Modified by Claudio Bisegni <Claudio.Bisegni@lnf.infn.it> (Italian InstituteOf Nuclear Phisycs - INFN)
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Last.fm Limited nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Last.fm ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Last.fm BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ketama.h"
#include "md5.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>           /* for reading last time modification   */
#include <unistd.h>         /* needed for usleep                    */
#include <math.h>           /* floor & floorf                       */
#include <sys/stat.h>       /* various type definitions             */
#include <sys/shm.h>        /* shared memory functions and structs  */
#ifdef DEBUG
#include <syslog.h>
#endif
#include <stdarg.h>


char k_error[255] = "";


int
ketama_compare(mcs *a,
               mcs *b )
{
    return ( a->point < b->point ) ?  -1 : ( ( a->point > b->point ) ? 1 : 0 );
}


void
ketama_md5_digest( char* in_string,
                  unsigned char md5pword[16] )
{
    md5_state_t md5state;

    md5_init( &md5state );
    md5_append( &md5state, (unsigned char *)in_string, (int)strlen( in_string ) );
    md5_finish( &md5state, md5pword );
}

int
ketama_update_continuum(ketama_t* ketama) {
    /* Check numservers first; if it is zero then there is no error message
     * and we need to set one. */
    if ( ketama->numservers < 1 ) {
       //no valid server found
        return -1;
    } else if ( ketama->slist == 0 )  {
        /* read_server_definitions must've set error message. */
        return -2;
    }
    /* Continuum will hold one mcs for each point on the circle: */
    mcs continuum[ ketama->numservers * 160 ];
    unsigned int i, k, cont = 0;
    
    for( i = 0; i < ketama->numservers; i++ )
    {
        float pct = (float)ketama->slist[i].memory / (float)ketama->memtotal;
        unsigned int ks = floorf( pct * 40.0 * (float)ketama->numservers );

        
        for( k = 0; k < ks; k++ )
        {
            /* 40 hashes, 4 numbers per hash = 160 points per server */
            char ss[KETAMA_HOST_NAME_LENGTH+8];
            unsigned char digest[16];
            
            sprintf( ss, "%s-%d", ketama->slist[i].addr, k );
            ketama_md5_digest( ss, digest );
            
            /* Use successive 4-bytes from hash as numbers
             * for the points on the circle: */
            int h;
            for( h = 0; h < 4; h++ )
            {
                continuum[cont].point = ( digest[3+h*4] << 24 )
                | ( digest[2+h*4] << 16 )
                | ( digest[1+h*4] <<  8 )
                |   digest[h*4];
                
                memcpy( continuum[cont].ip, ketama->slist[i].addr, KETAMA_HOST_NAME_LENGTH );
                cont++;
            }
        }
    }
    
    /* Sorts in ascending order of "point" */
    qsort( (void*) &continuum, cont, sizeof( mcs ), (compfn)ketama_compare );
    
    ketama->ketama_continuum.array = (mcs*)realloc(ketama->ketama_continuum.array, sizeof( mcs ) * cont);
    memcpy(ketama->ketama_continuum.array, &continuum, sizeof( mcs ) * cont );
    ketama->ketama_continuum.numpoints = cont;
    return 0;
}

static serverinfo
get_server_info(const char* server_address_port,
                unsigned long weight ) {
    serverinfo server;
    memset(&server, 0, sizeof(serverinfo));
    
    strncpy( server.addr, server_address_port, KETAMA_HOST_NAME_LENGTH - 1);
    server.memory = weight;
    
    return server;
}

int
ketama_add_server(ketama_t* ketama,
                  const char * server_address_port,
                  unsigned long weight) {
    serverinfo server = get_server_info( server_address_port, weight );
    if ( server.memory > 0 && strlen( server.addr ) ) {
        ketama->slist = (serverinfo*)realloc( ketama->slist, sizeof( serverinfo ) * ( ketama->numservers + 1 ) );
        memcpy( &ketama->slist[ketama->numservers], &server, sizeof( serverinfo ) );
        ketama->numservers++;
        ketama->memtotal += server.memory;
    } else {
        /* This kind of tells the parent code that
         * "there were servers but not really"
         */
        return -1;
    }
    return 0;
}

int
ketama_remove_server(ketama_t* ketama,
                     const char * server_address_port) {
    unsigned int new_server_number = 0;
    serverinfo* new_server_list = NULL;
    if(ketama->numservers==0 ||
       ketama->slist == NULL) return -1;
    
    //scan all server removing that one that is equal to the input c string
    for(int s_index = 0; s_index < ketama->numservers; s_index++) {
        //compare server name
        if(strncmp(ketama->slist[s_index].addr, server_address_port, KETAMA_HOST_NAME_LENGTH) == 0) {
            //we have found the server to remove
            continue;
        }
        
        //add server to keep into the new list
        new_server_list = (serverinfo*)realloc( new_server_list, sizeof( serverinfo ) * (new_server_number+1));
        memcpy( &new_server_list[new_server_number++], &ketama->slist[s_index], sizeof( serverinfo ) );
    }
   
    //we have scan all server so change the list
    free(ketama->slist);
    ketama->slist = new_server_list;
    ketama->numservers = new_server_number;
    return 0;
}

unsigned int
ketama_hashi(char* in_string) {
    unsigned char digest[16];

    ketama_md5_digest( in_string, digest );
    return (unsigned int)(( digest[3] << 24 )
                        | ( digest[2] << 16 )
                        | ( digest[1] <<  8 )
                        |   digest[0] );
}

mcs*
ketama_get_server(ketama_t *ketama,
                  char* key) {
    unsigned int h = ketama_hashi( key );
    int highp = ketama->ketama_continuum.numpoints;
    mcs *mcsarr = ketama->ketama_continuum.array;
    int lowp = 0, midp;
    unsigned int midval, midval1;

    // divide and conquer array search to find server with next biggest
    // point after what this key hashes to
    while ( 1 )
    {
        midp = (int)( ( lowp+highp ) / 2 );

        if ( midp == ketama->ketama_continuum.numpoints )
            return &( mcsarr[0] ); // if at the end, roll back to zeroth

        midval = mcsarr[midp].point;
        midval1 = midp == 0 ? 0 : mcsarr[midp-1].point;

        if ( h <= midval && h > midval1 )
            return &(mcsarr[midp]);

        if ( midval < h )
            lowp = midp + 1;
        else
            highp = midp - 1;

        if ( lowp > highp )
            return &( mcsarr[0] );
    }
}

void
ketama_init(ketama_t *ketama) {
    memset(ketama, 0, sizeof(ketama_t));
}

void
ketama_destroy(ketama_t *ketama ) {
    free(ketama->ketama_continuum.array);
    ketama_init(ketama);
}


void
ketama_print_continuum(ketama_t *ketama ) {
    int a;
    printf( "Numpoints in continuum: %d\n", ketama->ketama_continuum.numpoints );

    if ( ketama->ketama_continuum.array == 0 )
    {
        printf( "Continuum empty\n" );
    }
    else
    {
        mcs *mcsarr = ketama->ketama_continuum.array;
        for( a = 0; a < ketama->ketama_continuum.numpoints; a++ )
        {
            printf( "%s (%u)\n", mcsarr[a].ip, mcsarr[a].point );
        }
    }
}
