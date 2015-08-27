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

#ifndef KETAMA_LIBKETAMA_KETAMA_H__
#define KETAMA_LIBKETAMA_KETAMA_H__

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif
    
#define KETAMA_HOST_NAME_LENGTH 255
    
    typedef int (*compfn)( const void*, const void* );
    
    typedef struct
    {
        unsigned int point;  // point on circle
        char ip[KETAMA_HOST_NAME_LENGTH];
    } mcs;
    
    typedef struct
    {
        char addr[KETAMA_HOST_NAME_LENGTH];
        unsigned long memory;
    } serverinfo;
    
    typedef struct
    {
        int numpoints;
        void* modtime;
        mcs* array; //array of mcs structs
    } continuum;
    
    typedef continuum* ketama_continuum;
    
    typedef struct ketama_t {
        serverinfo* slist;
        unsigned int numservers;
        unsigned long memtotal;
        continuum ketama_continuum;
    }ketama_t;
    
    //!Initialize the ketama structure
    /*!
     perform the initilization the ketama structure taht contain information about continuum and server list
     \param ketama the ketama structure to initialize
     */
    void ketama_init(ketama_t *ketama);
    
    //!Destroy the ketama structure
    /*!
     drestoy all infromation and release all memory allocated into the ketama structure
     \param ketama the ketama structure to destroy
     */
    void ketama_destroy( ketama_t *ketama);
    
    //!Add a new server to the servers list.
    /*!
     Add a new server to a ketama structure, after that operation a call to the api ketama_update_continuum need to be
     done to update the internal ketama struct continuum.
     \param ketama the ketama structure where we need to add the server
     \param server_address_port the server address string "host:port"
     \param the waight of the server
     */
    int ketama_add_server(ketama_t* ketama,
                          const char * server_address_port,
                          unsigned long weight);
    
    //!Add a new server to the servers list.
    /*!
     Remove a server to a ketama structure, after that operation a call to the api ketama_update_continuum need to be
     done to update the internal ketama struct continuum.
     \param ketama the ketama structure where we need to remove the server
     \param server_address_port the server address string "host:port"
     */
    int ketama_remove_server(ketama_t* ketama,
                             const char * server_address_port);
    
    //!Update ketama continuum
    /*!
     Update the continuum according to the content of the server lists
     \param ketama the ketama structure where we need to udpate the continuum
     */
    int ketama_update_continuum(ketama_t* ketama);
    
    //!Maps a key onto a server in the continuum.
    /*!
     \param ketama the structure that we need ot use to identify the server
     \param key The key that you want to map to a specific server.
     \param cont Pointer to the continuum in which we will search.
     \return The mcs struct that the given key maps to.
     */
    mcs* ketama_get_server(ketama_t *ketama, char *key);
    
    //!Print the server list of a continuum to stdout.
    /*!
     \param ketama the structure to use
     */
    void ketama_print_continuum(ketama_t *ketama);
    
    //! Hashing function, converting a string to an unsigned int by using MD5.
    /*!
     \param in_string The string that you want to hash.
     \return The resulting hash.
     */
    unsigned int ketama_hashi(char* in_string);
    
    //! Hashinf function to 16 bytes char array using MD%.
    /*!
     \param inString The string that you want to hash.
     \param md5pword The resulting hash. 
     */
    void ketama_md5_digest(char* in_string, unsigned char md5pword[16] );
    
    
#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // KETAMA_LIBKETAMA_KETAMA_H__

