#include <iostream>
#include "fsm_split.h"

enum class STATUS { S_READY, S_RUNNING, S_WAITTING, T_RESET, T_PLAY, T_PAUSE };

int main(int argc, char** argv) {
  std::cout << "FSM: hello world" << std::endl;
  fsm::Stack<STATUS> fsm(STATUS::S_READY);

  fsm.on(STATUS::S_READY, STATUS::T_RESET) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_READY);
    LOG(ERROR) << "S_READY receive T_RESET [READY => READY]";
  };
  fsm.on(STATUS::S_READY, STATUS::T_PLAY) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_RUNNING);
    LOG(ERROR) << "S_READY receive T_PLAY [READY => RUNNING]";
  };
  fsm.on(STATUS::S_READY, STATUS::T_PAUSE) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_WAITTING);
    LOG(ERROR) << "S_READY receive T_PAUSE [READY => WAITTING]";
  };

  fsm.on(STATUS::S_WAITTING, STATUS::T_PAUSE) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_WAITTING);
    LOG(ERROR) << "S_WAITTING receive T_PAUSE [WAITTING => WAITTING]";
  };
  fsm.on(STATUS::S_WAITTING, STATUS::T_PLAY) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_RUNNING);
    LOG(ERROR) << "S_WAITTING receive T_PLAY [WAITTING => RUNNING]";
  };
  fsm.on(STATUS::S_WAITTING, STATUS::T_RESET) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_READY);
    LOG(ERROR) << "S_WAITTING receive T_RESET [WAITTING => READY]";
  };

  fsm.on(STATUS::S_RUNNING, STATUS::T_PLAY) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_RUNNING);
    LOG(ERROR) << "S_RUNNING receive T_PLAY [RUNNING => RUNNING]";
  };
  fsm.on(STATUS::S_RUNNING, STATUS::T_PAUSE) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_WAITTING);
    LOG(ERROR) << "S_RUNNING receive T_PAUSE [RUNNING => WAITTING]";
  };
  fsm.on(STATUS::S_RUNNING, STATUS::T_RESET) = [&](fsm::TArgs args) {
    fsm.set(STATUS::S_READY);
    LOG(ERROR) << "S_RUNNING receive T_RESET [RUNNING => READY]";
  };

  LOG(ERROR) << "Set on";
  fsm.set(STATUS::S_READY);

  while (true) {
    LOG(ERROR) << "While";
    char cmd = ' ';
    std::cin >> cmd;
    switch (cmd) {
      case 'r':
        LOG(ERROR) << "get r";
        fsm.command(STATUS::T_RESET, nullptr);
        break;
      case 'p':
        LOG(ERROR) << "get p";
        fsm.command(STATUS::T_PLAY, nullptr);
        break;
      case 'P':
        LOG(ERROR) << "get P";
        fsm.command(STATUS::T_PAUSE, nullptr);
        break;
      default:
        LOG(ERROR) << "default get " << cmd;
        break;
    }
  }

  return 0;
}
