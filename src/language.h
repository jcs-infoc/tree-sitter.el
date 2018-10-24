/*
 * Copyright (C) 2018 Karl Otness
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
#pragma once
#ifndef TSEL_LANGUAGE_H
#define TSEL_LANGUAGE_H
#include <stdbool.h>
#include <emacs-module.h>
#include <tree_sitter/runtime.h>

typedef struct TSElLanguage {
  // Tag to check type, must be "TSLanguage" followed by null char.
  char tag[11];
  TSLanguage *ptr;
} TSElLanguage;

bool tsel_language_init(emacs_env *env);
bool tsel_language_p(emacs_env *env, emacs_value obj);
TSElLanguage *tsel_language_get_ptr(emacs_env *env, emacs_value obj);
#endif //ifndef TSEL_LANGUAGE_H