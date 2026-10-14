/*
 * lang/lang.c - list of the layouts, in the order the !lang key cycles them.
 *
 * To add a language: copy lang/en.c to lang/xx.c, edit it, declare it in
 * qsk.h, add it here and add lang/xx.o to OBJS in the Makefile.
 */

#include "qsk.h"

const struct lang *const langs[] = { &lang_en, &lang_ru };
const int nlangs = sizeof(langs) / sizeof(langs[0]);
