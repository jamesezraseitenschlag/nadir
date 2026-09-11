"""
Test Client to verify the Test MySQL Server.
Connects via standard socket / MySQL wire protocol to localhost:3306.
"""

import socket
import struct
import sys

def test_mysql_connection(host="127.0.0.1", port=3306):
    print(f"Connecting to Test MySQL Server at {host}:{port}...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # 1. Read Handshake
    header = s.recv(4)
    payload_len = header[0] | (header[1] << 8) | (header[2] << 16)
    handshake = s.recv(payload_len)
    print(f"[OK] Received Handshake from Server: {handshake[1:20].decode('latin1', errors='ignore')}")

    # 2. Send Handshake Response (User: root, DB: test)
    resp = bytearray()
    # Client capabilities (4 bytes)
    resp.extend(struct.pack("<I", 0x80000200))
    # Max packet size (4 bytes)
    resp.extend(struct.pack("<I", 1024 * 1024))
    # Charset (1 byte)
    resp.append(33)
    # Reserved (23 bytes)
    resp.extend(b"\x00" * 23)
    # Username (null-terminated)
    resp.extend(b"root\x00")
    # Auth response length + data
    resp.append(0)

    packet_len = len(resp)
    header = struct.pack("<I", packet_len)[:3] + struct.pack("<B", 1)
    s.sendall(header + resp)

    # 3. Read Auth OK
    ok_header = s.recv(4)
    ok_payload_len = ok_header[0] | (ok_header[1] << 8) | (ok_header[2] << 16)
    ok_packet = s.recv(ok_payload_len)
    if ok_packet[0] == 0x00:
        print("[OK] Authenticated successfully!")

    def send_query(sql):
        q_bytes = b"\x03" + sql.encode("utf-8")
        h = struct.pack("<I", len(q_bytes))[:3] + struct.pack("<B", 0)
        s.sendall(h + q_bytes)

        res_hdr = s.recv(4)
        plen = res_hdr[0] | (res_hdr[1] << 8) | (res_hdr[2] << 16)
        p = s.recv(plen)
        if p[0] == 0x00:
            print(f"[Query OK] {sql}")
        elif p[0] == 0xFF:
            print(f"[Query Error] {p[3:].decode('utf-8')}")
        else:
            # Result set
            col_count = p[0]
            print(f"[Query Results ({col_count} columns)] {sql}")
            # Read columns
            for _ in range(col_count):
                ch = s.recv(4)
                s.recv(ch[0] | (ch[1] << 8) | (ch[2] << 16))
            # Read EOF
            eh = s.recv(4)
            s.recv(eh[0] | (eh[1] << 8) | (eh[2] << 16))
            # Read rows until EOF
            while True:
                rh = s.recv(4)
                rlen = rh[0] | (rh[1] << 8) | (rh[2] << 16)
                row_data = s.recv(rlen)
                if row_data[0] == 0xFE:
                    break
                print(f"  Row Data: {row_data}")

    # Test queries
    send_query("INSERT OR REPLACE INTO Account (Id, Name, Industry, AnnualRevenue) VALUES ('001000000001AAA', 'TechCorp Inc.', 'Technology', 5000000.00)")
    send_query("SELECT Id, Name, Industry FROM Account")

    s.close()
    print("[OK] Test finished successfully.")

if __name__ == "__main__":
    test_mysql_connection()
