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
#define PARENT		1
#define CHILD		0

// functions go here:
int** init_pipes(int num_pipes);
void init_sort(int *pfd, int *sfd);
void parser(int **pfd, int num_pipes);
FILE* merge_uniq(FILE **fpin, int cur, int max);
FILE* mrg_two(FILE **fpin);

void puke_exit(char *msg, int type);

// spikes go here:
void spike_fork();
void spike_pipe();
void spike_sort();
void spike_psrt();
void spike_stdin();

// test functions go here:
void run_tests();

/*_________________________END HEADER STUFF___________________________________*/



int main(int argc, char *argv[])
{
	int **pfd; // an array of pipe file descriptors for child-parent com
	int **sfd; // an array of pipe file descriptors for sort
	int i;
	int num_pipes;
	FILE **fpin;
	FILE *fpout;
	char buf[MAXLINE];
	
	if(argc < 2) {
		printf("uniqify: number of pipes is required.\n");
		exit(EXIT_FAILURE);
	}
	if(argc > 2) {
		printf("uniqify: too many arguments.\n");
		exit(EXIT_FAILURE);
	}
	
	// set timer
	
	num_pipes = atoi(argv[1]);
	fpin = (FILE **) malloc(num_pipes * sizeof(FILE *));
	pfd = init_pipes(num_pipes);
	sfd = init_pipes(num_pipes);
	
	for(i = 0; i < num_pipes; i++) {
		switch(fork()) {
			case -1:
				puke_exit("Fork", PARENT);
				break;
			case  0:
				printf("so many dicks wtf\n");
				init_sort(pfd[i], sfd[i]);
				break;
			default:
				break;
		}
	}
	
	/*for(i = 0; i < num_pipes; i++)
		fpin[i] = fdopen(sfd[i][0], "r"); // open each stream*/
	for(i = 0; i < num_pipes; i++) {
		close(sfd[i][1]);
		close(pfd[i][0]);
		fpin[i] = fdopen(sfd[i][0], "r");
	}
	parser(pfd, num_pipes);
	printf("sup about to merge yo\n");
	fpout = merge_uniq(fpin, 0, num_pipes - 1);
	printf("done mergin yo\n");
	while(fgets(buf, MAXLINE, fpout))
		printf("%s\n", buf);
	/*for(i = 0; i < num_pipes; i++) {
		while(fgets(buf, MAXLINE, fpout))
	}*/
	return 0;
}

int** init_pipes(int num_pipes)
{
	int i;
	int **pfd;
	
	pfd = (int **) malloc(num_pipes * sizeof(int *));
	
	for(i = 0; i < num_pipes; i++) {
		pfd[i] = (int *) malloc(2 * sizeof(int));
		if(pipe(pfd[i]))
			puke_exit("Pipes", PARENT);
	}
	
	return pfd;
}

void init_sort(int *pfd, int *sfd)
{
	char buf[MAXLINE];
	int i = 0;
	int result;
	printf("dup2 pfd\n");
	dup2(pfd[0], STDIN_FILENO);
	printf("closing: %d\n", i);
	i++;
	close(pfd[0]);
	
	printf("dup2 sfd\n");
	if((result = dup2(sfd[1], STDOUT_FILENO)) == -1)
		puke_exit("Fucking stdout sux dix", CHILD);
	printf("result: %d\n", result);
	printf("closing: %d\n", i);
	i++;
	close(sfd[1]);
	
	printf("closing: %d\n", i);
	i++;
	close(pfd[1]);
	
	printf("closing: %d\n", i);
	i++;
	close(sfd[0]);
	
	printf("looping through shit yo\n");
	while(fgets(buf, MAXLINE, stdin)) {
		printf("CHILD IS PUTTING: %s\n", buf);
		fputs(buf, stdout);
	}
	fclose(stdin);
	fclose(stdout);
	printf("done sorting;;;;\n");
	//execlp("sort", "sort", (char *) NULL);
	_exit(EXIT_SUCCESS);
}

/**
 * for now, just initializing the function
 * will add args and return values when appropriate
**/
void parser(int **pfd, int num_pipes)
{
	// parse stuff
	char buf[MAXLINE];
	
	int i;
	int result;
	FILE **fpout;
	
	fpout = (FILE **) malloc(num_pipes * sizeof(FILE *));
	// need to parse stuff yo
	
	for(i = 0; i < num_pipes; i++)
		fpout[i] = fdopen(pfd[i][1], "w"); // open each stream
	
	i = 0;
	while(fgets(buf, MAXLINE, stdin)) {
		// parse buf
		i = i % num_pipes;
		printf("FGETS: buf=%si=%d\n", buf, i);
		fputs(buf, fpout[i]);
		i++;
	}
	printf("\n\nGot out\n\n");
	for(i = 0; i < num_pipes; i++)
		fclose(fpout[i]);	// flush each stream
	printf("closed errthang\n");
	return;
}

/**
 * Description:
 *	Merges the information, and uniqifies the words.
 *
**/
FILE* merge_uniq(FILE **fpin, int cur, int max)
{
	printf("I'm merging but not really? cur=%d||max=%d\n", cur, max);
	if(cur == max)
		return fpin[max];
	printf("[BEFO]MERGIN AT: %d/%d\n", cur, max);
	FILE **merger;
	merger = (FILE **) malloc(2 * sizeof(FILE *));
	merger[0] = fpin[cur];
	merger[1] = merge_uniq(fpin, cur + 1, max);
	printf("[AFTA]MERGIN AT: %d/%d\n", cur, max);
	return mrg_two(merger);
}

