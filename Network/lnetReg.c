#include <lua.h>
#include <lauxlib.h>
#include <stdbool.h>

#define LUA_API_EXPORT __declspec(dllexport) 

extern bool Connect(const char* _ip, int _port);
extern void Send(const char* _msg, int _len);
extern void Recv(char* _msg, unsigned int* _len);
extern void Close();

static int
lconnect(lua_State *L) {
	size_t sz = 0;
	const char* ip = luaL_checklstring(L, 1, &sz);
	int port = luaL_checkinteger(L, 2);
	bool ret = Connect(ip, port);
	lua_pushboolean(L, ret);
	return 1;
}

static int
lsend(lua_State *L) {
	size_t sz = 0;
	const char* msg = luaL_checklstring(L, 1, &sz);
	Send(msg, sz);
	return 1;
}

static int
lrecv(lua_State *L) {
	unsigned int len = 0;
	char buffer[65535];
	Recv(buffer, &len);
	if (len > 0)
	{
		lua_pushlstring(L, buffer, len);
		return 1;
	}
	else
	{
		lua_pushnil(L);
		lua_pushinteger(L, 1); //lua中根据第二个来判断是否返回
		return 2;
	}
}

static int
lclose(lua_State *L) {
	Close();
	return 1;
}

LUA_API_EXPORT int
luaopen_network(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "connect", lconnect },
		{ "send", lsend },
		{ "recv", lrecv },
		{ "close", lclose },
		{ NULL, NULL },
	};
	//luaL_newlib(L, l);
	lua_newtable(L);
	luaL_openlib(L, "network", l, 0);
	return 1;
}