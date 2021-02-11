/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void testSnprintf(){
	char buf[4];
	snprintf(buf,sizeof(buf),"asdf");
	if(buf[3] != 0){
		fprintf(stderr,"Unsafe snprintf detected, exiting!\n");
		exit(1);
	}
}


/* Does some test to see if it is running in a sane environment */
void selfTest(){
	testSnprintf();
}
