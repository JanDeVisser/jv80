#include <iostream>
#include "../include/backplane.h"

int main(int argc, char **argv) {
  auto *system = new BackPlane();
  system -> defaultSetup();
  system -> run();
  return 0;
}
