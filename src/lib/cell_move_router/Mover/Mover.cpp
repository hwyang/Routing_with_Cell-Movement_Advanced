#include "cell_move_router/Mover/Mover.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include <unordered_map>
#include <chrono>

namespace cell_move_router {
namespace Mover {

void Mover::initalFreqMovedCell() {
  for (auto &Cell : InputPtr->getCellInsts()) {
    if (Cell.isMovable()) {
      FreqMovedCell.emplace(&Cell, 0);
    }
  }
}

std::pair<long long, bool> Mover::add_and_route(const Input::Processed::CellInst *CellPtr, const int Row, const int Col, bool virtual_route) {
  GridManager.addCell(CellPtr, Row, Col);
  if (GridManager.isOverflow()) {
    GridManager.removeCell(CellPtr);
    return {INT_MAX, false};
  }
  Router::GraphApproxRouter GraphApproxRouter(&GridManager);
  std::vector<std::pair<const Input::Processed::Net *, std::pair<std::vector<cell_move_router::Input::Processed::Route>, long long> > > OriginRoutes;
  bool Accept = true;
  long long final_cost_sum = 0;
  for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
    auto &OriginRoute = GridManager.getNetRoutes()[NetPtr];
    auto Pair = GraphApproxRouter.singleNetRoute(NetPtr, OriginRoute.first);
    OriginRoutes.emplace_back(NetPtr, std::move(OriginRoute));
    if (Pair.second == false) {
      Accept = false;
      break;
    }
    auto Cost = GridManager.getRouteCost(NetPtr, Pair.first);
    final_cost_sum += Cost;

    Input::Processed::Route::reduceRouteSegments(Pair.first);
    OriginRoute = {std::move(Pair.first), Cost};
    //bool Overflow = GridManager.isOverflow();
    GridManager.addNet(NetPtr);
    assert(!GridManager.isOverflow());
  }
  if (virtual_route == false) {
    if(Accept) return {final_cost_sum, true};
  }
  GridManager.getNetRoutes()[OriginRoutes.back().first] = std::move(OriginRoutes.back().second);
  OriginRoutes.pop_back();
  while (OriginRoutes.size()) {
    GridManager.removeNet(OriginRoutes.back().first);
    GridManager.getNetRoutes()[OriginRoutes.back().first] = std::move(OriginRoutes.back().second);
    OriginRoutes.pop_back();
  }
  GridManager.removeCell(CellPtr);
  if(virtual_route == true && Accept == false) return {INT_MAX, Accept};
  return {final_cost_sum, Accept};
}

void Mover::move(RegionCalculator::RegionCalculator &RC, std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > > startTime) {
  std::vector<std::pair<long long, const Input::Processed::CellInst *>> CellNetLength;
  // 1. Sort Cells for the order of cell movement (higher net cost first)
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      // 1.1 Cost of Cell Order, defined as its connected NetWeight * total_nets_cost(LayorFactor*NetWeight*NetLength)
      NetLength += GridManager.getNetRoutes()[NetPtr].second * NetPtr->getWeight();
    }
    CellNetLength.emplace_back(NetLength, CellPtr);
  }
  // 1.2 Sort Cells by Weighted Net Length (Higher Net Cost implies Critical Cell to Move First)
  std::sort(CellNetLength.begin(), CellNetLength.end(), std::greater<std::pair<long long, const Input::Processed::CellInst *>>());
  unsigned MoveCnt = 0;

  // 2. Constructing Candidate Position (in sorted order)
  for (auto &P : CellNetLength) {
    auto CellPtr = P.second;
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    // 2.1 Get this cell's optimal region (median of each connected nets)
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) = RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    
    // 2.2 Consider the case if this cell has Voltage Constraint
    if (GridManager.getCellVoltageArea(CellPtr).size()){
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
          for(auto &Coor : GridManager.getCellVoltageArea(CellPtr)){
            int Vol_R = std::get<0>(GridManager.coordinateInv(Coor));
            int Vol_C = std::get<1>(GridManager.coordinateInv(Coor));
            if(R == Vol_R && C == Vol_C){
              CandidatePos.emplace_back(R, C);
              break;
            }
          } 
        }  
      }
    }

    // 2.3 Consider the case if Cell does not have Voltage Constraint
    else{
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
          CandidatePos.emplace_back(R, C);
        }
      }
    }

    // 2.4 use FLUTE to estimate WL for situation after cell moved to each candidate position
    std::map<std::pair<int, int>, long long> Grid_to_WL;
    for (auto P : CandidatePos) {
      long long total_WL = 0;
      // 2.4.1 for all connected net
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)){
        std::vector<int> X;
        std::vector<int> Y;
        // 2.4.2 for all connected pin, add to a vector
        for(auto &Pin : NetPtr->getPins()) {
          auto c = Pin.getInst();
          if(c == CellPtr) continue;
          int x = c->getGGridRowIdx();
          int y = c->getGGridColIdx();
          X.push_back(x);
          Y.push_back(y);
        }
        // 2.4.3 Simulating that the cell is moved to this candidate position
        X.push_back(P.first);
        Y.push_back(P.second);

        // 2.4.4 Call FLUTE
        auto Wrapper = Flute::FluteWrapper::getInstance();
        auto FluteTree = Wrapper->runFlute(X, Y);

        // 2.4.5 Definition of Candidate_Region_Cost: FLUTE_Length * Net_Weight * Net_NumPins^2 (Net with More Pin is more impactful)
        total_WL += FluteTree.getLength() * NetPtr->getWeight() * pow(NetPtr->getNumPins(),2);
      }
      // 2.4.6 Record this candidate position's resulting total WL
      Grid_to_WL[P] = total_WL;
    }

    // 2.5 Sorting each Candidate Position by Candidate_Region_Cost (Smaller Cost is better)
    std::sort(CandidatePos.begin(), CandidatePos.end(), [&Grid_to_WL](std::pair<int, int> a, std::pair<int, int> b) {return Grid_to_WL[a] < Grid_to_WL[b]; });
    
    // 2.6 Remove Cell and its connected nets
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }

    // 2.7 Add back cell and reroute
    bool Success = false;
    for (auto P : CandidatePos) {
      if (add_and_route(CellPtr, P.first, P.second, false).second) {
        Success = true;
        break;
      }
    }

    // 2.8.1 Succes
    if (Success)
      ++MoveCnt;
    // 2.8.2 If failed add back as original 
    else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }

    // 2.9 Check the Max Cell Movement Constraint
    if (MoveCnt == InputPtr->getMaxCellMove()){
      std::cout << "MAX CELL MOVE\n";
      break;
    }

    // 2.10 Check Timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime);
    if(duration.count() > 3000.0) break;
  }
}

} // namespace Mover
} // namespace cell_move_router