FILE* mrg_two(FILE **fpin)
{
	printf("initing stuffffff\n");
	char cur[MAXLINE];
	char nxt[MAXLINE];
	int cmp;
	int pfd[2];
	int x;
	FILE **new;
	
	printf("boobs\n");
	if(!fgets(nxt, MAXLINE, fpin[0])) {
		fclose(fpin[0]);
		return fpin[1];
	}
	printf("butssssss\n");
	if(pipe(pfd))
		puke_exit("Piping", PARENT);
	printf("made pipes\n");
	new = (FILE **) malloc(2 * sizeof(FILE *));
	new[0] = fdopen(pfd[0], "r");
	new[1] = fdopen(pfd[1], "w");
	
	x = 1;
	printf("about to loooooop\n");
	while(fgets(cur, MAXLINE, fpin[x])) {
		cmp = strcmp(cur, nxt);
		if(cmp < 0) {
			fputs(cur, new[1]);
		}
		else {
			fputs(nxt, new[1]);
			strcpy(nxt, cur);
			x = (x + 1) % 2;
		}
	}
	printf("done loopen\n");
	
	x = (x + 1) % 2;
	printf("putting nxt\n");
	fputs(nxt, new[1]);
	printf("about to loooooop the last bit\n");
	while(fgets(cur, MAXLINE, fpin[x]))
		fputs(cur, new[1]);
	printf("done loopin\n");
	fclose(fpin[0]);
	fclose(fpin[1]);
	fclose(new[1]);
	return new[0];
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

/**
 * Description:
 *	Takes a message, and pukes & exits. If process type is unknown, puke
 *	error and continue the process.
 * char *msg:
 *	The message usually passed to the function perror(), perror() is called
 *	using msg.
 * int type:
 *	Used to determine how to exit. If type is the parent process => use
 *	exit(), if the type is a child process => use _exit().
 *	If type is not either PARENT or CHILD, puke_exit() does not exit and
 *	continues with the process. This is done because calling exit() on the
 *	wrong process could cause an unwanted outcome.
**/
void puke_exit(char *msg, int type)
{
	perror(msg);
	if(type == PARENT)
		exit(EXIT_FAILURE);
	else if(type == CHILD)
		_exit(EXIT_FAILURE);
	else
		printf("%s: unknown process type - continuing\n", msg);
	return;
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
	FILE *fpin;
	
	int result;
	int i;
	char buf[MAXLINE];
	char *colors[6] = { SET_RED, SET_GREEN, SET_YELLOW, SET_BLUE,
		SET_MAGENTA, SET_CYAN };
	
	char *sort_this[] = { "ant\n", "beegle\n", "zanzibar\n", "moo\n",
		"cow\n", "nope\n", "hit\n", "pork\n" };
	
	pipe(pfd);
	
	for(i = 0; i < 6; i++) {
		switch((result = fork())) {
		case -1:
			// parent effed up yo
			perror("oh no");
			exit(EXIT_FAILURE);
			break;
		case  0:
			// child case
			close(pfd[0]);
			fpin = fdopen(pfd[1], "r");
			fgets(buf, MAXLINE, fpin);
			printf("%sCHILD[%d]: read: %s " RESET_DA_COLOR 
				"\n", colors[i], i, buf);
			fclose(fpin);
			//execlp("sort", "sort",  (char*) NULL);
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

void spike_stdin()
{
	/*int pfd[2];
	int pfd2[2];
	int i;
	FILE *fpout;
	FILE *fpin;
	
	char buf[MAXLINE];
	
	char *old;
	char *content;
	
	int cnt_size;
	int result;
	
	pipe(pfd);
	pipe(pfd2);
	
	for(i = 0; i < 3; i++) {
	switch((result = fork())) {
	case -1:
		// parent effed up yo
		perror("oh no");
		exit(EXIT_FAILURE);
		break;
	case  0:
		// child case
		dup2(pfd[i][0], STDIN_FILENO);
		close(pfd[i][0]);
		dup2(pfd2[i][1], STDOUT_FILENO);
		close(pfd2[i][1]);
		close(pfd[i][1]);
		close(pfd2[i][0]);
		
		execlp("sort", "sort",  (char*) NULL);
		_exit(EXIT_FAILURE);
		break;
	default:
		// parent
		break;
	}
	}
	
	cnt_size = 0;
	
	close(pfd[i][0]);
	close(pfd2[i][1]);
	fpout = fdopen(pfd[1], "w");
		
	while(fgets(buf, MAXLINE, stdin))
		fputs(buf, fpout);
	fclose(fpout);
	
	printf("Done writing, sorting now...\n");
	fpin = fdopen(pfd2[0], "r");
	while(fgets(buf, MAXLINE, fpin))
		printf("%s\n", buf);
	fclose(fpin);
	
	return;	//*/
}
// Tests

void run_tests()
{
	
}






	      /* This white space is used for viewing pleasure */



/*_____________________________END OF FILE____________________________________*/