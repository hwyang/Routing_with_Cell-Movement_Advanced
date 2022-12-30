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

  Router.rerouteAll();
  long long FirstRerouteCost = GridManager.getCurrentCost();
  std::cerr << "After re-route:\n";
  std::cerr << "  CurrentCost: " << FirstRerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (OriginalCost - FirstRerouteCost) * 0.0001 << '\n';
  auto endTime = std::chrono::high_resolution_clock::now();
  std::cout << "reroute time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime_init).count() << '\n' ;


  auto moveStartTime = std::chrono::high_resolution_clock::now();
  Mover::Mover Mover(GridManager);
  // TODO: fix FinalRegion infinite loop bug
  //RegionCalculator::FinalRegion FR(&GridManager);
  //Mover.move(FR, 0);
  RegionCalculator::OptimalRegion OR(&GridManager);
  /*
  Mover.move(OR, 0);
  long long MoveCost = GridManager.getCurrentCost();
  std::cerr << "After move:\n";
  std::cerr << "  CurrentCost: " << MoveCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (FirstRerouteCost - MoveCost) * 0.0001
            << '\n';
  */
  
  Mover.move(OR, startTime_init);
  long long MoveCost = GridManager.getCurrentCost();
  std::cerr << "After move:\n";
  std::cerr << "  CurrentCost: " << MoveCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (FirstRerouteCost - MoveCost) * 0.0001 << '\n';
  // std::cout << "Move time: "<< Timer2->getDuration<>().count()/ 1e9 << '\n' ;
  auto moveEndTime = std::chrono::high_resolution_clock::now();
  std::cout << "move time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(moveEndTime - moveStartTime).count() << '\n' ;

  // first_time = false;
  // do{
  //   Router.rerouteAll();
  //   RerouteCost = GridManager.getCurrentCost();
  //   std::cerr << "After re-route:\n";
  //   std::cerr << "  CurrentCost: " << RerouteCost * 0.0001 << '\n';
  //   ReducedCost = OriginalCost - RerouteCost;
  //   if (!first_time){
  //     FirstReduced = ReducedCost;
  //     first_time = true;
  //   }
  //   std::cerr << "  ReducedCost: " << ReducedCost * 0.0001 << '\n';
  //   OriginalCost = RerouteCost;
  // }while(ReducedCost > FirstReduced / 3);
  // GlobalTimer::initialTimerAndSetTimeLimit(std::chrono::seconds(55 * 60));
  // auto Timer3 = GlobalTimer::getInstance();
  auto secondRerouteStartTime = std::chrono::high_resolution_clock::now();
  Router.rerouteAll();
  long long RerouteCost = GridManager.getCurrentCost();
  std::cerr << "After re-route:\n";
  std::cerr << "  CurrentCost: " << RerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (MoveCost - RerouteCost) * 0.0001 << '\n';
  auto secondRerouteEndTime = std::chrono::high_resolution_clock::now();
  std::cout << "reroute time: "<< std::chrono::duration_cast<std::chrono::duration<float>>(secondRerouteEndTime - secondRerouteStartTime).count() << '\n' ;
  // std::cout << "last reroute time: "<< Timer3->getDuration<>().count() / 1e9<< '\n' ;

}
} // namespace cell_move_router