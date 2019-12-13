#pragma once
#ifndef FSM_SPLIT_CPP_TJEN5JIV
#define FSM_SPLIT_CPP_TJEN5JIV
#include "fsm_split.h"
#include "glog/logging.h"

namespace fsm {
template <typename T> std::string to_string(const T& t) {
  std::stringstream ss;
  return ss << t ? ss.str() : std::string();
}

template <> std::string to_string(const std::string& t) {
  return t;
}

// Class State
template <typename TS>
State<TS>::State(const TS& r_s) : m_s(r_s), m_args(nullptr) {
}

template <typename TS> State<TS>::State() : m_s(TS(0)), m_args(nullptr) {
}

template <typename TS>
State<TS>::State(const TS& r_s, const TArgs& args) : m_args(args), m_s(r_s) {
}

template <typename TS> State<TS> State<TS>::operator()() const {
  LOG(INFO) << "operator ()";
  State copy_self = *this;
  copy_self.m_args = {};
  return copy_self;
}

template <typename TS>
State<TS> State<TS>::operator()(const TArgs& args) const {
  LOG(INFO) << "operator ()(const TArgs&)";
  State copy_self = *this;
  copy_self.m_args = args;
  return copy_self;
}

template <typename TS> State<TS>& State<TS>::operator=(const State<TS>& s) {
  LOG(INFO) << "operator = ()(const State<TS>&)";
  this->m_s = s.m_s;
  this->m_args = s.m_args;
  return *this;
}

template <typename TS> State<TS>::operator TS() const {
  // LOG(INFO) << "operator int()"; TODO
  return this->m_s;
}

template <typename TS> Stack<TS>::Stack() {
}

template <typename TS> Stack<TS>::Stack(const fsm::State<TS>& start) {
  this->m_deque[0] = start;
}
template <typename TS>
Stack<TS>::Stack(TS start) : Stack<TS>(fsm::State<TS>(start)) {
}

template <typename TS>
bool Stack<TS>::call(const State<TS>& from, const State<TS>& to) const {
  // typename std::map<bistate, fsm::TCallFunc>::iterator found;
  // typename decltype(m_callbacks)::iterator found;
  auto found = this->m_callbacks.find(bistate(from, to));
  if (found != m_callbacks.end()) {
    found->second(to.m_args);
    return true;
  }
  return false;
}

template <typename TS>
void Stack<TS>::replace(fsm::State<TS>& current, const fsm::State<TS>& next) {
  // call(current, 'quit');
  current = next;
  // call(current, 'init');
}

template <typename TS>
fsm::TCallFunc& Stack<TS>::on(const TS& from, const TS& to) {
  return this->m_callbacks[bistate(State<TS>(from), State<TS>(to))];  // TODO
}

template <typename TS> State<TS> Stack<TS>::get_trigger() const {
  return this->m_current_trigger;
}

template <typename TS>
fsm::State<TS> Stack<TS>::get_state(std::size_t pos) const {
  std::size_t sz = this->m_deque.size();
  return sz ? *(m_deque.begin() +
                (pos >= 0 ? pos % sz : sz - 1 + ((pos + 1) % sz)))
            : fsm::State<TS>();
}

template <typename TS> bool Stack<TS>::command(const State<TS>& trigger) {
  std::size_t sz = this->m_deque.size();
  if (!sz) {
    LOG(ERROR) << "The m_deque sz is 0";
    return false;
  }
  this->m_current_trigger = fsm::State<TS>();
  typename std::deque<fsm::State<TS>>::reverse_iterator it;
  for (it = this->m_deque.rbegin(); it != m_deque.rend(); ++it) {
    if (!call(*it, trigger)) {
      continue;
    }
    this->m_current_trigger = trigger;
    return true;
  }
  return false;
}

template <typename TS>
template <typename TARGS>
bool Stack<TS>::command(const TS& trigger, const TARGS& arg1) {
  this->command(State<TS>(trigger, arg1));
  return false;
}

template <typename TS> void Stack<TS>::set(const TS& s) {
  if (m_deque.size()) {
    replace(m_deque.back(), State<TS>(s));
  } else {
    this->m_deque.push_back(State<TS>(s));
  }
}


}  // namespace fsm

#endif /* end of include guard: FSM_SPLIT_CPP_TJEN5JIV */
