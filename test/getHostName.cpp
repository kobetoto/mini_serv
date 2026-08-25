#include <unistd.h>
#include <iostream>
#include <stdio.h>

int gethostname(char *name, size_t size);


int main(void){

  char n [255];
  std::cout << "HOSTNAME: \n";
  if (gethostname(n, 255) != 0){
    std::cout << "Err gethostname()\n";
    return (1);
  }
  printf("-%s-\n", n);
  return (0);
}
