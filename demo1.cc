#include <iostream>
#include "fsm.hpp"

enum {
  opening,
  closing,
  waiting,
  playing,

  open,
  close,
  play,
  stop,
  insert,
  eject,
};

struct cd_player {
  bool has_cd;
  bool good_disk_format() { return true; }

  // implementation actions
  void open_tray() { std::cout << "opening tray" << std::endl; }
  void close_tray() { std::cout << "closing tray" << std::endl; }
  void get_cd_info() { std::cout << "retrieving CD info" << std::endl; }
  void start_playback(const std::string& track) {
    std::cout << "playing track #" << track << std::endl;
  }

  // the core
  fsm::stack fsm;

  cd_player() : has_cd(false) {
    fsm.on(opening, close) = [&](const fsm::args& args) {
      close_tray();
      if (!has_cd) {
        fsm.set(closing);
      } else {
        get_cd_info();
        fsm.set(waiting);
      }
    };
    fsm.on(opening, insert) = [&](const fsm::args& args) {
      has_cd = true;
      fsm.set(opening);
    };
    fsm.on(opening, eject) = [&](const fsm::args& args) {
      has_cd = false;
      fsm.set(opening);
    };

    fsm.on(closing, open) = [&](const fsm::args& args) {
      open_tray();
      fsm.set(opening);
    };

    fsm.on(waiting, play) = [&](const fsm::args& args) {
      if (!good_disk_format()) {
        fsm.set(waiting);
      } else {
        char* re_args = reinterpret_cast<char*>(args);
        printf("%s", re_args);
        start_playback(std::string(re_args));
        fsm.set(playing);
      }
    };
    fsm.on(waiting, open) = [&](const fsm::args& args) {
      open_tray();
      fsm.set(opening);
    };

    fsm.on(playing, open) = [&](const fsm::args& args) {
      open_tray();
      fsm.set(opening);
    };
    fsm.on(playing, stop) = [&](const fsm::args& args) { fsm.set(waiting); };

    // set initial fsm state
    fsm.set(opening);
  }
};

// usage

int main() {
  cd_player cd;

  for (;;) {
    std::cout << "[" << cd.fsm.get_state() << "] ";
    std::cout
        << "(o)pen lid/(c)lose lid, (i)nsert cd/(e)ject cd, (p)lay/(s)top cd? ";

    char cmd;
    std::cin >> cmd;
    char cd_name[16] = "The Wind";

    switch (cmd) {
      case 'p':
        cd.fsm.command(play, &cd_name[0]);
        break;
      case 'o':
        cd.fsm.command(open);
        break;
      case 'c':
        cd.fsm.command(close);
        break;
      case 's':
        cd.fsm.command(stop);
        break;
      case 'i':
        cd.fsm.command(insert);
        break;
      case 'e':
        cd.fsm.command(eject);
        break;
      default:
        std::cout << "what?" << std::endl;
    }
  }
}
