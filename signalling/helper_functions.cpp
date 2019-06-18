/* 
   Big thanks to René Nyffenegger who has opensourced their base64 functions
   Copyright (C) 2004-2017 René Nyffenegger
   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:
   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.
   3. This notice may not be removed or altered from any source distribution.
   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

//implementation of helper_functions.cpp, collection of 
//ancillary stuff like encryption, decryption, etc, used by multiple sources
//all platform specific stuff SHOULD be here only so functions can be redefined
//based on platform

#include "helper_functions.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

//BEGIN HELPERS USED ONLY IN THIS FILE

static const EVP_CIPHER *CIPHER_USED = EVP_aes_256_cbc();
static const EVP_MD     *DIGEST_USED = EVP_sha256();

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static inline void copy_into_string ( unsigned char const* a, int len, std::string &s )
{
    s.clear();
    for ( int i = 0; i < len; i++ ) s.push_back( static_cast<char>( a[ i ]) );
}

static void bin_to_hex( unsigned char const* a, int len, std::string &out )
{
    out.clear();
    for ( int i = 0; i < len; i++ )
    {
        char temp = static_cast<char>( ( a[ i ] & 0xF0 ) >> 4 );
        
        if ( temp >= 10 )
            out.push_back( temp + 0x37 );
        else 
            out.push_back( temp + 0x30 );

        temp = static_cast<char>( a[ i ] & 0x0F );
        
        if ( temp >= 10 )
            out.push_back( temp + 0x37 );
        else 
            out.push_back( temp + 0x30 );
    }
}

//END HELPERS USED ONLY IN THIS FILE

std::string helper::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
          char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
          char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
          char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
          char_array_4[3] = char_array_3[2] & 0x3f;

          for(i = 0; (i <4) ; i++)
            ret += base64_chars[char_array_4[i]];
          i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
          char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;

}

std::string helper::base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

              char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
              char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
              char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

//pub_key and priv_key are paths to pem files to encrypt and sign with respectively
//OUTPUT  Stored in ENOCDED
//OUTPUT  FORMAT OF ENCODED:
//        2BYTES: e_key_len
//        2BYTES: slen
//        E_KEY BYTES: e_key
//        iv
//        cipher_text
//        SLEN BTES: sig ( of e_key + iv + cipher_text )
//Returns 0 on success, !=0 on failure
int helper::rsa_encrypt_sign( const std::string &msg, const std::string &pub_key,
                      const std::string &priv_key, std::string &encoded )
{
    //Everything must be declared ontop to allow us to "goto" and cleanup
    //load keys from file
    FILE* pub_f  = fopen( pub_key.c_str(), "r");
    FILE* priv_f = fopen( priv_key.c_str(), "r" );
    
    if ( pub_f == NULL || priv_f == NULL )
    {
        if ( pub_f  != NULL ) fclose( pub_f );
        if ( priv_f != NULL ) fclose( priv_f );
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR OPENING PUB_KEY OR PRIV_KEY FILE" << std::endl;
        return 2;
    }

    EVP_CIPHER_CTX *rsactx = EVP_CIPHER_CTX_new();
    EVP_MD_CTX     *mdctx  = EVP_MD_CTX_create();

    EVP_PKEY *pub_k  = NULL; 
    EVP_PKEY *priv_k = NULL;
    
    pub_k  = PEM_read_PUBKEY    ( pub_f,  NULL, NULL, NULL );
    priv_k = PEM_read_PrivateKey( priv_f, NULL, NULL, NULL );

    int iv_len = EVP_CIPHER_iv_length( CIPHER_USED );

    unsigned char*iv          = static_cast<unsigned char*> ( calloc( iv_len, sizeof(*iv) ) );
    unsigned char*cipher_text = static_cast<unsigned char*> ( calloc( msg.size() + iv_len, sizeof(*cipher_text) ) );
    unsigned char*e_key       = static_cast<unsigned char*> ( calloc( EVP_PKEY_size( pub_k ) , sizeof(*e_key) ) );
    unsigned char*sig         = NULL;

    int    error              = 0;
    int    cipher_text_length = 0;
    int    block_length       = 0;
    int    e_key_length       = 0;
    size_t slen               = 0;
    
    std::string e_key_length_str( 2, 0 );
    std::string slen_str( 2, 0 );
    std::string e_key_str;
    std::string iv_str;
    std::string cipher_text_str;
    std::string to_sign;
    std::string sig_str;

    if ( iv == NULL || cipher_text == NULL || e_key == NULL 
         || pub_k == NULL || priv_k == NULL )
    {
        error = 3;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR INITIALIZING" << std::endl;
        goto cleanup;
    }

    //generate random encryption key, encrypt it with the public key, and store in e_key
    //generate a random iv and store it in iv
    if ( 1 != EVP_SealInit( rsactx, CIPHER_USED, &e_key, &e_key_length, iv, &pub_k, 1) )
    {
        error = 4;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR ENCRYPTING" << std::endl;
        goto cleanup;
    }
    //encrypt
    if ( 1 != EVP_SealUpdate( rsactx, cipher_text + cipher_text_length, (int*)&block_length, 
                    reinterpret_cast<const unsigned char*>(msg.data()), msg.size() ) )
    {
        error = 5;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR ENCRYPTING" << std::endl;
        goto cleanup;
    }

    cipher_text_length += block_length;

    if ( 1 != EVP_SealFinal( rsactx, cipher_text + cipher_text_length, (int*)&block_length) )
    {
        error = 6;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR ENCRYPTING" << std::endl;
        goto cleanup;
    }
    
    cipher_text_length += block_length;
    
    //Output e_key appended to iv appended to the message itself
    copy_into_string( e_key      , e_key_length      , e_key_str );
    copy_into_string( iv         , iv_len            , iv_str );
    copy_into_string( cipher_text, cipher_text_length, cipher_text_str );

    //sign e_key+iv+cipher_text and then append the signature to the end
    to_sign = e_key_str + iv_str + cipher_text_str;

    if ( 1 != EVP_DigestSignInit  ( mdctx, NULL, DIGEST_USED, NULL, priv_k) )
    {
        error = 7;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR SIGNING" << std::endl;
        goto cleanup;
    }
    if( 1 != EVP_DigestSignUpdate( mdctx, reinterpret_cast<const unsigned char*>( to_sign.data() ), to_sign.size()) )
    {
        error = 8;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR SIGNING" << std::endl;
        goto cleanup;
    }
    //measure how big the signature will be, then allocate sig appropriately
    if ( 1 != EVP_DigestSignFinal ( mdctx, NULL, &slen ) )
    {
        error = 9;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR SIGNING" << std::endl;
        goto cleanup;
    }
    
    sig = static_cast<unsigned char*> ( calloc( slen, sizeof(*sig) ) );
    if ( sig == NULL )
    {
        error = 10;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR SIGNING" << std::endl;
        goto cleanup;
    }

    if ( 1 != EVP_DigestSignFinal ( mdctx, sig, &slen ) )
    {
        error = 11;
        std::cerr << ">RSA_ENCRYPT_SIGN ERROR SIGNING" << std::endl;
        goto cleanup;
    }

    copy_into_string( sig, slen, sig_str );

    e_key_length_str[ 0 ] = e_key_length / 256;
    e_key_length_str[ 1 ] = e_key_length % 256;

    slen_str[ 0 ] = slen / 256;
    slen_str[ 1 ] = slen % 256;

    //store the final message
    encoded = ( e_key_length_str + slen_str + to_sign + sig_str );

    cleanup:
    free( iv );
    free( cipher_text );
    free( e_key );
    free( sig );

    EVP_CIPHER_CTX_free(rsactx);
    EVP_MD_CTX_destroy(mdctx);
    EVP_PKEY_free( pub_k );
    EVP_PKEY_free( priv_k );

    fclose( priv_f );
    fclose( pub_f );
    
    return error;
}

//pub_key and priv_key are paths to pem files to verify and decrypt with respectively
//OUTPUT  Stored in DECODED
//Expected format of MSG msg:
//        2BYTES: e_key_len
//        2BYTES: slen
//        E_KEY BYTES: e_key
//        iv
//        cipher_text
//        SLEN BTES: sig ( of e_key + iv + cipher_text )
//Returns 0 on success, !=0 on failure
int helper::rsa_decrypt_verify( const std::string &msg, const std::string &pub_key,
                        const std::string &priv_key, std::string  &decoded )
{
    //verify msg is as long as it claims / dont falsely parse something
    int iv_len = EVP_CIPHER_iv_length( CIPHER_USED );

    if ( msg.size() <  4 + iv_len )
    {
        std::cerr << ">RSA_DECRYPT_VERIFY INVALID INPUT MESSAGE" << std::endl;
        return 1;
    }

    std::string e_key_length_str = msg.substr( 0, 2 );
    int e_key_length = 256 * e_key_length_str[ 0 ] + e_key_length_str[ 1 ];

    std::string sig_key_length_str = msg.substr( 2, 2 );
    int slen = 256 * sig_key_length_str[ 0 ] + sig_key_length_str[ 1 ];

    if ( msg.size() < 4 + e_key_length + iv_len + slen + 1 )
    {
        std::cerr << ">RSA_DECRYPT_VERIFY INVALID INPUT MESSAGE" << std::endl;
        return 1;
    }
    
    //parse out the encryption key, iv, message, and signature
    std::string e_key_str = msg.substr( 4, e_key_length );
    std::string iv_str    = msg.substr( 4 + e_key_length, iv_len );
    std::string enc_msg   = msg.substr( 4 + e_key_length + iv_len, msg.size() - 4 - e_key_length - iv_len - slen );
    std::string sig_str   = msg.substr( msg.size() - slen );

    //load keys from file
    FILE* pub_f  = fopen( pub_key.c_str(), "r");
    FILE* priv_f = fopen( priv_key.c_str(), "r" );
    
    if ( pub_f == NULL || priv_f == NULL )
    {
        if ( pub_f  != NULL ) fclose( pub_f );
        if ( priv_f != NULL ) fclose( priv_f );
        std::cerr << ">RSA_DECRYPT_VERIFY ERROR OPENING PUB_KEY OR PRIV_KEY FILE" << std::endl;
        return 2;
    }

    EVP_CIPHER_CTX *rsactx    = EVP_CIPHER_CTX_new();
    EVP_MD_CTX     *mdctx     = EVP_MD_CTX_create();

    EVP_PKEY *pub_k  = NULL; 
    EVP_PKEY *priv_k = NULL;
    
    pub_k  = PEM_read_PUBKEY    ( pub_f,  NULL, NULL, NULL );
    priv_k = PEM_read_PrivateKey( priv_f, NULL, NULL, NULL );
    
    unsigned char*decode_text = static_cast<unsigned char*> ( calloc( enc_msg.size() + iv_len, sizeof(*decode_text) ) );

    int error              = 0;
    int decode_text_length = 0;
    int block_length       = 0;
    
    if ( decode_text == NULL || pub_k == NULL || priv_k == NULL )
    {
        error = 3;
        std::cerr << ">RSA_DECRYPT_VERIFY ERROR INITIALIZING" << std::endl;
        goto cleanup;
    }
    
    //decrypt e_key with priv key
    if ( 1 != EVP_OpenInit( rsactx, CIPHER_USED, reinterpret_cast<const unsigned char*>( e_key_str.data() ), e_key_str.size(),
                            reinterpret_cast<const unsigned char*>( iv_str.data() ), priv_k ) )
    {
        error = 4;
        std::cerr << ">RSA_DECRYPT_VERIFY ERROR DECRYPTING" << std::endl;
        goto cleanup;
    }

    //decrypt message
    if( 1 != EVP_OpenUpdate( rsactx, decode_text + decode_text_length, (int*)&block_length, 
                    reinterpret_cast<const unsigned char*>(enc_msg.data()), enc_msg.size() ) )
    {
        error = 5;
        std::cerr << ">RSA_DECRYPT_VERIFY ERROR DECRYPTING" << std::endl;
        goto cleanup;
    }

    decode_text_length += block_length;

    if ( 1 != EVP_OpenFinal( rsactx, decode_text + decode_text_length, (int*)&block_length ) )
    {
        error = 6;
        std::cerr << ">RSA_DECRYPT_VERIFY ERROR DECRYPTING" << std::endl;
        goto cleanup;
    }

    decode_text_length += block_length;

    //store result in decoded
    copy_into_string( decode_text, decode_text_length, decoded );

    //verify signature of e_key+iv+message
    if ( 1 != EVP_DigestVerifyInit(mdctx, NULL, DIGEST_USED, NULL, pub_k) )
    {
        error = 7;
        std::cerr << ">RSA_DECRYPT_VERIFY: ERROR VERIFYING SIGNATURE" << std::endl;
        goto cleanup;
    }
    
    if ( 1 != EVP_DigestVerifyUpdate(mdctx, 
        reinterpret_cast<const unsigned char*>( ( e_key_str + iv_str + enc_msg ).data() ), ( e_key_str + iv_str + enc_msg ).size() ) )
    {
        error = 8;
        std::cerr << ">RSA_DECRYPT_VERIFY: ERROR VERIFYING SIGNATURE" << std::endl;
        goto cleanup;
    }
    
    if ( 1 != EVP_DigestVerifyFinal(mdctx, reinterpret_cast<const unsigned char*>(sig_str.data()), sig_str.size()) )
    {
        error = 9;
        std::cerr << ">RSA_DECRYPT_VERIFY: ERROR VERIFYING SIGNATURE" << std::endl;
        goto cleanup;
    }


    cleanup:
    free( decode_text );
   
    EVP_CIPHER_CTX_free(rsactx);
    EVP_MD_CTX_destroy(mdctx);
    EVP_PKEY_free( pub_k );
    EVP_PKEY_free( priv_k );

    fclose( priv_f );
    fclose( pub_f );

    return ( error );

}

//generates rsa key pair and writes to
//name.pub.pem and name.priv.pem
//Returns 0 on success, !=0 on failure
int helper::rsa_genkeypair    ( const std::string &name )
{
    //default is 2048, else we could use 4096?
    int key_length = 2048;
    int error = 0;

    FILE* pub_f;
    FILE* priv_f;

    EVP_PKEY *pkey    = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id( EVP_PKEY_RSA, NULL );

    if ( ctx == NULL )
    {
        error = 1;
        std::cerr << ">RSA_GENKEYPAIR ERROR INISITALIZING" << std::endl;
        goto cleanup;
    }

    if ( 1 != EVP_PKEY_keygen_init( ctx ) )
    {
        error = 2;
        std::cerr << ">RSA_GENKEYPAIR ERROR INISITALIZING" << std::endl;
        goto cleanup;
    }
    if( 1 != EVP_PKEY_CTX_set_rsa_keygen_bits( ctx, key_length ) )
    {
        error = 3;
        std::cerr << ">RSA_GENKEYPAIR ERROR INISITALIZING" << std::endl;
        goto cleanup;
    }
   
    if( 1 != EVP_PKEY_keygen( ctx, &pkey ) )
    {
        error = 5;
        std::cerr << ">RSA_GENKEYPAIR ERROR INISITALIZING" << std::endl;
        goto cleanup;
    }

    pub_f  = fopen( ( name + ".pub.pem" ).c_str(), "w" );
    priv_f = fopen( ( name + ".priv.pem" ).c_str(), "w");
    
    if ( pub_f == NULL || priv_f == NULL )
    {
        if ( pub_f  != NULL ) fclose( pub_f );
        if ( priv_f != NULL ) fclose( priv_f );
        std::cerr << ">RSA_GENKEYPAIR ERROR OPENING FILES TO WRITE" << std::endl;
        error = 4;
        goto cleanup;
    }

    if ( 1 != PEM_write_PUBKEY( pub_f, pkey ) )
    {
        error = 6;
        std::cerr << ">RSA_GENKEYPAIR ERROR WRITING TO PUBK FILE" << std::endl;
        goto cleanup;
    }
    
    if ( 1 != PEM_write_PrivateKey( priv_f, pkey, NULL, NULL, 0, NULL, NULL ) )
    {
        error = 7;
        std::cerr << ">RSA_GENKEYPAIR ERROR WRITING TO PUBK FILE" << std::endl;
        goto cleanup;
    }

    cleanup:
    
    fclose( pub_f  );
    fclose( priv_f );

    EVP_PKEY_CTX_free( ctx );
    EVP_PKEY_free( pkey );

    return error;

}

// Uses openssl's random byte generator to make a key key_len_bits long
// convert to a hex string and store in hexKeyOut
// key_len_bits must be a multiple of 8
// returns 0 on success, !=0 on failure
int helper::gen_hex_aes_key( int key_len_bits, std::string &hexKeyOut )
{

    if ( key_len_bits % 8 )
    {
        std::cerr << ">gen_hex_aes_key bits must be divisible by 8" << std::endl;
        return 1;
    }

    unsigned char*aes_key_bin = static_cast<unsigned char*> ( calloc( key_len_bits / 8, sizeof(*aes_key_bin) ) );
    
    if ( aes_key_bin == NULL )
    {
        std::cerr << ">gen_hex_aes_key Error Initializing" << std::endl;
        return 2;
    }
    
    if ( 1 != RAND_bytes( aes_key_bin, key_len_bits / 8 ) )
    {
        free( aes_key_bin );
        std::cerr << ">gen_hex_aes_key making bytes" << std::endl;
        return 3;
    }

    bin_to_hex( aes_key_bin, key_len_bits / 8, hexKeyOut );

    free( aes_key_bin );

    return 0;

}

//runs command cmd, storing stdout in out_s and returning the error code as an int
int helper::exec( const std::string &cmd, std::string &out_s )
{
    char buffer[128];
    out_s.clear();
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    
    try 
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
            out_s += buffer;
    }
    catch (...) 
    {
        pclose(pipe);
        throw;
    }
    return pclose(pipe);
}

//return 1 if a file doesnt exist
int helper::file_doesnt_exist( const std::string &file )
{
    std::ifstream test_f ( file.c_str() );
    if ( test_f )
    {
        return 0;
    }
    return 1;
}

//returns a string path to the configuration directory.
//In linux, will expand and return ~/.config/PORTAL_PASS/
//Can be refined for windows
std::string helper::get_conf_dir ( void )
{
    std::string homedir;
    if ((homedir = getenv("HOME")) == "") 
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return ( homedir + "/.config/PORTAL_PASS/" );
}

//directory of python files, needed to cd in to get access to the
//dependancies until we can install as modules
std::string helper::get_python_dir( void )
{
    std::string homedir;
    if ((homedir = getenv("HOME")) == "") 
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    
    return ( homedir + "/PORTAL_PASS_TEST/scripts/" );
}

std::string helper::get_home_dir( void )
{
    std::string homedir;
    if ((homedir = getenv("HOME")) == "") 
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}
