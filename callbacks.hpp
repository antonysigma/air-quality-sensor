#pragma once
#include <cib/cib.hpp>

// Main routine
using cib::RuntimeInit;
class MainLoop : public callback::service<uint32_t> {};
class OnTimer0Interrupt : public callback::service<> {};