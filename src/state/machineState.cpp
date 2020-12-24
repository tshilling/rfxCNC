#include "machineState.h"
namespace CNC_ENGINE{
    machine_state_class machine_state;
    machine_state_class planner_state;
    void init_machine_state(){
        machine_state.init();
        planner_state.init();
    }
}