// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#library("chat_server");
#import("dart:io");
#import("dart:isolate");
#import("dart:json");

typedef void RequestHandler(HttpRequest request, HttpResponse response);

class ChatServer extends IsolatedServer {
  ChatServer() : super() {
    addHandler("/",
               (HttpRequest request, HttpResponse response) =>
                   redirectPageHandler(
                       request, response, "dart_client/index.html"));
    addHandler("/js_client/index.html",
               (HttpRequest request, HttpResponse response) =>
                   fileHandler(request, response));
    addHandler("/js_client/code.js",
               (HttpRequest request, HttpResponse response) =>
                   fileHandler(request, response));
    addHandler("/dart_client/index.html",
               (HttpRequest request, HttpResponse response) =>
                   fileHandler(request, response));
    addHandler("/out/dart_client/chat.dart.app.js",
               (HttpRequest request, HttpResponse response) =>
                   fileHandler(request, response));
    addHandler("/favicon.ico",
               (HttpRequest request, HttpResponse response) =>
                   fileHandler(request, response, "static/favicon.ico"));

    addHandler("/join", _joinHandler);
    addHandler("/leave", _leaveHandler);
    addHandler("/message", _messageHandler);
    addHandler("/receive", _receiveHandler);
  }
}

class ServerMain {
  ServerMain.start(IsolatedServer server,
                   String hostAddress,
                   int tcpPort,
                   [int listenBacklog = 5])
      : _statusPort = new ReceivePort(),
        _serverPort = null {
    server.spawn().then((SendPort port) {
      _serverPort = port;
      _start(hostAddress, tcpPort, listenBacklog);
    });
    // We can only guess this is the right URL. At least it gives a
    // hint to the user.
    print('Server starting http://${hostAddress}:${tcpPort}/');
  }

    void _start(String hostAddress, int tcpPort, int listenBacklog) {
    // Handle status messages from the server.
    _statusPort.receive((var message, SendPort replyTo) {
      String status = message.message;
      print("Received status: $status");
    });

    // Send server start message to the server.
    var command = new ChatServerCommand.start(hostAddress,
                                              tcpPort,
                                              backlog: listenBacklog);
    _serverPort.send(command, _statusPort.toSendPort());
  }

  void shutdown() {
    // Send server stop message to the server.
    _serverPort.send(new ChatServerCommand.stop(), _statusPort.toSendPort());
    _statusPort.close();
  }

  ReceivePort _statusPort;  // Port for receiving messages from the server.
  SendPort _serverPort;  // Port for sending messages to the server.
}


class User {
  User(this._handle) {
    // TODO(sgjesse) Generate more secure and unique session id's.
    _sessionId = 'a' + ((Math.random() * 1000000).toInt()).toString();
    markActivity();
  }

  void markActivity() => _lastActive = new Date.now();
  Duration idleTime(Date now) => now.difference(_lastActive);

  String get handle() => _handle;
  String get sessionId() => _sessionId;

  String _handle;
  String _sessionId;
  Date _lastActive;
}


class Message {
  static final int JOIN = 0;
  static final int MESSAGE = 1;
  static final int LEAVE = 2;
  static final int TIMEOUT = 3;
  static final List<String> _typeName =
      const [ "join", "message", "leave", "timeout"];

  Message.join(this._from)
      : _received = new Date.now(), _type = JOIN;
  Message(this._from, this._message)
      : _received = new Date.now(), _type = MESSAGE;
  Message.leave(this._from)
      : _received = new Date.now(), _type = LEAVE;
  Message.timeout(this._from)
      : _received = new Date.now(), _type = TIMEOUT;

  User get from() => _from;
  Date get received() => _received;
  String get message() => _message;
  void set messageNumber(int n) => _messageNumber = n;

  Map toMap() {
    Map map = new Map();
    map["from"] = _from.handle;
    map["received"] = _received.toString();
    map["type"] = _typeName[_type];
    if (_type == MESSAGE) map["message"] = _message;
    map["number"] = _messageNumber;
    return map;
  }

  User _from;
  Date _received;
  int _type;
  String _message;
  int _messageNumber;
}


