#include "cell_move_router/Mover/Mover.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include <unordered_map>
#include <chrono>

namespace cell_move_router {
namespace Mover {

void Mover::initalFreqMovedCell() {
  for (auto &Cell : InputPtr->getCellInsts()) {
    if (Cell.isMovable()) {
      //if (GridManager.getCellVoltageArea(&Cell).size())
        //for(auto &Coor : GridManager.getCellVoltageArea(&Cell))
          //std::cout << "X:" << std::get<0>(GridManager.coordinateInv(Coor)) << " Y:" << std::get<1>(GridManager.coordinateInv(Coor));
        //continue; // TODO: handle Cell in VoltageArea
      FreqMovedCell.emplace(&Cell, 0);
    }
  }
}
// bool Mover::add_and_route(const Input::Processed::CellInst *CellPtr, const int Row, const int Col, bool virtual_route) {
//   GridManager.addCell(CellPtr, Row, Col);
//   if (GridManager.isOverflow()) {
//     GridManager.removeCell(CellPtr);
//     return false;
//   }
//   Router::GraphApproxRouter GraphApproxRouter(&GridManager);
//   std::vector<std::pair<const Input::Processed::Net *, std::pair<std::vector<cell_move_router::Input::Processed::Route>, long long> > > OriginRoutes;
//   bool Accept = true;
//   // long long original_cost_sum = 0;
//   // long long final_cost_sum = 0;
//   for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
//     auto &OriginRoute = GridManager.getNetRoutes()[NetPtr];
//     // original_cost_sum += OriginRoute.second;
//     auto Pair = GraphApproxRouter.singleNetRoute(NetPtr, OriginRoute.first);
//     OriginRoutes.emplace_back(NetPtr, std::move(OriginRoute));
//     if (Pair.second == false) {
//       Accept = false;
//       break;
//     }
//     auto Cost = GridManager.getRouteCost(NetPtr, Pair.first);
//     // final_cost_sum += Cost;

//     Input::Processed::Route::reduceRouteSegments(Pair.first);
//     OriginRoute = {std::move(Pair.first), Cost};
//     bool Overflow = GridManager.isOverflow();
//     GridManager.addNet(NetPtr);
//     assert(!GridManager.isOverflow());
//   }
//   // if(original_cost_sum <= final_cost_sum){
//   //  std::cout << "rejected!" << '\n';
//   //  Accept = false;
//   // }
//   if (Accept) {
//     return true;
//   }

//   GridManager.getNetRoutes()[OriginRoutes.back().first] = std::move(OriginRoutes.back().second);
//   OriginRoutes.pop_back();
//   while (OriginRoutes.size()) {
//     GridManager.removeNet(OriginRoutes.back().first);
//     GridManager.getNetRoutes()[OriginRoutes.back().first] = std::move(OriginRoutes.back().second);
//     OriginRoutes.pop_back();
//   }
//   GridManager.removeCell(CellPtr);
//   return false;
// }

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
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second * NetPtr->getWeight();
      // auto &OriginRoute = GridManager.getNetRoutes()[NetPtr].first;
      // NetLength += GridManager.getRouteCost(NetPtr, OriginRoute ) * NetPtr->getNumPins();
      // 
    }
    CellNetLength.emplace_back(NetLength, CellPtr);
  }
  std::sort(CellNetLength.begin(), CellNetLength.end(), std::greater<std::pair<long long, const Input::Processed::CellInst *>>());
  unsigned MoveCnt = 0;
  for (auto &P : CellNetLength) {
    auto CellPtr = P.second;
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) = RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    
    if (GridManager.getCellVoltageArea(CellPtr).size()){ //if this cell has voltage constraint
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
    else{
      for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
        for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
          CandidatePos.emplace_back(R, C);
        }
      }
    }
    //flute
    std::map<std::pair<int, int>, long long> Grid_to_WL;
    for (auto P : CandidatePos) {
      //std::cout << "Candidate Pos:" << P.first << ' ' << P.second << '\n';
      long long total_WL = 0;
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)){
        std::vector<int> X;
        std::vector<int> Y;
        for(auto &Pin : NetPtr->getPins()) { //pin vector
          auto c = Pin.getInst();
          if(c == CellPtr) continue;
          int x = c->getGGridRowIdx();
          int y = c->getGGridColIdx();
          X.push_back(x);
          Y.push_back(y);
        }
        
        // std::cout << CellPtr->getGGridRowIdx() << '\n';
        // std::cout << CellPtr->getGGridColIdx() << '\n';

        // for(auto x : X) std::cout << x << ' ';
        // std::cout << '\n';
        // for(auto y : Y) std::cout << y << ' ';
        // std::cout << '\n';
        
        X.push_back(P.first);
        Y.push_back(P.second);

        auto Wrapper = Flute::FluteWrapper::getInstance();
        auto FluteTree = Wrapper->runFlute(X, Y);
        //std::cout << FluteTree.getLength() << '\n';
        total_WL += FluteTree.getLength() * NetPtr->getWeight() * pow(NetPtr->getNumPins(),2);
      }
      Grid_to_WL[P] = total_WL;
    }
    std::sort(CandidatePos.begin(), CandidatePos.end(), [&Grid_to_WL](std::pair<int, int> a, std::pair<int, int> b) {return Grid_to_WL[a] < Grid_to_WL[b]; }); //revised
    //std::shuffle(CandidatePos.begin(), CandidatePos.end(), Random); //original version
    
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    {
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.removeNet(NetPtr);
      }
      GridManager.removeCell(CellPtr);
    }
    bool Success = false;

    //Second Attempt of Sorting Candidate Position
    // for (auto P : CandidatePos) {
    //   auto tmp_result = add_and_route(CellPtr, P.first, P.second, true);
    //   std::cout << "flute: " << Grid_to_WL[P] << " add_cost: " << 100 * (tmp_result.first / INT_MAX) << '\n';
    //   Grid_to_WL[P] += 100 * (tmp_result.first / INT_MAX);
    // }
    //std::sort(CandidatePos.begin(), CandidatePos.end(), [&Grid_to_WL](std::pair<int, int> a, std::pair<int, int> b) {return Grid_to_WL[a] < Grid_to_WL[b]; }); //revised
    //for(auto P : CandidatePos) std::cout << P.first << ' ' << P.second << ' ' << Grid_to_WL[P] << '\n';
    
    for (auto P : CandidatePos) {
      if (add_and_route(CellPtr, P.first, P.second, false).second) {
        Success = true;
        break;
      }
    }
    if (Success)
      ++MoveCnt;
    else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    if (MoveCnt == InputPtr->getMaxCellMove()){
      std::cout << "MAX CELL MOVE\n";
      break;
    }
    ////Timer
    auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime);
    //std::cout << "TIME: " << duration.count() << '\n';
    if(duration.count() > 3000.0) break;
  }
}

} // namespace Mover
} // namespace cell_move_router