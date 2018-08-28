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
	ssize_t result;

	for(i=0;i<101;i++)
        msg_wr[i]='a';
	
	msg_wr [100]='\0';
	
	fd = open ("/dev/scull_device", O_RDWR, S_IRUSR);

	if(fd == -1)
		{
			printf ("open fail\n");
			return 0;
		}	

	result = write (fd, msg_wr, 100);
	
	if (result == -1)
	{
		printf("write operation failed\n");
		close(fd);
		return -1;
	}

	printf ("write finish, size written %ld\n", result);

/*	result = read (fd, msg_wr, 100);
	
	if (result == -1)
	{
		printf("read operation failed\n");
		close(fd);
		return -1;

	}
*/	printf ("read finish, size read %ld\n", result);

	close(fd);
	return 0;
}