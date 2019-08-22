/*
 * Copyright (c) 2013 Arturo Borrero Gonzalez <arturo.borrero.glez@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <stdlib.h>

#include "utils.h"

void xfree(const void *ptr)
{
	free((void *)ptr);
}
