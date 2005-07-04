// template definitions for gcc on Linux

#include "collection.h"
#include "collection.cc"

class LqnActivity;
class LqnDevice;
class LqnEntry;
class LqnTask;
class LqnMstack;
class LqnMessage;

template class LLItem<LqnActivity*>;
template class LLItem<LqnDevice*>;
template class LLItem<LqnEntry*>;
template class LLItem<LqnTask*>;
template class LLItem<LqnMessage*>;
template class LLItem<LqnMstack*>;

template class Cltn<LqnActivity*>;
template class Cltn<LqnDevice*>;
template class Cltn<LqnEntry*>;
template class Cltn<LqnTask*>;
template class Cltn<LqnMessage*>;
template class Cltn<LqnMstack*>;

