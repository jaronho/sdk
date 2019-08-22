#ifndef INTERNAL_H
#define INTERNAL_H 1

#include "config.h"
#ifdef HAVE_VISIBILITY_HIDDEN
#	define __EXPORTED	__attribute__((visibility("default")))
#else
#	define __EXPORTED
#endif

#endif
