//! Only for CI: generates an ephemeral public key or personalizes a copy.
//! Never embeds or writes a private signing key in the game.
use std::{env, path::Path, process};
use anyhow::{bail, Result};

fn hex(bytes: &[u8]) -> String {
    const DIGITS: &[u8; 16] = b"0123456789abcdef";
    let mut out = String::with_capacity(bytes.len() * 2);
    for &b in bytes {
        out.push(DIGITS[(b >> 4) as usize] as char);
        out.push(DIGITS[(b & 0xf) as usize] as char);
    }
    out
}

fn run() -> Result<()> {
    let args: Vec<String> = env::args().skip(1).collect();
    match args.as_slice() {
        [cmd] if cmd == "public-key" => {
            let key = disk_seal::generate_signing_key();
            println!("{}", hex(key.verifying_key().as_bytes()));
            Ok(())
        }
        [cmd, template, target] if cmd == "personalize" => {
            let id = disk_seal::new_installation_id();
            disk_seal::personalize_executable(
                Path::new(template), Path::new(target), &id,
            )?;
            println!("{id}");
            Ok(())
        }
        _ => bail!("Usage: seal-test-fixture public-key | personalize TEMPLATE TARGET"),
    }
}
fn main() {
    if let Err(error) = run() {
        eprintln!("{error:#}");
        process::exit(1);
    }
}
