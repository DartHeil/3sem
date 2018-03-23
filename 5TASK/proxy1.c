#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define BUFSIZE 10000
#define PCONST 100

struct pipe_buf {
	int fd[2];
	char *buf;
	int buf_size;
	int read_num;
};


void move(char *buf, int start, int end) {
	int i = 0;
	for(i; i < (end - start); i++)
		buf[i] = buf[i + start];
}

int max(int a, int b) {
	return (a > b)? a : b;
}

int power(int x, int y) {
	int res = 1;
	for(int i = 0; i < y; i++)
		res *= x;
	return res;
}

int main(int argc, char const *argv[]) {
	if(argc != 3) {
		perror("Invalid number of arguments!");
		return -1;
	}

	char *end_ptr = NULL;
	fd_set rd_set, wr_set;

	int num = strtoll(argv[1], &end_ptr, 10);
	int i = 0;
	int pid;

	struct pipe_buf pipes[2 * num];


	int pipefd1[2];
	int pipefd2[2];
	int fd_max = 0;


	for(i = 0; i < num; i++) {
		pipe(pipefd1);
		pipe(pipefd2);

		pid = fork();
		if(pid) {
			if(i == (num -1)) {
				pipes[i - 1].fd[1] = pipefd1[1]; 
				fcntl(pipes[i - 1].fd[1], F_SETFL, O_NONBLOCK);
				close(pipefd1[0]);
				close(pipefd2[0]);
				close(pipefd2[1]);
				int size = power(3, num - i) * PCONST;
			    pipes[i].buf = calloc(size, sizeof(char));
				fd_max = max(fd_max, pipefd1[1]);
				break;
			}
	        int size = power(3, num - i) * PCONST;
			pipes[i].buf = calloc(size, sizeof(char));
			pipes[i].buf_size = size;
			pipes[i].read_num = 0;

			pipes[i].fd[0] = pipefd2[0]; 
			if(i != 0) {
				pipes[i - 1].fd[1] = pipefd1[1]; 
				fcntl(pipes[i - 1].fd[1], F_SETFL, O_NONBLOCK);
			} else
				close(pipefd1[1]);

			int maximum = max(pipefd1[1], pipefd2[0]);
			if(fd_max < maximum)
				fd_max = maximum;

			close(pipefd1[0]);
			close(pipefd2[1]);
			fcntl(pipes[i].fd[0], F_SETFL, O_NONBLOCK);
		}
		else {
			close(pipefd1[1]);
			close(pipefd2[0]);
			pipes[i].buf = calloc(BUFSIZE + 1, sizeof(char));
			if(i == 0) {
				close(pipefd1[0]);
				pipes[i].fd[0] = open(argv[2], O_RDONLY);
			}
			else 
				pipes[i].fd[0] = pipefd1[0];
			if(i == (num - 1)) {
				close(pipefd2[1]);
				pipes[i].fd[1] = STDOUT_FILENO;
			}
			else
				pipes[i].fd[1] = pipefd2[1];

			int j = 0;
			for(; j < i; j++) {
				close(pipes[j].fd[0]);
				if(j != (i - 1))
					close(pipes[j].fd[1]);
			}
			break;
		}
	}

	if(pid) {
		int state = 1;
		while(state) {
			FD_ZERO(&rd_set);
			FD_ZERO(&wr_set);

			state = 0;
			for(i = 0; i < (num - 1); i++) {
				if(pipes[i].buf_size != pipes[i].read_num && (pipes[i].fd[0] != 0)) {
					FD_SET(pipes[i].fd[0], &rd_set);
					state++;
				}
				if((pipes[i].fd[1] != 0) && pipes[i].read_num) {
					state++;
					FD_SET(pipes[i].fd[1], &wr_set);
				}
			}
			if(state == 0)
				break;

			state = select(fd_max + 1, &rd_set, &wr_set, NULL, NULL);
			if(state <= 0)
				break;
			int ln = 0;
			for(i = 0; i < (num -1); i++) {
				if(FD_ISSET(pipes[i].fd[0], &rd_set)) {
					ln = read(pipes[i].fd[0], pipes[i].buf + pipes[i].read_num, 
							pipes[i].buf_size - pipes[i].read_num);

					if(ln > 0)
						pipes[i].read_num += ln;
					if(ln <= 0) {
						close(pipes[i].fd[0]);
						pipes[i].fd[0] = 0;
					}	

				}
			}

			for(i = 0; i < (num - 1); i++) {
				if(FD_ISSET(pipes[i].fd[1], &wr_set)) {
					ln = write(pipes[i].fd[1], pipes[i].buf,pipes[i].read_num);
					if(ln > 0) {
						move(pipes[i].buf, ln,pipes[i].read_num);
						pipes[i].read_num -= ln;
					}
					if((ln <= 0) && (pipes[i].read_num == 0) && (pipes[i].fd[0] == 0)) {
						close(pipes[i].fd[1]);
						pipes[i].fd[1] = 0;
					}
				}
			}

			for( i = 0 ; i < (num - 1); i++) {
            	if ( pipes[i].fd[1] && ( pipes[i].fd[0] == 0) && ( pipes[i].read_num == 0 ) ) {
               		close ( pipes[i].fd[1] );
            		pipes[i].fd[1] = 0;
           		}	
           	}
		}

	}
	else {
		int ln = 1;
		while(ln > 0) {
			ln = read(pipes[i].fd[0], pipes[i].buf, BUFSIZE);   
			write(pipes[i].fd[1], pipes[i].buf, ln);
		}

		close(pipes[i].fd[0]);
		close(pipes[i].fd[1]);
		for(int p = 0; p <= i; p++)
			free(pipes[p].buf);
		return 0;
	}


	for(i = 0; i <= (num -  1); i++)
			free(pipes[i].buf);
	return 0;
}
