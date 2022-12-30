#include "Flute3Wrapper.hpp"
#include <cassert>
#include <limits>

namespace Flute {

void FluteWrapper::Tree::print(std::ostream &Out) const {
  Flute::printtree(WrapperTree, Out);
}
void FluteWrapper::Tree::plot(std::ostream &Out) const {
  Flute::plottree(WrapperTree, Out);
}
void FluteWrapper::Tree::writeSVG(std::ostream &Out, double Scale) const {
  Flute::write_svg(WrapperTree, Out, Scale);
}

std::unique_ptr<FluteWrapper> FluteWrapper::UniqueFluteWrapper = nullptr;

FluteWrapper::FluteWrapper() { readLUT(); }

FluteWrapper::~FluteWrapper() { deleteLUT(); }

FluteWrapper *FluteWrapper::getInstance() {
  if (UniqueFluteWrapper == nullptr)
    UniqueFluteWrapper = std::unique_ptr<FluteWrapper>(new FluteWrapper());
  return UniqueFluteWrapper.get();
}

FluteWrapper::Tree FluteWrapper::runFlute(std::vector<int> &X,
                                          std::vector<int> &Y) {
  auto Size = X.size();
  assert(Size == Y.size() && Size > 0);
  auto FluteTree = flute(X.size(), X.data(), Y.data(), FLUTE_ACCURACY);
  return Tree(std::move(FluteTree));
}
int FluteWrapper::runFlute_wl(std::vector<int> &X, std::vector<int> &Y) {
  auto Size = X.size();
  assert(Size == Y.size() && Size > 0);
  return flute_wl(X.size(), X.data(), Y.data(), FLUTE_ACCURACY);
}

} // namespace Flute