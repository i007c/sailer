use hmac::{Hmac, KeyInit, Mac};
use sha1::Sha1;
use std::process::{Command, Stdio, exit};
use std::time::{SystemTime, UNIX_EPOCH};

/// HOTP (RFC 4226): HMAC-SHA1 over the counter, dynamic truncation, 6 digits.
fn hotp(secret: &[u8], counter: u64) -> String {
    let mut mac = Hmac::<Sha1>::new_from_slice(secret).unwrap();
    mac.update(&counter.to_be_bytes());
    let h = mac.finalize().into_bytes();
    let o = (h[19] & 0x0f) as usize;
    let bin = u32::from_be_bytes([h[o] & 0x7f, h[o + 1], h[o + 2], h[o + 3]]);
    format!("{:06}", bin % 1_000_000)
}

/// TOTP (RFC 6238): HOTP where the counter is the current 30-second window.
fn totp(secret: &[u8]) -> String {
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs();
    hotp(secret, now / 30)
}

/// Decode base32 (what authenticator apps / QR codes use for secrets).
fn base32_decode(s: &str) -> Option<Vec<u8>> {
    let (mut bits, mut n, mut out) = (0u32, 0, Vec::new());
    for c in s.bytes().filter(|c| *c != b'=' && *c != b' ') {
        let c = c.to_ascii_uppercase();
        let v = match c {
            b'A'..=b'Z' => c - b'A',
            b'2'..=b'7' => c - b'2' + 26,
            _ => return None,
        };
        bits = (bits << 5) | v as u32;
        n += 5;
        if n >= 8 {
            n -= 8;
            out.push((bits >> n) as u8);
            bits &= (1 << n) - 1;
        }
    }
    Some(out)
}

/// Read the secret from `pass show 2fa/<name>` (first line only).
/// Accepts either a bare base32 secret or an otpauth:// URI.
fn get_secret(name: &str) -> String {
    let out = Command::new("pass")
        .arg("show")
        .arg(format!("2fa/{name}"))
        .stderr(Stdio::inherit()) // lets gpg/pinentry prompts and errors show up
        .output()
        .expect("failed to run `pass` (is it installed and in PATH?)");
    if !out.status.success() {
        exit(1);
    }
    let text = String::from_utf8(out.stdout).expect("pass output is not valid UTF-8");
    let line = text.lines().next().unwrap_or("").trim();
    match line.split("secret=").nth(1) {
        Some(rest) => rest.split('&').next().unwrap().to_string(),
        None => line.to_string(),
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("usage: 2fa <name> [hotp-counter] (reads pass entry 2fa/<name>)");
        exit(1);
    }

    let secret =
        base32_decode(&get_secret(&args[1])).expect("secret in pass entry is not valid base32");

    match args.get(2) {
        Some(c) => println!(
            "{}",
            hotp(&secret, c.parse().expect("counter must be a number"))
        ),
        None => println!("{}", totp(&secret)),
    }
}
