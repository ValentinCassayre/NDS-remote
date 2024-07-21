import socket
import struct


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


def int_to_bools(key_press):
    binary_string = "{0:016b}".format(key_press)[::-1]
    key_press_list = [digit == "1" for digit in binary_string]

    print(binary_string)
    return key_press_list


def num_to_action_gamepad(key_press):
    import vgamepad as vg

    key_press_list = int_to_bools(key_press)

    gamepad = vg.VX360Gamepad()

    actions = [None] * 16
    actions[0] = vg.XUSB_BUTTON.XUSB_GAMEPAD_A  # A
    actions[1] = vg.XUSB_BUTTON.XUSB_GAMEPAD_B  # B
    actions[2] = vg.XUSB_BUTTON.XUSB_GAMEPAD_START  # start
    actions[3] = vg.XUSB_BUTTON.XUSB_GAMEPAD_BACK  # select
    actions[4] = vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_RIGHT  # ->
    actions[5] = vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_LEFT  # <-
    actions[6] = vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_UP  # /\
    actions[7] = vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_DOWN  # \/
    actions[8] = vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER  # L
    actions[9] = vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER  # R
    actions[10] = vg.XUSB_BUTTON.XUSB_GAMEPAD_X  # X
    actions[11] = vg.XUSB_BUTTON.XUSB_GAMEPAD_Y  # Y
    actions[12] = vg.XUSB_BUTTON.XUSB_GAMEPAD_GUIDE  # click on screen

    for index, control in enumerate(key_press_list):
        if actions[index] is None:
            continue
        if control:
            gamepad.press_button(button=actions[index])
        else:
            gamepad.release_button(button=actions[index])

    gamepad.update()


def num_to_action_keyboard(key_press):
    import pyautogui

    key_press_list = int_to_bools(key_press)

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
    actions[12] = pyautogui.click(button="left")  # click on screen

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
        num_to_action_gamepad(key_press)

    conn.close()


if __name__ == "__main__":
    ip = get_private_ip()
    print(f"Private IP Address: {ip}")
    receive_from_ds(ip, PORT)
