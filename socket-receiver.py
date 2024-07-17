import socket

PORT = 8888


def get_private_ip():
    try:
        # Connect to an external host, but doesn't actually send data
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        private_ip = s.getsockname()[0]
        s.close()
        return private_ip

    except Exception as e:
        print(f"Error: {e}")
        return None


def receive_from_ds(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((ip, port))
    s.listen(1)
    print(f"Listening on {ip}:{port}")

    conn, addr = s.accept()
    print(f"Connection from: {addr}")

    while True:
        data = conn.recv(1024)
        if not data:
            break
        print(f"Received: {data.decode()}")

    conn.close()


if __name__ == "__main__":
    ip = get_private_ip()
    print(f"Private IP Address: {ip}")
    receive_from_ds("0.0.0.0", PORT)
