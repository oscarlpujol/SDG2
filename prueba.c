
#include <stdio.h>
// #include <conio.h>

int main() {
	int c;
	printf("Waiting for a character to be pressed from the keybord to exit\n");
	c = getchar();
	printf("You pressed %d \n", c);
	return 0;
}