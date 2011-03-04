#include "proxy.h"

Proxy * Proxy::m_singleton = new Proxy();

Proxy::Proxy() {}

Proxy* Proxy::singleton() {

    return m_singleton;

}