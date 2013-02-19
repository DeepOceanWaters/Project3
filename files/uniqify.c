
/*__________________________HEADER STUFF______________________________________*/

// includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

// defines...
#define MAXLINE		512
#define PARENT		1
#define CHILD		0

// functions go here:
void pipe_children(FILE **fpin, int **pfd, int **sfd, int cur);
void init_sort(int *pfd, int *sfd);
void parser(int **pfd, int num_pipes);
void parse_buf(char *buf, FILE **fpout, int *i, int num_pipes);
FILE* merge_uniq(FILE **fpin, int cur);
FILE* mrg_two(FILE **fpin);
void rmdup(FILE *fpin);

void puke_exit(char *msg, int type);



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
	//time(&now);
	
	num_pipes = atoi(argv[1]);
	pfd = (int **) malloc(num_pipes * sizeof(int *));
	sfd = (int **) malloc(num_pipes * sizeof(int *));
	fpin = (FILE **) malloc(num_pipes * sizeof(FILE *));
	
	for(i = 0; i < num_pipes; i++)
		pipe_children(fpin, pfd, sfd, i);
	
	parser(pfd, num_pipes);
	fpout = merge_uniq(fpin, num_pipes - 1);
	rmdup(fpout);
	
	return 0;
}

/**
 * pipe_children(FILE **fpin, int **pfd, int **sfd, int cur)
 *
 * Description:
 *	Creates and initializes the pipes relavant to the child process about
 *	to be fork()'d into existance. When the child is created, it closes all
 *	irrelivant pipe file descriptors.
 * FILE **fpin:
 *	A FILE array of size num_pipes, this function uses this to initialize
 *	a FILE in the array for the parent process.
 * int **pfd:
 *	The array of pipes pointing from parent to child.
 * int **sfd:
 *	The array of pipes pointing from child to parent.
 * int cur:
 *	The current child that is being created.
**/
void pipe_children(FILE **fpin, int **pfd, int **sfd, int cur)
{
	int j;
	
	pfd[cur] = (int *) malloc(2 * sizeof(int));
	sfd[cur] = (int *) malloc(2 * sizeof(int));
	
	if(pipe(pfd[cur]))
		puke_exit("Pipes(pfd)", PARENT);
	if(pipe(sfd[cur]))
		puke_exit("Pipes(sfd)", PARENT);
		
	switch(fork()) {
	case -1:
		puke_exit("Fork", PARENT);
		break;
	case  0:
		for(j = 0; j < cur; j++) {
			close(sfd[j][0]);
			close(pfd[j][1]);
		}
		init_sort(pfd[cur], sfd[cur]);
		break;
	default:
		close(sfd[cur][1]);
		close(pfd[cur][0]);
		fpin[cur] = fdopen(sfd[cur][0], "r");
		break;
	}
	return;
}

/**
 * init_sort(int *pfd, int *sfd)
 *
 * Description:
 *	Called by a newly forked child - dups stdin/stdout appropriately then
 *	closes all other pipes and exec()'s the sort command.
 * int *pfd:
 *	The pipe pointing from parent to child relevant to this child.
 * int *sfd:
 *	The pipe pointing from child to parent relevant to this child.
**/
void init_sort(int *pfd, int *sfd)
{
	//close(STDIN_FILENO);
	dup2(pfd[0], STDIN_FILENO);
	
	
	//close(STDOUT_FILENO);
	dup2(sfd[1], STDOUT_FILENO);
	
	close(sfd[1]);
	close(pfd[1]);
	close(sfd[0]);
	close(pfd[0]);
	

	execlp("sort", "sort", (char *) NULL);
	_exit(EXIT_FAILURE);
}

/**
 * parser(int **pfd, int num_pipes)
 *
 * Description:
 *	Opens the write end of the pipes going from the parent to its children,
 *	then reads from stdin and calls parse_buf() to parse the input and 
 *	round-robin place the parsed buffer into the opened write FILEs.
 *	Finally, closes the write FILEs and returns.
 *
 * int **pfd:
 *	An array of pipes; specifically the pipes that writes from the parent
 *	to its children.
 *
 * int num_pipes:
 *	Number of pipes specified by user.
**/
void parser(int **pfd, int num_pipes)
{
	char buf[MAXLINE];
	int i;
	
	FILE **fpout;
	
	fpout = (FILE **) malloc(num_pipes * sizeof(FILE *));
	
	for(i = 0; i < num_pipes; i++)
		fpout[i] = fdopen(pfd[i][1], "w"); // open each stream
	
	while(fgets(buf, MAXLINE, stdin))
		parse_buf(buf, fpout, &i, num_pipes);
	
	for(i = 0; i < num_pipes; i++)
		fclose(fpout[i]);	// flush each stream
	
	return;
}

