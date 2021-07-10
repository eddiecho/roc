use std::env;
use std::fs;

#[macro_use]
pub mod macros;

pub mod scanner;
pub mod token;

const ROC_VERSION_MAJOR: usize = 0;
const ROC_VERSION_MINOR: usize = 0;
const ROC_VERSION_PATCH: usize = 1;

fn print_version() {
    println!(
        "Roc version {}.{}.{}",
        ROC_VERSION_MAJOR, ROC_VERSION_MINOR, ROC_VERSION_PATCH
    );
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 1 {
        print_version();
        println!("  Usage: {} [script]\n", args[0]);

        return ();
    }

    if args[1] == "--version" {
        print_version();

        return ();
    }

    let main_file = fs::read_to_string(&args[1]);
    match main_file {
        Err(e) => {
            println!("{:?}", e);
            println!("Unable to parse input file: {}", args[1]);
        }
        Ok(content) => {
            let mut scanner = scanner::Scanner::new(content.as_str());
            scanner.scan();
            scanner.print();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const INPUT: &'static str = "
() // single line comment
/*
multi line \
comment
 */ 1234
";

    #[test]
    fn test_scanner() {
        let mut scanner = scanner::Scanner::new(INPUT);
        scanner.scan();
        scanner.print();
    }
}
