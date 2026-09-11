"""
Lightweight Mock MySQL Server in Python with SQLite Backend
Implements the MySQL Wire Protocol (Handshake, HandshakeResponse, COM_QUERY, ResultSets).
Allows connecting via any MySQL client or library to localhost:3306 (or configured port).
"""

import socket
import struct
import sqlite3
import threading
import sys
import os

class MockMySQLServer:
    def __init__(self, host="127.0.0.1", port=3306, db_path="salesforce_mock.db"):
        self.host = host
        self.port = port
        self.db_path = db_path
        self.running = False
        self.server_sock = None
        self._init_sqlite()

    def _init_sqlite(self):
        conn = sqlite3.connect(self.db_path)
        cur = conn.cursor()
        # Create standard Salesforce tables
        cur.execute("""
            CREATE TABLE IF NOT EXISTS Account (
                Id TEXT PRIMARY KEY,
                Name TEXT,
                Industry TEXT,
                AnnualRevenue REAL,
                CreatedDate TEXT
            )
        """)
        cur.execute("""
            CREATE TABLE IF NOT EXISTS Contact (
                Id TEXT PRIMARY KEY,
                FirstName TEXT,
                LastName TEXT,
                Email TEXT,
                AccountId TEXT,
                CreatedDate TEXT,
                FOREIGN KEY(AccountId) REFERENCES Account(Id)
            )
        """)
        conn.commit()
        conn.close()

    def _make_packet(self, seq, payload):
        length = len(payload)
        header = struct.pack("<I", length)[:3] + struct.pack("<B", seq)
        return header + payload

    def _send_handshake(self, client_sock):
        # MySQL Handshake V10 Packet
        payload = bytearray()
        payload.append(10)  # Protocol version
        payload.extend(b"8.0.32-MockMySQL\x00")  # Server version
        payload.extend(struct.pack("<I", 1))    # Connection ID
        payload.extend(b"12345678")             # Auth plugin data part 1
        payload.append(0)                       # Filter
        # Capability flags (lower 2 bytes)
        payload.extend(struct.pack("<H", 0xF7FF))
        payload.append(33)                      # Character set utf8_general_ci
        payload.extend(struct.pack("<H", 0x0002)) # Server status: autocommit
        # Capability flags (upper 2 bytes)
        payload.extend(struct.pack("<H", 0x81FF))
        payload.append(21)                      # Auth data len
        payload.extend(b"\x00" * 10)            # Reserved
        payload.extend(b"123456789012\x00")     # Auth plugin data part 2
        payload.extend(b"mysql_native_password\x00")

        client_sock.sendall(self._make_packet(0, payload))

    def _send_ok_packet(self, client_sock, seq, affected_rows=0, last_insert_id=0):
        payload = bytearray([0x00])  # OK header
        # Length-encoded integer for affected_rows
        payload.append(affected_rows & 0xFF)
        payload.append(last_insert_id & 0xFF)
        payload.extend(struct.pack("<H", 0x0002))  # Status flags
        payload.extend(struct.pack("<H", 0x0000))  # Warnings
        client_sock.sendall(self._make_packet(seq, payload))

    def _send_error_packet(self, client_sock, seq, err_msg):
        payload = bytearray([0xFF])
        payload.extend(struct.pack("<H", 1064))  # Error code
        payload.extend(b"#42000")                # SQL state
        payload.extend(err_msg.encode("utf-8"))
        client_sock.sendall(self._make_packet(seq, payload))

    def _send_result_set(self, client_sock, seq, columns, rows):
        # 1. Column count packet
        col_count_payload = struct.pack("<B", len(columns))
        client_sock.sendall(self._make_packet(seq, col_count_payload))
        seq += 1

        # 2. Column definition packets
        for col_name in columns:
            col_def = bytearray()
            col_def.extend(b"\x03def")           # Catalog
            col_def.extend(b"\x00")              # Schema
            col_def.extend(b"\x00")              # Table
            col_def.extend(b"\x00")              # Org table
            # Column name
            col_def.append(len(col_name))
            col_def.extend(col_name.encode("utf-8"))
            # Org column name
            col_def.append(len(col_name))
            col_def.extend(col_name.encode("utf-8"))
            col_def.append(0x0C)                 # Length of fixed fields
            col_def.extend(struct.pack("<H", 33)) # Charset
            col_def.extend(struct.pack("<I", 255)) # Column length
            col_def.append(0xFD)                 # Type: VAR_STRING
            col_def.extend(struct.pack("<H", 0)) # Flags
            col_def.append(0)                    # Decimals
            col_def.extend(b"\x00\x00")          # Reserved

            client_sock.sendall(self._make_packet(seq, col_def))
            seq += 1

        # 3. EOF packet after columns
        eof_payload = bytearray([0xFE, 0x00, 0x00, 0x02, 0x00])
        client_sock.sendall(self._make_packet(seq, eof_payload))
        seq += 1

        # 4. Row packets
        for row in rows:
            row_payload = bytearray()
            for val in row:
                if val is None:
                    row_payload.append(0xFB)
                else:
                    s_val = str(val).encode("utf-8")
                    row_payload.append(len(s_val))
                    row_payload.extend(s_val)
            client_sock.sendall(self._make_packet(seq, row_payload))
            seq += 1

        # 5. EOF packet after rows
        eof_payload = bytearray([0xFE, 0x00, 0x00, 0x02, 0x00])
        client_sock.sendall(self._make_packet(seq, eof_payload))

    def _handle_client(self, client_sock, addr):
        print(f"[*] MySQL Client connected from {addr[0]}:{addr[1]}")
        try:
            self._send_handshake(client_sock)

            # Receive Auth Response
            data = client_sock.recv(1024)
            if not data:
                return

            # Send Auth OK
            self._send_ok_packet(client_sock, 2)

            conn = sqlite3.connect(self.db_path)
            conn.row_factory = sqlite3.Row

            while self.running:
                header = client_sock.recv(4)
                if not header or len(header) < 4:
                    break
                payload_len = header[0] | (header[1] << 8) | (header[2] << 16)
                seq = header[3]

                payload = bytearray()
                while len(payload) < payload_len:
                    chunk = client_sock.recv(payload_len - len(payload))
                    if not chunk:
                        break
                    payload.extend(chunk)

                if not payload:
                    break

                cmd = payload[0]
                if cmd == 0x01:  # COM_QUIT
                    break
                elif cmd == 0x03:  # COM_QUERY
                    query = payload[1:].decode("utf-8", errors="ignore").strip()
                    print(f"[SQL Query] {query}")

                    # Handle special internal commands
                    q_lower = query.lower()
                    if "select @@version" in q_lower or "select version()" in q_lower:
                        self._send_result_set(client_sock, seq + 1, ["@@version"], [("8.0.32-MockMySQL",)])
                    elif q_lower.startswith("set ") or q_lower.startswith("use ") or q_lower.startswith("rollback") or q_lower.startswith("commit"):
                        self._send_ok_packet(client_sock, seq + 1)
                    else:
                        cur = conn.cursor()
                        try:
                            cur.execute(query)
                            if cur.description:
                                cols = [col[0] for col in cur.description]
                                rows = cur.fetchall()
                                self._send_result_set(client_sock, seq + 1, cols, rows)
                            else:
                                conn.commit()
                                self._send_ok_packet(client_sock, seq + 1, affected_rows=cur.rowcount)
                        except Exception as e:
                            self._send_error_packet(client_sock, seq + 1, str(e))
                else:
                    self._send_ok_packet(client_sock, seq + 1)

            conn.close()
        except Exception as e:
            print(f"[-] Client connection error: {e}")
        finally:
            client_sock.close()
            print(f"[*] Client disconnected: {addr[0]}:{addr[1]}")

    def start(self):
        self.running = True
        self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_sock.bind((self.host, self.port))
        self.server_sock.listen(5)
        print(f"==================================================")
        print(f"  Test MySQL Server started on {self.host}:{self.port} ")
        print(f"  SQLite Storage: {self.db_path}                  ")
        print(f"==================================================")

        while self.running:
            try:
                client_sock, addr = self.server_sock.accept()
                t = threading.Thread(target=self._handle_client, args=(client_sock, addr), daemon=True)
                t.start()
            except Exception:
                break

    def stop(self):
        self.running = False
        if self.server_sock:
            self.server_sock.close()

if __name__ == "__main__":
    port = 3306
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    server = MockMySQLServer(port=port)
    try:
        server.start()
    except KeyboardInterrupt:
        server.stop()
        print("Server stopped.")
