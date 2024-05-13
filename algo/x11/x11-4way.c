#include "cpuminer-config.h"
#include "x11-gate.h"
#include <string.h>
#include <stdint.h>
#include "algo/blake/blake512-hash.h"
#include "algo/bmw/bmw-hash-4way.h"
#include "algo/groestl/aes_ni/hash-groestl.h"
#include "algo/skein/skein-hash-4way.h"
#include "algo/jh/jh-hash-4way.h"
#include "algo/keccak/keccak-hash-4way.h"
#include "algo/luffa/luffa-hash-2way.h"
#include "algo/cubehash/cube-hash-2way.h"
#include "algo/cubehash/cubehash_sse2.h"
#include "algo/shavite/sph_shavite.h"
#include "algo/simd/simd-hash-2way.h"
#include "algo/echo/aes_ni/hash_api.h"
#if defined(__VAES__)
  #include "algo/groestl/groestl512-hash-4way.h"
  #include "algo/shavite/shavite-hash-4way.h"
  #include "algo/echo/echo-hash-4way.h"
#endif

#if defined (X11_8WAY)

typedef struct {
    blake512_8way_context   blake;
    bmw512_8way_context     bmw;
    skein512_8way_context   skein;
    jh512_8way_context      jh;
    keccak512_8way_context  keccak;
    luffa_4way_context      luffa;
    cube_4way_context       cube;
    simd_4way_context       simd;
#if defined(__VAES__)
    groestl512_4way_context groestl;
    shavite512_4way_context shavite;
    echo_4way_context       echo;
#else
    hashState_groestl       groestl;
    sph_shavite512_context  shavite;
    hashState_echo          echo;
#endif
} x11_8way_ctx_holder;

x11_8way_ctx_holder x11_8way_ctx;

void init_x11_8way_ctx()
{
     blake512_8way_init( &x11_8way_ctx.blake );
     bmw512_8way_init( &x11_8way_ctx.bmw );
     skein512_8way_init( &x11_8way_ctx.skein );
     jh512_8way_init( &x11_8way_ctx.jh );
     keccak512_8way_init( &x11_8way_ctx.keccak );
     luffa_4way_init( &x11_8way_ctx.luffa, 512 );
     cube_4way_init( &x11_8way_ctx.cube, 512, 16, 32 );
     simd_4way_init( &x11_8way_ctx.simd, 512 );
#if defined(__VAES__)
     groestl512_4way_init( &x11_8way_ctx.groestl, 64 );
     shavite512_4way_init( &x11_8way_ctx.shavite );
     echo_4way_init( &x11_8way_ctx.echo, 512 );
#else
     init_groestl( &x11_8way_ctx.groestl, 64 );
     sph_shavite512_init( &x11_8way_ctx.shavite );
     init_echo( &x11_8way_ctx.echo, 512 );
#endif
}

static __thread __m512i x11_8way_midstate[16] __attribute__((aligned(64)));


