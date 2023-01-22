#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include "cell_move_router/CoordinateCodec.hpp"

namespace cell_move_router {
namespace Router {

void RoutingGraphManager::createGraph(const std::vector<long long> &LayerFactor,
                                      const std::vector<char> &LayerDir) {
  G.clear();
  auto &Codec = BondaryInfo->getCodec();
  G.setVertexNum(Codec.max());

  // First, Extract the Boundary from BoundaryInfo
  int MinR = BondaryInfo->getMinR();
  int MinC = BondaryInfo->getMinC();
  int MinL = BondaryInfo->getMinL();
  int MaxR = BondaryInfo->getMaxR();
  int MaxC = BondaryInfo->getMaxC();
  int MaxL = BondaryInfo->getMaxL();

  // Second, For the Possible Routing Segment of Wires(Horizontal or Vertical)
  for (int L = MinL; L <= MaxL; ++L) {
    // Case of Horizontal
    if (LayerDir.at(L) == 'H') {
      for (int R = MinR; R <= MaxR; ++R) {
        for (int C = MinC; C < MaxC; ++C) {
          
          // 1. Check if the Grid is Overflow or not
          if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
              GridManager->getGrid(R, C + 1, L).getSupply() <= 0)
            continue;
          
          // 2. Get the Coordinate of Grid
          auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                     (unsigned long long)(C - MinC),
                                     (unsigned long long)(L - MinL)});
          
          // 3. Get the Coordinate of Neighbor Grid
          auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR),
                                        (unsigned long long)(C - MinC + 1),
                                        (unsigned long long)(L - MinL)});

          // 4. Define the Cost of This Routing Segment
          long long cap_v1 = GridManager->getGrid(R, C, L).getSupply();       // More Remaining Supply, More Cost (Avoid Cold Area)
          long long cap_v2 = GridManager->getGrid(R, C + 1, L).getSupply();   // More Remaining Supply, More Cost (Avoid Cold Area)
          long long op_v = 0;
          if (cap_v1 <= 0 || cap_v2 <= 0) op_v = INT_MAX;                     // If is Overflow Grid, Punish with Large Cost
          long long Weight = LayerFactor.at(L)*2 + op_v + (cap_v1 + cap_v2)/2;// Total Cost += Square of LayorFactor (Consider Layer Factor Critically)

