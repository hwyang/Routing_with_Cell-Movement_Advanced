#pragma once
#include "cell_move_router/Grid/GridManager.hpp"
#include "Third/Flute3-Cpp-Wrapper/Flute3Wrapper.hpp"
#include "cell_move_router/Input/Processed/Input.hpp"
#include "cell_move_router/RegionCalculator/RegionCalculator.hpp"
#include <random>
#include<map>
#include <chrono>
#include <unordered_map>


namespace cell_move_router {
namespace Mover {

class Mover {
  Grid::GridManager &GridManager;
  const Input::Processed::Input *InputPtr;
  std::default_random_engine Random;
  std::unordered_map<const Input::Processed::CellInst *, int> FreqMovedCell;

  void initalFreqMovedCell();

public:
  Mover(Grid::GridManager &GridManager)
      : GridManager(GridManager), InputPtr(GridManager.getInputPtr()) {
    Random.seed(7122);
    initalFreqMovedCell();
  }
  // bool add_and_route(const Input::Processed::CellInst *CellPtr, const int Row, const int Col);
  std::pair<long long, bool> add_and_route(const Input::Processed::CellInst *CellPtr, const int Row, const int Col, bool virtual_route);
  
  void move(RegionCalculator::RegionCalculator &RC, std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > > startTime);
};

} // namespace Mover
} // namespace cell_move_router