class Topic {
  static final int DEFAULT_IDLE_TIMEOUT = 60 * 60 * 1000;  // One hour.
  Topic()
      : _activeUsers = new Map(),
        _messages = new List(),
        _nextMessageNumber = 0,
        _callbacks = new Map();

  int get activeUsers() => _activeUsers.length;

  User _userJoined(String handle) {
    User user = new User(handle);
    _activeUsers[user.sessionId] = user;
    Message message = new Message.join(user);
    _addMessage(message);
    return user;
  }

  User _userLookup(String sessionId) => _activeUsers[sessionId];

  void _userLeft(String sessionId) {
    User user = _userLookup(sessionId);
    Message message = new Message.leave(user);
    _addMessage(message);
    _activeUsers.remove(sessionId);
  }

  bool _addMessage(Message message) {
    message.messageNumber = _nextMessageNumber++;
    _messages.add(message);

    // Send the new message to all polling clients.
    List messages = new List();
    messages.add(message.toMap());
    _callbacks.forEach((String sessionId, Function callback) {
      callback(messages);
    });
    _callbacks = new Map();
  }

  bool _userMessage(Map requestData) {
    String sessionId = requestData["sessionId"];
    User user = _userLookup(sessionId);
    if (user == null) return false;
    String handle = user.handle;
    String messageText = requestData["message"];
    if (messageText == null) return false;

    // Add new message.
    Message message = new Message(user, messageText);
    _addMessage(message);
    user.markActivity();

    return true;
  }

  List messagesFrom(int messageNumber, int maxMessages) {
    if (_messages.length > messageNumber) {
      if (maxMessages != null) {
        if (_messages.length - messageNumber > maxMessages) {
          messageNumber = _messages.length - maxMessages;
        }
      }
      List messages = new List();
      for (int i = messageNumber; i < _messages.length; i++) {
        messages.add(_messages[i].toMap());
      }
      return messages;
    } else {
      return null;
    }
  }

  void registerChangeCallback(String sessionId, var callback) {
    _callbacks[sessionId] = callback;
  }

  void _handleTimer(Timer timer) {
    Set inactiveSessions = new Set();
    // Collect all sessions which have not been active for some time.
    Date now = new Date.now();
    _activeUsers.forEach((String sessionId, User user) {
      if (user.idleTime(now).inMilliseconds > DEFAULT_IDLE_TIMEOUT) {
        inactiveSessions.add(sessionId);
      }
    });
    // Terminate the inactive sessions.
    inactiveSessions.forEach((String sessionId) {
      Function callback = _callbacks.remove(sessionId);
      if (callback != null) callback(null);
      User user = _activeUsers.remove(sessionId);
      Message message = new Message.timeout(user);
      _addMessage(message);
    });
  }

  Map<String, User> _activeUsers;
  List<Message> _messages;
  int _nextMessageNumber;
  Map<String, Function> _callbacks;
}


class ChatServerCommand {
  static final START = 0;
  static final STOP = 1;

  ChatServerCommand.start(String this._host,
                          int this._port,
                          [int backlog = 5,
                           bool logging = false])
      : _command = START, _backlog = backlog, _logging = logging;
  ChatServerCommand.stop() : _command = STOP;

  bool get isStart() => _command == START;
  bool get isStop() => _command == STOP;

  String get host() => _host;
  int get port() => _port;
  bool get logging() => _logging;
  int get backlog() => _backlog;

  int _command;
  String _host;
  int _port;
  int _backlog;
  bool _logging;
}


class ChatServerStatus {
  static final STARTING = 0;
  static final STARTED = 1;
  static final STOPPING = 2;
  static final STOPPED = 3;
  static final ERROR = 4;

  ChatServerStatus(this._state, this._message);
  ChatServerStatus.starting() : _state = STARTING;
  ChatServerStatus.started(this._port) : _state = STARTED;
  ChatServerStatus.stopping() : _state = STOPPING;
  ChatServerStatus.stopped() : _state = STOPPED;
  ChatServerStatus.error([this._error]) : _state = ERROR;

  bool get isStarting() => _state == STARTING;
  bool get isStarted() => _state == STARTED;
  bool get isStopping() => _state == STOPPING;
  bool get isStopped() => _state == STOPPED;
  bool get isError() => _state == ERROR;

