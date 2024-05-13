#ifndef BLAKE256_HASH__
#define BLAKE256_HASH__ 1

#include <stddef.h>
#include "simd-utils.h"
#include "sph_blake.h"

////////////////////////////
//
//  Blake-256 1 way SSE2
//

//#define blake256_context    sph_blake256_context
#define blake256_init       sph_blake256_init
#define blake256_update     sph_blake256
#define blake256_update_le  sph_blake256_update_le
#define blake256_close      sph_blake256_close

//TODO decouple from SPH

typedef struct
{
   unsigned char buf[64];  
   size_t ptr;
   uint32_t H[8];
   uint32_t S[4];
   uint32_t T0, T1;
} blake256_context __attribute__ ((aligned (32)));

void blake256_transform_le( uint32_t *H, const uint32_t *buf,
                            const uint32_t T0, const uint32_t T1, int rounds );
/*
void blake256_init( blake256_context *sc );
void blake256_update( blake256_context *sc, const void *data, size_t len );
void blake256_close( blake256_context *sc, void *dst );
void blake256_full( blake256_context *sc, void *dst, const void *data,
                    size_t len );
*/

//////////////////////////////////
//
//   Blake-256 4 way SSE2, NEON

typedef struct
{
   unsigned char buf[64<<2];
   uint32_t H[8<<2];
   size_t ptr;
   uint32_t T0, T1;
   int rounds;   // 14 for blake, 8 for blakecoin & vanilla
} blake_4x32_small_context __attribute__ ((aligned (64)));

// Default, 14 rounds
typedef blake_4x32_small_context blake256_4x32_context;
void blake256_4x32_init(void *ctx);
void blake256_4x32_update(void *ctx, const void *data, size_t len);
void blake256_4x32_close(void *ctx, void *dst);

// 14 rounds
typedef blake_4x32_small_context blake256r14_4x32_context;
void blake256r14_4x32_init(void *cc);
void blake256r14_4x32_update(void *cc, const void *data, size_t len);
void blake256r14_4x32_close(void *cc, void *dst);

// 8 rounds, blakecoin, vanilla
typedef blake_4x32_small_context blake256r8_4x32_context;
void blake256r8_4x32_init(void *cc);
void blake256r8_4x32_update(void *cc, const void *data, size_t len);
void blake256r8_4x32_close(void *cc, void *dst);

void blake256_4x32_round0_prehash_le( void *midstate, const void *midhash,
                                      void *data );
void blake256_4x32_final_rounds_le( void *final_hash, const void *midstate,
                     const void *midhash, const void *data, const int rounds );

#define blake_4way_small_context         blake256_4x32_context
#define blake256_4way_context            blake256_4x32_context
#define blake256_4way_init               blake256_4x32_init
#define blake256_4way_update             blake256_4x32_update
#define blake256_4way_close              blake256_4x32_close
#define blake256_4way_update_le          blake256_4x32_update_le
#define blake256_4way_close_le           blake256_4x32_close_le
#define blake256_4way_round0_prehash_le  blake256_4x32_round0_prehash_le
#define blake256_4way_final_rounds_le    blake256_4x32_final_rounds_le
#define blake256r14_4way_context         blake256r14_4x32_context
#define blake256r14_4way_init            blake256r14_4x32_init
#define blake256r14_4way_update          blake256r14_4x32_update
#define blake256r14_4way_close           blake256r14_4x32_close
#define blake256r8_4way_context          blake256r14_4x32_context
#define blake256r8_4way_init             blake256r14_4x32_init
#define blake256r8_4way_update           blake256r14_4x32_update
#define blake256r8_4way_close            blake256r14_4x32_close

#ifdef __AVX2__

//////////////////////////////
//
//   Blake-256 8 way AVX2

typedef struct
{
   __m256i buf[16] __attribute__ ((aligned (64)));
   __m256i H[8];
   size_t ptr;
   uint32_t T0, T1;
   int rounds;   // 14 for blake, 8 for blakecoin & vanilla
} blake_8way_small_context;

// Default 14 rounds
typedef blake_8way_small_context blake256_8way_context;
void blake256_8way_init(void *cc);
void blake256_8way_update(void *cc, const void *data, size_t len);
void blake256_8way_close(void *cc, void *dst);
void blake256_8way_update_le(void *cc, const void *data, size_t len);
void blake256_8way_close_le(void *cc, void *dst);
void blake256_8way_round0_prehash_le( void *midstate, const void *midhash,
                                      void *data );