void x11_8way_hash( void *state, const void *input )
{
     uint64_t vhash[8*8] __attribute__ ((aligned (128)));
     uint64_t vhashA[4*8] __attribute__ ((aligned (64)));
     uint64_t vhashB[4*8] __attribute__ ((aligned (64)));
     uint64_t hash0[8] __attribute__ ((aligned (64)));
     uint64_t hash1[8] __attribute__ ((aligned (64)));
     uint64_t hash2[8] __attribute__ ((aligned (64)));
     uint64_t hash3[8] __attribute__ ((aligned (64)));
     uint64_t hash4[8] __attribute__ ((aligned (64)));
     uint64_t hash5[8] __attribute__ ((aligned (64)));
     uint64_t hash6[8] __attribute__ ((aligned (64)));
     uint64_t hash7[8] __attribute__ ((aligned (64)));
     x11_8way_ctx_holder ctx;
     memcpy( &ctx, &x11_8way_ctx, sizeof(x11_8way_ctx) );

     blake512_8way_final_le( &ctx.blake, vhash, casti_m512i( input, 9 ),
                             x11_8way_midstate );

     bmw512_8way_update( &ctx.bmw, vhash, 64 );
     bmw512_8way_close( &ctx.bmw, vhash );

#if defined(__VAES__)

     rintrlv_8x64_4x128( vhashA, vhashB, vhash, 512 );

     groestl512_4way_update_close( &ctx.groestl, vhashA, vhashA, 512 );
     groestl512_4way_init( &ctx.groestl, 64 );
     groestl512_4way_update_close( &ctx.groestl, vhashB, vhashB, 512 );

     rintrlv_4x128_8x64( vhash, vhashA, vhashB, 512 );

#else

     dintrlv_8x64_512( hash0, hash1, hash2, hash3, hash4, hash5, hash6, hash7,
                   vhash );

     update_and_final_groestl( &ctx.groestl, (char*)hash0, (char*)hash0, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash1, (char*)hash1, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash2, (char*)hash2, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash3, (char*)hash3, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash4, (char*)hash4, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash5, (char*)hash5, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash6, (char*)hash6, 512 );
     memcpy( &ctx.groestl, &x11_8way_ctx.groestl,
             sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash7, (char*)hash7, 512 );

     intrlv_8x64_512( vhash, hash0, hash1, hash2, hash3, hash4, hash5, hash6,
                  hash7 );

#endif

     skein512_8way_update( &ctx.skein, vhash, 64 );
     skein512_8way_close( &ctx.skein, vhash );

     jh512_8way_update( &ctx.jh, vhash, 64 );
     jh512_8way_close( &ctx.jh, vhash );

     keccak512_8way_update( &ctx.keccak, vhash, 64 );
     keccak512_8way_close( &ctx.keccak, vhash );

     rintrlv_8x64_4x128( vhashA, vhashB, vhash, 512 );

     luffa_4way_update_close( &ctx.luffa, vhashA, vhashA, 64 );
     luffa_4way_init( &ctx.luffa, 512 );
     luffa_4way_update_close( &ctx.luffa, vhashB, vhashB, 64 );

     cube_4way_update_close( &ctx.cube, vhashA, vhashA, 64 );
     cube_4way_init( &ctx.cube, 512, 16, 32 );
     cube_4way_update_close( &ctx.cube, vhashB, vhashB, 64 );

#if defined(__VAES__)

     shavite512_4way_update_close( &ctx.shavite, vhashA, vhashA, 64 );
     shavite512_4way_init( &ctx.shavite );
     shavite512_4way_update_close( &ctx.shavite, vhashB, vhashB, 64 );

#else
     
     dintrlv_4x128_512( hash0, hash1, hash2, hash3, vhashA );
     dintrlv_4x128_512( hash4, hash5, hash6, hash7, vhashB );

     sph_shavite512( &ctx.shavite, hash0, 64 );
     sph_shavite512_close( &ctx.shavite, hash0 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash1, 64 );
     sph_shavite512_close( &ctx.shavite, hash1 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash2, 64 );
     sph_shavite512_close( &ctx.shavite, hash2 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash3, 64 );
     sph_shavite512_close( &ctx.shavite, hash3 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash4, 64 );
     sph_shavite512_close( &ctx.shavite, hash4 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash5, 64 );
     sph_shavite512_close( &ctx.shavite, hash5 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash6, 64 );
     sph_shavite512_close( &ctx.shavite, hash6 );
     memcpy( &ctx.shavite, &x11_8way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash7, 64 );
     sph_shavite512_close( &ctx.shavite, hash7 );

     intrlv_4x128_512( vhashA, hash0, hash1, hash2, hash3 );
     intrlv_4x128_512( vhashB, hash4, hash5, hash6, hash7 );

#endif

     simd_4way_update_close( &ctx.simd, vhashA, vhashA, 512 );
     simd_4way_init( &ctx.simd, 512 );
     simd_4way_update_close( &ctx.simd, vhashB, vhashB, 512 );

#if defined(__VAES__)

     echo_4way_update_close( &ctx.echo, vhashA, vhashA, 512 );
     echo_4way_init( &ctx.echo, 512 );
     echo_4way_update_close( &ctx.echo, vhashB, vhashB, 512 );

     dintrlv_4x128_512( hash0, hash1, hash2, hash3, vhashA );
     dintrlv_4x128_512( hash4, hash5, hash6, hash7, vhashB );
     
#else

     dintrlv_4x128_512( hash0, hash1, hash2, hash3, vhashA );
     dintrlv_4x128_512( hash4, hash5, hash6, hash7, vhashB );

     update_final_echo( &ctx.echo, (BitSequence *)hash0,
                       (const BitSequence *) hash0, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash1,
                       (const BitSequence *) hash1, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash2,
                       (const BitSequence *) hash2, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash3,
                       (const BitSequence *) hash3, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash4,
                       (const BitSequence *) hash4, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash5,
                       (const BitSequence *) hash5, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash6,
                       (const BitSequence *) hash6, 512 );
     memcpy( &ctx.echo, &x11_8way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash7,
                       (const BitSequence *) hash7, 512 );

#endif

     memcpy( state,     hash0, 32 );
     memcpy( state+ 32, hash1, 32 );
     memcpy( state+ 64, hash2, 32 );
     memcpy( state+ 96, hash3, 32 );
     memcpy( state+128, hash4, 32 );
     memcpy( state+160, hash5, 32 );
     memcpy( state+192, hash6, 32 );
     memcpy( state+224, hash7, 32 );
}

