#include <stdbool.h>
#include <assert.h>
#include <sodium.h>

bool blake2b(const char *in, const uint8_t len, unsigned char *out){
    crypto_generichash_blake2b_state S;
    crypto_generichash_init(&S, NULL, 0, len);
    crypto_generichash_blake2b_update(&S, in, 10); 
    crypto_generichash_blake2b_final(&S, out, len);
    uint8_t _len = strlen(out);
    if(_len < len)
        return 0;
    else 
        return 1;
}