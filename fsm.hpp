// Simple FSM/HFSM class
// - rlyeh [2011..2015], zlib/libpng licensed.

// [ref]  http://en.wikipedia.org/wiki/Finite-state_machine
// [todo] GOAP? behavior trees?
// [todo] counters
// [note] common actions are 'init', 'quit', 'push', 'back' (integers)
//        - init and quit are called everytime a state is created or destroyed.
//        - push and back are called everytime a state is paused or resumed. Ie,
//        when pushing and popping the stack tree.
// [note] on child states (tree of fsm's):
//        - actions are handled to the most inner active state in the decision
//        tree
//        - unhandled actions are delegated to the parent state handler until
//        handled or discarded by root state

#pragma once

#define FSM_VERSION                                                            \
  "1.0.0" /* (2015/11/29) Code revisited to use fourcc integers (much faster); \
clean ups suggested by Chang Qian                                              \
#define FSM_VERSION "0.0.0" // (2014/02/15) Initial version */

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fsm {
template <typename T>
inline std::string to_string(const T& t) {
  std::stringstream ss;
  return ss << t ? ss.str() : std::string();
}

template <>
inline std::string to_string(const std::string& t) {
  return t;
}

typedef void* args;
typedef std::function<void(const fsm::args& args)> call;

struct state {
  int name;
  fsm::args args;

  state(const int& name = 'null') : name(name) {}

  state operator()() const {
    state self = *this;
    self.args = {};
    return self;
  }

  template <typename T0>
  state operator()(const T0& t0) const {
    state self = *this;
    self.args = const_cast<T0>(t0);
    return self;
  }


  template <typename T0, typename T1>
  state operator()(const T0& t0, const T1& t1) const {
    state self = *this;
    self.args = {fsm::to_string(t0), fsm::to_string(t1)};
    return self;
  }

  operator int() const { return name; }

  bool operator<(const state& other) const { return name < other.name; }
  bool operator==(const state& other) const { return name == other.name; }

  /* TODO
  template <typename ostream>
  inline friend ostream& operator<<(ostream& out, const state& t)
  {
      if(t.name >= 256)
      {
          out << char((t.name >> 24) & 0xff);
          out << char((t.name >> 16) & 0xff);
          out << char((t.name >> 8) & 0xff);
          out << char((t.name >> 0) & 0xff);
      }
      else
      {
          out << t.name;
      }
      out << "(";
      std::string sep;
      for(auto& arg : t.args)
      {
          out << sep << arg;
          sep = ',';
      }
      out << ")";
      return out;
  }
  */
};

typedef state trigger;

struct transition {
  fsm::state previous, trigger, current;

  template <typename ostream>
  inline friend ostream& operator<<(ostream& out, const transition& t) {
    out << t.previous << " -> " << t.trigger << " -> " << t.current;
    return out;
  }
};

class stack {
 public:
  stack(const fsm::state& start = 'null') : deque(1) {
    deque[0] = start;
    call(deque.back(), 'init');
  }

  stack(int start) : stack(fsm::state(start)) {}

  ~stack() {
    // ensure state destructors are called (w/ 'quit')
    while (size()) {
      pop();
    }
  }

  // pause current state (w/ 'push') and create a new active child (w/ 'init')
  void push(const fsm::state& state) {
    if (deque.size() && deque.back() == state) {
      return;
    }
    // queue
    call(deque.back(), 'push');
    deque.push_back(state);
    call(deque.back(), 'init');
  }

  // terminate current state and return to parent (if any)
  void pop() {
    if (deque.size()) {
      call(deque.back(), 'quit');
      deque.pop_back();
    }
    if (deque.size()) {
      call(deque.back(), 'back');
    }
  }

  // set current active state
  void set(const fsm::state& state) {
    if (deque.size()) {
      replace(deque.back(), state);
    } else {
      push(state);
    }
  }

  // number of children (stack)
  size_t size() const { return deque.size(); }

  // info
  // [] classic behaviour: "hello"[5] = undefined, "hello"[-1] = undefined
  // [] extended behaviour: "hello"[5] = h, "hello"[-1] = o, "hello"[-2] = l
  fsm::state get_state(signed pos = -1) const {
    signed size = (signed)(deque.size());
    return size ? *(deque.begin() +
                    (pos >= 0 ? pos % size : size - 1 + ((pos + 1) % size)))
                : fsm::state();
  }
  fsm::transition get_log(signed pos = -1) const {
    signed size = (signed)(log.size());
    return size ? *(log.begin() +
                    (pos >= 0 ? pos % size : size - 1 + ((pos + 1) % size)))
                : fsm::transition();
  }
  std::string get_trigger() const {
    std::stringstream ss;
    return ss << current_trigger, ss.str();
  }

  bool is_state(const fsm::state& state) const {
    return deque.empty() ? false : (deque.back() == state);
  }

  // setup
  fsm::call& on(const fsm::state& from, const fsm::state& to) {
    return callbacks[bistate(from, to)];
  }

  // generic call
  bool call(const fsm::state& from, const fsm::state& to) const {
    std::map<bistate, fsm::call>::const_iterator found =
        callbacks.find(bistate(from, to));
    if (found != callbacks.end()) {
      log.push_back({from, current_trigger, to});
      if (log.size() > 50) {
        log.pop_front();
      }
      found->second(to.args);
      return true;
    }
    return false;
  }

  // user commands
  bool command(const fsm::state& trigger) {
    size_t size = this->size();
    if (!size) {
      return false;
    }
    current_trigger = fsm::state();
    std::deque<states::reverse_iterator> aborted;
    for (auto it = deque.rbegin(); it != deque.rend(); ++it) {
      fsm::state& self = *it;
      if (!call(self, trigger)) {
        aborted.push_back(it);
        continue;
      }
      for (auto it = aborted.begin(), end = aborted.end(); it != end; ++it) {
        call(**it, 'quit');
        deque.erase(--(it->base()));
      }
      current_trigger = trigger;
      return true;
    }
    return false;
  }

  template <typename T>
  bool command(const fsm::state& trigger, const T& arg1) {
    return command(trigger(arg1));
  }

  template <typename T, typename U>
  bool command(const fsm::state& trigger, const T& arg1, const U& arg2) {
    return command(trigger(arg1, arg2));
  }

  // debug
  template <typename ostream>
  ostream& debug(ostream& out) {
    int total = log.size();
    out << "status {" << std::endl;
    std::string sep = "\t";
    for (states::const_reverse_iterator it = deque.rbegin(), end = deque.rend();
         it != end; ++it) {
      out << sep << *it;
      sep = " -> ";
    }
    out << std::endl;
    out << "} log (" << total << " entries) {" << std::endl;
    for (int i = 0; i < total; ++i) {
      out << "\t" << log[i] << std::endl;
    }
    out << "}" << std::endl;
    return out;
  }

  // aliases
  bool operator()(const fsm::state& trigger) { return command(trigger); }
  template <typename T>
  bool operator()(const fsm::state& trigger, const T& arg1) {
    return command(trigger(arg1));
  }
  template <typename T, typename U>
  bool operator()(const fsm::state& trigger, const T& arg1, const U& arg2) {
    return command(trigger(arg1, arg2));
  }
  template <typename ostream>
  inline friend ostream& operator<<(ostream& out, const stack& t) {
    return t.debug(out), out;
  }

 protected:
  void replace(fsm::state& current, const fsm::state& next) {
    call(current, 'quit');
    current = next;
    call(current, 'init');
  }

  typedef std::pair<int, int> bistate;
  std::map<bistate, fsm::call> callbacks;

  mutable std::deque<fsm::transition> log;
  std::deque<fsm::state> deque;
  fsm::state current_trigger;

  typedef std::deque<fsm::state> states;
};
}  // namespace fsm
