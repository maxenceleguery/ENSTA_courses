#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

int digit_to_int(char d) {
	char str[2];

	str[0] = d;
	str[1] = '\0';
	return (int) strtol(str, NULL, 10);
}

int main() {
	char buffer[50];
	char result[50];
	read(0,buffer,50);
	if (fork() != 0) {
		read(0,buffer,50);
	} else {
		prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT); //Encapsulation of the process to block the attack
		switch (buffer[0])
		{
		case '+':
			sprintf(result,"%d\n",digit_to_int(buffer[1])+digit_to_int(buffer[3]));
			write(1,result,50);
			break;
		case '-':
			sprintf(result,"%d\n",digit_to_int(buffer[1])-digit_to_int(buffer[3]));
			write(1,result,50);
			break;
		case 'e':
			char cmd[50];
			for (int i = 1;i<50;i++) {
				cmd[i-1]=buffer[i];
			}
			system(cmd);
			break;
		
		default:
			sprintf(result,"%s\n","Command unknown");
			write(1,result,50);
			break;
		}
	}
	syscall(SYS_exit, 0);
}