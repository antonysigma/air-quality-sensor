#pragma once
#include <cib/cib.hpp>

#include "data-models/timing.hpp"

// Main routine
using cib::RuntimeInit;
class MainLoop : public callback::service<data_models::TimeMs> {};
class OnTimer0Interrupt : public callback::service<> {};