          G.addEdge(Coord, NeiCoord, Weight);
        }
      }
    }
    // Third, Case of Vertical
    if (LayerDir.at(L) == 'V') {
      for (int R = MinR; R < MaxR; ++R) {
        for (int C = MinC; C <= MaxC; ++C) {

          // 1. Check if the Grid is Overflow or not
          if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
              GridManager->getGrid(R + 1, C, L).getSupply() <= 0)
            continue;

          // 2. Get the Coordinate of Grid
          auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                     (unsigned long long)(C - MinC),
                                     (unsigned long long)(L - MinL)});
          
          // 3. Get the Coordinate of Neighbor Grid
          auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR + 1),
                                        (unsigned long long)(C - MinC),
                                        (unsigned long long)(L - MinL)});
          
          // 4. Define the Cost of This Routing Segment
          long long cap_v1 = GridManager->getGrid(R, C, L).getSupply();       // More Remaining Supply, More Cost (Avoid Cold Area)
          long long cap_v2 = GridManager->getGrid(R + 1, C, L).getSupply();   // More Remaining Supply, More Cost (Avoid Cold Area)
          long long op_v = 0;
          if (cap_v1 <= 0 || cap_v2 <= 0) op_v = INT_MAX;                     // If is Overflow Grid, Punish with Large Cost
          long long Weight = LayerFactor.at(L)*2 + op_v + (cap_v1 + cap_v2)/2;// Total Cost += Square of LayorFactor (Consider Layer Factor Critically)

          G.addEdge(Coord, NeiCoord, Weight);
        }
      }
    }
  }
  // For the Possible Routing Segment of Via
  for (int L = MinL; L < MaxL; ++L) {
    for (int R = MinR; R <= MaxR; ++R) {
      for (int C = MinC; C <= MaxC; ++C) {

        // 1. Check if the Grid is Overflow or not
        if (GridManager->getGrid(R, C, L).getSupply() <= 0 ||
            GridManager->getGrid(R, C, L + 1).getSupply() <= 0)
          continue;

        // 2. Get the Coordinate of Grid
        auto Coord = Codec.encode({(unsigned long long)(R - MinR),
                                   (unsigned long long)(C - MinC),
                                   (unsigned long long)(L - MinL)});
        
        // 3. Get the Coordinate of Neighbor Grid
        auto NeiCoord = Codec.encode({(unsigned long long)(R - MinR),
                                      (unsigned long long)(C - MinC),
                                      (unsigned long long)(L - MinL + 1)});
        
        // 4. Define the Cost of This Via as Avg of LayorFactors
        long long Weight = LayerFactor.at(L) + LayerFactor.at(L + 1);

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

    // Decode Coordinate back to R, C, L
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

  // 1. Create the "Possible Rerouted Area Boundary" by Current Routed Net
  Builder.createBondaryInfo();
  if (!Builder.isBondaryInfoExist())
    return {{}, false};
  auto BondaryInfo = Builder.getBondaryInfo();
  RGM.setBondaryInfo(&BondaryInfo);

  // 2. Generate an Undirected Graph -> As Reference for Reroute Solver (where to route)
  RGM.createGraph(getLayerFactor(), getLayerDir());
  const auto &G = RGM.getGraph();

  // 3. Use Third Party Shortest Path Solver: Steiner-Tree 2 Approx Solver to reroute
  steiner_tree::Solver<long long> Solver(G);
  auto Res = Solver.solve(BondaryInfo.getTerminals());
  if (!Res)
    return {{}, false};

  // 4. Generate Final Route Based on solver
  auto FinalRoute = RGM.createFinalRoute(*Res, Net);
  return {FinalRoute, true};
}

void GraphApproxRouter::rerouteAll() {
  std::vector<std::pair<const Input::Processed::Net *, long long>> NetPtrs;

  // 1. Define the Cost for Net reroute order: net_length * num_pins
  for (const auto &NetRoute : getGridManager()->getNetRoutes()) {
    NetPtrs.emplace_back(NetRoute.first, NetRoute.second.second * pow(NetRoute.first->getNumPins(),1));
  }
  
  // 2. Define the comparison function: consider net weight first (Larger Weight First);
  // 2. Then, consider the net_length * num_pins (Smaller Cost First)
  auto NetCmp =
      [&](const std::pair<const Input::Processed::Net *, long long> &A,
          const std::pair<const Input::Processed::Net *, long long> &B) {
        if(A.first->getWeight() == B.first->getWeight()) return A.second < B.second;
        else return (A.first->getWeight() > B.first->getWeight());
      };
  
  // 3. Sort the nets by NetCmp serving as the compare function
  std::sort(NetPtrs.begin(), NetPtrs.end(), NetCmp);

  for (auto &P : NetPtrs) {
    auto NetPtr = P.first;
    auto &OriginRoute = getGridManager()->getNetRoutes()[NetPtr];
    
    // 4. Remove Net in sorted vector
    getGridManager()->removeNet(NetPtr);

    // 5. Route Single Net
    auto Routes = singleNetRoute(NetPtr, OriginRoute.first).first;

    // 6. Further Reduce Redundant Routing Segments
    Input::Processed::Route::reduceRouteSegments(Routes);
    auto Cost = getGridManager()->getRouteCost(NetPtr, Routes);
    if (Cost < OriginRoute.second) {
      OriginRoute = {std::move(Routes), Cost};
    }

    // 7. Add Net back
    getGridManager()->addNet(NetPtr);
  }
}

} // namespace Router
} // namespace cell_move_router