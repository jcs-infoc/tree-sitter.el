/*
 * Copyright (C) 2018, 2019 Karl Otness
 *
 * This file is part of tree-sitter.el.
 *
 * tree-sitter.el is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * tree-sitter.el is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tree-sitter.el. If not, see
 * <https://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <string.h>
#include "common.h"

emacs_value tsel_Qnil;
emacs_value tsel_Qt;

bool tsel_common_init(emacs_env *env) {
  tsel_Qt = env->make_global_ref(env, env->intern(env, "t"));
  tsel_Qnil = env->make_global_ref(env, env->intern(env, "nil"));
  return !tsel_pending_nonlocal_exit(env);
}

bool tsel_pending_nonlocal_exit(emacs_env *env) {
  return env->non_local_exit_check(env) != emacs_funcall_exit_return;
}

void tsel_signal_wrong_type(emacs_env *env, char *type_pred_name, emacs_value val_provided) {
  emacs_value Qlist = env->intern(env, "list");
  emacs_value Qwrong_type_arg = env->intern(env, "wrong-type-argument");
  emacs_value Qtype_pred = env->intern(env, type_pred_name);
  emacs_value args[2] = { Qtype_pred, val_provided};
  emacs_value err_info = env->funcall(env, Qlist, 2, args);
  env->non_local_exit_signal(env, Qwrong_type_arg, err_info);
}

bool tsel_define_function(emacs_env *env, char *function_name, emacs_function *func,
                          ptrdiff_t min_arg_count, ptrdiff_t max_arg_count, const char *doc,
                          void *data) {
  emacs_value efunc = env->make_function(env, min_arg_count, max_arg_count, func, doc, data);
  if(tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  emacs_value f_name = env->intern(env, function_name);
  emacs_value Qdefalias = env->intern(env, "defalias");
  emacs_value args[2] = { f_name, efunc };
  if(tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  env->funcall(env, Qdefalias, 2, args);
  if(tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  return true;
}

bool tsel_record_get_field(emacs_env *env, emacs_value obj, uint8_t field, emacs_value *out) {
  emacs_value Qaref = env->intern(env, "aref");
  emacs_value num = env->make_integer(env, field);
  emacs_value args[2] = { obj, num };
  emacs_value value = env->funcall(env, Qaref, 2, args);
  if(tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  *out = value;
  return true;
}

bool tsel_check_record_type(emacs_env *env, char *record_type, emacs_value obj, int num_fields) {
  // Ensure the pointer is wrapped in the proper Emacs record type
  emacs_value Qrecordp = env->intern(env, "recordp");
  emacs_value args[2] = { obj, NULL};
  if(!env->eq(env, env->funcall(env, Qrecordp, 1, args), tsel_Qt) ||
     tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  // Check the length of the record
  emacs_value Qlen = env->intern(env, "length");
  emacs_value result = env->funcall(env, Qlen, 1, args);
  intmax_t length = env->extract_integer(env, result);
  if(tsel_pending_nonlocal_exit(env) || length != num_fields + 1) {
    return false;
  }
  // Check the record type tag
  emacs_value Qaref = env->intern(env, "aref");
  emacs_value zero = env->make_integer(env, 0);
  emacs_value type_symbol = env->intern(env, record_type);
  args[0] = obj;
  args[1] = zero;
  if(!env->eq(env, env->funcall(env, Qaref, 2, args), type_symbol) ||
     tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  return true;
}

bool tsel_string_p(emacs_env *env, emacs_value obj) {
  emacs_value Qstringp = env->intern(env, "stringp");
  emacs_value args[1] = { obj };
  if(!env->eq(env, env->funcall(env, Qstringp, 1, args), tsel_Qt) ||
     tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  return true;
}

bool tsel_extract_string(emacs_env *env, emacs_value obj, char **res) {
  if(!tsel_string_p(env, obj)) {
    tsel_signal_wrong_type(env, "stringp", obj);
    return false;
  }
  ptrdiff_t size = 0;
  if(!env->copy_string_contents(env, obj, NULL, &size)) {
    return false;
  }
  char *buf = malloc(sizeof(char) * size);
  if(!buf) {
    return false;
  }
  if(!env->copy_string_contents(env, obj, buf, &size) || tsel_pending_nonlocal_exit(env)) {
    free(buf);
    return false;
  }
  *res = buf;
  return true;
}

void tsel_signal_error(emacs_env *env, char *message) {
  emacs_value str = env->make_string(env, message, strlen(message));
  emacs_value Qlist = env->intern(env, "list");
  emacs_value Qerror = env->intern(env, "error");
  emacs_value args[1] = { str };
  emacs_value payload = env->funcall(env, Qlist, 1, args);
  env->non_local_exit_signal(env, Qerror, payload);
}

bool tsel_integer_p(emacs_env *env, emacs_value obj) {
  emacs_value Qstringp = env->intern(env, "integerp");
  emacs_value args[1] = { obj };
  if(!env->eq(env, env->funcall(env, Qstringp, 1, args), tsel_Qt) ||
     tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  return true;
}

bool tsel_extract_integer(emacs_env *env, emacs_value obj, intmax_t *res) {
  if(!tsel_integer_p(env, obj)) {
    tsel_signal_wrong_type(env, "integerp", obj);
    return false;
  }
  *res = env->extract_integer(env, obj);
  if(tsel_pending_nonlocal_exit(env)) {
    return false;
  }
  return true;
}

bool tsel_extract_buffer(emacs_env *env, emacs_value obj, emacs_value *res) {
  emacs_value Qbufferp = env->intern(env, "bufferp");
  if(!env->eq(env, tsel_Qt, env->funcall(env, Qbufferp, 1, &obj)) ||
     tsel_pending_nonlocal_exit(env)) {
    tsel_signal_wrong_type(env, "bufferp", obj);
    return false;
  }
  *res = obj;
  return true;
}
