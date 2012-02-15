// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// example of spawning an isolate from a URI
#library('spawn_tests');
#import("../../../client/testing/unittest/unittest.dart");
#import("dart:dom"); // import added so test.dart can treat this as a webtest.

main() {
  asyncTest("isolate fromUri - send and reply ", 1, () {
    ReceivePort port = new ReceivePort();
    port.receive((msg, _) {
      expect(msg).equals("re: hi");
      port.close();
      callbackDone();
    });

    // TODO(eub): make this work for non-JS targets.
    Isolate2 c = new Isolate2.fromUri("./APIv2_spawnUriChildIsolate.js");
    c.sendPort.send("hi", port.toSendPort());
  });
}
