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
#include <stdint.h>
#include <ctype.h>
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // mfs.c only supports ten arguments


// Function: Filename Comparison
// Purpose: Modify user-inputted filename to see it matches the corresponding filename stored in the directory entry.
// 	    Subdirectories are considered to be files, and thus will work with this function.
// Returns: 0 if match, -1 if no match
int fcmp(char *uname, char *dname)
{
	int i;

	// This is used as an intermediary string. This will hold the converted version of uname.
	char cname[11];

	// Used to calculate the length of the user-inputted filename. This is used in the comparison.
	int ulen = 1;

	// If there is a dot in the user-inputted filename, the dotIndex will be the position of that character.
	// Otherwise, dotIndex will be equal to -1.
	int dotIndex = -1;

	// Convert every value in the user-inputted filename to an uppercase character as that is what is in the directory entries.
	for(i = 0; uname[i]; i++)
	{
		uname[i] = (char) toupper(uname[i]);
		if(uname[i] == '.')
		{
			dotIndex = i;
		}
		ulen += 1;
	}
	

	if((strcmp(uname, ".") == 0) || (strcmp(uname, "..") == 0))
	{
		for(i = 0; i < 11; i++)
		{
			cname[i] = dname[i];
		}
	
	        if(strcmp(cname, dname) == 0)
		{
			return 0;
		}
	}

	else if(dotIndex != -1)
	{

		// Copy every value from uname to cname up to the '.' character.
		for(i = 0; i < dotIndex; i++)
		{
			cname[i] = uname[i];
		}

		// If applicable, append spaces so that the format is similar to the directory entry's name.
		for(; i < 8; i++)
		{
			cname[i] = ' ';
		}

		// Add the file extension in the last three indexes.
		for(; i < 11; i++)
		{
			cname[i] = uname[(dotIndex + 1) + (i - 8)];
		}

	}
	
	//Check to see if 'uname' is a directory
	else
	{
		// Copy every value from uname to cname until there are no more characters in uname.
		// Note that we do not wish to copy the null-terminator.
		for(i = 0; i < (ulen - 1); i++)
		{
			cname[i] = uname[i];
		}

		// If applicable, append spaces to cname so that the format is similar to the directory entry's name.
		for (; i < 11; i++)
		{
			cname[i] = ' ';
		}
		cname[11] = '\0';
	}

	// DEBUG printf("Cname: %s \tDname: %s \tUlen: %d \tClen: %d \tDlen: %d \n", cname, dname, ulen, strlen(cname), strlen(dname));
	if(strcmp(cname, dname) == 0)
	{
		//NOTE: Delete print
		printf("FILE FOUND: %s\n", cname);
		return 0;
	}

	return -1;
}

// Function: Recursive Read
// Purpose: Read the data from a file in chunks. Grants ability to read bytes from multiple clusters.
// Returns: void
// NOTE: How to ensure that we stay at same pointer.

// arg2 and arg3 acts as a 'bytes-left' counter
// arg2 will be 0 once the beginning position has been found.
// arg3 will be 0 once the last character has been read.
void rread(FILE* fp, int rootStart, long arg2, long arg3, int currClusNum, int numOfClusters)
{
	int i;
	int tmp;
	char buffer[513];
	memset(buffer, 0, sizeof(buffer));

	// Recursively go through and determine which cluster contains the initial offset.
	// I.E. 1st cluster, 2nd cluster, etc. (This is held by numOfClusters, which should always intially be 1)
	if(arg2 > 511)
	{
		rread(fp, rootStart, arg2 - 512, arg3, currClusNum, numOfClusters + 1);
		return;
	}

	// Then go to that particular cluster
	else
	{
		// Reference the FAT to determine what the cluster number is.
		// This determines the actual cluster number.
		// note: the FAT starts at the 16384th byte
		for(int i = 0; i < numOfClusters; i++)
		{
			fseek(fp, 16384 + (currClusNum * 4), SEEK_SET);
			fread(&tmp, 1, 4, fp);

			// 268435455 is the 0F FF FF FF marker that signifies no more clusters, and is therefore not a valid cluster.
			if (tmp != 268435455)
			{
				currClusNum = tmp;
			}
			// DEBUG: printf("currClusNum is now: %d\n", currClusNum);
		}

		// Fseek to the data for that cluster
		fseek(fp, rootStart + (currClusNum * 512) + arg2 - 1024, SEEK_SET);
		if (arg3 <= 512)
		{
			fread(buffer, 1, arg3, fp);
			buffer[512] = '\0';
			printf("%s", buffer);
			memset(buffer, 0, sizeof(buffer));
			return;
		}
		else
		{
			fread(buffer, 1, 512, fp);
			buffer[512] = '\0';
			printf("%s", buffer);
			memset(buffer, 0, sizeof(buffer));
			rread(fp, rootStart, arg2, arg3 - 512, currClusNum, numOfClusters);
		}
	}
}

