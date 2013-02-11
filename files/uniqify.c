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
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
// more includes...

// defines...
#define SET_RED     	"\x1b[31m"
#define SET_GREEN   	"\x1b[32m"
#define SET_YELLOW  	"\x1b[33m"
#define SET_BLUE    	"\x1b[34m"
#define SET_MAGENTA 	"\x1b[35m"
#define SET_CYAN    	"\x1b[36m"
#define RESET_DA_COLOR  "\x1b[0m"
#define MAXLINE		512

// functions go here:
void parser();
void rmdup();

// spikes go here:
void spike_fork();
void spike_pipe();
void spike_sort();
void spike_psrt();

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
	//spike_fork();
	//spike_pipe();
	//spike_sort();
	spike_psrt();
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

void spike_fork()
{
	int result;
	int status;
	int i;
	int child_pid;
	char buf[MAXLINE];
	
	char *colors[6] = { SET_RED, SET_GREEN, SET_YELLOW, SET_BLUE,
		SET_MAGENTA, SET_CYAN };
		
	FILE *input;
	FILE *output;
	
	int pipefds[2];
	
	printf("spike_fork: starting spike test\n");
	if(pipe(pipefds) != 0){
		perror("Pipes are screwed up, call a plumber");
		exit(-1);
	}
	
	printf("spike_fork: starting spike test\n");
	
	for(i = 0; i < 6; i++) {
		printf("\nspike_fork: PARENT: creating %sCHILD[%d]" 
			RESET_DA_COLOR "\n", colors[i], i);
		
		switch((result = fork())){
	
		case -1:
			//in parent, oops
			printf("spike_fork:  PARENT: forking error\n");
			perror("Forking failed");
			exit(EXIT_FAILURE);
			break;
		case 0:
			//child case
			close(pipefds[0]);
			printf("spike_fork: %sCHILD[%d]" RESET_DA_COLOR 
				": writing to pipe\n", colors[i], i);
			input = fdopen(pipefds[1], "w");
			fputs(colors[i], input);
			fputs("\n", input);
			printf("spike_fork: %sCHILD[%d]"
				RESET_DA_COLOR ": exiting\n", colors[i], i);
			fclose(input);
			_exit(0);
			break;
		default:
			//parent case -- result holds pid of child
			
			break;
		}
	}
	
	close(pipefds[1]);
	output = fdopen(pipefds[0], "r");
	// probably won't work the way I want it to, i.e. fdopen truncates
	while((child_pid = wait(&status)) != -1) {
		fgets(buf, MAXLINE, output);
		printf("%sCHILD[] finished..." RESET_DA_COLOR "\n", buf);
	}
	fclose(output);
	printf("spike_fork: finished\n");
	if(errno != ECHILD) {
		perror("Error with child");
		exit(EXIT_FAILURE);
	}
	return;
}

void spike_pipe()
{
	char line[MAXLINE];
	FILE *fpin;
	FILE *fpout;
	int result;
	int fd[2];
	int i;
	int status;

	pipe(fd);

	switch((result = fork())) {

	case 0: 
		close(fd[0]);
	
		fpout = fdopen(fd[1], "w");
	
		for(i = 0; i < 10; i++)
			fputs("jeronimooo...", fpout);
	
		fclose(fpout);
		_exit(EXIT_SUCCESS);
		break;
	default:
		close(fd[1]);
	
		fpin = fdopen(fd[0], "r");
		wait(&status);
		while(fgets(line, MAXLINE, fpin) != NULL)
			printf("%d: %s", i++, line);
	
		fclose(fpin);
		break;
	}
	
	printf("spike_pipe:  PARENT: spike test finished\n");
	return;
}

void spike_sort()
{
	int pfd[2];
	FILE *fpout;
	char sort_this[] = "ant\nbeegle\nzanzibar\nmoo\ncow\nnope\nhit\npork";
	
	pipe(pfd);
	
	fpout = fdopen(pfd[1], "w");
	fputs(sort_this, fpout);
	fclose(fpout);
	
	dup2(pfd[0], STDIN_FILENO);
	close(pfd[0]);
	dup2(pfd[1], STDOUT_FILENO);
	close(pfd[1]);
	
	execlp("sort", "sort", (char *) NULL);
	
	return;
}

void spike_psrt()
{
	int pfd[2];
	FILE *fpout;
	
	int result;
	int i;
	
	char *sort_this[] = { "ant\n", "beegle\n", "zanzibar\n", "moo\n",
		"cow\n", "nope\n", "hit\n", "pork\n" };
	
	pipe(pfd);
	
	for(i = 0; i < 5; i++) {
		switch((result = fork())) {
		case -1:
			// parent effed up yo
			perror("oh no");
			exit(EXIT_FAILURE);
			break;
		case  0:
			// child case
			close(pfd[1]);
			dup2(pfd[0], STDIN_FILENO);
			close(pfd[0]);
			execlp("sort", "sort",  (char*) NULL);
			_exit(EXIT_FAILURE);
			break;
		default:
			// parent
			break;
		}
	}
	close(pfd[0]);
	fpout = fdopen(pfd[1], "w");
	for(i = 0; i < 8; i++)
		fputs(sort_this[i], fpout);
	fclose(fpout);
	
	return;
}
// Tests

void run_tests()
{
	
}






	      /* This white space is used for viewing pleasure */



/*_____________________________END OF FILE____________________________________*/