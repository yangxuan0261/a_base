#include <lua.h>
#include <lauxlib.h>

#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/aes.h>

#ifndef LUA_API_EXPORT
#define LUA_API_EXPORT __declspec(dllexport) 
#endif

#define AES_KEY_LENGTH 16

static int
lencrypt (lua_State *L) {
	size_t len = 0;
	const char *text = lua_tolstring (L, 1, &len);
	const char *key = lua_tostring (L, 2);

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	unsigned char iv[16] = { 0 };
	char *output = malloc (len + AES_BLOCK_SIZE);
	int olen1;
	int olen2;

	EVP_EncryptInit (ctx, EVP_aes_128_cbc(), (unsigned char *)key, iv);
	EVP_EncryptUpdate (ctx, (unsigned char *)output, &olen1, (const unsigned char *)text, len);
	int ok = EVP_EncryptFinal (ctx, (unsigned char *)output + olen1, &olen2);
	if (!ok) {
		free (output);
		EVP_CIPHER_CTX_free (ctx);
		return 0;
	}
	
	lua_pushlstring (L, output, olen1 + olen2);
	free (output);
	EVP_CIPHER_CTX_free (ctx);
	return 1;
}

static int
ldecrypt (lua_State *L) {
	size_t len = 0;
	const char *text = lua_tolstring (L, 1, &len);
	const char *key = lua_tostring (L, 2);

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	unsigned char iv[16] = { 0 };
	char *output = malloc (len);
	int olen1;
	int olen2;

	EVP_DecryptInit (ctx, EVP_aes_128_cbc(), (unsigned char *)key, iv);
	EVP_DecryptUpdate (ctx, (unsigned char *)output, &olen1, (const unsigned char *)text, len);
	int ok = EVP_DecryptFinal (ctx, (unsigned char *)(output + olen1), &olen2);
	if (!ok) {
		free (output);
		EVP_CIPHER_CTX_free (ctx);
		return 0;
	}
	
	lua_pushlstring (L, output, olen1 + olen2);
	free (output);
	EVP_CIPHER_CTX_free (ctx);
	return 1;
}

LUA_API_EXPORT int
luaopen_aes (lua_State *L) {
	luaL_checkversion (L);
	luaL_Reg l[] = {
		{ "encrypt", lencrypt },
		{ "decrypt", ldecrypt },
		{ NULL, NULL },
	};
	//luaL_newlib (L,l);
	lua_newtable(L);
	luaL_openlib(L, "aes", l, 0);
	return 1;
}