int main()
{

  // This variable is used to determine which file system is open.
  // That way, this program can handle the case where the user tries to close a file system that has not been opened.
  // This is out of the loop so that memset doesn't run every iteration.
  char open_file_sys[MAX_COMMAND_SIZE];
  memset(open_file_sys, 0, MAX_COMMAND_SIZE);

  // Keeps track of all the directories and their location in the clusters
  int dir_location[5];

  // Keeps track of what index in dir_location we are currently at. (i.e. current working directory)
  int cwd = 0;

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

    // Rename data, so that the variable name is more intuitive
    char *cmd = token[0];
    char *arg1 = token[1];
    char *arg2 = token[2];
    char *arg3 = token[3];

    //NOTE: ADD DESCRIPTION
    int16_t bps;

    //NOTE: ADD DESCRIPTION
    int8_t spc;

    //NOTE: ADD DESCRIPTION
    int16_t rsc;

    //NOTE: ADD DESCRIPTION
    int8_t numFATS;

    //NOTE: ADD DESCRIPTION
    char volLab[12];

    //NOTE: ADD DESCRIPTION
    int32_t FATSz32;

    //NOTE: ADD DESCRIPTION
    int rootStart;

    //NOTE: ADD DESCRIPTION
    int i;

    //NOTE: ADD DESCRIPTION
    int file_flag = 0;

    //NOTE: ADD DESCRIPTION
    struct  __attribute__ ((__packed__)) DirectoryEntry {
    	char DIR_Name[12];
    	uint8_t DIR_Attr;
    	uint8_t Unused1[8];
    	uint16_t DIR_FirstClusterHigh;
    	uint8_t Unused2[4];
    	uint16_t DIR_FirstClusterLow;
    	uint32_t DIR_FileSize;
    };

    //NOTE: ADD DESCRIPTION
    struct DirectoryEntry dir[16];

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

		//Skip to BPB_BytsPerSec
		fseek(fp, 11, SEEK_SET);

		//Read BPB_BytsPerSec
		fread(&bps, 1, 2, fp);

		//Read BPB_SecPerClus
		fread(&spc, 1, 1, fp);

		//Read BPB_RsvdSecCnt
		fread(&rsc, 1, 2, fp);

		//Read BPB_NumFATS
		fread(&numFATS, 1, 1, fp);

		//Skip to BPB_FATSz32
		fseek(fp, 19, SEEK_CUR);

		//Read BPB_FATSz32
		fread(&FATSz32, 1, 4, fp);

		rootStart = (numFATS * FATSz32 * bps) + (rsc * bps);

		// Move to the start of the root directory.
		// That way, we can read all of its data.
		fseek(fp, rootStart, SEEK_SET);
		for(i = 0; i < 16; i++)
		{
			// Read all the needed data from the directory and store it in its appropriate index.
			fread(dir[i].DIR_Name, 1, 11, fp);
			fread(&dir[i].DIR_Attr, 1, (uint8_t) 1, fp);
			fseek(fp, 8, SEEK_CUR);
			fread(&dir[i].DIR_FirstClusterHigh, 1, 2, fp);
			fseek(fp, 4, SEEK_CUR);
			fread(&dir[i].DIR_FirstClusterLow, 1, 2, fp);
			fread(&dir[i].DIR_FileSize, 1, 4, fp);

			// Add Null-Terminator to the end of the filename.
			dir[i].DIR_Name[11] = '\0';
		}
		dir_location[0] = rootStart;
		fseek(fp, rootStart, SEEK_SET);
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
		printf("Bytes Per Sector \t in Decimal: %5d\t In Hex: %4x\n", bps, bps);
		printf("Sectors Per Cluster \t in Decimal: %5d\t In Hex: %4x\n", spc, spc);
		printf("Reserved Sector Count \t in Decimal: %5d\t In Hex: %4x\n", rsc, rsc);
		printf("Number of FATS \t\t in Decimal: %5d\t In Hex: %4x\n", numFATS, numFATS);
		printf("FATSz32 \t\t in Decimal: %5d\t In Hex: %4x\n", FATSz32, FATSz32);
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}
    }
    else if(strcmp(cmd, "volume") == 0)
    {

	if(strcmp(open_file_sys, "") != 0)
	{
		//Skip to BS_VolLab
		fseek(fp, 71, SEEK_SET);

		//Read BS_VolLab
		//We must null-terminate the string so that extra garbage doesn't print out.
		fread(volLab, 1, 11, fp);
		volLab[11] = '\0';
		
		//NOTE: FIX the If-statement
		if(1 == 1)
		{
			printf("VolLab: %s\n", volLab);
		}
		else
		{
			printf("Error: volume name not found.\n");
		}
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}
    }

    else if(strcmp (cmd, "ls") == 0)
    {
	if(strcmp(open_file_sys, "") != 0)
	{
		printf("Name       |Attr   |HClus |LClus     |Size \n");
		for(i = 0; i < 16; i++)
		{
			// If the filename's first character has a hex-value of 0xE5 or 0x05, the file has been deleted.
			// Therefore, we do not show it.
			// If the file's attribute has a hex-value of 0x01, 0x10, or 0x20.
			if (((dir[i].DIR_Name[0] != (char) 5) && (dir[i].DIR_Name[0] != (char) 229)) &&
				((dir[i].DIR_Attr == (uint8_t) 1) || (dir[i].DIR_Attr == (uint8_t) 16) || (dir[i].DIR_Attr == (uint8_t) 32)))
			{
				//NOTE: Only need to print the filename.
				printf("%s \t %d \t %d \t %d \t %d \n", dir[i].DIR_Name, dir[i].DIR_Attr,
					dir[i].DIR_FirstClusterHigh, dir[i].DIR_FirstClusterLow, dir[i].DIR_FileSize);
			}
		}
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}

    }

    else if(strcmp(cmd, "stat") == 0)
    {
	if (strcmp(open_file_sys, "") != 0)
	{
		for (i = 0; i < 16; i++)
		{
			if (fcmp(arg1, dir[i].DIR_Name) == 0)
			{
				file_flag = 1;
				break;
			}
		}
		if (file_flag == 1)
		{
			printf("Attributes: %d\nLow Cluster Number: %d\n", dir[i].DIR_Attr, dir[i].DIR_FirstClusterLow);
			file_flag = 0;
		}
		else
		{
			printf("Error: File not found.\n");
		}
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}
    }

    else if(strcmp (cmd, "read") == 0)
    {
	if(strcmp(open_file_sys, "") != 0)
	{
		for(i = 0; i < 16; i++)
		{
			if(fcmp(arg1, dir[i].DIR_Name) == 0)
			{
				file_flag = 1;
				break;
			}
		}
		if(file_flag == 1)
		{
			if (atol(arg2) + atol(arg3) > dir[i].DIR_FileSize)
			{
				printf("Error: File Size Exceeded.\n");
			}
			else
			{
				//NOTE: FIX THIS. What happens if we reach the end of the cluster? multiple clusters?
				//Check user-input to ensure they enter the correct data-type.
				//fseek(fp, (long int) arg2, SEEK_SET);
				rread(fp, rootStart, atol(arg2), atol(arg3), dir[i].DIR_FirstClusterLow, 1);
				//reset fp
			}
			printf("\n");
			file_flag = 0;
		}
		else
		{
			printf("Error: File not found.\n");
		}
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}

    }

    else if(strcmp(cmd, "cd") == 0)
    {

	if(strcmp(open_file_sys, "") != 0)
	{
		for (i = 0; i < 16; i++)
		{
			// Look for subdirectories
			// Note that subdirectories have the value of 16 (decimal) for their attribute.
			// DEBUG: printf("arg1: %s, DIR_Name: %s, DIR_Attr: %d\n", arg1, dir[i].DIR_Name, dir[i].DIR_Attr);
			if ((fcmp(arg1, dir[i].DIR_Name) == 0) && (dir[i].DIR_Attr == 16))
			{
				file_flag = 1;
				break;
			}
		}
		if (file_flag == 1)
		{
			if (strcmp(arg1, ".") == 0)
			{
			}
		
			else if (strcmp(arg1, "..") == 0)
			{
				// If we're already at the root directory, we do nothing.
				if (cwd != 0)
				{
					fseek(fp, dir_location[cwd - 1], SEEK_SET);
					for(i = 0; i < 16; i++)
                                	{
                                        	// Read all the needed data from the directory and store it in its appropriate index.
                                        	fread(dir[i].DIR_Name, 1, 11, fp);
                                        	fread(&dir[i].DIR_Attr, 1, (uint8_t) 1, fp);
                                        	fseek(fp, 8, SEEK_CUR);
                                        	fread(&dir[i].DIR_FirstClusterHigh, 1, 2, fp);
                	                        fseek(fp, 4, SEEK_CUR);
               	        	                fread(&dir[i].DIR_FirstClusterLow, 1, 2, fp);
        	                                fread(&dir[i].DIR_FileSize, 1, 4, fp);
	
                                        	// Add Null-Terminator to the end of the filename.
                                	        dir[i].DIR_Name[11] = '\0';
                                	}
                                	fseek(fp, dir_location[cwd - 1], SEEK_SET);
                                	//DEBUG printf("FP is now at: %d\n", dir_location[cwd - 1]); 
					cwd -= 1;
				}
				
			}

			else
			{	
				cwd += 1;
				dir_location[cwd] = rootStart + ((dir[i].DIR_FirstClusterLow * 512) - 1024);	
				fseek(fp, rootStart + ((dir[i].DIR_FirstClusterLow * 512) - 1024), SEEK_SET);
				for(i = 0; i < 16; i++)
				{
					// Read all the needed data from the directory and store it in its appropriate index.
					fread(dir[i].DIR_Name, 1, 11, fp);
					fread(&dir[i].DIR_Attr, 1, (uint8_t) 1, fp);
					fseek(fp, 8, SEEK_CUR);
					fread(&dir[i].DIR_FirstClusterHigh, 1, 2, fp);
					fseek(fp, 4, SEEK_CUR);
					fread(&dir[i].DIR_FirstClusterLow, 1, 2, fp);
					fread(&dir[i].DIR_FileSize, 1, 4, fp);

					// Add Null-Terminator to the end of the filename.
					dir[i].DIR_Name[11] = '\0';
				}
				fseek(fp, dir_location[cwd], SEEK_SET);
				//DEBUG printf("FP is now at: %d\n", dir_location[cwd]);
			}
			file_flag = 0;
		}
		else
		{
			printf("Error: Directory not found.\n");
		}
	}
	else
	{
		printf("Error: File system image must be opened first.\n");
	}
    }

    printf("\n");
    // END STUDENT CODE IN MAIN()
    free( working_root );

  }
  return -1;
}
