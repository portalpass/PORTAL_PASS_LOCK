#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <string>

//odds and ends reused in multiple source files

namespace helper
{

    //encode / decode data to/from base64 representation
    std::string base64_encode(unsigned char const* , unsigned int len);
    std::string base64_decode(std::string const& s);

    //Encrypts msg into encoded
    //Uses the pem file in "pub_key" to encrypt and the pemfile at "priv_key" to sign
    //returns 0 on success, !=0 on failure
    int rsa_encrypt_sign( const std::string &msg, const std::string &pub_key,
                          const std::string &priv_key, std::string &encoded );

    //Decrypts msg into decoded
    //Uses the pem file in "priv_key" to decrypt and the pemfile at "pub_key" to verify
    //returns 0 on success, !=0 on failure
    int rsa_decrypt_verify( const std::string &msg, const std::string &pub_key,
                            const std::string &priv_key, std::string &decoded );

    //writes a PEM key pair at
    //name.pub.pem & name.priv.pem
    //returns 0 on success, !=0 on failure. Depending on how far it gets before failing
    //    may create the files and leave them empty
    int rsa_genkeypair    ( const std::string &name );

    // Uses openssl's random byte generator to make a key key_len_bits long
    // convert to a hex string and store in hexKeyOut
    // key_len_bits must be a multiple of 8
    // returns 0 on success, !=0 on failure
    int gen_hex_aes_key   ( int key_len_bits, std::string &hexKeyOut );

    //runs command cmd, storing stdout in out_s and returning the error code as an int
    int exec( const std::string &cmd, std::string &out_s );

    //return 1 if a file doesnt exist
    int file_doesnt_exist( const std::string &file );

    //returns a string path to the configuration directory.
    //In linux, will expand and return ~/.config/PORTAL_PASS/
    //Can be refined for windows
    std::string get_conf_dir ( void );

    //directory of python files, needed to cd in to get access to the
    //dependancies until we can install as modules
    std::string get_python_dir( void );

    std::string get_home_dir( void );
}

#endif