  int get state() => _state;
  String get message() {
    if (_message != null) return _message;
    switch (_state) {
      case STARTING: return "Server starting";
      case STARTED: return "Server listening";
      case STOPPING: return "Server stopping";
      case STOPPED: return "Server stopped";
      case ERROR:
        if (_error == null) {
          return "Server error";
        } else {
          return "Server error: $_error";
        }
    }
  }

  int get port() => _port;
  Dynamic get error() => _error;

  int _state;
  String _message;
  int _port;
  var _error;
}


class IsolatedServer extends Isolate {
  static final String redirectPageHtml = """
<html>
<head><title>Welcome to the dart server</title></head>
<body><h1>Redirecting to the front page...</h1></body>
</html>""";
  static final String notFoundPageHtml = """
<html><head>
<title>404 Not Found</title>
</head><body>
<h1>Not Found</h1>
<p>The requested URL was not found on this server.</p>
</body></html>""";

  void _sendJSONResponse(HttpResponse response, Map responseData) {
    response.setHeader("Content-Type", "application/json; charset=UTF-8");
    response.outputStream.writeString(JSON.stringify(responseData));
    response.outputStream.close();
  }

  IsolatedServer() : super() {
    _requestHandlers = new Map();
  }

  void redirectPageHandler(HttpRequest request,
                           HttpResponse response,
                           String redirectPath) {
    if (_redirectPage == null) {
      _redirectPage = redirectPageHtml.charCodes();
    }
    response.statusCode = HttpStatus.FOUND;
    response.setHeader(
        "Location", "http://$_host:$_port/${redirectPath}");
    response.contentLength = _redirectPage.length;
    response.outputStream.write(_redirectPage);
    response.outputStream.close();
  }

  // Serve the content of a file.
  void fileHandler(
      HttpRequest request, HttpResponse response, [String fileName = null]) {
    final int BUFFER_SIZE = 4096;
    if (fileName == null) {
      fileName = request.path.substring(1);
    }
    File file = new File(fileName);
    if (file.existsSync()) {
      String mimeType = "text/html; charset=UTF-8";
      int lastDot = fileName.lastIndexOf(".", fileName.length);
      if (lastDot != -1) {
        String extension = fileName.substring(lastDot);
        if (extension == ".css") { mimeType = "text/css"; }
        if (extension == ".js") { mimeType = "application/javascript"; }
        if (extension == ".ico") { mimeType = "image/vnd.microsoft.icon"; }
        if (extension == ".png") { mimeType = "image/png"; }
      }
      response.setHeader("Content-Type", mimeType);
      // Get the length of the file for setting the Content-Length header.
      RandomAccessFile openedFile = file.openSync();
      response.contentLength = openedFile.lengthSync();
      openedFile.close();
      // Pipe the file content into the response.
      file.openInputStreamSync().pipe(response.outputStream);
    } else {
      print("File not found: $fileName");
      _notFoundHandler(request, response);
    }
  }

  // Serve the not found page.
  void _notFoundHandler(HttpRequest request, HttpResponse response) {
    if (_notFoundPage == null) {
      _notFoundPage = notFoundPageHtml.charCodes();
    }
    response.statusCode = HttpStatus.NOT_FOUND;
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.contentLength = _notFoundPage.length;
    response.outputStream.write(_notFoundPage);
    response.outputStream.close();
  }

  // Unexpected protocol data.
  void _protocolError(HttpRequest request, HttpResponse response) {
    response.statusCode = HttpStatus.INTERNAL_SERVER_ERROR;
    response.contentLength = 0;
    response.outputStream.close();
  }

  // Join request:
  // { "request": "join",
  //   "handle": <handle> }
  void _joinHandler(HttpRequest request, HttpResponse response) {
    StringBuffer body = new StringBuffer();
    StringInputStream input = new StringInputStream(request.inputStream);
    input.onData = () => body.add(input.read());
    input.onClosed = () {
      String data = body.toString();
      if (data != null) {
        var requestData = JSON.parse(data);
        if (requestData["request"] == "join") {
          String handle = requestData["handle"];
          if (handle != null) {
            // New user joining.
            User user = _topic._userJoined(handle);

            // Send response.
            Map responseData = new Map();
            responseData["response"] = "join";
            responseData["sessionId"] = user.sessionId;
            _sendJSONResponse(response, responseData);
          } else {
            _protocolError(request, response);
          }
        } else {
          _protocolError(request, response);
        }
      } else {
        _protocolError(request, response);
      }
    };
  }

