#include "luv.h"

void luv_stack_dump(lua_State* L, const char* name) {
  int i, l;
  fprintf(stderr, "\nAPI STACK DUMP %p %d: %s\n", L, lua_status(L), name);
  for (i = 1, l = lua_gettop(L); i <= l; i++) {
    int type = lua_type(L, i);
    switch (type) {
      case LUA_TSTRING:
        fprintf(stderr, "  %d %s \"%s\"\n", i, lua_typename(L, type), lua_tostring(L, i));
        break;
      case LUA_TNUMBER:
        fprintf(stderr, "  %d %s %ld\n", i, lua_typename(L, type), (long int) lua_tointeger(L, i));
        break;
      case LUA_TUSERDATA:
        fprintf(stderr, "  %d %s %p\n", i, lua_typename(L, type), lua_touserdata(L, i));
        break;
      default:
        fprintf(stderr, "  %d %s\n", i, lua_typename(L, type));
        break;
    }
  }
  assert(l == lua_gettop(L));
}

static int luv_error(lua_State* L, int status) {
  lua_pushnil(L);
  lua_pushfstring(L, "%s: %s", uv_err_name(status), uv_strerror(status));
  lua_pushstring(L, uv_err_name(status));
  return 3;
}

static void luv_status(lua_State* L, int status) {
  if (status < 0) {
    lua_pushstring(L, uv_err_name(status));
  }
  else {
    lua_pushnil(L);
  }
}

#if LUV_UV_VERSION_GEQ(1, 10, 0)
static int luv_translate_sys_error(lua_State* L) {
  int status = luaL_checkinteger(L, 1);
  status = uv_translate_sys_error(status);
  if (status < 0) {
    luv_error(L, status);
    lua_remove(L, -3);
    return 2;
  }
  return 0;
}
#endif
