# Smart TV IR Control for Flipper Zero

This application allows you to control your Smart TV using the Flipper Zero's jog wheel. The Smart TV IR Control app sends infrared signals to your TV, enabling you to perform various actions such as changing channels, adjusting the volume, and navigating menus.

## Features

- **Channel Control**: Change channels up and down.
- **Volume Control**: Increase or decrease the volume.
- **Menu Navigation**: Navigate through the TV's menu options.
- **Power Control**: Turn the TV on or off.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/meirm/flip-smart-tv.git
    ```
2. Navigate to the user's application directory on your firmware and copy the content of the repo into "smart_tv"
    ```sh
    cd flipperzero-firmware/applications_user/
    ```
3. Build and upload the firmware to your Flipper Zero:
    ```sh
    ./fbt flash_app
    ```

## Usage

1. Navigate to the Smart TV IR Control app on your Flipper Zero.
2. Use the jog wheel to select the desired function (e.g., volume up, channel down).
3. Press the center button to send the IR signal to your TV.

## Supported Devices

This application supports a wide range of Smart TVs. However, compatibility may vary depending on the TV model and manufacturer.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the GPL 2.0 License. See the [LICENSE](../LICENSE) file for details.

## Acknowledgements

Special thanks to the Flipper Zero community for their support and contributions.
