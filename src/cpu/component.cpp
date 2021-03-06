#include "../include/component.h"

ComponentListener * Component::setListener(ComponentListener *l) {
  ComponentListener *old = listener;
  listener = l;
  return old;
}

void Component::sendEvent(int ev) {
  if (listener) {
    listener->componentEvent(this, ev);
  }
}

std::ostream & operator << (std::ostream &os, Component &c) {
  return c.status(os);
}