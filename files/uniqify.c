/**
 *
 * Divide Program into 3 types of processes:
 *	1. Single process that reads the input, parsing the lines into words.
 *	2. A group of processes does the sorting.
 *	3. Single process suppresses duplicate words & writes the output.
 *	
 * Must use the system sort command (/bin/sort) [Maybe use execve()?] with no
 * arguments to do the sorting. This program must arrange to start the processes
 * and plubm the pipes [no popen()]. The number of processes is a command line
 * argument to the program. The parser distributes the words round-robin to the
 * sorting processes.
 *	:: argv[1] or argv[2] == # of processes.
 * I/O to and from pipes => use the stream functions: fgets() & fputs()
 * Use fgets() & fputs() with fdopen() for attaching to pipes.
 * Use fclose() for flushing the streams.
 *
 * After each sort, much merge and uniqify words.
 *
 * Words are all:
 *	- Alphabetic
 *	- Case insensitive
 *	- Convert to lower [the parser should do this]
 *
 * All processes should terminate cleanly on the signals:
 *	- INTR
 * 	- QUIT
 *	- HANGUP
 *	[Use a signal handler: sigaction] [Do not use signal]
 *	[Ensure to issue QUIT signals to all children]
 *
 * Provide timings based on:
 * 	- Size of input file
 *	[Test with multiple types & sizes of files]
 *	[Include word lists & free form prose]
 *	[Timings should be plotted in a fashion that makes sense]
 *	[Recommended: matlab, gnuplot, R, or octave] [Use to generate plot]
 *	[Save the plot as a postscript file]
 *
 *
 * _________________EXTRA CREDIT________________________________________________
 *
 * Provide output similar to:
 *		uniq -c
 *	[Each word is output with its frequency]
 *	[For full credit: must calculate this "on the fly"]
 *	[This means: no storing all the words in memory]
 *
 * _____________________________________________________________________________
 *
 *
**/
	 
/*__________________________HEADER STUFF______________________________________*/

#include <stdio.h>
#include <stdlib.h>
// more includes...

// functions go here:
void parser();
void rmdup();
void spike_fork();

// test functions go here:
void run_tests();

/*_________________________END HEADER STUFF___________________________________*/



int main(int argc, char *argv[])
{
	// fill with stuff
	
	// parse the .txt file
	//parser();
	
	// sort
	
	// remove duplicates
	//rmdup();
	
	// testing
	spike_fork(10);
	return 0;
}

/**
 * for now, just initializing the function
 * will add args and return values when appropriate
**/
void parser()
{
	// parse stuff
}

/**
 * rmdup => remove duplicates
 *
 * for now, just initializing the function
 * will add args and return values when appropriate
**/
void rmdup()
{
	// removes duplicates from sorted list of wurds
}

// Spikes

void spike_fork(int n)
{
	int result;
	int status;
	int i;
	int child_pid;
	
	for(i = 0; i < n; i++) {
		printf("\nspike_fork: PARENT: forking once\n");
		
		switch((result = fork())){
	
		case -1:
			//in parent, oops
			printf("spike_fork:  PARENT: forking error\n");
			perror("Forking failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			//child case
		
			printf("spike_fork: CHILD[%d]: closing.\n", i);
			_exit(0);
			break;
		default:
			//parent case -- result holds pid of child
			break;
		}
	}
	
	while((child_pid = wait(&status) != -1))
		printf("...\n");
	printf("spike_fork: finished waiting for children?\n");
	if(errno != ECHILD) {
		perror("Error with child");
		exit(EXIT_FAILURE);
	}
	
}


// Tests

void run_tests()
{
	
}






	      /* This white space is used for viewing pleasure */



/*_____________________________END OF FILE____________________________________*/