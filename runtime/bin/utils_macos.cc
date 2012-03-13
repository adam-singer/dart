// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include <errno.h>

#include "bin/utils.h"

OSError::OSError() : code_(0), message_(NULL) {
  set_code(errno);
  SetMessage(strerror(errno));
}