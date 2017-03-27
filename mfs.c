/*
 * Name: Shaun Sartin
 * ID #: 1000992189
 */

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // BEGIN STUDENT CODE IN MAIN()

    // Rename/Relocate data, so that the variable name is more intuitive
    char *cmd = token[0];
    char *arg1 = token[1];

    //NOTE: ADD DESCRIPTION
    short bps;

    //NOTE: ADD DESCRIPTION
    short spc;

    //NOTE: ADD DESCRIPTION
    short rsc;

    // This variable is used to determine which file system is open. 
    // That way, this program can handle the case where the user tries to close a file system that has not been opened.
    char open_file_sys[MAX_COMMAND_SIZE];

    // This file pointer will be used to determine whether or not the fat32.img file is open.
    FILE *fp;

    if((strcmp(cmd, "quit") == 0) || (strcmp(cmd, "exit") == 0))
    {
	free(working_root);
	return 0;
    }
    
    // Attempt to open the file system image and, if it is opened, store the name in the 'open_file_sys' string. 
    else if(strcmp(cmd, "open") == 0)
    {

	fp = fopen(arg1, "r");
	if (fp == NULL)
	{
		printf("Error: File system image not found.\n");
	}

	else
	{
		printf("File image: %s was successfully opened.\n", arg1);
		strcpy(open_file_sys, arg1);
	}
    }

    // Attempt to close the file system image. If sucessful, reset the 'open_file_sys' string.
    else if(strcmp(cmd, "close") == 0)
    {
	if(arg1 == NULL)
	{
		printf("No file system image specified. Try 'close <filename>'\n");
	}

	else if(strcmp(open_file_sys, arg1) == 0)
	{
		printf("File image: %s was successfully closed.\n", arg1);
		memset(open_file_sys, 0, MAX_COMMAND_SIZE);
		fclose(fp);
	}

	else
	{
		printf("Error: File system image must be opened first.\n");
	}
    }

    else if(strcmp(cmd, "info") == 0)
    {
	// If open_file_sys has not been reset, then there is a file system image open.
        // Therefore, we can perform the 'info' operation.
	if(strcmp(open_file_sys, "") != 0)
	{
		//Skip to BPB_BytsPerSec
		fseek(fp, 11, SEEK_SET);

		//Read BPB_BytsPerSec
		fread(&bps, 2, 1, fp);
		printf("BytesPerSec: %hi\n", bps);

		//Read BPB_SecPerClus
		fread(&spc, 1, 1, fp);
		printf("SecPerClus: %hi\n", spc);
		
		//Read BPB_RsvdSecCnt
		fread(&rsc, 2, 1, fp);
		printf("RsvdSecCnt: %hi\n", rsc);
	}    
    }
    // END STUDENT CODE IN MAIN()
    free( working_root );

  }
  return -1;
}
