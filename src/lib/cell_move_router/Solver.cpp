#include "cell_move_router/Solver.hpp"
#include "cell_move_router/Mover/Mover.hpp"
#include "cell_move_router/RegionCalculator/FinalRegion.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include <chrono>
#include <iostream>

namespace cell_move_router {
void Solver::solve() {
  cell_move_router::Router::GraphApproxRouter Router(&GridManager);

  auto startTime_init = std::chrono::high_resolution_clock::now();
  long long OriginalCost = GridManager.getCurrentCost();
  std::cerr << "Original:\n";
  std::cerr << "  CurrentCost: " << OriginalCost * 0.0001 << '\n';

  // 1. Reroute First, also make sure there's no Open Net
  Router.rerouteAll();

  long long FirstRerouteCost = GridManager.getCurrentCost();
  std::cerr << "After re-route:\n";
  std::cerr << "  CurrentCost: " << FirstRerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (OriginalCost - FirstRerouteCost) * 0.0001 << '\n';
  auto endTime = std::chrono::high_resolution_clock::now();
  std::cout << "reroute time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime_init).count() << '\n' ;
  auto moveStartTime = std::chrono::high_resolution_clock::now();

  Mover::Mover Mover(GridManager);
  // 2. Construct an Optimal Region For Cell Movement
  RegionCalculator::OptimalRegion OR(&GridManager);
  
  // 3. Move Cell Based on Optimal Region
  Mover.move(OR, startTime_init);

  long long MoveCost = GridManager.getCurrentCost();
  std::cerr << "After move:\n";
  std::cerr << "  CurrentCost: " << MoveCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (FirstRerouteCost - MoveCost) * 0.0001 << '\n';
  auto moveEndTime = std::chrono::high_resolution_clock::now();
  std::cout << "move time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(moveEndTime - moveStartTime).count() << '\n' ;
  auto secondRerouteStartTime = std::chrono::high_resolution_clock::now();

  // 4. Reroute Again to further reduce total wirelength
  Router.rerouteAll();

  long long RerouteCost = GridManager.getCurrentCost();
  std::cerr << "After re-route:\n";
  std::cerr << "  CurrentCost: " << RerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (MoveCost - RerouteCost) * 0.0001 << '\n';
  auto secondRerouteEndTime = std::chrono::high_resolution_clock::now();
  std::cout << "reroute time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(secondRerouteEndTime - secondRerouteStartTime).count() << '\n' ;

}
} // namespace cell_move_router