
class _SVGPathSegMovetoRelImpl extends _SVGPathSegImpl implements SVGPathSegMovetoRel {
  _SVGPathSegMovetoRelImpl._wrap(ptr) : super._wrap(ptr);

  num get x() => _wrap(_ptr.x);

  void set x(num value) { _ptr.x = _unwrap(value); }

  num get y() => _wrap(_ptr.y);

  void set y(num value) { _ptr.y = _unwrap(value); }
}