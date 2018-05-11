#pragma once
#include <cib/cib.hpp>

// Main routine
class RuntimeInit : public flow::service<> {};
class MainLoop : public cib::callback_meta<> {};
class OnTimer0Interrupt : public cib::callback_meta<> {};
