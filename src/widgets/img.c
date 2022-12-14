#include <lauxlib.h>
#include <lua.h>

#include <lvgl.h>
#include <stdlib.h>

#include "lugl.h"
#include "private.h"

static int lugl_img_create(lua_State *L)
{
  return lugl_obj_create_helper(L, lv_img_create);
}

static void _lv_img_set_pivot(void *obj, lua_State *L)
{
  if (!lua_istable(L, -1)) {
    luaL_argerror(L, -1, "should be table.");
    debug("para should be table.");
    return;
  }

  lua_getfield(L, -1, "x");
  lv_coord_t x = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, -1, "y");
  lv_coord_t y = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lv_img_set_pivot(obj, x, y);
}

static const lugl_value_setter_t img_property_table[] = {
    {"src", SETTER_TYPE_STACK, {.setter_stack = _lv_dummy_set}},
    {"offset_x", 0, {.setter = (setter_int_t)lv_img_set_offset_x}},
    {"offset_y", 0, {.setter = (setter_int_t)lv_img_set_offset_y}},
    {"angle", 0, {.setter = (setter_int_t)lv_img_set_angle}},
    {"zoom", 0, {.setter = (setter_int_t)lv_img_set_zoom}},
    {"antialias", 0, {.setter = (setter_int_t)lv_img_set_antialias}},
    {"pivot", SETTER_TYPE_STACK, {.setter_stack = _lv_img_set_pivot}},
};

static int lugl_img_set_property_kv(lua_State *L, void *data)
{
  lv_obj_t *obj = data;
  int ret = lugl_set_property(L, obj, img_property_table);

  if (ret == 0) {
    return 0;
  }

  /* a base obj property? */
  ret = lugl_obj_set_property_kv(L, obj);
  if (ret != 0) {
    debug("unkown property for image.\n");
  }

  return -1;
}


static int lugl_img_set(lua_State *L)
{
  lv_obj_t *obj = lugl_to_obj(L, 1);
  if (obj == NULL) {
    luaL_argerror(L, 1, "null obj");
    return 0;
  }

  if (!lua_istable(L, -1)) {
    luaL_error(L, "expect a table on 2nd para.");
    return 0;
  }

  /* lvgl requires img src should be set firstly */
  lua_getfield(L, -1, "src");
  if (!lua_isnoneornil(L, -1)) {
    const char *src = NULL;
    if (lua_isuserdata(L, -1)) {
      src = lua_touserdata(L, -1);
      debug("set img src to user data: %p\n", src);
    } else {
      src = lua_tostring(L, -1);
    }
    lv_img_set_src(obj, src);
  }
  lua_pop(L, 1);

  lugl_iterate(L, -1, lugl_img_set_property_kv, obj);

  return 0;
}

/**
 * img.set_src(img, "path")
 */
static int lugl_img_set_src(lua_State *L)
{
  lv_obj_t *obj = lugl_to_obj(L, 1);
  if (obj == NULL) {
    luaL_argerror(L, 1, "null obj");
    return 0;
  }

  const char *src = lugl_toimgsrc(L, 2);
  if (src != NULL) {
    lv_img_set_src(obj, src);
  }

  return 0;
}

/**
 * img:set_offset({x=10})
 * img:set_offset({x=10})
 * img:set_offset({x=10, y=100})
 */
static int lugl_img_set_offset(lua_State *L)
{
  lv_obj_t *obj = lugl_to_obj(L, 1);
  if (obj == NULL) {
    luaL_argerror(L, 1, "null obj");
    return 0;
  }

  if (!lua_istable(L, -1)) {
    luaL_argerror(L, -1, "should be table {x=0,y=0,anim=true}");
  }

  lv_coord_t v;
  lua_getfield(L, -1, "x");
  if (!lua_isnil(L, -1)) {
    v = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lv_img_set_offset_x(obj, v);
  }

  lua_getfield(L, -1, "y");
  if (!lua_isnil(L, -1)) {
    v = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lv_img_set_offset_y(obj, v);
  }

  return 0;
}

/**
 * img:set_pivot({x=10, y=100})
 */
static int lugl_img_set_pivot(lua_State *L)
{
  lv_obj_t *obj = lugl_to_obj(L, 1);
  if (obj == NULL) {
    luaL_argerror(L, 1, "null obj");
    return 0;
  }

  if (!lua_istable(L, -1)) {
    luaL_argerror(L, -1, "should be table {x=0,y=0,anim=true}");
  }

  lv_coord_t x = 0, y = 0;
  lua_getfield(L, -1, "x");
  x = lua_tointeger(L, -1);

  lua_getfield(L, -1, "y");
  y = lua_tointeger(L, -1);

  lv_img_set_pivot(obj, x, y);

  return 0;
}

/**
 * return image size w, h
 * img:get_img_size() -- get size of this image
 * img:get_img_size("src") -- get size of img "src"
 */
static int lugl_get_img_size(lua_State *L)
{
  lv_obj_t *obj = lugl_to_obj(L, 1);
  if (obj == NULL) {
    luaL_argerror(L, 1, "null obj");
    return 0;
  }

  const void *src = NULL;
  if (lua_isnoneornil(L, 2)) {
    src = lv_img_get_src(obj);
  } else {
    src = lugl_toimgsrc(L, 2);
  }

  lv_img_header_t header;
  if (src == NULL || lv_img_decoder_get_info(src, &header) != LV_RES_OK) {
    lua_pushnil(L);
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, header.w);
    lua_pushinteger(L, header.h);
  }

  return 2;
}

static const luaL_Reg lugl_img_methods[] = {
    // img.c
    {"set", lugl_img_set},

    {"set_src", lugl_img_set_src},
    {"set_offset", lugl_img_set_offset},
    {"set_pivot", lugl_img_set_pivot},
    {"get_img_size", lugl_get_img_size},

    {NULL, NULL},
};

static void lugl_img_init(lua_State *L)
{
  lugl_obj_newmetatable(L, &lv_img_class, "lv_img",
                        lugl_img_methods);
  lua_pop(L, 1);
}
