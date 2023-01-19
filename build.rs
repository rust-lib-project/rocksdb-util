extern crate bindgen;
extern crate cc;
extern crate cmake;

use cc::Build;
use cmake::Config;
use std::path::{Path, PathBuf};
use std::{env, str};
// Generate the bindings to rocksdb C-API.
// Try to disable the generation of platform-related bindings.
fn bindgen_rocksdb(file_path: &Path) {
    let bindings = bindgen::Builder::default()
        .header("include/rocksdb/c.h")
        .ctypes_prefix("libc")
        .generate()
        .expect("unable to generate rocksdb bindings");

    bindings
        .write_to_file(file_path)
        .expect("unable to write rocksdb bindings");
}

// Determine if need to update bindings. Supported platforms do not
// need to be updated by default unless the UPDATE_BIND is specified.
// Other platforms use bindgen to generate the bindings every time.
fn config_binding_path() {
    let file_path: PathBuf;

    let target = env::var("TARGET").unwrap_or_else(|_| "".to_owned());
    match target.as_str() {
        "x86_64-unknown-linux-gnu" | "aarch64-unknown-linux-gnu" => {
            file_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
                .join("bindings")
                .join(format!("{}-bindings.rs", target));
            if env::var("UPDATE_BIND")
                .map(|s| s.as_str() == "1")
                .unwrap_or(false)
            {
                bindgen_rocksdb(&file_path);
            }
        }
        _ => {
            file_path = PathBuf::from(env::var("OUT_DIR").unwrap()).join("rocksdb-bindings.rs");
            bindgen_rocksdb(&file_path);
        }
    };
    println!(
        "cargo:rustc-env=BINDING_PATH={}",
        file_path.to_str().unwrap()
    );
}

fn main() {
    println!("cargo:rerun-if-env-changed=UPDATE_BIND");

    let mut build = build_rocksdb();

    build.cpp(true).file("c.cc");
    if !cfg!(target_os = "windows") {
        build.flag("-std=c++11");
        build.flag("-fno-rtti");
    }
    link_cpp(&mut build);
    build.warnings(false).compile("libcrocksdb.a");
}

fn link_cpp(build: &mut Build) {
    let tool = build.get_compiler();
    let stdlib = if tool.is_like_gnu() {
        "libstdc++.a"
    } else if tool.is_like_clang() {
        "libc++.a"
    } else {
        // Don't link to c++ statically on windows.
        return;
    };
    let output = tool
        .to_command()
        .arg("--print-file-name")
        .arg(stdlib)
        .output()
        .unwrap();
    if !output.status.success() || output.stdout.is_empty() {
        // fallback to dynamically
        return;
    }
    let path = match str::from_utf8(&output.stdout) {
        Ok(path) => PathBuf::from(path),
        Err(_) => return,
    };
    if !path.is_absolute() {
        return;
    }
    // remove lib prefix and .a postfix.
    let libname = &stdlib[3..stdlib.len() - 2];
    // optional static linking
    if cfg!(feature = "static_libcpp") {
        println!("cargo:rustc-link-lib=static={}", &libname);
    } else {
        println!("cargo:rustc-link-lib=dylib={}", &libname);
    }
    println!(
        "cargo:rustc-link-search=native={}",
        path.parent().unwrap().display()
    );
    build.cpp_link_stdlib(None);
}

fn build_rocksdb() -> Build {
    let mut cfg = Config::new("");
    if cfg!(feature = "portable") {
        cfg.define("PORTABLE", "ON");
    }
    if cfg!(feature = "sse") {
        cfg.define("FORCE_SSE42", "ON");
    }
    let dst = cfg.build_target("rocksdb-util").very_verbose(true).build();
    let build_dir = format!("{}/build", dst.display());
    // let build_dir = format!("{}", dst.display());
    // println!("build dir: {}", build_dir);

    let mut build = Build::new();
    if cfg!(target_os = "windows") {
        let profile = match &*env::var("PROFILE").unwrap_or_else(|_| "debug".to_owned()) {
            "bench" | "release" => "Release",
            _ => "Debug",
        };
        println!("cargo:rustc-link-search=native={}/{}", build_dir, profile);
        build.define("OS_WIN", None);
    } else {
        println!("cargo:rustc-link-search=native={}", build_dir);
        build.define("ROCKSDB_PLATFORM_POSIX", None);
    }
    if cfg!(target_os = "macos") {
        build.define("OS_MACOSX", None);
    } else if cfg!(target_os = "freebsd") {
        build.define("OS_FREEBSD", None);
    }

    config_binding_path();

    let cur_dir = env::current_dir().unwrap();
    build.include(cur_dir.join("include"));
    build.include(cur_dir.clone());

    // Adding rocksdb specific compile macros.
    // TODO: should make sure crocksdb compile options is the same as rocksdb and titan.
    build.define("ROCKSDB_SUPPORT_THREAD_LOCAL", None);
    if cfg!(feature = "encryption") {
        build.define("OPENSSL", None);
    }

    println!("cargo:rustc-link-lib=static=rocksdb-util");
    println!("cargo:rerun-if-changed={}", cur_dir.display());
    build
}
