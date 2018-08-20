#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


int main()
{
	int fd, i;
	char msg [101];
	for(i=0;i<101;i++)
        msg[i]='\0';
	
	fd = open ("/dev/scull_device",  O_RDONLY, S_IRUSR);

	if(fd == -1)
		{
			printf ("open fail\n");
			return 0;
		}	

	read (fd, msg, 100);
	printf ("%s\n", msg);
	close(fd);
	printf ("read finish\n");

	return 0;
}