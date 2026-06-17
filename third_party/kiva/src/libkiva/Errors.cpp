/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include <iostream>

#include "Errors.hpp"

namespace Kiva {

KivaCallbackFunction kivaCallbackFunction;
void *messageCallbackContextPtr;

void setMessageCallback(KivaCallbackFunction callBackFunction, void *contextPtr) {
  kivaCallbackFunction = callBackFunction;
  messageCallbackContextPtr = contextPtr;
}

void showMessage(const int messageType, const std::string message) {
  if (kivaCallbackFunction != NULL) {
    (*kivaCallbackFunction)(messageType, message, messageCallbackContextPtr);
  } else {
    if (messageType == MSG_ERR) {
      std::cerr << "Error: " << message << std::endl;
      exit(EXIT_FAILURE);
    } else if (messageType == MSG_WARN) {
      std::cerr << "Warning: " << message << std::endl;
    } else /*if (messageType == MSG_INFO)*/ {
      std::cout << "Note: " << message << std::endl;
    }
  }
}

} // namespace Kiva
