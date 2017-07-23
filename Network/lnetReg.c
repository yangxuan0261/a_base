#include <lua.h>
#include <lauxlib.h>
#include <stdbool.h>

#define LUA_API_EXPORT __declspec(dllexport) 

typedef void(*Callback)(int);

extern bool Connect(const char* _ip, int _port);
extern void Send(const char* _msg, int _len);
extern void Recv(char* _msg, unsigned int* _len);
extern void Close();
extern void RegCallback(Callback _cb);
extern void StartSRThread();
extern void Clear();

static char lcbFunc[32] = {0};
static lua_State* selfls = NULL;

static int
lconnect(lua_State *L) {
	size_t sz = 0;
	const char* ip = luaL_checklstring(L, 1, &sz);
	int port = luaL_checkinteger(L, 2);
	bool ret = Connect(ip, port);
	lua_pushboolean(L, ret);
	return 1;
}

static void lcallBack(int _flag) {
	if (lcbFunc == NULL || selfls == NULL)
		return;

	int oldTop = lua_gettop(selfls);
	lua_getglobal(selfls, lcbFunc);

	if (!lua_isfunction(selfls, -1))
	{
		printf("--- error: lua_isfunction false");
		lua_settop(selfls, oldTop);
		return;
	}

	lua_pushinteger(selfls, _flag);
	int iRet = lua_pcall(selfls, 1, 0, 0);
	if (iRet)                       
	{
		printf("--- error: lcallBack, flag:%d", _flag);
		lua_settop(selfls, oldTop);
		return;
	}
	lua_settop(selfls, oldTop);
}

static int
lregCallback(lua_State *L) {
	selfls = L;
	size_t sz = 0;
	const char* cbFunc = luaL_checklstring(L, 1, &sz);
	memcpy(lcbFunc, cbFunc, sz);
	printf("--- reg callback func:%s\n", lcbFunc);
	RegCallback(lcallBack);
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

static int
lstartSRThread(lua_State *L) {
	StartSRThread();
	return 1;
}

static int
lclear(lua_State *L) {
	Clear();
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
		{ "regCallback", lregCallback },
		{ "startSRThread", lstartSRThread },
		{ "clear", lclear },
		{ NULL, NULL },
	};
	//luaL_newlib(L, l);
	lua_newtable(L);
	luaL_openlib(L, "network", l, 0);
	return 1;
}