  // Leave request:
  // { "request": "leave",
  //   "sessionId": <sessionId> }
  void _leaveHandler(HttpRequest request, HttpResponse response) {
    StringBuffer body = new StringBuffer();
    StringInputStream input = new StringInputStream(request.inputStream);
    input.onData = () => body.add(input.read());
    input.onClosed = () {
      String data = body.toString();
      var requestData = JSON.parse(data);
      if (requestData["request"] == "leave") {
        String sessionId = requestData["sessionId"];
        if (sessionId != null) {
          // User leaving.
          _topic._userLeft(sessionId);

          // Send response.
          Map responseData = new Map();
          responseData["response"] = "leave";
          _sendJSONResponse(response, responseData);
        } else {
          _protocolError(request, response);
        }
      } else {
        _protocolError(request, response);
      }
    };
  }

  // Message request:
  // { "request": "message",
  //   "sessionId": <sessionId>,
  //   "message": <message> }
  void _messageHandler(HttpRequest request, HttpResponse response) {
    StringBuffer body = new StringBuffer();
    StringInputStream input = new StringInputStream(request.inputStream);
    input.onData = () => body.add(input.read());
    input.onClosed = () {
      String data = body.toString();
      _messageCount++;
      _messageRate.record(1);
      var requestData = JSON.parse(data);
      if (requestData["request"] == "message") {
        String sessionId = requestData["sessionId"];
        if (sessionId != null) {
          // New message from user.
          bool success = _topic._userMessage(requestData);

          // Send response.
          if (success) {
            Map responseData = new Map();
            responseData["response"] = "message";
            _sendJSONResponse(response, responseData);
          } else {
            _protocolError(request, response);
          }
        } else {
          _protocolError(request, response);
        }
      } else {
        _protocolError(request, response);
      }
    };
  }

  // Receive request:
  // { "request": "receive",
  //   "sessionId": <sessionId>,
  //   "nextMessage": <nextMessage>,
  //   "maxMessages": <maxMesssages> }
  void _receiveHandler(HttpRequest request, HttpResponse response) {
    StringBuffer body = new StringBuffer();
    StringInputStream input = new StringInputStream(request.inputStream);
    input.onData = () => body.add(input.read());
    input.onClosed = () {
      String data = body.toString();
      var requestData = JSON.parse(data);
      if (requestData["request"] == "receive") {
        String sessionId = requestData["sessionId"];
        int nextMessage = requestData["nextMessage"];
        int maxMessages = requestData["maxMessages"];
        if (sessionId != null && nextMessage != null) {

          void sendResponse(messages) {
            // Send response.
            Map responseData = new Map();
            responseData["response"] = "receive";
            if (messages != null) {
              responseData["messages"] = messages;
              responseData["activeUsers"] = _topic.activeUsers;
              responseData["upTime"] =
                  new Date.now().difference(_serverStart).inMilliseconds;
            } else {
              responseData["disconnect"] = true;
            }
            _sendJSONResponse(response, responseData);
          }

          // Receive request from user.
          List messages = _topic.messagesFrom(nextMessage, maxMessages);
          if (messages == null) {
            _topic.registerChangeCallback(sessionId, sendResponse);
          } else {
            sendResponse(messages);
          }

        } else {
          _protocolError(request, response);
        }
      } else {
        _protocolError(request, response);
      }
    };
  }

  void addHandler(String path,
                  void handler(HttpRequest request, HttpResponse response)) {
    _requestHandlers[path] = handler;
  }

