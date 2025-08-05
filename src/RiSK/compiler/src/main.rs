mod lexer;
mod parser;
mod tools;

use std::io;

fn main() -> io::Result<()> {
    let source: String = tools::fs::read_file(
        "/Users/rohit/rohit-project-work/rxevm/src/RiSK/samples/syntax-example.risk",
    )?;

    Ok(())
}
