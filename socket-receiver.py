import socket
import struct
import pyautogui

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


def num_to_action(key_press):
    binary_string = "{0:016b}".format(key_press)[::-1]
    key_press_list = [digit == "1" for digit in binary_string]
    print(binary_string)
    actions = [None] * 16
    actions[0] = pyautogui.press("a")  # A
    actions[1] = pyautogui.press("b")  # B
    actions[2] = pyautogui.press("enter")  # start
    actions[3] = pyautogui.press("escape")  # select
    actions[4] = pyautogui.press("right")  # ->
    actions[5] = pyautogui.press("left")  # <-
    actions[6] = pyautogui.press("up")  # /\
    actions[7] = pyautogui.press("down")  # \/
    actions[8] = pyautogui.press("l")  # L
    actions[9] = pyautogui.press("r")  # R
    actions[10] = pyautogui.press("x")  # X
    actions[11] = pyautogui.press("y")  # Y
    actions[12] = pyautogui.click(button="left")

    for index, control in enumerate(key_press_list):
        if control and actions[index]:
            actions[index]()


def receive_from_ds(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((ip, port))
    s.listen(1)
    print(f"Listening on {ip}:{port}")

    conn, addr = s.accept()
    print(f"Connection from: {addr}")

    while True:
        data = conn.recv(4)  # Expecting 4 bytes for an unsigned int
        if not data:
            break
        # Unpack the received bytes into an integer
        key_press = struct.unpack("<I", data)[
            0
        ]  # Use '<I' for little-endian, 'I' is for unsigned int
        print(f"Received: {key_press}")
        num_to_action(key_press)

    conn.close()


if __name__ == "__main__":
    ip = get_private_ip()
    print(f"Private IP Address: {ip}")
    receive_from_ds(ip, PORT)
