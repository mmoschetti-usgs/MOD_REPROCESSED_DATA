/* Tiny CSV Reader */
/* Copyright (C) 2015, Deligiannidis Konstantinos

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://w...content-available-to-author-only...u.org/licenses/>.  */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NCOLS 300

/* For more that 100 columns or lines (when delimiter = \n), minor modifications are needed. */
int getcols( const char * const line, const char * const delim, char ***out_storage )

{
    const char *start_ptr, *end_ptr, *iter;
    char **out;
    int i;                                          //For "for" loops in the old c style.
    int tokens_found = 1, delim_size, line_size;    //Calculate "line_size" indirectly, without strlen() call.
    int start_idx[NCOLS], end_idx[NCOLS];   //Store the indexes of tokens. Example "Power;": loc('P')=1, loc(';')=6
    //Change 100 with MAX_TOKENS or use malloc() for more than 100 tokens. Example: "b1;b2;b3;...;b200"

    if ( *out_storage != NULL )                 return -4;  //This SHOULD be NULL: Not Already Allocated
    if ( !line || !delim )                      return -1;  //NULL pointers Rejected Here
    if ( (delim_size = strlen( delim )) == 0 )  return -2;  //Delimiter not provided

    start_ptr = line;   //Start visiting input. We will distinguish tokens in a single pass, for good performance.
                        //Then we are allocating one unified memory region & doing one memory copy.
    while ( ( end_ptr = strstr( start_ptr, delim ) ) ) {

        start_idx[ tokens_found -1 ] = start_ptr - line;    //Store the Index of current token
        end_idx[ tokens_found - 1 ] = end_ptr - line;       //Store Index of first character that will be replaced with
                                                            //'\0'. Example: "arg1||arg2||end" -> "arg1\0|arg2\0|end"
        tokens_found++;                                     //Accumulate the count of tokens.
        start_ptr = end_ptr + delim_size;                   //Set pointer to the next c-string within the line
    }

    for ( iter = start_ptr; (*iter!='\0') ; iter++ );

    start_idx[ tokens_found -1 ] = start_ptr - line;    //Store the Index of current token: of last token here.
    end_idx[ tokens_found -1 ] = iter - line;           //and the last element that will be replaced with \0

    line_size = iter - line;    //Saving CPU cycles: Indirectly Count the size of *line without using strlen();

    int size_ptr_region = (1 + tokens_found)*sizeof( char* );   //The size to store pointers to c-strings + 1 (*NULL).
    out = (char**) malloc( size_ptr_region + ( line_size + 1 ) + 5 );   //Fit everything there...it is all memory.
    //It reserves a contiguous space for both (char**) pointers AND string region. 5 Bytes for "Out of Range" tests.
    *out_storage = out;     //Update the char** pointer of the caller function.

    //"Out of Range" TEST. Verify that the extra reserved characters will not be changed. Assign Some Values.
    //char *extra_chars = (char*) out + size_ptr_region + ( line_size + 1 );
    //extra_chars[0] = 1; extra_chars[1] = 2; extra_chars[2] = 3; extra_chars[3] = 4; extra_chars[4] = 5;

    for ( i = 0; i < tokens_found; i++ )    //Assign adresses first part of the allocated memory pointers that point to
        out[ i ] = (char*) out + size_ptr_region + start_idx[ i ];  //the second part of the memory, reserved for Data.
    out[ tokens_found ] = (char*) NULL; //[ ptr1, ptr2, ... , ptrN, (char*) NULL, ... ]: We just added the (char*) NULL.
                                                        //Now assign the Data: c-strings. (\0 terminated strings):
    char *str_region = (char*) out + size_ptr_region;   //Region inside allocated memory which contains the String Data.
    memcpy( str_region, line, line_size );   //Copy input with delimiter characters: They will be replaced with \0.

    //Now we should replace: "arg1||arg2||arg3" with "arg1\0|arg2\0|arg3". Don't worry for characters after '\0'
    //They are not used in standard c lbraries.
    for( i = 0; i < tokens_found; i++) str_region[ end_idx[ i ] ] = '\0';

    //"Out of Range" TEST. Wait until Assigned Values are Printed back.
    //for ( int i=0; i < 5; i++ ) printf("c=%x ", extra_chars[i] ); printf("\n");

    // *out memory should now contain (example data):
    //[ ptr1, ptr2,...,ptrN, (char*) NULL, "token1\0", "token2\0",...,"tokenN\0", 5 bytes for tests ]
    //   |__________________________________^           ^              ^             ^
    //          |_______________________________________|              |             |
    //                   |_____________________________________________|      These 5 Bytes should be intact.

    return tokens_found;
}
