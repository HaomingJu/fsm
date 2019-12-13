#pragma once
#ifndef FSM_SPLIT_H_2E9HIQ7L
#define FSM_SPLIT_H_2E9HIQ7L
#include <deque>       // using std::deque
#include <functional>  // using std::function
#include <map>         // using std::map
#include <sstream>     // using std::stringstream
#include <string>      // using std::string

namespace fsm {
// Normal funcs
template <typename T> std::string to_string(const T& t);
template <> std::string to_string(const std::string& t);

// Type define
typedef void* TArgs;
typedef std::function<void(const TArgs& args)> TCallFunc;
#define INIT_VALUE 0


// Class state
template <typename TS> class State {
 public:
  State();
  State(const TS&, const TArgs&);
  explicit State(const TS& name);

  State operator()() const;
  State operator()(const TArgs&) const;
  operator TS() const;
  State& operator=(const State&);

 public:
  TS m_s;
  TArgs m_args;
};

template <typename TS> class Stack {
 public:
  Stack();
  explicit Stack(const fsm::State<TS>& start);
  explicit Stack(TS start);
  bool command(const State<TS>& trigger);
  template <typename TARGS> bool command(const TS& trigger, const TARGS& arg1);
  fsm::TCallFunc& on(const TS& from, const TS& to);
  State<TS> get_trigger() const;
  fsm::State<TS> get_state(std::size_t pos = -1) const;
  void set(const TS&);


 private:
  typedef std::pair<State<TS>, State<TS>> bistate;
  typedef std::deque<fsm::State<TS>> States;
  std::map<bistate, TCallFunc> m_callbacks;
  States m_deque;
  fsm::State<TS> m_current_trigger;

 private:
  bool call(const State<TS>& from, const State<TS>& to) const;
  void replace(fsm::State<TS>& current, const fsm::State<TS>& next);
};


}  // namespace fsm

#include "fsm_split.cpp"
#endif /* end of include guard: FSM_SPLIT_H_2E9HIQ7L */
