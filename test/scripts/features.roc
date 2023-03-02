fun testing(a) {
  // scopes yield values
  var b = {
    var c = 10;
    yield a + c;
  }

  // if-expressions
  var d = if (a > 10) {
    yield 10;
  } else {
    yield a;
  }

  // for-expressions
  var e = for i in 0..10 {
    if (IsPrime(i)) {
      yield i;
    }
  }
  // var-binding
  // if and for-expressions without an else block will return Option<>;
  var e = e.Some();

  // for-else
  var f = for i in 0..10 {
    if (i > 10) {
      yield i;
    }
  } else {
    yield 20;
  }

  // nested break + yield
  var ff = outer: for i in 0..10 {
    for j in 0..10 {
      if (i + j > 20) {
        break outer yield i + j;
      }
    }
  }

  // accumulator for-expresssion
  // automatically yields final value of init variable
  var acc = for i in 0..10 |temp = 0| {
    temp += i;
  }

  // match-expression
  var m = match (a) {
    0 => {
      yield "0";
    }
    1 => {
      yield "1";
    }
    _ => {
      yield "idk";
    }
  }

  print(m);

  return b + d + e + f + ff + acc;
}
