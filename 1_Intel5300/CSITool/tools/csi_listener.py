import re
import subprocess
import time
from typing import List


def get_wireless_adapters() -> List[str]:
    output = subprocess.check_output(['ip', '-o', 'link', 'show']).decode()
    adapters = re.findall(r'\d+: (\w+):', output)
    wireless_adapters = [adapter for adapter in adapters if adapter != 'lo']
    return wireless_adapters


def _get_mode(interface: str) -> str:
    iw_config_output = subprocess.check_output(['iwconfig', interface]).decode()
    mode = re.search(r'Mode:(\w+)', iw_config_output).group(1)
    return mode


def _change_mode(interface: str, mode: str):
    subprocess.run(['sudo', 'ifconfig', interface, 'down'])
    subprocess.run(['sudo', 'iwconfig', interface, 'mode', mode])
    subprocess.run(['sudo', 'ifconfig', interface, 'up'])


def set_monitor_mode(interface: str):

    # Attempt to change the mode of the interface to monitor mode, at least 5 times
    for i in range(5):

        # Get the current mode of the interface
        mode = _get_mode(interface)

        # Print out the current mode of the interface
        if mode == "Managed":
            print(f"Interface {interface} is in managed mode")
        elif mode == "Ad-Hoc":
            print(f"Interface {interface} is in ad-hoc mode")
        elif mode == "Master":
            print(f"Interface {interface} is in master mode")
        elif mode == "Repeater":
            print(f"Interface {interface} is in repeater mode")
        elif mode == "Secondary":
            print(f"Interface {interface} is in secondary mode")
        elif mode == "Monitor":
            print(f"Interface {interface} is already in monitor mode")
            return
        
        # If the interface is not in monitor mode, attempt to change it
        if mode != "Monitor":
            print(f"Attempting to change interface {interface} to monitor mode")
            _change_mode(interface, "monitor")

        # Wait for the interface to change mode for 1 second
        time.sleep(1)

    # It is possible that the interface is not able to change to monitor mode
    print(f"Unable to change interface {interface} to monitor mode...")
    print("Exiting...")

    # Exit the program
    exit()


def log_csi_data():
    print("Starting CSI logging...")
    subprocess.run(['sudo', 'modprobe', '-r', 'iwlwifi', 'mac80211'])
    subprocess.run(['sudo', 'modprobe', 'iwlwifi', 'connector_log=0x1'])
    subprocess.run(['sudo', './netlink/log_to_file', 'csi.dat'])


def transmit_csi():
    print("Starting CSI logging...")
    subprocess.run(['sudo', 'modprobe', '-r', 'iwlwifi', 'mac80211'])
    subprocess.run(['sudo', 'modprobe', 'iwlwifi', 'connector_log=0x1'])
    subprocess.run(['sudo', './netlink/log_to_client'])


def main():
    wireless_adapters = get_wireless_adapters()

    if len(wireless_adapters) == 0:
        print("No wireless adapters found.")
        return

    if len(wireless_adapters) == 1:
        interface = wireless_adapters[0]
        print(f"Using wireless adapter: {interface}")
    else:
        print("Multiple wireless adapters found. Please choose one:")
        for index, adapter in enumerate(wireless_adapters):
            print(f"{index + 1}. {adapter}")

        choice = int(input("Enter the number of the adapter you want to use: "))
        interface = wireless_adapters[choice - 1]
        print(f"Using wireless adapter: {interface}")

    set_monitor_mode(interface)

    # Let user choose whether to log CSI data to file or transmit it
    print("Do you want to log CSI data to file or transmit it to a server?")
    print("1. Log to file")
    print("2. Transmit")
    choice = int(input("Enter the number of your choice: "))

    if choice == 1:
        log_csi_data()
    elif choice == 2:
        transmit_csi()
    else:
        print("Invalid choice. Exiting...")


if __name__ == "__main__":
    main()