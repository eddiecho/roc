#[macro_export]
macro_rules! For {
    // for (; ...; ...) { ... }
    (; $($rest: tt)*) => {
        cfor!((); $($rest)*)
    };
    // for ($init; ; ...) { ... }
    ($($init: stmt),+; ; $($rest: tt)*) => {
        // avoid the `while true` lint
        cfor!($($init),+; !false; $($rest)*)
    };

    // for ($init; $cond; ) { ... }
    ($($init: stmt),+; $cond: expr; ; $body: block) => {
        cfor!{$($init),+; $cond; (); $body}
    };

    // for ($init; $cond; $step) { $body }
    ($($init: stmt),+; $cond: expr; $($step: expr),+; $body: block) => {
        {
            $($init)+
            while $cond {
                let mut _first = true;
                let mut _continue = false;
                // this loop runs once, allowing us to use `break` and
                // `continue` as `goto` to skip forward to the
                // condition.
                //
                // the booleans above are very transparent to the
                // optimiser, since they are modified exactly once,
                // with nice control flow, and this this optimises to
                // be similar to C for loop.
                loop {
                    // if we *don't* hit this, there was a `break` in
                    // the body (otherwise the loop fell-through or
                    // was `continue`d.)
                    if !_first { _continue = true; break }
                    _first = false;

                    $body
                }
                if !_continue {
                    // the `if` wasn't hit, so we should propagate the
                    // `break`.
                    break
                }

                $($step;)+
            }
        }
    };
}
