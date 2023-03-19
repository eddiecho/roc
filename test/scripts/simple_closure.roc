fun outer() {
  var x = 1;
  fun inner() {
    return x + 1;
  }

  return inner();
}

outer();
