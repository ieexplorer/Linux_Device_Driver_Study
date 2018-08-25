#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


int main()
{
	int fd, i;
	char msg_wr [101];
	char msg_rd [101];

	for(i=0;i<101;i++)
        msg_wr[i]='a';
	
	msg_wr [100]='\0';
	
	fd = open ("/dev/scull_device", O_RDWR, S_IRUSR);

	if(fd == -1)
		{
			printf ("open fail\n");
			return 0;
		}	
	write (fd, msg_wr, 100);
	printf ("write finish\n");

	read (fd, msg_rd, 100);
	printf ("%s\n", msg_rd);
	close(fd);
	printf ("read finish\n");

	return 0;
}