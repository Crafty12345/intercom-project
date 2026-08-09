# Intercom Project
This project is an experiment aimed at creating a home intercom system. The idea is that the server will receive raw audio data and play them, and the client will capture audio input, and send it to the server. It is expected that each intercom node will run both the client and the server, essentially acting as a two-way peer-to-peer connection.

# Config
## config.py
The config for the application is stored in `config.py`. Most of the configuration can be left as the default, so the only fields which need to be modified are:
- `INPUT_DEVICE`: The device for receiving microphone input. The value of this will depend on your system, but when running the program, it will output a list of possible devices.
- `OUTPUT_DEVICE`: Same as `INPUT_DEVICE`, except for audio output from the server, instead of input.
- `BIND_IP`: Generally speaking, this should be set to your local IP address. More specifically, it should be set to the local IP address of the device which the application is running on.
- `PORT`: This refers to the port which the application will run on. To keep config files simple, this should be the same for all intercom nodes, however this may be changed in future.
## targets.json
**Please note that this file is not committed to Github for privacy reasons, so you'll have to create it yourself.**
`targets.json` is a JSON file which sets a key-value mapping of a keyboard key to an IP address. For example, if you have four intercom nodes set up in your house, your `targets.json` may look something like this:
```json
{
    "1": "192.168.0.2",
    "2": "192.168.0.3",
    "3": "192.168.1.10",
    "4": "192.168.1.11"
}
```
Please once again note that the IP addresses will be different depending on your setup.

# Usage

Before running the application, **please** make sure that you have configured the application correctly, as per [config](#config).

In order for intercom nodes to be able to communicate with each other, make sure that the port defined in [config](#config) has been opened on each device running the application.

It is expected that [uv](https://docs.astral.sh/uv/) is used to run and set up the project, as it automatically handles package installation. Thus, make sure to install [uv](https://docs.astral.sh/uv/) before attempting to run the application.

Once the above steps have been completed, you can now run the program:
```bash
uv run intercom.py
```
Once started, hold down the key corresponding to the IP address of the intercom node that you would like to communicate with, in order to send audio data to it.
A simple keyboard interrupt (`Ctrl + C` on most systems) can be used to stop the program.