#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include "cell_move_router/CoordinateCodec.hpp"

namespace cell_move_router {
namespace Router {

void RoutingGraphManager::createGraph(const std::vector<long long> &LayerFactor,
                                      const std::vector<char> &LayerDir) {
  G.clear();
  auto &Codec = BondaryInfo->getCodec();
  G.setVertexNum(Codec.max());

  int MinR = BondaryInfo->getMinR();
  int MinC = BondaryInfo->getMinC();
  int MinL = BondaryInfo->getMinL();

  int MaxR = BondaryInfo->getMaxR();
  int MaxC = BondaryInfo->getMaxC();
  int MaxL = BondaryInfo->getMaxL();

  // wire
  for (int L = MinL; L <= MaxL; ++L) {
    if (LayerDir.at(L) == 'H') {
      for (int R = MinR; R <= MaxR; ++R) {
        for (int C = MinC; C < MaxC; ++C) {
          if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
              GridManager->getGrid(R, C + 1, L).getSupply() <= 0)
            continue;
          auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                     (unsigned long long)(C - MinC),
                                     (unsigned long long)(L - MinL)});
          auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR),
                                        (unsigned long long)(C - MinC + 1),
                                        (unsigned long long)(L - MinL)});
                                        
        
          long long cap_v1 = GridManager->getGrid(R, C, L).getSupply();
          long long cap_v2 = GridManager->getGrid(R, C + 1, L).getSupply();
          //long long congestion_v = 1 / 1 + exp(cap_v);
          long long op_v = 0;
          if (cap_v1 <= 0 || cap_v2 <= 0) op_v = INT_MAX;
          long long Weight = LayerFactor.at(L)*2 + op_v + (cap_v1 + cap_v2)/2;
          //std::cout << "HOR" << Weight << '\n';
          //long long Weight = LayerFactor.at(L) * 2;
          G.addEdge(Coord, NeiCoord, Weight);
        }
      }
    }
    if (LayerDir.at(L) == 'V') {
      for (int R = MinR; R < MaxR; ++R) {
        for (int C = MinC; C <= MaxC; ++C) {
          if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
              GridManager->getGrid(R + 1, C, L).getSupply() <= 0)
            continue;
          auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                     (unsigned long long)(C - MinC),
                                     (unsigned long long)(L - MinL)});
          auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR + 1),
                                        (unsigned long long)(C - MinC),
                                        (unsigned long long)(L - MinL)});
          
          long long cap_v1 = GridManager->getGrid(R, C, L).getSupply();
          long long cap_v2 = GridManager->getGrid(R + 1, C, L).getSupply();
          //long long congestion_v = 1 / 1 + exp(cap_v);
          long long op_v = 0;
          if (cap_v1 <= 0 || cap_v2 <= 0) op_v = INT_MAX;
          long long Weight = LayerFactor.at(L)*2 + op_v + (cap_v1 + cap_v2)/2;
          //std::cout << "VER" << Weight << '\n';
          //long long Weight = LayerFactor.at(L) * 2;
          G.addEdge(Coord, NeiCoord, Weight);
        }
      }
    }
  }
  // via
  for (int L = MinL; L < MaxL; ++L) {
    for (int R = MinR; R <= MaxR; ++R) {
      for (int C = MinC; C <= MaxC; ++C) {
        if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
            GridManager->getGrid(R, C, L + 1).getSupply() <= 0)
          continue;
        auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                   (unsigned long long)(C - MinC),
                                   (unsigned long long)(L - MinL)});
        auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR),
                                      (unsigned long long)(C - MinC),
                                      (unsigned long long)(L - MinL + 1)});
        
        // int cap_v1 = GridManager->getGrid(R, C, L).getSupply();
        // int cap_v2 = GridManager->getGrid(R, C, L + 1).getSupply();
        // long long op_v = 0;
        // if (cap_v1 <= 0 || cap_v2 <= 0) op_v = INT_MAX;
        long long Weight = LayerFactor.at(L) + LayerFactor.at(L + 1);
        //long long Weight = LayerFactor.at(L) + LayerFactor.at(L + 1);
        //std::cout << "VIA" << Weight << '\n';
        G.addEdge(Coord, NeiCoord, Weight);
      }
    }
  }
}
std::vector<Input::Processed::Route>
RoutingGraphManager::createFinalRoute(const std::vector<size_t> &Eids,
                                      const Input::Processed::Net *Net) {
  auto Route = std::move(BondaryInfo->getRouteUnderMinLayer());
  for (auto &EdgeIdx : Eids) {
    auto &Edge = G.getEdge(EdgeIdx);
    auto Decode1 = BondaryInfo->getCodec().decode(Edge.v1);
    auto Decode2 = BondaryInfo->getCodec().decode(Edge.v2);
    int MinR = BondaryInfo->getMinR();
    int MinC = BondaryInfo->getMinC();
    int MinL = BondaryInfo->getMinL();
    unsigned long long R1 = Decode1[0] + MinR, R2 = Decode2[0] + MinR;
    unsigned long long C1 = Decode1[1] + MinC, C2 = Decode2[1] + MinC;
    unsigned long long L1 = Decode1[2] + MinL, L2 = Decode2[2] + MinL;
    Route.emplace_back(R1, C1, L1, R2, C2, L2, Net);
  }
  return Route;
}

