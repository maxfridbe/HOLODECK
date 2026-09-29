//! Client for the multi-user world server (TCP, port 1307).
//!
//! Connecting happens on a background thread so the UI never freezes. The
//! browser has no raw sockets, so there connecting reports an error.

mod header;

use std::sync::Mutex;
use std::sync::mpsc::{Receiver, channel};

use bevy::prelude::*;

pub use header::{HEADER_SIZE, Header, RequestSpecific, RequestType};

use crate::ui3d::actions::ShowMessage;

pub const DEFAULT_PORT: u16 = 1307;

/// Asks to connect to a server.
#[derive(Event)]
pub struct ConnectRequest {
    pub host: String,
    pub port: u16,
}

#[derive(Debug, PartialEq, Eq)]
#[cfg_attr(target_arch = "wasm32", allow(dead_code))]
enum Notice {
    Connected,
    Failed(String),
    Rejected(RequestSpecific),
    Disconnected,
}

#[derive(Resource, Default)]
struct Client {
    notices: Option<Mutex<Receiver<Notice>>>,
    connected: bool,
}

pub struct NetPlugin;

impl Plugin for NetPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<Client>().add_event::<ConnectRequest>().add_systems(Update, (start_connections, poll_notices));
    }
}

fn start_connections(mut requests: EventReader<ConnectRequest>, mut client: ResMut<Client>, mut messages: EventWriter<ShowMessage>) {
    for request in requests.read() {
        #[cfg(not(target_arch = "wasm32"))]
        {
            let (tx, rx) = channel();
            client.notices = Some(Mutex::new(rx));
            client.connected = false;
            let (host, port) = (request.host.clone(), request.port);
            std::thread::spawn(move || run_connection(&host, port, &tx));
        }
        #[cfg(target_arch = "wasm32")]
        {
            let _ = (&mut client, request, channel::<Notice>);
            messages.send(ShowMessage::new("Network", "Networking is not available in the browser."));
        }
    }
    let _ = &mut messages;
}

#[cfg(not(target_arch = "wasm32"))]
fn run_connection(host: &str, port: u16, tx: &std::sync::mpsc::Sender<Notice>) {
    use std::io::{Read, Write};
    use std::net::{TcpStream, ToSocketAddrs};
    use std::time::Duration;

    let address = match (host, port).to_socket_addrs().ok().and_then(|mut a| a.next()) {
        Some(a) => a,
        None => return drop(tx.send(Notice::Failed(format!("Could not resolve '{host}'.")))),
    };
    let mut stream = match TcpStream::connect_timeout(&address, Duration::from_secs(5)) {
        Ok(s) => s,
        Err(e) => return drop(tx.send(Notice::Failed(format!("Could not connect to {address}: {e}")))),
    };
    if let Err(e) = stream.write_all(&Header::new(RequestType::Connect).encode()) {
        return drop(tx.send(Notice::Failed(format!("Could not talk to the server: {e}"))));
    }

    let mut buffer = [0u8; HEADER_SIZE];
    while stream.read_exact(&mut buffer).is_ok() {
        let notice = match Header::decode(&buffer) {
            Some(Header { request: RequestType::Connect, .. }) => Notice::Connected,
            Some(Header { request: RequestType::Error, specific, .. }) => Notice::Rejected(specific),
            _ => continue,
        };
        if tx.send(notice).is_err() {
            return;
        }
    }
    let _ = tx.send(Notice::Disconnected);
}

fn poll_notices(mut client: ResMut<Client>, mut messages: EventWriter<ShowMessage>) {
    let Some(notices) = &client.notices else { return };
    let received: Vec<Notice> = notices.lock().map(|rx| rx.try_iter().collect()).unwrap_or_default();
    for notice in received {
        match notice {
            Notice::Connected => {
                client.connected = true;
                messages.send(ShowMessage::new("Network", "Connected to the server."));
            }
            Notice::Failed(reason) => {
                messages.send(ShowMessage::new("Error", reason));
            }
            Notice::Rejected(RequestSpecific::TooManyClients) => {
                messages.send(ShowMessage::new("Error", "The server is full."));
            }
            Notice::Rejected(RequestSpecific::None) => {
                messages.send(ShowMessage::new("Error", "The server refused the connection."));
            }
            Notice::Disconnected => {
                if std::mem::take(&mut client.connected) {
                    messages.send(ShowMessage::new("Network", "Disconnected from the server."));
                }
            }
        }
    }
}

#[cfg(all(test, not(target_arch = "wasm32")))]
mod tests {
    use super::*;
    use std::io::{Read, Write};
    use std::net::TcpListener;

    #[test]
    fn connects_sends_a_connect_header_and_sees_the_ack() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let port = listener.local_addr().unwrap().port();
        let server = std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            let mut buf = [0u8; HEADER_SIZE];
            socket.read_exact(&mut buf).unwrap();
            let request = Header::decode(&buf).unwrap();
            socket.write_all(&Header::new(RequestType::Connect).encode()).unwrap();
            request
        });

        let (tx, rx) = channel();
        std::thread::spawn(move || run_connection("127.0.0.1", port, &tx));
        assert_eq!(rx.recv_timeout(std::time::Duration::from_secs(5)).unwrap(), Notice::Connected);
        assert_eq!(server.join().unwrap().request, RequestType::Connect);
        // Server closed: we notice.
        assert_eq!(rx.recv_timeout(std::time::Duration::from_secs(5)).unwrap(), Notice::Disconnected);
    }

    #[test]
    fn a_full_server_is_reported() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let port = listener.local_addr().unwrap().port();
        std::thread::spawn(move || {
            let (mut socket, _) = listener.accept().unwrap();
            let full = Header { request: RequestType::Error, specific: RequestSpecific::TooManyClients, size: 0 };
            socket.write_all(&full.encode()).unwrap();
        });
        let (tx, rx) = channel();
        std::thread::spawn(move || run_connection("127.0.0.1", port, &tx));
        assert_eq!(rx.recv_timeout(std::time::Duration::from_secs(5)).unwrap(), Notice::Rejected(RequestSpecific::TooManyClients));
    }

    #[test]
    fn refused_connections_report_a_failure() {
        let port = {
            let l = TcpListener::bind("127.0.0.1:0").unwrap();
            l.local_addr().unwrap().port()
        };
        let (tx, rx) = channel();
        run_connection("127.0.0.1", port, &tx);
        assert!(matches!(rx.recv().unwrap(), Notice::Failed(_)));
    }
}
