extern crate libc;
use libc::{c_char, c_void, size_t};
#[repr(C)]
pub struct FilterBitsBuilder(c_void);
#[repr(C)]
pub struct FilterBitsReader(c_void);

extern "C" {
    fn rocksdb_filterbits_builder_destroy(builder: *mut FilterBitsBuilder);
    fn rocksdb_ribbon_filterbits_builder_create(bits_per_key: f64) -> *mut FilterBitsBuilder;
    fn rocksdb_fast_bloom_filter_builder_create(bits_per_key: f64) -> *mut FilterBitsBuilder;
    fn rocksdb_filterbits_builder_add_key(
        builder: *mut FilterBitsBuilder,
        key: *const c_char,
        key_len: size_t,
    );
    fn rocksdb_filterbits_builder_add_key_hash(builder: *mut FilterBitsBuilder, hash: u64);
    fn rocksdb_filterbits_builder_finish(
        builder: *mut FilterBitsBuilder,
        buf_len: *mut size_t,
    ) -> *const c_char;
    fn rocksdb_filterbits_reader_destroy(filter: *mut FilterBitsReader);
    fn rocksdb_filterbits_reader_create(
        data: *const c_char,
        data_len: size_t,
    ) -> *mut FilterBitsReader;
    fn rocksdb_filterbits_reader_may_match(
        reader: *const FilterBitsReader,
        data: *const c_char,
        key_len: size_t,
    ) -> bool;
    fn rocksdb_filterbits_reader_may_match_hash(
        reader: *const FilterBitsReader,
        value: u64,
    ) -> bool;
}

pub enum FilterType {
    FastLocalBloom,
    Ribbon,
}

pub struct FilterBitsBuilderWrapper {
    inner: *mut FilterBitsBuilder,
}

impl FilterBitsBuilderWrapper {
    pub fn create(bits_per_key: f64, tp: FilterType) -> Self {
        unsafe {
            let inner = match tp {
                FilterType::FastLocalBloom => {
                    rocksdb_fast_bloom_filter_builder_create(bits_per_key)
                }
                FilterType::Ribbon => rocksdb_ribbon_filterbits_builder_create(bits_per_key),
            };
            FilterBitsBuilderWrapper { inner }
        }
    }

    pub fn add_key(&mut self, key: &[u8]) {
        unsafe {
            rocksdb_filterbits_builder_add_key(self.inner, key.as_ptr() as _, key.len() as size_t);
        }
    }

    pub fn add_key_hash(&mut self, hash: u64) {
        unsafe {
            rocksdb_filterbits_builder_add_key_hash(self.inner, hash);
        }
    }

    pub fn finish(self) -> Vec<u8> {
        unsafe {
            let mut buf_len: size_t = 0;
            let data = rocksdb_filterbits_builder_finish(self.inner, &mut buf_len);
            Vec::from_raw_parts(data as *mut u8, buf_len, buf_len)
        }
    }
}

impl Drop for FilterBitsBuilderWrapper {
    fn drop(&mut self) {
        unsafe {
            rocksdb_filterbits_builder_destroy(self.inner);
        }
    }
}

pub struct FilterBitsReaderWrapper {
    _data: Vec<u8>,
    inner: *mut FilterBitsReader,
}

impl FilterBitsReaderWrapper {
    pub fn create(data: Vec<u8>) -> Self {
        unsafe {
            let inner =
                rocksdb_filterbits_reader_create(data.as_ptr() as *const c_char, data.len());
            FilterBitsReaderWrapper { inner, _data: data }
        }
    }

    pub fn may_match(&self, key: &[u8]) -> bool {
        unsafe {
            rocksdb_filterbits_reader_may_match(
                self.inner,
                key.as_ptr() as *const c_char,
                key.len(),
            )
        }
    }

    pub fn hash_may_match(&self, key: u64) -> bool {
        unsafe { rocksdb_filterbits_reader_may_match_hash(self.inner, key) }
    }
}

unsafe impl Send for FilterBitsReaderWrapper {}
unsafe impl Sync for FilterBitsReaderWrapper {}

impl Drop for FilterBitsReaderWrapper {
    fn drop(&mut self) {
        unsafe {
            rocksdb_filterbits_reader_destroy(self.inner);
        }
    }
}

#[cfg(test)]
mod tests {
    use FilterBitsReaderWrapper;
    use {FilterBitsBuilderWrapper, FilterType};

    #[test]
    fn basic_test() {
        let mut builder = FilterBitsBuilderWrapper::create(10.0, FilterType::Ribbon);
        builder.add_key(b"abcd");
        builder.add_key(b"cdef");
        let d = builder.finish();
        println!("{}", d.len());
        let reader = FilterBitsReaderWrapper::create(d);
        assert!(reader.may_match(b"abcd"));
        assert!(reader.may_match(b"cdef"));
        assert!(!reader.may_match(b"aaaa"));
    }
}
