#pragma once

/**
 * @brief SM3 context structure
 */
typedef struct
{
    unsigned long total[2]; /* number of bytes processed */
    unsigned long state[8]; /* intermediate digest state */
    unsigned char buffer[64]; /* data block being processed */
    unsigned char ipad[64]; /* HMAC: inner padding */
    unsigned char opad[64]; /* HMAC: outer padding */
} sm3_context;

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief SM3 context setup
     * @param ctx context to be initialized
     */
    void sm3Start(sm3_context* ctx);

    /**
     * @brief SM3 process buffer
     * @param ctx SM3 context
     * @param input buffer holding the  data
     * @param ilen length of the input data
     */
    void sm3Update(sm3_context* ctx, const unsigned char* input, int ilen);

    /**
     * @brief SM3 final digest
     * @param ctx SM3 context
     */
    void sm3Finish(sm3_context* ctx, unsigned char output[32]);

    /**
     * @brief Output = SM3(input buffer)
     * @param input buffer holding the  data
     * @param ilen length of the input data
     * @param output SM3 checksum result
     */
    void sm3Sign(const unsigned char* input, int ilen, unsigned char output[32]);

    /**
     * @brief Output = SM3(file contents)
     * @param path input file name
     * @param output SM3 checksum result
     * @return 0-if successful, 1-if fopen failed, 2-if fread failed
     */
    int sm3File(char* path, unsigned char output[32]);

    /**
     * @brief SM3 HMAC context setup
     * @param ctx HMAC context to be initialized
     * @param key HMAC secret key
     * @param keylen length of the HMAC key
     */
    void sm3HmacStart(sm3_context* ctx, const unsigned char* key, int keylen);

    /**
     * @brief SM3 HMAC process buffer
     * @param ctx HMAC context
     * @param input buffer holding the data
     * @param ilen length of the input data
     */
    void sm3HmacUpdate(sm3_context* ctx, const unsigned char* input, int ilen);

    /**
     * @brief SM3 HMAC final digest
     * @param ctx HMAC context
     * @param output SM3 HMAC checksum result
     */
    void sm3HmacFinish(sm3_context* ctx, unsigned char output[32]);

    /**
     * @brief Output = HMAC-SM3(hmac key, input buffer)
     * @param key HMAC secret key
     * @param keylen length of the HMAC key
     * @param input buffer holding the data
     * @param ilen length of the input data
     * @param output HMAC-SM3 result
     */
    void sm3Hmac(unsigned char* key, int keylen, const unsigned char* input, int ilen, unsigned char output[32]);
#ifdef __cplusplus
}
#endif
