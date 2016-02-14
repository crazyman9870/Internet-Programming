#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

int main() {
	int pid = fork();
	std::cout << "PID id is " << pid << std::endl;
	if(pid == 0) { //Child
		std::cout << "About to exec " << std::endl;
		execl("/bin/ls","/bin/ls",(char *)0);
	}
	else { //Parent
		int status;
		wait(&status);
		std::cout << "After wait" << std::endl;
	}
}