void blake256_8way_final_rounds_le( void *final_hash, const void *midstate,
                    const void *midhash, const void *data, const int rounds );

// 14 rounds, blake, decred
typedef blake_8way_small_context blake256r14_8way_context;
void blake256r14_8way_init(void *cc);
void blake256r14_8way_update(void *cc, const void *data, size_t len);
void blake256r14_8way_close(void *cc, void *dst);

// 8 rounds, blakecoin, vanilla
typedef blake_8way_small_context blake256r8_8way_context;
void blake256r8_8way_init(void *cc);
void blake256r8_8way_update(void *cc, const void *data, size_t len);
void blake256r8_8way_close(void *cc, void *dst);

#define blake_8x32_small_context      blake256_8way_context
#define blake_8x32_init               blake256_8way_init
#define blake_8x32_update             blake256_8way_update
#define blake_8x32_close              blake256_8way_close
#define blake_8x32_update_le          blake256_8way_update_le
#define blake_8x32_close_le           blake256_8way_close_le
#define blake_8x32_round0_prehash_le  blake256_8way_round0_prehash
#define blake_8x32_final_rounds_le    blake256_8way_final_rounds_le
#define blake256r14_8x32_context      blake256r14_8way_context
#define blake256r14_8x32_init         blake256r14_8way_init
#define blake256r14_8x32_update       blake256r14_8way_update
#define blake256r14_8x32_close        blake256r14_8way_close
#define blake256r8_8x32_context       blake256r14_8way_context
#define blake256r8_8x32_init          blake256r14_8way_init
#define blake256r8_8x32_update        blake256r14_8way_update
#define blake256r8_8x32_close         blake256r14_8way_close

#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)

///////////////////////////////////
//
//   Blake-256 16 way AVX512

typedef struct
{
   __m512i buf[16];
   __m512i H[8];
   size_t ptr;
   uint32_t T0, T1;
   int rounds;   // 14 for blake, 8 for blakecoin & vanilla
} blake_16way_small_context __attribute__ ((aligned (128)));

// Default 14 rounds
typedef blake_16way_small_context blake256_16way_context;
void blake256_16way_init(void *cc);
void blake256_16way_update(void *cc, const void *data, size_t len);
void blake256_16way_close(void *cc, void *dst);
// Expects data in little endian order, no byte swap needed
void blake256_16way_update_le(void *cc, const void *data, size_t len);
void blake256_16way_close_le(void *cc, void *dst);
void blake256_16way_round0_prehash_le( void *midstate, const void *midhash,
                                       void *data );
void blake256_16way_final_rounds_le( void *final_hash, const void *midstate,
                     const void *midhash, const void *data, const int rounds );

// 14 rounds, blake, decred
typedef blake_16way_small_context blake256r14_16way_context;
void blake256r14_16way_init(void *cc);
void blake256r14_16way_update(void *cc, const void *data, size_t len);
void blake256r14_16way_close(void *cc, void *dst);

// 8 rounds, blakecoin, vanilla
typedef blake_16way_small_context blake256r8_16way_context;
void blake256r8_16way_init(void *cc);
void blake256r8_16way_update(void *cc, const void *data, size_t len);
void blake256r8_16way_close(void *cc, void *dst);

#define blake_16x32_small_context      blake256_16way_context
#define blake_16x32_init               blake256_16way_init
#define blake_16x32_update             blake256_16way_update
#define blake_16x32_close              blake256_16way_close
#define blake_16x32_update_le          blake256_16way_update_le
#define blake_16x32_close_le           blake256_16way_close_le
#define blake_16x32_round0_prehash_le  blake256_16way_round0_prehash
#define blake_16x32_final_rounds_le    blake256_16way_final_rounds_le
#define blake256r14_16x32_context      blake256r14_16way_context
#define blake256r14_16x32_init         blake256r14_16way_init
#define blake256r14_16x32_update       blake256r14_16way_update
#define blake256r14_16x32_close        blake256r14_16way_close
#define blake256r8_16x32_context       blake256r8_16way_context
#define blake256r8_16x32_init          blake256r8_16way_init
#define blake256r8_16x32_update        blake256r8_16way_update
#define blake256r8_16x32_close         blake256r8_16way_close

#endif  // AVX512
#endif  // AVX2

#endif  // BLAKE256_HASH_H__
