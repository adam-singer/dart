// Copyright (c) 2011, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// Imported library has source file with library tags.

#library("Script2NegativeTest.dart");
#import("Script2NegativeLib.dart");

main() {
  print("Should not reach here.");
}