  void main() {
    _logRequests = false;
    _topic = new Topic();
    _serverStart = new Date.now();
    _messageCount = 0;
    _messageRate = new Rate();

    // Start a timer for cleanup events.
    _cleanupTimer =
        new Timer.repeating(10000, (timer) => _topic._handleTimer(timer));

    // Start timer for periodic logging.
    void _handleLogging(Timer timer) {
      if (_logging) {
        print((_messageRate.rate).toString() +
                       " messages/s (total " +
                       _messageCount +
                       " messages)");
      }
    }

    this.port.receive((var message, SendPort replyTo) {
      if (message.isStart) {
        _host = message.host;
        _port = message.port;
        _logging = message.logging;
        replyTo.send(new ChatServerStatus.starting(), null);
        _server = new HttpServer();
        try {
          _server.listen(_host, _port, backlog: message.backlog);
          _server.onRequest = (HttpRequest req, HttpResponse rsp) =>
              _requestReceivedHandler(req, rsp);
          replyTo.send(new ChatServerStatus.started(_server.port), null);
          _loggingTimer = new Timer.repeating(1000, _handleLogging);
        } catch (var e) {
          replyTo.send(new ChatServerStatus.error(e.toString()), null);
        }
      } else if (message.isStop) {
        replyTo.send(new ChatServerStatus.stopping(), null);
        stop();
        replyTo.send(new ChatServerStatus.stopped(), null);
      }
    });
  }

  stop() {
    _server.close();
    _cleanupTimer.cancel();
    this.port.close();
  }

  void _requestReceivedHandler(HttpRequest request, HttpResponse response) {
    if (_logRequests) {
      String method = request.method;
      String uri = request.uri;
      print("Request: $method $uri");
      print("Request headers:");
      request.headers.forEach(
          (String name, String value) => print("$name: $value"));
      print("Request parameters:");
      request.queryParameters.forEach(
          (String name, String value) => print("$name = $value"));
      print("");
    }

    var requestHandler =_requestHandlers[request.path];
    if (requestHandler != null) {
      requestHandler(request, response);
    } else {
      print('No request handler found for ${request.path}');
      _notFoundHandler(request, response);
    }
  }

  String _host;
  int _port;
  HttpServer _server;  // HTTP server instance.
  Map _requestHandlers;
  bool _logRequests;

  Topic _topic;
  Timer _cleanupTimer;
  Timer _loggingTimer;
  Date _serverStart;

  bool _logging;
  int _messageCount;
  Rate _messageRate;

  // Static HTML.
  List<int> _redirectPage;
  List<int> _notFoundPage;
}


// Calculate the rate of events over a given time range. The time
// range is split over a number of buckets where each bucket collects
// the number of events happening in that time sub-range. The first
// constructor arument specifies the time range in milliseconds. The
// buckets are in the list _buckets organized at a circular buffer
// with _currentBucket marking the bucket where an event was last
// recorded. A current sum of the content of all buckets except the
// one pointed a by _currentBucket is kept in _sum.
class Rate {
  Rate([int timeRange = 1000, int buckets = 10])
      : _timeRange = timeRange,
        _buckets = new List(buckets + 1),  // Current bucket is not in the sum.
        _currentBucket = 0,
        _currentBucketTime = new Date.now().value,
        _sum = 0 {
    _bucketTimeRange = (_timeRange / buckets).toInt();
    for (int i = 0; i < _buckets.length; i++) {
      _buckets[i] = 0;
    }
  }

  // Record the specified number of events.
  void record(int count) {
    _timePassed();
    _buckets[_currentBucket] = _buckets[_currentBucket] + count;
  }

  // Returns the current rate of events for the time range.
  num get rate() {
    _timePassed();
    return _sum;
  }

  // Update the current sum as time passes. If time has passed by the
  // current bucket add it to the sum and move forward to the bucket
  // matching the current time. Subtract all buckets vacated from the
  // sum as bucket for current time is located.
  void _timePassed() {
    int time = new Date.now().value;
    if (time < _currentBucketTime + _bucketTimeRange) {
      // Still same bucket.
      return;
    }

    // Add collected bucket to the sum.
    _sum += _buckets[_currentBucket];

    // Find the bucket for the current time. Subtract all buckets
    // reused from the sum.
    while (time >= _currentBucketTime + _bucketTimeRange) {
      _currentBucket = (_currentBucket + 1) % _buckets.length;
      _sum -= _buckets[_currentBucket];
      _buckets[_currentBucket] = 0;
      _currentBucketTime += _bucketTimeRange;
    }
  }

  int _timeRange;
  List<int> _buckets;
  int _currentBucket;
  int _currentBucketTime;
  num _bucketTimeRange;
  int _sum;
}
