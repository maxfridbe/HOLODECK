//! The 8-byte message header shared with the C# server:
//! `i16 request, i16 specific, i32 size`, little-endian.

pub const HEADER_SIZE: usize = 8;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum RequestType {
    None,
    Connect,
    Disconnect,
    Error,
    Position,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum RequestSpecific {
    None,
    TooManyClients,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Header {
    pub request: RequestType,
    pub specific: RequestSpecific,
    /// Bytes of payload following the header.
    pub size: i32,
}

impl Header {
    pub fn new(request: RequestType) -> Self {
        Self { request, specific: RequestSpecific::None, size: 0 }
    }

    pub fn encode(&self) -> [u8; HEADER_SIZE] {
        let mut out = [0u8; HEADER_SIZE];
        out[0..2].copy_from_slice(&(self.request as i16).to_le_bytes());
        out[2..4].copy_from_slice(&(self.specific as i16).to_le_bytes());
        out[4..8].copy_from_slice(&self.size.to_le_bytes());
        out
    }

    /// `None` if the request or specific code is unknown.
    pub fn decode(bytes: &[u8; HEADER_SIZE]) -> Option<Self> {
        let request = match i16::from_le_bytes([bytes[0], bytes[1]]) {
            0 => RequestType::None,
            1 => RequestType::Connect,
            2 => RequestType::Disconnect,
            3 => RequestType::Error,
            4 => RequestType::Position,
            _ => return None,
        };
        let specific = match i16::from_le_bytes([bytes[2], bytes[3]]) {
            0 => RequestSpecific::None,
            1 => RequestSpecific::TooManyClients,
            _ => return None,
        };
        Some(Self { request, specific, size: i32::from_le_bytes([bytes[4], bytes[5], bytes[6], bytes[7]]) })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn connect_header_matches_the_wire_format_the_server_expects() {
        // request=1 (Connect), specific=0, size=0.
        assert_eq!(Header::new(RequestType::Connect).encode(), [1, 0, 0, 0, 0, 0, 0, 0]);
    }

    #[test]
    fn round_trips() {
        let h = Header { request: RequestType::Error, specific: RequestSpecific::TooManyClients, size: 24 };
        assert_eq!(Header::decode(&h.encode()), Some(h));
        assert_eq!(h.encode(), [3, 0, 1, 0, 24, 0, 0, 0]);
    }

    #[test]
    fn unknown_codes_are_rejected() {
        assert_eq!(Header::decode(&[9, 0, 0, 0, 0, 0, 0, 0]), None);
        assert_eq!(Header::decode(&[1, 0, 7, 0, 0, 0, 0, 0]), None);
    }
}
