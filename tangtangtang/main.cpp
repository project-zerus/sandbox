#include <stdio.h>

int main(int argc, char** argv) {
  wchar_t stack[2];
  wchar_t* heap = new wchar_t[2];
  printf("heap %c %d\n", heap[0], heap[0]);
  printf("heap %c %d\n", heap[1], heap[1]);
  printf("heap %c %d\n", heap[2], heap[2]);
  printf("heap %c %d\n", heap[3], heap[3]);
  printf("stack %c %d\n", stack[0], stack[0]);
  printf("stack %c %d\n", stack[1], stack[1]);
  printf("stack %c %d\n", stack[2], stack[2]);
  printf("stack %c %d\n", stack[3], stack[3]);
	return 0;
}
