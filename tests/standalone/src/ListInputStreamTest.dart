// Copyright (c) 2011, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

void testEmptyListInputStream() {
  InputStream stream = new ListInputStream([]);
  ReceivePort donePort = new ReceivePort();

  void onData() {
    throw "No data expected";
  }

  void onClose() {
    donePort.toSendPort().send(null);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

void testEmptyDynamicListInputStream() {
  InputStream stream = new DynamicListInputStream();
  ReceivePort donePort = new ReceivePort();

  void onData() {
    throw "No data expected";
  }

  void onClose() {
    donePort.toSendPort().send(null);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;
  stream.markEndOfStream();

  donePort.receive((x,y) => donePort.close());
}

void testListInputStream1() {
  List<int> data = [0x00, 0x01, 0x10, 0x11, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff];
  InputStream stream = new ListInputStream(data);
  int count = 0;
  ReceivePort donePort = new ReceivePort();

  void onData() {
    List<int> x = stream.read(1);
    Expect.equals(1, x.length);
    Expect.equals(data[count++], x[0]);
  }

  void onClose() {
    Expect.equals(data.length, count);
    donePort.toSendPort().send(count);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

void testListInputStream2() {
  List<int> data = [0x00, 0x01, 0x10, 0x11, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff];
  InputStream stream = new ListInputStream(data);
  int count = 0;
  ReceivePort donePort = new ReceivePort();

  void onData() {
    List<int> x = new List<int>(2);
    var bytesRead = stream.readInto(x);
    Expect.equals(2, bytesRead);
    Expect.equals(data[count++], x[0]);
    Expect.equals(data[count++], x[1]);
  }

  void onClose() {
    Expect.equals(data.length, count);
    donePort.toSendPort().send(count);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

void testListInputStreamPipe1() {
  List<int> data = [0x00, 0x01, 0x10, 0x11, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff];
  InputStream input = new ListInputStream(data);
  OutputStream output = new ListOutputStream();
  ReceivePort donePort = new ReceivePort();

  void onClose() {
    var contents = output.contents();
    Expect.equals(data.length, contents.length);
    donePort.toSendPort().send(null);
  }

  input.closeHandler = onClose;
  input.pipe(output);

  donePort.receive((x,y) => donePort.close());
}

void testListInputStreamPipe2() {
  List<int> data = [0x00, 0x01, 0x10, 0x11, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff];
  OutputStream output = new ListOutputStream();
  ReceivePort donePort = new ReceivePort();
  int count = 0;

  void onClose() {
    if (count < 10) {
      InputStream input = new ListInputStream(data);
      input.closeHandler = onClose;
      if (count < 9) {
        input.pipe(output, close: false);
      } else {
        input.pipe(output);
      }
      count++;
    } else {
      var contents = output.contents();
      Expect.equals(data.length * 10, contents.length);
      donePort.toSendPort().send(null);
    }
  }

  InputStream input = new ListInputStream(data);
  input.closeHandler = onClose;
  input.pipe(output, close: false);
  count++;

  donePort.receive((x,y) => donePort.close());
}

void testListInputClose1() {
  List<int> data = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
  InputStream stream = new ListInputStream(data);
  ReceivePort donePort = new ReceivePort();

  void onData() {
    throw "No data expected";
  }

  void onClose() {
    donePort.toSendPort().send(null);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;
  stream.close();

  donePort.receive((x,y) => donePort.close());
}

void testListInputClose2() {
  List<int> data = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
  InputStream stream = new ListInputStream(data);
  ReceivePort donePort = new ReceivePort();
  int count = 0;

  void onData() {
    count += stream.read(2).length;
    stream.close();
  }

  void onClose() {
    Expect.equals(2, count);
    donePort.toSendPort().send(count);
  }

  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

void testDynamicListInputStream() {
  List<int> data = [0x00, 0x01, 0x10, 0x11, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff];
  InputStream stream = new DynamicListInputStream();
  int count = 0;
  ReceivePort donePort = new ReceivePort();

  void onData() {
    List<int> x = stream.read(1);
    Expect.equals(1, x.length);
    x = stream.read();
    Expect.equals(9, x.length);
    count++;
    if (count < 10) {
      stream.write(data);
    } else {
      stream.markEndOfStream();
    }
  }

  void onClose() {
    Expect.equals(data.length, count);
    donePort.toSendPort().send(count);
  }

  stream.write(data);
  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

void testDynamicListInputClose1() {
  List<int> data = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
  InputStream stream = new DynamicListInputStream();
  ReceivePort donePort = new ReceivePort();

  void onData() {
    throw "No data expected";
  }

  void onClose() {
    donePort.toSendPort().send(null);
  }

  stream.write(data);
  stream.dataHandler = onData;
  stream.closeHandler = onClose;
  stream.close();
  Expect.throws(() => stream.write(data), (e) => e is StreamException);

  donePort.receive((x,y) => donePort.close());
}

void testDynamicListInputClose2() {
  List<int> data = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
  InputStream stream = new DynamicListInputStream();
  ReceivePort donePort = new ReceivePort();
  int count = 0;

  void onData() {
    count += stream.read(15).length;
    stream.close();
    Expect.throws(() => stream.write(data), (e) => e is StreamException);
  }

  void onClose() {
    Expect.equals(15, count);
    donePort.toSendPort().send(null);
  }

  stream.write(data);
  stream.write(data);
  stream.write(data);
  stream.dataHandler = onData;
  stream.closeHandler = onClose;

  donePort.receive((x,y) => donePort.close());
}

main() {
  testEmptyListInputStream();
  testEmptyDynamicListInputStream();
  testListInputStream1();
  testListInputStream2();
  testListInputStreamPipe1();
  testListInputStreamPipe2();
  testListInputClose1();
  testListInputClose2();
  testDynamicListInputStream();
  testDynamicListInputClose1();
  testDynamicListInputClose2();
}
