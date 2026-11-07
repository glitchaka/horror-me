//! Static native FFI bridge. Verification executes inside HorrorMe.exe,
//! not in a replaceable launcher or command-line utility.
#![deny(unsafe_op_in_unsafe_fn)]

#[cfg(windows)]
mod windows_bridge {
    use std::{ffi::OsString, os::windows::ffi::OsStringExt, path::PathBuf};

    pub fn verify(executable: *const u16, length: usize, key: *const u8, key_length: usize) -> i32 {
        if executable.is_null() || key.is_null() || !(1..=32767).contains(&length)
            || key_length != 64
        {
            return 30;
        }
        let path = unsafe { std::slice::from_raw_parts(executable, length) };
        if path.contains(&0) { return 30; }
        let executable = PathBuf::from(OsString::from_wide(path));
        if disk_seal::read_executable_id(&executable).is_err() {
            return 21;
        }
        let key_hex = unsafe { std::slice::from_raw_parts(key, key_length) };
        let mut public = [0u8; 32];
        for (index, pair) in key_hex.chunks(2).enumerate() {
            let Ok(two_digits) = std::str::from_utf8(pair) else { return 30; };
            let Ok(value) = u8::from_str_radix(two_digits, 16) else { return 30; };
            public[index] = value;
        }
        let Some(root) = executable.parent() else { return 30; };
        let result = disk_seal::verify_authorized_installation(
            &executable,
            &root.join("data").join("installation.seal"),
            &root.join("data").join("installation.license"),
            "horror-me",
            &public,
        );
        if result.is_ok() { 0 } else { 22 }
    }
}

/// 0=authorized, 21=missing embedded ID, 22=invalid signed
/// license/seal/storage, 30=invalid invocation, 90=internal panic.
/// All nonzero results MUST prevent gameplay.
#[unsafe(no_mangle)]
pub extern "C" fn horror_disk_seal_verify(
    executable: *const u16,
    length: usize,
    public_key_hex: *const u8,
    key_length: usize,
) -> i32 {
    std::panic::catch_unwind(|| {
        #[cfg(windows)]
        { windows_bridge::verify(executable, length, public_key_hex, key_length) }
        #[cfg(not(windows))]
        {
            let _ = (executable, length, public_key_hex, key_length);
            30
        }
    }).unwrap_or(90)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn cannot_authorize_missing_arguments() {
        assert_ne!(horror_disk_seal_verify(
            std::ptr::null(), 0, std::ptr::null(), 0
        ), 0);
    }
}
