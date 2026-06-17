/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef Errors_HPP
#define Errors_HPP

#include "libkiva_export.h"
#include <string>

namespace Kiva {

const int MSG_INFO = 0;
const int MSG_WARN = 1;
const int MSG_ERR = 2;

typedef void (*KivaCallbackFunction)(const int messageType, const std::string message,
                                     void *contextPtr);

extern KivaCallbackFunction kivaCallbackFunction;
extern void *messageCallbackContextPtr;

void LIBKIVA_EXPORT setMessageCallback(KivaCallbackFunction callbackFunction, void *contextPtr);

void LIBKIVA_EXPORT showMessage(const int messageType, const std::string message);

} // namespace Kiva

#endif