/**
 * parse_buf(char *buf, FILE **fpout, int *i, int num_pipes)
 *
 * Description:
 *	Used to parse a buffered line read from stdin. This function is used
 * 	by parser(). Also round-robin reads the parsed buffer into the different
 *	pipes.
 * char *buf:
 *	The buf read from stdin.
 * FILE **fpout:
 *	The different pipes, write end.
 * int *i:
 *	Pointer to an int to keep track of which fpout[] the function should be
 * 	writing to.
 * int num_pipes:
 * 	The number of pipes specified by the user. Used to keep i from going out
 * 	of bounds.
**/
void parse_buf(char *buf, FILE **fpout, int *i, int num_pipes)
{
	char new_buf[MAXLINE];
	int j;
	int leng;
	int k;
	
	k = 0;
	leng = strlen(buf);
	
	for(j = 0; j < leng; j++) {
		if(isalpha(buf[j])) {
			new_buf[k] = tolower(buf[j]);
			k++;
		}
		else if(strlen(new_buf) > 1) {
			new_buf[k] = *"\n";
			k++;
			new_buf[k] = *"\0";
			*i = *i % num_pipes;
			fputs(new_buf, fpout[*i]);
			strcpy(new_buf, "");
			(*i)++;
			k = 0;
		}
	}
	
	return;
}

/**
 * merge_uniq(FILE **fpin, int cur)
 *
 * Description:
 *	Recursive function that merges two FILEs (with the help of mrg_two()),
 *	and returns the result of the merged files. This result is used in 
 *	previous calls to merge_uniq().
 * FILE **fpin:
 *	A FILE array the size of num_pipes (however many the user specifies).
 * int cur:
 *	Used to determine the current place in the array. Each recursive call
 * 	to merge_uniq() subtracts 1 from cur until it reaches 0. When cur is
 * 	0, merge_uniq() returns fpin[0].
 *
**/
FILE* merge_uniq(FILE **fpin, int cur)
{
	if(cur == 0)
		return fpin[0];
	FILE **merger;
	merger = (FILE **) malloc(2 * sizeof(FILE *));
	merger[0] = fpin[cur];
	merger[1] = merge_uniq(fpin, cur - 1);
	return mrg_two(merger);
}

/**
 * mrg_two(FILE **fpin)
 *
 * Description:
 * 	mrg_two() takes a FILE array of size two and merges the two FILEs into
 * 	a single pipe. Used in conjunction with merge_uniq().
 * FILE **fpin:
 *	A FILE array of size two. This contains the two FILEs to be merged.
**/
FILE* mrg_two(FILE **fpin)
{
	char cur[MAXLINE];
	char nxt[MAXLINE];
	int cmp;
	int pfd[2];
	int x;
	FILE **new;
	
	if(!fgets(nxt, MAXLINE, fpin[0])) {
		fclose(fpin[0]);
		return fpin[1];
	}
	
	if(pipe(pfd))
		puke_exit("Piping", PARENT);
	
	new = (FILE **) malloc(2 * sizeof(FILE *));
	new[0] = fdopen(pfd[0], "r");
	new[1] = fdopen(pfd[1], "w");
	
	x = 1;
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
	
	x = (x + 1) % 2;
	
	fputs(nxt, new[1]);
	
	while(fgets(cur, MAXLINE, fpin[x]))
		fputs(cur, new[1]);
		
	fclose(fpin[0]);
	fclose(fpin[1]);
	fclose(new[1]);
	return new[0];
}

/**
 * rmdup(FILE *fpout)
 *
 * Description:
 *	Takes a sorted pipe, reads from it, and then uniqifies it and counts
 *	the number of times a word appears like uniq -c.
 * FILE *fpout:
 * 	The file that points to the read end of the final merged and sorted
 *	pipe.
**/
void rmdup(FILE *fpout)
{
	char buf[MAXLINE];
	char old_buf[MAXLINE];
	char new_buf[MAXLINE];
	int i;
	
	i = 1;
	fgets(old_buf, MAXLINE, fpout);
	while(fgets(buf, MAXLINE, fpout)) {
		if(strcmp(old_buf, buf) == 0)
			i++;
		else {
			old_buf[strlen(old_buf) - 1] = *"\0";
			sprintf(new_buf, "\t%d %s\n", i, old_buf);
			fputs(new_buf, stdout);
			strcpy(old_buf, buf);
			i = 1;
		}
	}
	
	fclose(fpout);
	return;
}

/**
 * puke_exit(char *msg, int type)
 *
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



	      /* This white space is used for viewing pleasure */



/*_____________________________END OF FILE____________________________________*/