int scanhash_x11_8way( struct work *work, uint32_t max_nonce,
                   uint64_t *hashes_done, struct thr_info *mythr )
{
   uint32_t hash[8*8] __attribute__ ((aligned (128)));
   uint32_t vdata[20*8] __attribute__ ((aligned (64)));
   __m128i edata[5] __attribute__ ((aligned (64)));
   uint32_t *pdata = work->data;
   uint32_t *ptarget = work->target;
   uint32_t n = pdata[19];
   const uint32_t first_nonce = pdata[19];
   int thr_id = mythr->id;
   __m512i  *noncev = (__m512i*)vdata + 9; 
   const uint32_t last_nonce = max_nonce -8;
   const __m512i eight = _mm512_set1_epi64( 8 );

   // convert LE32 to LE64
   edata[0] = v128_swap64_32( casti_v128( pdata, 0 ) );
   edata[1] = v128_swap64_32( casti_v128( pdata, 1 ) );
   edata[2] = v128_swap64_32( casti_v128( pdata, 2 ) );
   edata[3] = v128_swap64_32( casti_v128( pdata, 3 ) );
   edata[4] = v128_swap64_32( casti_v128( pdata, 4 ) );

   mm512_intrlv80_8x64( vdata, edata );
   *noncev = _mm512_add_epi32( *noncev, _mm512_set_epi32(
                                    0,7, 0,6, 0,5, 0,4, 0,3, 0,2, 0,1, 0,0 ) );
   blake512_8way_prehash_le( &x11_8way_ctx.blake, x11_8way_midstate, vdata );

   do
   {
      x11_8way_hash( hash, vdata );

      for ( int i = 0; i < 8; i++ )
      if ( unlikely( valid_hash( hash+(i<<3), ptarget ) && !opt_benchmark ))
      {
          pdata[19] = n+i;
          submit_solution( work, hash+(i<<3), mythr );
      }
      *noncev = _mm512_add_epi32( *noncev, eight );
      n += 8;
   } while ( ( n < last_nonce ) && !work_restart[thr_id].restart );
   *hashes_done = n - first_nonce;
   return 0;
}


#elif defined (X11_4WAY)


typedef struct {
    blake512_4way_context   blake;
    bmw512_4way_context     bmw;
    hashState_groestl       groestl;
    skein512_4way_context   skein;
    jh512_4way_context      jh;    
    keccak512_4way_context  keccak;    
    luffa_2way_context      luffa;
    cubehashParam           cube;
    sph_shavite512_context  shavite;
    simd_2way_context       simd;
    hashState_echo          echo;
} x11_4way_ctx_holder;

