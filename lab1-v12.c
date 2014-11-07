#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#define SWITCHED 1

int fd1, fd2;
int pipe1[2], pipe2[2];
pid_t pid1;
pid_t pid2;
char *file1;

void switch_files(int sig_type)
{
	if (sig_type == SIGUSR1)
	{
		write(pipe2[1], file1, strlen(file1)+1);
   		 if (switch_file(pipe1[0], &fd1, 1) == SWITCHED)
   		 {
   		 	kill(pid2, SIGUSR2);
   		 }
		
	}
	if (sig_type == SIGUSR2)
	{
		switch_file(pipe2[0], &fd2, 2);
	}
}

int switch_file(int checking_desc, int *switching_desc, int p_idx)
{
	char buf[64];
	if (read(checking_desc, buf, 64) > 0)
	{
		int openning = open(buf, O_RDONLY);
		if (openning > 0)
		{
			if (p_idx == 1)
			{
				char *new_file_name = (char *)malloc(256);
				memset(new_file_name, 0, 256);
				strcpy(new_file_name, buf);
				file1 = new_file_name;
				//printf("file1 %s\n", file1);
			}
			close(*switching_desc);
			*switching_desc = openning;
		}
		else
		{
			perror(buf);
			return -1;
		}
		return SWITCHED;
	}
	return 0;
}

void terminate(int sig_type)
{
	_exit(0);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "give 2 args\n");
		exit(-1);
	}

	file1 = argv[1];
	char *file2 = argv[2];

	pipe(pipe1);
	pipe(pipe2);

	signal(SIGINT, terminate);


	if ((pid2 = fork()) == 0)
	{
		printf("2 process pid is %d\n", getpid());
		signal(SIGUSR2, switch_files);
		fd2 = open(file2, O_RDONLY);
		if (fd2 < 0)
		{
			fprintf(stderr, "error opening file %s\n", file2);
			return -1;
		}
		int read_bytes = 0;
		char buf[1];
		while(1)
		{
			read_bytes = read(fd2, buf, 1);
			if (read_bytes > 0)
			{
				fprintf(stdout, "\t%c\n", tolower(buf[0]));
			}
			usleep(500000);
		}
		close(fd2);
		printf("fd2 closed\n");
		return 0;
	}

	if ((pid1 = fork()) == 0)
	{
		signal(SIGUSR1, switch_files);
		fd1 = open(file1, O_RDONLY);
		if (fd1 < 0)
		{
			fprintf(stderr, "error opening file %s\n", file1);
			return -1;
		}
		int read_bytes = 0;
		char buf[64];
		while(1)
		{
			read_bytes = read(fd1, buf, 1);
			if (read_bytes > 0)
			{ 
				fprintf(stdout, "%c\n", toupper(buf[0]));
			}
			usleep(500000);			
		}
		close(fd1);
		printf("fd1 closed\n");
		return 0;
	}

	char input[256];
	while(scanf("%s", input) > 0)
	{
		write(pipe1[1], input, strlen(input) + 1);
		kill(pid1, SIGUSR1);
	}

	waitpid(pid1, 0, 0);
	waitpid(pid2, 0, 0);
	printf("exit\n");
}
