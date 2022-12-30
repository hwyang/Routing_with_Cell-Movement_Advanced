#include "Flute3Wrapper.hpp"
#include <fstream>
#include <iostream>

int main() {
  auto Wrapper = Flute::FluteWrapper::getInstance();
  std::vector<int> X = {20, 80, 60, 50, 30, 40};
  std::vector<int> Y = {30, 20, 60, 40, 50, 20};

  auto FluteTree = Wrapper->runFlute(X, Y);

  std::cout << "FLUTE wirelength only = " << Wrapper->runFlute_wl(X, Y) << '\n';
  std::cout << "FLUTE wirelength = " << FluteTree.getLength() << '\n';
  std::cout << "FLUTE degree = " << FluteTree.getDegree() << '\n';
  std::cout << "FLUTE printtree\n";
  FluteTree.print(std::cout);
  std::cout << "FLUTE plottree\n";
  FluteTree.plot(std::cout);
  auto SVG_Name = "test.svg";
  std::cout << "FLUTE writeSVG: " << SVG_Name << '\n';
  std::ofstream Fout(SVG_Name);
  FluteTree.writeSVG(Fout, 5);

  return 0;
}