std::pair<std::vector<Input::Processed::Route>, bool>
GraphApproxRouter::singleNetRoute(const Input::Processed::Net *Net, const std::vector<Input::Processed::Route> &OriginRoute) {
  BondaryBuilder Builder(getGridManager(), Net, OriginRoute);
  Builder.createBondaryInfo();
  if (!Builder.isBondaryInfoExist())
    return {{}, false};
  auto BondaryInfo = Builder.getBondaryInfo();
  RGM.setBondaryInfo(&BondaryInfo);
  RGM.createGraph(getLayerFactor(), getLayerDir());
  const auto &G = RGM.getGraph();
  
  //for(size_t i = 0; i < G.getEdgeNum(); i++) std::cout << G.getEdge(i).v1 << ' ' << G.getEdge(i).v2 << ' ' << G.getEdge(i).cost << std::endl;
  //v1 <= Coord = Codec.encode  v2 <= NeiCoord = Codec.encode
  //first decode => R,C,L

  steiner_tree::Solver<long long> Solver(G);
  //for(auto t : BondaryInfo.getTerminals()) std::cout << t << ' ';
  //std::cout << '\n';

  auto Res = Solver.solve(BondaryInfo.getTerminals());
  if (!Res)
    return {{}, false};

  auto FinalRoute = RGM.createFinalRoute(*Res, Net);
  return {FinalRoute, true};
}

void GraphApproxRouter::rerouteAll() {
  std::vector<std::pair<const Input::Processed::Net *, long long>> NetPtrs;
  //std::cout << "dcasds" << "\n";
  for (const auto &NetRoute : getGridManager()->getNetRoutes()) {
    //std::cout << NetRoute << "\n";
    //NTHU ROUTE COST
    //long long NTHU_COST = INT_MAX / NetRoute.first->getNumPins() + NetRoute.second.second;
    //NetPtrs.emplace_back(NetRoute.first, NTHU_COST);
    NetPtrs.emplace_back(NetRoute.first, NetRoute.second.second * pow(NetRoute.first->getNumPins(),1));
  }
  
  // std::vector<std::pair<const Input::Processed::Net *, int>> NetPtrs;
  // for (auto &net : getGridManager()->getInputPtr()->getNets()){
  //   std::pair<int, int> lower_left = {INT_MAX, INT_MAX};
  //   std::pair<int, int> upper_right = {0, 0};
  //   for(auto &Pin : net.getPins()){
  //     int x = Pin.getInst()->getGGridRowIdx();
  //     int y = Pin.getInst()->getGGridColIdx();
  //     if(x <= lower_left.first && y <= lower_left.second){
  //       lower_left.first = x;
  //       lower_left.second = y;
  //     }
  //     if(x >= lower_left.first && y >= lower_left.second){
  //       upper_right.first = x;
  //       upper_right.second = y;
  //     }
  //   }
  //   std::cout << &net << ":\n";
  //   std::cout << lower_left.first << ' ' << lower_left.second << '\n';
  //   std::cout << upper_right.first << ' ' << upper_right.second << '\n';
  //   std::cout << '\n';
  //   int bounding_box_surrounded = 0;
  //   for (auto &Cell : getGridManager()->getInputPtr()->getCellInsts()) {
  //     int x = Cell.getGGridRowIdx();
  //     int y = Cell.getGGridColIdx();
  //     if(x > lower_left.first && y > lower_left.second && x < upper_right.first && y < upper_right.second){
  //       bounding_box_surrounded += 1;
  //     }
  //   }
  //   NetPtrs.emplace_back(&net, bounding_box_surrounded);
  // }
  // auto NetCmp = [&](const std::pair<const Input::Processed::Net *, int> &A, 
  //                   const std::pair<const Input::Processed::Net *, int> &B) {
  //   return A.second < B.second;
  // };
  auto NetCmp =
      [&](const std::pair<const Input::Processed::Net *, long long> &A,
          const std::pair<const Input::Processed::Net *, long long> &B) {
        //return A.second < B.second;
        if(A.first->getWeight() == B.first->getWeight()) return A.second < B.second;
        else return (A.first->getWeight() > B.first->getWeight());
      };
  std::sort(NetPtrs.begin(), NetPtrs.end(), NetCmp);
  // for (auto &P : NetPtrs){
  //   auto NetPtr = P.first;
  //   getGridManager()->removeNet(NetPtr);
  // }
  for (auto &P : NetPtrs) {
    auto NetPtr = P.first;
    auto &OriginRoute = getGridManager()->getNetRoutes()[NetPtr];
    
    getGridManager()->removeNet(NetPtr);
    // auto tmp = singleNetRoute(NetPtr, OriginRoute.first);
    // std::cout << tmp.second << '\n';
    // auto Routes = tmp.first;
    auto Routes = singleNetRoute(NetPtr, OriginRoute.first).first;

    Input::Processed::Route::reduceRouteSegments(Routes);
    auto Cost = getGridManager()->getRouteCost(NetPtr, Routes);
    if (Cost < OriginRoute.second) {
      OriginRoute = {std::move(Routes), Cost};
    }
    getGridManager()->addNet(NetPtr);
  }
}

} // namespace Router
} // namespace cell_move_router