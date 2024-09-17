#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	printf("ECOLE 2600 MY INIT\n");
	while (2600)
	{
		pid_t pid = fork();
		int status = 0;
		if (pid)
		{
			waitpid(pid, &status, 0);
			printf("Respawn\n");
			pid = 0;
		}
		else
		{
			char *tab[] = {"/usr/bin/setsid", "cttyhack", "sh", NULL};
		execv("/usr/bin/setsid", tab);
		}
	}
}
