#include "luv.h"
#include "lthreadpool.h"

static uv_async_t* luv_check_async(lua_State* L, int index) {
  uv_async_t* handle = (uv_async_t*)luv_checkudata(L, index, "uv_async");
  luaL_argcheck(L, handle->type == UV_ASYNC && handle->data, index, "Expected uv_async_t");
  return handle;
}

static void luv_async_cb(uv_async_t* handle) {
  lua_State* L = luv_state(handle->loop);
  luv_handle_t* data = (luv_handle_t*)handle->data;
  int n = luv_thread_arg_push(L, (const luv_thread_arg_t*)data->extra, 0);
  luv_call_callback(L, data, LUV_ASYNC, n);
  luv_thread_arg_clear(L, (luv_thread_arg_t*)data->extra, 0);
}

static int luv_new_async(lua_State* L) {
  uv_async_t* handle;
  luv_handle_t* data;
  int ret;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  handle = (uv_async_t*)luv_newuserdata(L, sizeof(*handle));
  ret = uv_async_init(luv_loop(L), handle, luv_async_cb);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  data = luv_setup_handle(L);
  data->extra = (luv_thread_arg_t*)malloc(sizeof(luv_thread_arg_t));
  memset(data->extra, 0, sizeof(luv_thread_arg_t));
  handle->data = data;
  luv_check_callback(L, (luv_handle_t*)handle->data, LUV_ASYNC, 1);
  return 1;
}

static int luv_async_send(lua_State* L) {
  int ret;
  uv_async_t* handle = luv_check_async(L, 1);
  luv_thread_arg_t* arg = (luv_thread_arg_t *)((luv_handle_t*) handle->data)->extra;
  
  luv_thread_arg_set(L, arg, 2, lua_gettop(L), 0);
  ret = uv_async_send(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}