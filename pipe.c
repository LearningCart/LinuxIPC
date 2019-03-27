#include <stdio.h>

int main(int argc, char **argv)
{

	char buff[10];
	int files[2];

	pipe(files);

	write(files[1],"hi",2);


	memset(buff,0,10);
	read(files[0],buff,10);

	printf("data : %s\n",buff);

}
