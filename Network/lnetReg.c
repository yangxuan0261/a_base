#include <lua.h>
#include <lauxlib.h>

#define LUA_API_EXPORT __declspec(dllexport) 

extern void Connect(const char* _ip, int _port);
extern void Send(const char* _msg);
//extern void Recv(const char* _recv);
extern void Close();

static int
lconnect(lua_State *L) {
	size_t sz = 0;
	const char* ip = luaL_checklstring(L, 1, &sz);
	int port = luaL_checkinteger(L, 2);
	Connect(ip, port);
	return 1;
}

static int
lsend(lua_State *L) {
	size_t sz = 0;
	const char* msg = luaL_checklstring(L, 1, &sz);
	Send(msg);
	return 1;
}

//static int
//lrecv(lua_State *L) {
//	std::string content = Recv();
//	if (content.length() > 0) {
//		lua_pushlstring(L, content.c_str(), content.length());
//	}
//	return 1;
//}

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
		//{ "recv", lrecv },
		{ "close", lclose },
		{ NULL, NULL },
	};
	//luaL_newlib(L, l);
	lua_newtable(L);
	luaL_openlib(L, "network", l, 0);
	return 1;
}