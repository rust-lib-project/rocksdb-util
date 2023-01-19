extern crate libc;
use libc::{c_char, c_double, c_float, c_int, c_uchar, c_void, size_t};
#[repr(C)]
pub struct FilterBitsBuilder(c_void);
extern "C" {
    fn rocksdb_filterbits_builder_destroy(builder: *mut FilterBitsBuilder);
    fn rocksdb_ribbon_filterbits_builder_create(bits_per_key: c_double) -> *mut FilterBitsBuilder;
    fn rocksdb_fast_bloom_filter_builder_create(bits_per_key: c_double) -> *mut FilterBitsBuilder;
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        let result = 2 + 2;
        assert_eq!(result, 4);
    }
}
