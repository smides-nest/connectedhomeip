/*
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *  This file defines the <tt>Inet::InterfaceId</tt> type alias and related
 *  classes for iterating on the list of system network interfaces and the list
 *  of system interface addresses.
 */

#ifndef INETINTERFACE_H
#define INETINTERFACE_H

#include <stddef.h>
#include <stdint.h>

#include <support/DLLUtil.h>
#include <IPAddress.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/netif.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
struct if_nameindex;
struct ifaddrs;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {
namespace Inet {

class IPAddress;
class IPPrefix;


/**
 * @typedef     InterfaceId
 *
 * @brief       Indicator for system network interfaces.
 *
 * @details
 *  Portability depends on never witnessing this alias. It may be replaced by a
 *  concrete opaque class in the future.
 *
 *  Note Well: The term "interface identifier" also conventionally refers to
 *  the lower 64 bits of an IPv6 address in all the relevant IETF standards
 *  documents, where the abbreviation "IID" is often used. In this text, the
 *  term "interface indicator" refers to values of this type alias.
 */

#if CHIP_SYSTEM_CONFIG_USE_LWIP
typedef struct netif *InterfaceId;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
typedef unsigned InterfaceId;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS


/**
 * @def     INET_NULL_INTERFACEID
 *
 * @brief   The distinguished value indicating no network interface.
 *
 * @details
 *  Note Well: This is not the indicator of a "null" network interface. This
 *  value can be used to indicate the absence of a specific network interface,
 *  or to specify that any applicable network interface is acceptable. Usage
 *  varies depending on context.
 */

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define INET_NULL_INTERFACEID NULL
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#define INET_NULL_INTERFACEID 0
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS


/**
 * @brief   Test \c ID for inequivalence with \c INET_NULL_INTERFACEID
 *
 * @details
 *  This macro resolves to an expression that evaluates \c false if the
 *  argument is equivalent to \c INET_NULL_INTERFACEID and \c true otherwise.
 */
#define IsInterfaceIdPresent(intfId) ((intfId) != INET_NULL_INTERFACEID)

extern INET_ERROR GetInterfaceName(InterfaceId intfId, char *nameBuf, size_t nameBufSize);
extern INET_ERROR InterfaceNameToId(const char *intfName, InterfaceId& intfId);
extern uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen);

/**
 * @brief   Iterator for the list of system network interfaces.
 *
 * @details
 *  Use objects of this class to iterate the list of system network interfaces.
 *
 *  Methods on an individual instance of this class are *not* thread-safe;
 *  however separate instances may be used simultaneously by multiple threads.
 *
 *  On multi-threaded LwIP systems, instances are thread-safe relative to other
 *  threads accessing the global LwIP state provided that the other threads hold
 *  the LwIP core lock while mutating the list of netifs, and that netif object
 *  themselves are never destroyed.
 *
 *  On sockets-based systems, iteration is always stable in the face of changes
 *  to the underlying system's interfaces.
 *
 *  On LwIP systems, iteration is stable except in the case where the currently
 *  selected interface is removed from the list, in which case iteration ends
 *  immediately.
 */
class InterfaceIterator
{
public:
    InterfaceIterator(void);
    ~InterfaceIterator(void);

    bool HasCurrent(void);
    bool Next(void);
    InterfaceId GetInterface(void);
    InterfaceId GetInterfaceId(void);
    INET_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize);
    bool IsUp(void);
    bool SupportsMulticast(void);
    bool HasBroadcastAddress(void);

protected:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    struct netif * mCurNetif;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    struct if_nameindex  * mIntfArray;
    size_t mCurIntf;
    short mIntfFlags;
    bool mIntfFlagsCached;

    short GetFlags(void);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};

/**
 * @brief   Iterator for the list of system network interface IP addresses.
 *
 * @details
 *  Use objects of this class to iterate the list of system network interface
 *  interface IP addresses.
 *
 *  Methods on an individual instance of this class are *not* thread-safe;
 *  however separate instances may be used simultaneously by multiple threads.
 *
 *  On multi-threaded LwIP systems, instances are thread-safe relative to other
 *  threads accessing the global LwIP state provided that: 1) other threads hold
 *  the LwIP core lock while mutating the list of netifs; and 2) netif object
 *  themselves are never destroyed.
 *
 *  On sockets-based systems, iteration is always stable in the face of changes
 *  to the underlying system's interfaces and/or addresses.
 *
 *  On LwIP systems, iteration is stable except in the case where the interface
 *  associated with the current address is removed, in which case iteration may
 *  end prematurely.
 */
class DLL_EXPORT InterfaceAddressIterator
{
public:
    InterfaceAddressIterator(void);
    ~InterfaceAddressIterator(void);

    bool HasCurrent(void);
    bool Next(void);
    IPAddress GetAddress(void);
    uint8_t GetPrefixLength(void);
    uint8_t GetIPv6PrefixLength(void);
    void GetAddressWithPrefix(IPPrefix & addrWithPrefix);
    InterfaceId GetInterface(void);
    InterfaceId GetInterfaceId(void);
    INET_ERROR GetInterfaceName(char * nameBuf, size_t nameBufSize);
    bool IsUp(void);
    bool SupportsMulticast(void);
    bool HasBroadcastAddress(void);

private:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    enum
    {
        kBeforeStartIndex = -1
    };

    InterfaceIterator mIntfIter;
    int mCurAddrIndex;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    struct ifaddrs * mAddrsList;
    struct ifaddrs * mCurAddr;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};


#if CHIP_SYSTEM_CONFIG_USE_LWIP

inline InterfaceIterator::InterfaceIterator(void)
{
    mCurNetif = netif_list;
}

inline InterfaceIterator::~InterfaceIterator(void)
{
}

inline bool InterfaceIterator::HasCurrent(void)
{
    return mCurNetif != NULL;
}

inline InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return mCurNetif;
}

inline InterfaceAddressIterator::InterfaceAddressIterator(void)
{
	mCurAddrIndex = kBeforeStartIndex;
}

inline InterfaceAddressIterator::~InterfaceAddressIterator(void)
{
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP


/**
 * @brief    Deprecated alias for \c GetInterfaceId(void)
 */
inline InterfaceId InterfaceIterator::GetInterface(void)
{
    return GetInterfaceId();
}

/**
 * @brief    Deprecated alias for \c GetInterfaceId(void)
 */
inline InterfaceId InterfaceAddressIterator::GetInterface(void)
{
    return GetInterfaceId();
}

/**
 * @brief    Deprecated alias for \c GetPrefixLength(void)
 */
inline uint8_t InterfaceAddressIterator::GetIPv6PrefixLength(void)
{
	return GetPrefixLength();
}

} // namespace Inet
} // namespace chip

#endif /* INETINTERFACE_H */
