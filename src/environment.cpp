#include "environment.h"
#include "configurations.h"
#include "stdio.h"

Environment env;

void env_init()
{
  env.n_slaves = 0; // initialize with no slaves
  env.mode = -1;    // initialize in invalid mode
  env.p_en = 1;     // enable pedestrian by default
  env.r_ext_en = 1;
  env.p_t = 0;
  env.r_ext_t = 0;
  memset(env.so_g, 0, sizeof(env.so_g)); // set all timers to 0
  memset(env.so_amb, 0, sizeof(env.so_amb));
  memset(env.md_g, 0, sizeof(env.md_g));
  memset(env.md_amb, 0, sizeof(env.md_amb));
  memset(env.slave_en, 0, sizeof(env.md_amb)); // Set all slaves to disabled

  int bl_freq = INT16_MAX;
}

void env_set(JsonObject &p)
{
  env.n_slaves = p["n"];
  env.mode = p["mode"];
  JsonObject &params = p["params"];

  switch (env.mode)
  {
    char s[10];
  case MODE_MD:
    for (int i = 0; i < env.n_slaves; i++)
    {
      sprintf(s, "g%d", i + 1);
      env.md_g[i] = params[s];
      sprintf(s, "a%d", i + 1);
      env.md_amb[i] = params[s];
    }
    break;

  case MODE_SO:
    for (int i = 0; i < env_ceil(env.n_slaves); i++)
    {
      sprintf(s, "g%d", i + 1);
      env.so_g[i] = params[s];
      sprintf(s, "a%d", i + 1);
      env.so_amb[i] = params[s];
    }
    break;

  case MODE_BL:
    env.bl_freq = params["f"];
    break;

  default:
    break;
  }

  env.p_en = p["pedestrian"];
  env.r_ext_en = p["red_extension"];

  if (env.p_en)
    env.p_t = p["ped_timer"];

  if (env.r_ext_en)
    env.r_ext_t = p["red_ext_timer"];

  ArduinoJson::JsonObject &slave_enables = p["slave_enables"];
  for (int i = 0; i < env.n_slaves; i++)
  {
    char slave_num[2];
    sprintf(slave_num, "%d", i + 1);
    env.slave_en[i] = slave_enables[slave_num];
  }
}

void env_setNumSlaves(int n)
{
  env.n_slaves = n;
}

void env_setMode(int mode)
{
  env.mode = mode;
}

int env_getNumSlaves()
{
  return env.n_slaves;
}

int env_getMode()
{
  return env.mode;
}

void env_setPedEnable(bool p)
{
  env.p_en = p;
}

void env_setRedExtEnable(bool r)
{
  env.r_ext_en = r;
}

int env_getPedEnable()
{
  return env.p_en;
}

int env_getRedExtEnable()
{
  return env.r_ext_en;
}

void env_setParams(int mode, int *param_array)
{
  switch (mode)
  {

  case MODE_MD:
    for (int i = 0; i < 4; i++)
    {
      env.md_g[i] = param_array[i];
      env.md_amb[i] = param_array[i + 4];
    }
    break;

  case MODE_SO:
    for (int i = 0; i < 2; i++)
    {
      env.so_g[i] = param_array[i];
      env.so_amb[i] = param_array[i + 2];
    }
    break;

  case MODE_BL:
    env.bl_freq = param_array[0];
    break;

  default:
    break;
  }
}

int *env_getParams(int mode)
{
  switch ((mode))
  {
  case MODE_MD:
    return env.md_g;
  case MODE_SO:
    return env.so_g; // can do this because struct is stored contiguously
  case MODE_BL:
    int *a;
    a = &env.bl_freq;
    return a;
  default:
    break;
  }
}

void env_setRedExtTimer(int t)
{
  env.r_ext_t = t;
}

void env_setPedTimer(int t)
{
  env.p_t = t;
}

int env_getRedExtTimer()
{
  return env.r_ext_t;
}

int env_getPedTimer()
{
  return env.p_t;
}

bool env_getSlaveEnableStatus(int slaveId)
{
  return env.slave_en[slaveId - 1];
}

/* NOT USED HERE BUT IN slots.cpp*/
Environment env_returnStruct(JsonObject &p)
{
  Environment e;
  e.n_slaves = p["n"];
  e.mode = p["mode"];
  JsonObject &params = p["params"];

  switch (e.mode)
  {
    char s[10];
  case MODE_MD:
    for (int i = 0; i < 4; i++)
    {
      sprintf(s, "g%d", i);
      e.md_g[i] = params[s];
      sprintf(s, "amb%d", i);
      e.md_amb[i] = params[s];
    }
    break;

  case MODE_SO:
    for (int i = 0; i < 2; i++)
    {
      sprintf(s, "g%d", i);
      e.so_g[i] = params[s];
      sprintf(s, "amb%d", i);
      e.so_amb[i] = params[s];
    }
    break;

  case MODE_BL:
    e.bl_freq = params["f"];
    break;

  default:
    break;
  }

  e.p_en = p["pedestrian"];
  e.r_ext_en = p["red_extension"];

  if (e.p_en)
    e.p_t = p["ped_timer"];

  if (e.r_ext_en)
    e.r_ext_t = p["red_ext_timer"];

  return e;
}

void env_set(Environment e)
{
  env = e;
}

/* HELPER FUNCTIONS */
static int env_ceil(int n)
{
  if (n % 2 == 0)
    return n / 2;
  else
    return n / 2 + 1;
}