x11_4way_ctx_holder x11_4way_ctx;

void init_x11_4way_ctx()
{
     blake512_4way_init( &x11_4way_ctx.blake );
     bmw512_4way_init( &x11_4way_ctx.bmw );
     init_groestl( &x11_4way_ctx.groestl, 64 );
     skein512_4way_init( &x11_4way_ctx.skein );
     jh512_4way_init( &x11_4way_ctx.jh );
     keccak512_4way_init( &x11_4way_ctx.keccak );
     luffa_2way_init( &x11_4way_ctx.luffa, 512 );
     cubehashInit( &x11_4way_ctx.cube, 512, 16, 32 );
     sph_shavite512_init( &x11_4way_ctx.shavite );
     simd_2way_init( &x11_4way_ctx.simd, 512 );
     init_echo( &x11_4way_ctx.echo, 512 );
}

void x11_4way_hash( void *state, const void *input )
{
     uint64_t hash0[8] __attribute__ ((aligned (64)));
     uint64_t hash1[8] __attribute__ ((aligned (64)));
     uint64_t hash2[8] __attribute__ ((aligned (64)));
     uint64_t hash3[8] __attribute__ ((aligned (64)));
     uint64_t vhash[8*4] __attribute__ ((aligned (64)));
     uint64_t vhashB[8*2] __attribute__ ((aligned (64)));

     x11_4way_ctx_holder ctx;
     memcpy( &ctx, &x11_4way_ctx, sizeof(x11_4way_ctx) );

     // 1 Blake 4way
     blake512_4way_update( &ctx.blake, input, 80 );
     blake512_4way_close( &ctx.blake, vhash );

     // 2 Bmw
     bmw512_4way_update( &ctx.bmw, vhash, 64 );
     bmw512_4way_close( &ctx.bmw, vhash );

     // Serial
     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );

     // 3 Groestl
     update_and_final_groestl( &ctx.groestl, (char*)hash0, (char*)hash0, 512 );
     memcpy( &ctx.groestl, &x11_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash1, (char*)hash1, 512 );
     memcpy( &ctx.groestl, &x11_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash2, (char*)hash2, 512 );
     memcpy( &ctx.groestl, &x11_4way_ctx.groestl, sizeof(hashState_groestl) );
     update_and_final_groestl( &ctx.groestl, (char*)hash3, (char*)hash3, 512 );

     // 4way
     intrlv_4x64( vhash, hash0, hash1, hash2, hash3, 512 );

     // 4 Skein
     skein512_4way_update( &ctx.skein, vhash, 64 );
     skein512_4way_close( &ctx.skein, vhash );

     // 5 JH
     jh512_4way_update( &ctx.jh, vhash, 64 );
     jh512_4way_close( &ctx.jh, vhash );

     // 6 Keccak
     keccak512_4way_update( &ctx.keccak, vhash, 64 );
     keccak512_4way_close( &ctx.keccak, vhash );

     dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );

     // 7 Luffa parallel 2 way 128 bit
     intrlv_2x128( vhash, hash0, hash1, 512 );
     intrlv_2x128( vhashB, hash2, hash3, 512 );
     luffa_2way_update_close( &ctx.luffa, vhash, vhash, 64 );
     luffa_2way_init( &ctx.luffa, 512 );
     luffa_2way_update_close( &ctx.luffa, vhashB, vhashB, 64 );
     dintrlv_2x128( hash0, hash1, vhash, 512 );
     dintrlv_2x128( hash2, hash3, vhashB, 512 );

     // 8 Cubehash
     cubehashUpdateDigest( &ctx.cube, (byte*)hash0, (const byte*) hash0, 64 );
     memcpy( &ctx.cube, &x11_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash1, (const byte*) hash1, 64 );
     memcpy( &ctx.cube, &x11_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash2, (const byte*) hash2, 64 );
     memcpy( &ctx.cube, &x11_4way_ctx.cube, sizeof(cubehashParam) );
     cubehashUpdateDigest( &ctx.cube, (byte*)hash3, (const byte*) hash3, 64 );

     // 9 Shavite
     sph_shavite512( &ctx.shavite, hash0, 64 );
     sph_shavite512_close( &ctx.shavite, hash0 );
     memcpy( &ctx.shavite, &x11_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash1, 64 );
     sph_shavite512_close( &ctx.shavite, hash1 );
     memcpy( &ctx.shavite, &x11_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash2, 64 );
     sph_shavite512_close( &ctx.shavite, hash2 );
     memcpy( &ctx.shavite, &x11_4way_ctx.shavite,
             sizeof(sph_shavite512_context) );
     sph_shavite512( &ctx.shavite, hash3, 64 );
     sph_shavite512_close( &ctx.shavite, hash3 );

     // 10 Simd
     intrlv_2x128( vhash, hash0, hash1, 512 );
     intrlv_2x128( vhashB, hash2, hash3, 512 );
     simd_2way_update_close( &ctx.simd, vhash, vhash, 512 );
     simd_2way_init( &ctx.simd, 512 );
     simd_2way_update_close( &ctx.simd, vhashB, vhashB, 512 );
     dintrlv_2x128( hash0, hash1, vhash, 512 );
     dintrlv_2x128( hash2, hash3, vhashB, 512 );

     // 11 Echo
     update_final_echo( &ctx.echo, (BitSequence *)hash0,
                       (const BitSequence *) hash0, 512 );
     memcpy( &ctx.echo, &x11_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash1,
                       (const BitSequence *) hash1, 512 );
     memcpy( &ctx.echo, &x11_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash2,
                       (const BitSequence *) hash2, 512 );
     memcpy( &ctx.echo, &x11_4way_ctx.echo, sizeof(hashState_echo) );
     update_final_echo( &ctx.echo, (BitSequence *)hash3,
                       (const BitSequence *) hash3, 512 );

     memcpy( state,    hash0, 32 );
     memcpy( state+32, hash1, 32 );
     memcpy( state+64, hash2, 32 );
     memcpy( state+96, hash3, 32 );
}

