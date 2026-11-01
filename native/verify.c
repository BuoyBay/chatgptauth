/* RSASSA-PKCS1-v1_5/SHA-256 verification using the host OS crypto provider.
 * Inputs are unsigned, big-endian RSA modulus/exponent and exact JWS bytes. */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#define BCA_API __declspec(dllexport)
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#define BCA_API __attribute__((visibility("default")))
#else
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#define BCA_API __attribute__((visibility("default")))
#endif

#ifdef __APPLE__
static size_t der_length(unsigned char *out, size_t n) {
    if (n < 128) { out[0] = (unsigned char)n; return 1; }
    if (n < 256) { out[0] = 0x81; out[1] = (unsigned char)n; return 2; }
    out[0] = 0x82; out[1] = (unsigned char)(n >> 8); out[2] = (unsigned char)n; return 3;
}
static size_t der_integer(unsigned char *out, const unsigned char *input, size_t n) {
    size_t pad = (input[0] & 0x80) != 0;
    out[0] = 2;
    size_t header = 1 + der_length(out + 1, n + pad);
    if (pad) out[header] = 0;
    memcpy(out + header + pad, input, n);
    return header + pad + n;
}
#endif

BCA_API int64_t bca_verify_rs256(const unsigned char *modulus, int64_t modulus_len,
    const unsigned char *exponent, int64_t exponent_len, const unsigned char *message,
    int64_t message_len, const unsigned char *signature, int64_t signature_len) {
    if (!modulus || !exponent || !message || !signature || modulus_len < 256 || modulus_len > 1024 ||
        exponent_len < 1 || exponent_len > 8 || message_len < 1 || message_len > 1048576 ||
        signature_len != modulus_len || modulus[0] == 0 || exponent[0] == 0) return 0;
    if (modulus_len == 256 && !(modulus[0] & 0x80)) return 0;
    uint64_t exponent_value = 0;
    for (int64_t i = 0; i < exponent_len; i++) exponent_value = (exponent_value << 8) | exponent[i];
    if (exponent_value < 3 || !(exponent_value & 1)) return 0;
    int valid = 0;
#ifdef _WIN32
    BCRYPT_ALG_HANDLE rsa = NULL, sha = NULL;
    BCRYPT_KEY_HANDLE key = NULL;
    unsigned char hash[32];
    size_t blob_len = sizeof(BCRYPT_RSAKEY_BLOB) + (size_t)exponent_len + (size_t)modulus_len;
    unsigned char *blob = calloc(1, blob_len);
    if (!blob) return 0;
    BCRYPT_RSAKEY_BLOB *header = (BCRYPT_RSAKEY_BLOB *)blob;
    header->Magic = BCRYPT_RSAPUBLIC_MAGIC;
    header->BitLength = (ULONG)modulus_len * 8;
    header->cbPublicExp = (ULONG)exponent_len;
    header->cbModulus = (ULONG)modulus_len;
    memcpy(blob + sizeof(*header), exponent, (size_t)exponent_len);
    memcpy(blob + sizeof(*header) + (size_t)exponent_len, modulus, (size_t)modulus_len);
    if (BCryptOpenAlgorithmProvider(&rsa, BCRYPT_RSA_ALGORITHM, NULL, 0) < 0) goto done;
    if (BCryptImportKeyPair(rsa, NULL, BCRYPT_RSAPUBLIC_BLOB, &key, blob, (ULONG)blob_len, 0) < 0) goto done;
    if (BCryptOpenAlgorithmProvider(&sha, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0) goto done;
    if (BCryptHash(sha, NULL, 0, (PUCHAR)message, (ULONG)message_len, hash, sizeof(hash)) < 0) goto done;
    BCRYPT_PKCS1_PADDING_INFO padding = {BCRYPT_SHA256_ALGORITHM};
    valid = BCryptVerifySignature(key, &padding, hash, sizeof(hash), (PUCHAR)signature,
                                  (ULONG)signature_len, BCRYPT_PAD_PKCS1) >= 0;
done:
    if (key) BCryptDestroyKey(key);
    if (sha) BCryptCloseAlgorithmProvider(sha, 0);
    if (rsa) BCryptCloseAlgorithmProvider(rsa, 0);
    free(blob);
#elif defined(__APPLE__)
    unsigned char integers[1100], encoded[1110];
    size_t content_len = der_integer(integers, modulus, (size_t)modulus_len);
    content_len += der_integer(integers + content_len, exponent, (size_t)exponent_len);
    encoded[0] = 0x30;
    size_t header_len = 1 + der_length(encoded + 1, content_len);
    memcpy(encoded + header_len, integers, content_len);
    CFDataRef der = CFDataCreate(kCFAllocatorDefault, encoded, (CFIndex)(header_len + content_len));
    const void *keys[] = {kSecAttrKeyType, kSecAttrKeyClass};
    const void *values[] = {kSecAttrKeyTypeRSA, kSecAttrKeyClassPublic};
    CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    SecKeyRef key = der && attributes ? SecKeyCreateWithData(der, attributes, NULL) : NULL;
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, message, (CFIndex)message_len);
    CFDataRef sig = CFDataCreate(kCFAllocatorDefault, signature, (CFIndex)signature_len);
    if (key && data && sig)
        valid = SecKeyVerifySignature(key, kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA256, data, sig, NULL);
    if (sig) CFRelease(sig);
    if (data) CFRelease(data);
    if (key) CFRelease(key);
    if (attributes) CFRelease(attributes);
    if (der) CFRelease(der);
#else
    BIGNUM *n = BN_bin2bn(modulus, (int)modulus_len, NULL);
    BIGNUM *e = BN_bin2bn(exponent, (int)exponent_len, NULL);
    RSA *rsa = RSA_new();
    EVP_PKEY *key = EVP_PKEY_new();
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    if (!n || !e || !rsa || !key || !context) goto done;
    if (!RSA_set0_key(rsa, n, e, NULL)) goto done;
    n = NULL; e = NULL;
    if (!EVP_PKEY_assign_RSA(key, rsa)) goto done;
    rsa = NULL;
    EVP_PKEY_CTX *key_context = NULL;
    if (EVP_DigestVerifyInit(context, &key_context, EVP_sha256(), NULL, key) != 1) goto done;
    if (EVP_PKEY_CTX_set_rsa_padding(key_context, RSA_PKCS1_PADDING) <= 0) goto done;
    valid = EVP_DigestVerify(context, signature, (size_t)signature_len, message, (size_t)message_len) == 1;
done:
    EVP_MD_CTX_free(context);
    EVP_PKEY_free(key);
    RSA_free(rsa);
    BN_free(n); BN_free(e);
#endif
    return valid;
}
