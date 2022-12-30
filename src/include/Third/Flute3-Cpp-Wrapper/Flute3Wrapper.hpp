#pragma once
#include "flute3/flute.h"
#include <memory>
#include <ostream>
#include <vector>

namespace Flute {

class FluteWrapper {
  static std::unique_ptr<FluteWrapper> UniqueFluteWrapper;
  FluteWrapper();

public:
  class Tree {
    Flute::Tree WrapperTree;

  public:
    Tree(Flute::Tree &&WrapperTree) : WrapperTree(std::move(WrapperTree)) {}
    ~Tree() { Flute::free_tree(WrapperTree); }
    const Branch *getBranches() const { return WrapperTree.branch; }
    int getBranchesSize() const { return 2 * WrapperTree.deg - 2; }
    int getLength() const { return WrapperTree.length; }
    int getDegree() const { return WrapperTree.deg; }

    void print(std::ostream &Out) const;
    void plot(std::ostream &Out) const;
    void writeSVG(std::ostream &Out, double Scale = 1.0) const;
  };

  static FluteWrapper *getInstance();
  ~FluteWrapper();
  Tree runFlute(std::vector<int> &X, std::vector<int> &Y);
  int runFlute_wl(std::vector<int> &X, std::vector<int> &Y);
};

} // namespace Flute