int scanhash_x11_4way( struct work *work, uint32_t max_nonce,
                   uint64_t *hashes_done, struct thr_info *mythr )
{
     uint32_t hash[4*8] __attribute__ ((aligned (64)));
     uint32_t vdata[24*4] __attribute__ ((aligned (64)));
     uint32_t *pdata = work->data;
     uint32_t *ptarget = work->target;
     uint32_t n = pdata[19];
     const uint32_t first_nonce = pdata[19];
     int thr_id = mythr->id;  // thr_id arg is deprecated
     __m256i  *noncev = (__m256i*)vdata + 9;   // aligned
     const uint32_t Htarg = ptarget[7];
     uint64_t htmax[] = {          0,        0xF,       0xFF,
                               0xFFF,     0xFFFF, 0x10000000  };
     uint32_t masks[] = { 0xFFFFFFFF, 0xFFFFFFF0, 0xFFFFFF00,
                          0xFFFFF000, 0xFFFF0000,          0  };

     mm256_bswap32_intrlv80_4x64( vdata, pdata );

     for (int m=0; m < 6; m++) 
       if (Htarg <= htmax[m])
       {
         uint32_t mask = masks[m];
         do
         {
           *noncev = mm256_intrlv_blend_32( mm256_bswap_32(
                 _mm256_set_epi32( n+3, 0, n+2, 0, n+1, 0, n, 0 ) ), *noncev );

            x11_4way_hash( hash, vdata );
            pdata[19] = n;

            for ( int i = 0; i < 4; i++ )
            if ( ( ( (hash+(i<<3))[7] & mask ) == 0 )
                 && fulltest( hash+(i<<3), ptarget ) && !opt_benchmark )
            {
               pdata[19] = n+i;
               submit_solution( work, hash+(i<<3), mythr );
            }
            n += 4;
         } while ( ( n < max_nonce ) && !work_restart[thr_id].restart );
         break;
       }

     *hashes_done = n - first_nonce + 1;
     return 0;
}

#endif
