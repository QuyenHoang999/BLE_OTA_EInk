# CrossPoint-Equivalent User Guide for XG24 Dev Kit

Welcome to the XG24 dev kit reader firmware guide. This document migrates the
CrossPoint user guide to the Silicon Labs EFR32MG24 / BRD2601B XG24 dev kit
target and keeps the intended feature set aligned with CrossPoint firmware.

Current project note: the checked-in `BLE_OTA_EInk` firmware currently provides
BLE advertising and application-level OTA firmware update plumbing. The
CrossPoint reader application features below are the product target for this XG24
firmware and should be preserved as implementation work continues.

## 1. Hardware Overview

### Target Board

| Item             | Value                             |
| ---------------- | --------------------------------- |
| MCU / board      | EFR32MG24 / BRD2601B XG24 dev kit |
| Logic level      | 3.3 V                             |
| SDK              | Simplicity SDK 2025.6.2           |
| Toolchain        | GNU ARM v12.2.1                   |
| Project          | `BLE_OTA_EInk.slcp`               |
| Current BLE name | `BLE OTA EInk`                    |

Use 3.3 V logic for all GPIO connections. Do not connect 5 V logic directly to
EFR32MG24 GPIO pins.

### Button Layout

The CrossPoint source firmware uses a device with Back, Confirm, Left, Right,
Power, Volume Up, Volume Down, and Reset controls. The XG24 dev kit product
should preserve those same user-facing actions through the generated button
instances below.

| CrossPoint action       | XG24 firmware name | Suggested GPIO | Notes                                                  |
| ----------------------- | ------------------ | -------------- | ------------------------------------------------------ |
| Power / sleep / wake    | `button_power`     | `PB1`          | EM4 wake capable. Best choice for wake/power behavior. |
| Left / previous page    | `button_page_prev` | `PD0`          | Side or front page button.                             |
| Right / next page       | `button_page_next` | `PD1`          | Side or front page button.                             |
| Volume Up / menu up     | `button_nav_up`    | `PD2`          | EM4 wake capable.                                      |
| Volume Down / menu down | `button_nav_down`  | `PD3`          | Front navigation.                                      |
| Confirm / select        | `button_select`    | `PD4`          | Front navigation.                                      |
| Back / menu back        | `button_back`      | `PD5`          | EM4 wake capable.                                      |
| Reset                   | hardware`RESETn`   | `RESETn` pin   | Prefer a real reset-line button, not an app GPIO.      |

Wire mechanical buttons as active-low inputs:

```text
GPIO pin ---- button ---- GND
```

Configure each button GPIO as an input with pull-up and debounce in Simplicity
Studio Pin Tool.

Button layout should remain customizable in **[Controls Settings](#363-controls)**.

### Taking A Screenshot

Target CrossPoint-compatible behavior:

- Press **Power** and **Volume Down / Menu Down** at the same time to save a
  screenshot in `screenshots/`.
- Alternatively, while reading a book, press **Confirm / Select** to open the
  reader menu and select **Take screenshot**.

Current project status: screenshot capture is not implemented yet.

### Waveshare 2.7 Inch E-Paper Module

Typical Waveshare SPI e-paper pins:

```text
VCC
GND
DIN / MOSI
CLK / SCK
CS
DC
RST
BUSY
```

Suggested mapping:

| E-paper pin    | Suggested XG24 GPIO | Direction  | Notes                     |
| -------------- | ------------------- | ---------- | ------------------------- |
| `VCC`          | `3V3`               | Power      | Use 3.3 V.                |
| `GND`          | `GND`               | Power      | Common ground.            |
| `DIN` / `MOSI` | `PA1`               | MCU output | SPI MOSI.                 |
| `CLK` / `SCK`  | `PA0`               | MCU output | SPI clock.                |
| `CS`           | `PA2`               | MCU output | Chip select.              |
| `DC`           | `PA4`               | MCU output | Data/command select.      |
| `RST`          | `PB0`               | MCU output | E-paper reset.            |
| `BUSY`         | `PB2`               | MCU input  | Busy status from display. |

Use `USART0` or a software SPI driver for the display. Keep `EUSART0` for
VCOM/debug UART and avoid `EUSART1` pins currently used by the MX25 flash
shutdown configuration.

### Pins To Avoid

These pins are already used or reserved in the current project configuration:

| Pin   | Current use              |
| ----- | ------------------------ |
| `PA3` | SWO debug                |
| `PA5` | VCOM UART TX             |
| `PA6` | VCOM UART RX             |
| `PC0` | MX25 flash shutdown CS   |
| `PC1` | MX25 flash shutdown SCLK |
| `PC2` | MX25 flash shutdown RX   |
| `PC3` | MX25 flash shutdown TX   |
| `PC6` | PTI DOUT                 |
| `PC7` | PTI DFRAME               |
| `PC8` | Board microphone enable  |
| `PC9` | Board sensor enable      |

---

## 2. Power And Startup

### Power On / Off

Target CrossPoint-compatible behavior:

To turn the reader on or off, press and hold the **Power** button for about half
a second. In **[Controls Settings](#363-controls)**, the power button can be
configured to turn the device off with a short press instead of a long press.

To reboot the device after a firmware update or if the device is frozen, press
and release the hardware **Reset** button, then quickly press and hold **Power**
for a few seconds.

s
shutdown behavior.

### First Launch

Target CrossPoint-compatible behavior:

On first launch, the firmware opens the **[Home Screen](#31-home-screen)**.

> [!NOTE]
> On later restarts, the firmware automatically reopens the last book you were
> reading.

Current project status: the checked-in firmware starts BLE advertising and does
not yet show a reader UI.

### Current BLE OTA Startup

The current XG24 firmware performs this startup sequence:

1. The board boots the application.
2. `ota_init()` initializes the Gecko Bootloader interface.
3. The Bluetooth stack starts connectable legacy advertising.
4. The device advertises as `BLE OTA EInk`.
5. A BLE central can connect and access Generic Access, Device Information, and
   Silicon Labs OTA services.

---

## 3. Screens

### 3.1 Home Screen

The Home screen is the main entry point to the firmware. From here you can open
the most recently read book in **[Reading Mode](#4-reading-mode)**, browse the
filesystem, view recent books, open file transfer, or adjust settings.

Target entries:

- **Read current / last book**
- **Browse Files**
- **Recent Books**
- **File Transfer**
- **Settings**

Current project status: not implemented yet.

### 3.2 Reading Mode

See **[Reading Mode](#4-reading-mode)** for page turning, chapter navigation,
bookmarks, footnotes, and reader menu behavior.

Current project status: not implemented yet.

### 3.3 Browse Files Screen

The Browse Files screen acts as a file and folder browser. The full path to the
current directory is shown at the top of the screen. File extensions are shown
beside filenames, directories are shown with brackets such as `[folder-name]`,
and hidden directories beginning with `.` are visible.

- **Navigate List:** Use **Left / Previous Page** or **Volume Up / Menu Up**, and
  **Right / Next Page** or **Volume Down / Menu Down** to move the selection
  cursor through folders and books. Long-press these buttons to scroll a full
  page up or down.
- **Open Selection:** Press **Confirm / Select** to open a folder or start
  reading a selected book. Selecting a `.bmp` file opens the image viewer.
- **Delete Files or Folders:** Hold and release **Confirm / Select** to delete
  the selected file or folder. A confirmation prompt lets you confirm or cancel.
  Multiple files can be selected for deletion in one operation.
- **Rename or Move:** Files can be renamed or moved to a different folder from
  the browse screen.

Current project status: not implemented yet.

### 3.4 Recent Books Screen

The Recent Books screen lists the most recently opened books in chronological
order, displaying title and author.

Current project status: not implemented yet.

### 3.5 File Transfer Screen

The File Transfer screen allows you to upload and manage files on the device.
For the XG24 product, file transfer should use product-supported local
transports such as BLE file transfer, USB, removable storage, or a debug/serial
maintenance tool.

Target XG24 product behavior should preserve these capabilities:

- Upload books, fonts, screenshots, and settings files.
- Download books or screenshots from the device.
- Manage files from a companion app, USB tool, removable storage, or serial
  maintenance tool.
- Keep firmware update separate through the existing BLE OTA path.

Current project status: book/file transfer is not implemented. The current
firmware only supports BLE application OTA firmware upload.

> [!TIP]
> Advanced users should be able to manage files programmatically once a local
> transfer protocol is implemented.

> [!TIP]
> The product should preserve CrossPoint's built-in EPUB Optimizer feature so
> incompatible EPUBs can be cleaned and reprocessed directly on the device.

### 3.5.1 Calibre Transfers

Target CrossPoint-compatible behavior:

The reader supports sending books from Calibre using an XG24-compatible Calibre
plugin through USB, BLE, removable storage, or a companion bridge.

#### Installing The Plugin In Calibre

1. Download the latest `crosspoint_reader` plugin release from the CrossPoint
   Calibre plugin repository.
2. Download the zip file.
3. Open Calibre -> Preferences -> Plugins -> Load plugin from file.
4. Select the zip file.
5. Restart Calibre.

#### Configuring The Plugin In Calibre

1. In Calibre, open Preferences.
2. Open Plugins.
3. Search for `crosspoint`.
4. Click **Customize plugin**.
5. Select the XG24 transfer backend, such as USB, BLE, removable storage, or a
   companion bridge.
6. Leave the other settings unchanged unless needed.
7. Optionally set **Upload path** to a subfolder instead of the root `/`, for
   example `/mybooks`.
8. Restart Calibre.

#### Uploading Books

1. On the device, open File Transfer and select the configured local transfer
   mode.
2. In Calibre, select one or more books.
3. Right-click the selection.
4. Select Send to Device -> Send to main memory.

The plugin transfers to the device, creates a folder for the book author in the
configured upload folder, and copies the book into that folder.

#### Removing A Book

Books cannot be removed from the device through Calibre. Use the device file
browser or local management tool instead.

Current project status: not implemented yet.

### 3.6 Settings

The Settings screen configures display behavior, reading behavior, controls, and
system services.

Current project status: not implemented yet, but the settings below are retained
as product requirements.

#### 3.6.1 Display

- **Sleep Screen:** Which sleep screen to display when the device sleeps:
  - `Dark` (default): Dark CrossPoint-style logo sleep screen.
  - `Light`: The same sleep screen on a white background.
  - `Custom`: Custom images from storage; see **[Sleep Screen](#37-sleep-screen)**.
  - `Cover`: The current book cover image. Experimental.
  - `None`: A blank screen.
  - `Cover + Custom`: Book cover while actively reading, otherwise custom sleep
    screen behavior.
  - `Quick resume`: The last page text is shown on the sleep screen with a moon
    icon. Waking returns to the same page without a full reader reload.

- **Sleep Screen Cover Mode:** How to display the book cover:
  - `Fit` (default): Scale the image to fit centered on screen, with white
    borders as needed.
  - `Crop`: Scale and crop to fill the screen. Experimental.

- **Sleep Screen Cover Filter:** Filter used for book covers:
  - `None` (default): Convert cover to grayscale.
  - `Contrast`: Show as black and white without grayscale conversion.
  - `Inverted`: Show inverted black and white without grayscale conversion.

- **Quick Resume on Timeout:** Enable Quick Resume when the device sleeps due to
  inactivity under System -> Time to Sleep. This overrides Sleep Screen Cover
  Mode when enabled.
- **Status Bar:** Configure the reading status bar:
  - `None`
  - `No Progress`
  - `Full w/ Percentage`
  - `Full w/ Book Bar`
  - `Book Bar Only`
  - `Full w/ Chapter Bar`

- **Hide Battery %:** Configure battery percentage display:
  - `Never` (default)
  - `In Reader`
  - `Always`

- **Refresh Frequency:** Set how often the screen does a full refresh while
  reading to reduce ghosting: every 1, 5, 10, 15, or 30 pages.
- **UI Theme:** Preserve CrossPoint theme support:
  - `Classic`
  - `Lyra`
  - `Lyra Extended`
  - `RoundedRaff`

- **Sunlight Fading Fix:** Preserve the setting for hardware variants that need
  a software mitigation for display fading in direct sunlight:
  - `OFF` (default)
  - `ON`

> [!NOTE]
> A battery charging indicator should be shown on the battery icon whenever the
> device is actively charging.

#### 3.6.2 Reader

- **Reader Font Family:** Choose the font used for reading:
  - `Noto Serif` (default)
  - `Noto Sans`
- **Reader Font Size:** `Small`, `Medium` (default), `Large`, or `X Large`.
- **Reader Line Spacing:** `Tight`, `Normal` (default), or `Wide`.
- **Reader Screen Margin:** Controls Reading Mode margins from 5 to 40 pixels in
  5-pixel increments.
- **Reader Paragraph Alignment:** `Justified` (default), `Left`, `Center`, or
  `Right`.
- **Embedded Style:** Use EPUB embedded HTML/CSS styling: `ON` or `OFF`.
- **Hyphenation:** Hyphenate text in Reading Mode: `ON` or `OFF`.
- **Reading Orientation:**
  - `Portrait` (default)
  - `Landscape CW`
  - `Inverted`
  - `Landscape CCW`
- **Extra Paragraph Spacing:**
  - `ON`: Add vertical space between paragraphs.
  - `OFF`: No extra vertical space; use first-line indentation.
- **Text Anti-Aliasing:** Show smooth gray text edges. This can slow page turns.
- **Images:** Display embedded JPG/PNG images in EPUB files: `ON` (default) or
  `OFF`.
- **Focus Reading:** Bold the first part of each word to create visual fixation
  points, similar to Bionic Reading: `ON` or `OFF` (default).

#### 3.6.3 Controls

- **Remap Front Buttons:** Customize the function of each front/button-panel
  control.
- **Side Button Layout (reader):** Swap side button order from `Prev/Next`
  (default) to `Next/Prev`, or disable them entirely while reading.
- **Long-press Chapter Skip:**
  - `Chapter Skip` (default): Long-press skips to next/previous chapter.
  - `Page Scroll`: Long-press scrolls one page up/down.
- **Long-press Menu:** Selects the function bound to holding
  **Confirm / Select** while reading an EPUB. This cycles through available
  functions when the setting is selected:
  - `Bookmark` (default): Hold Confirm for about 0.4 seconds to create a
    bookmark at the current page.
  - `KOSync`: Hold Confirm for about 1 second to launch KOReader Sync directly.
  - `Disabled`: Long-press is ignored; short-press still opens the Reader Menu.
- **Short Power Button Click:**
  - `Ignore` (default): Require long press to turn off the device.
  - `Sleep`: Short press puts the device into sleep mode.
  - `Page Turn`: Short press in reading mode turns to the next page; long press
    turns the device off.
  - `Footnotes`: Short press opens the footnotes submenu in reading mode. If one
    footnote is present, it opens directly. The short power press can select the
    footnote and return to the original page.
  - `Refresh`: Short press triggers a manual full-screen refresh.
- **Quick-return from footnotes:** When active, short-pressing Power acts as Back
  from a footnote page.

#### 3.6.4 System

- **Time to Sleep:** Inactivity timeout before sleep: 1, 3, 5, 10 (default), 15,
  or 30 minutes.
- **Transfer Mode:** Configure local file transfer, such as BLE file transfer,
  USB, removable storage, or serial maintenance mode.
- **KOReader Sync:** Configure syncing book progress.
- **OPDS Servers:** Manage OPDS libraries for browsing and downloading books.
- **Clear Reading Cache:** Clear the internal reading cache.
- **Check for updates:** Check for firmware updates. On XG24, BLE OTA is already
  implemented as the current firmware update path; future UI should expose it
  from the product settings flow.
- **Language:** Preserve CrossPoint's language target: English, Spanish, French,
  German, Czech, Brazilian Portuguese, Russian, Swedish, Romanian, Catalan,
  Ukrainian, Belarusian, Italian, Polish, Finnish, Danish, Dutch, Turkish,
  Kazakh, Hungarian, Lithuanian, Slovenian, Valencian, and Hebrew.
- **Manage Fonts:** Browse, download, and manage custom font families installed
  from storage.

#### 3.6.5 OPDS Servers (Multiple Libraries)

The product should support saving multiple OPDS servers and switching between
them when browsing catalogs.

1. Open Settings -> System -> OPDS Servers.
2. Select **Add Server** to create a new entry, or select an existing server to
   edit it.
3. Configure these fields:
   - **Server Name:** Optional display name, such as `Home Calibre` or
     `Public Catalog`.
   - **OPDS Server URL:** Full catalog root URL. For Calibre Content Server,
     this usually ends with `/opds`.
   - **Username / Password:** Optional credentials for authenticated servers.
4. Use **Delete Server** inside a server entry to remove it.

Behavior notes:

- Store up to 8 OPDS servers.
- OPDS authentication supports HTTP Basic auth. If using Calibre Content Server
  with authentication enabled, set it to Basic rather than Digest.

Local management target:

1. Connect through the product-supported transfer tool.
2. Open the settings management view.
3. Use the **OPDS Servers** card or equivalent screen to add, edit, or delete
   entries.

Current project status: not implemented yet.

#### 3.6.6 Local Settings Management

While in File Transfer mode, the local management tool should include settings
for transfer configuration and **OPDS Servers**.

1. On device, open File Transfer and select the configured local transfer mode.
2. In the companion app, USB tool, removable-storage workflow, or serial
   maintenance tool, open settings management.
3. In transfer settings, configure the selected local transfer backend.
4. In OPDS Servers, add, edit, or delete OPDS catalogs.

Behavior notes:

- Passwords are never shown back in the management UI after saving.
- Leaving Password blank while editing keeps the existing saved password.

Current project status: not implemented yet.

#### 3.6.7 KOReader Sync Quick Setup

The product should sync reading progress with KOReader-compatible sync servers
and interoperate with KOReader apps/devices when they use the same server and
credentials.

##### Option A: Free Public Server (`sync.koreader.rocks`)

1. Register a user once, only if needed:

```bash
USERNAME="user"
PASSWORD="pass"
PASSWORD_MD5="$(printf '%s' "$PASSWORD" | openssl md5 | awk '{print $2}')"

curl -i "https://sync.koreader.rocks/users/create" \
  -H "Accept: application/vnd.koreader.v1+json" \
  -H "Content-Type: application/json" \
  --data "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD_MD5\"}"
```

Already have KOReader Sync credentials? Skip registration. Basic sync only
requires the same username/password on all devices.

When this returns `HTTP 402` with `{"code":2002,"message":"Username is already registered."}`,
pick a different username or use that existing account.

2. On each reader:
   - Go to Settings -> System -> KOReader Sync.
   - Set Username and Password. Enter the plain password; the firmware computes
     MD5 internally.
   - Set Sync Server URL to `https://sync.koreader.rocks`, or leave it empty if
     that is the default.
   - Run **Authenticate**.
3. While reading, press **Confirm / Select** to open the reader menu, then select
   **Sync Progress**.
   - Choose **Apply Remote** to jump to remote progress.
   - Choose **Upload Local** to push current progress.

##### Option B: Self-Hosted Server (Docker Compose)

1. Start a sync server:

```bash
mkdir -p kosync-quickstart
cd kosync-quickstart

cat > compose.yaml <<'YAML'
services:
  kosync:
    image: koreader/kosync:latest
    ports:
      - "7200:7200"
      - "17200:17200"
    volumes:
      - ./data/redis:/var/lib/redis
    environment:
      - ENABLE_USER_REGISTRATION=true
    restart: unless-stopped
YAML

docker compose up -d
```

> [!NOTE]
> `ENABLE_USER_REGISTRATION=true` is convenient for first setup. After creating
> users, set it to `false` or remove it to avoid unexpected registrations.

2. Verify the server:

```bash
curl -H "Accept: application/vnd.koreader.v1+json" "http://<server-ip>:17200/healthcheck"
# Expected: {"state":"OK"}
```

3. Register a user once using the MD5 of your password:

> [!WARNING]
> Sending a reusable MD5-derived password over plain HTTP is insecure. Create
> unique sync-only credentials and do not reuse main account passwords. Prefer
> HTTPS whenever traffic leaves a trusted LAN.

```bash
USERNAME="user"
PASSWORD="pass"
PASSWORD_MD5="$(printf '%s' "$PASSWORD" | openssl md5 | awk '{print $2}')"

curl -i "http://<server-ip>:17200/users/create" \
  -H "Accept: application/vnd.koreader.v1+json" \
  -H "Content-Type: application/json" \
  --data "{\"username\":\"$USERNAME\",\"password\":\"$PASSWORD_MD5\"}"
```

4. On each reader:
   - Go to Settings -> System -> KOReader Sync.
   - Set Username and Password.
   - Set Sync Server URL to `http://<server-ip>:17200`, or
     `https://<server-ip>:7200` for the HTTPS listener.
   - Run **Authenticate**.
5. While reading, open Reader Menu -> Sync Progress.
   - Choose **Apply Remote** to pull progress.
   - Choose **Upload Local** to push progress.

Current project status: not implemented yet.

### 3.7 Sleep Screen

The **Sleep Screen** setting controls what appears when the device sleeps.

| Mode               | Behavior                                                                            |
| ------------------ | ----------------------------------------------------------------------------------- |
| **Dark** (default) | CrossPoint-style logo on a dark background.                                         |
| **Light**          | CrossPoint-style logo on a white background.                                        |
| **Custom**         | Custom image from storage. Falls back to**Dark** if no custom image is found.       |
| **Cover**          | Cover of the currently open book. Falls back to**Dark** if no book is open.         |
| **Cover + Custom** | Cover of the currently open book while actively reading; otherwise custom behavior. |
| **Quick resume**   | Text of the last read page with a sleep icon; waking resumes the same page.         |
| **None**           | Blank screen.                                                                       |

#### Cover Settings

When using **Cover** or **Cover + Custom**, two additional settings apply:

- **Sleep Screen Cover Mode:** **Fit** or **Crop**.
- **Sleep Screen Cover Filter:** **None**, **Contrast**, or **Inverted**.

#### Custom Images

To use custom sleep images, set the sleep screen mode to **Custom** or
**Cover + Custom**, then place images on storage:

- **Multiple Images (recommended):** Create a `.sleep` directory in the root of
  storage and place `.bmp` images inside. One is randomly selected each time the
  device sleeps. A directory named `sleep` is accepted as a fallback.
- **Single Image:** Place `sleep.bmp` in the root directory. This is used as a
  fallback if no valid images are found in `.sleep` or `sleep`.

> [!TIP]
> For best results, use uncompressed 24-bit BMP files at the final e-paper panel
> resolution. For a Waveshare 2.7 inch panel, verify the exact pixel resolution
> for the selected module before preparing production images.

> [!TIP]
> The BMP image viewer should allow setting an image as the sleep screen from
> **[Browse Files](#33-browse-files-screen)**.

Current project status: not implemented yet.

### 3.8 Custom Fonts

The product should support loading additional fonts from storage, extending
beyond the built-in Noto Serif and Noto Sans families. Custom fonts enable CJK
and other scripts that are not covered by the built-in reader fonts.

Install paths to preserve:

1. **Download from device:** Settings -> System -> Manage Fonts.
2. **Upload via local management tool:** File Transfer -> Fonts tab.
3. **Manual storage copy:** Copy font packages to `/.fonts/` or `/fonts/`.

Once installed, custom fonts appear in Settings -> Reader -> Font Family.

Current project status: not implemented yet.

---

## 4. Reading Mode

Once a book is open, the button layout changes to facilitate reading.

### Page Turning

| Action            | Buttons                                                   |
| ----------------- | --------------------------------------------------------- |
| **Previous Page** | Press**Left / Previous Page** or **Volume Up / Menu Up**  |
| **Next Page**     | Press**Right / Next Page** or **Volume Down / Menu Down** |

The role of side buttons can be swapped in **[Controls Settings](#363-controls)**.

If **Short Power Button Click** is set to `Page Turn`, briefly pressing Power in
reading mode also turns to the next page.

### Chapter Navigation

- **Next Chapter:** Press and hold **Right / Next Page** or
  **Volume Down / Menu Down**, then release.
- **Previous Chapter:** Press and hold **Left / Previous Page** or
  **Volume Up / Menu Up**, then release.

This feature can be disabled in **[Controls Settings](#363-controls)** to avoid
accidental chapter jumps.

### Auto Page Turn

Auto Page Turn advances pages at a set interval for hands-free reading. Enable
and configure it from the **[Reader Menu](#5-reader-menu)** while reading an
EPUB.

### Tilt Page Turn

CrossPoint supports tilt page turn on Xteink X3 hardware. For the XG24 dev kit,
this should remain an optional product feature only if a suitable motion sensor
is present in the final hardware design.

### Footnote Navigation

When reading an EPUB with footnotes, select the footnote reference to navigate to
the footnote text. From the footnote, return to the original reading position.

If the device sleeps or the book closes while viewing a footnote, it should
reopen to the original reading position rather than the footnote.

### System Navigation

- **Return to Home:** Press **Back** to close the book and return to the
  **[Home Screen](#31-home-screen)**.
- **Return to Browse Files:** Press and hold **Back** to close the book and
  return to **[Browse Files](#33-browse-files-screen)**.
- **Reader Menu:** Press **Confirm / Select** to open the
  **[Reader Menu](#5-reader-menu)**.
- **Long-press Confirm / Select:** Runs the configured **Long-press Menu**
  action: `Bookmark`, `KOSync`, or `Disabled`.

### Supported Languages

The reader should preserve CrossPoint's text rendering coverage:

- **Latin Script:** English, German, French, Spanish, Portuguese, Italian, Dutch,
  Swedish, Norwegian, Danish, Finnish, Polish, Czech, Hungarian, Romanian,
  Slovak, Slovenian, Turkish, Catalan, and others.
- **Cyrillic Script:** Russian, Ukrainian, Belarusian, Bulgarian, Serbian,
  Macedonian, Kazakh, Kyrgyz, Mongolian, and others.
- **Vietnamese:** Supported through extended Latin glyph coverage in built-in
  reader fonts.

Built-in fonts do not fully cover Chinese, Japanese, Korean, Arabic, Greek,
Hebrew, and Farsi. These should be enabled by installing custom fonts; see
**[Custom Fonts](#38-custom-fonts)**.

Current project status: reading mode is not implemented yet.

---

## 5. Reader Menu

Press **Confirm / Select** while reading to open the Reader Menu. From here you
can access reading utilities and navigation options without leaving the book.

Target menu options:

- **Select Chapter:** Open the table of contents to jump to a chapter.
- **Footnotes:** Navigate to footnotes for the current section. Shown only when
  the book contains footnotes.
- **Reading Orientation:** Cycle through screen orientations.
- **Auto Turn (Pages Per Minute):** Cycle through automatic page turn speeds.
- **Go to %:** Jump to a specific position in the book.
- **Take screenshot:** Save a screenshot of the current page to `screenshots/`.
- **Show page as QR:** Display a QR code encoding the current reading position.
- **Go Home:** Close the book and return to Home.
- **Sync Progress:** Push or pull progress with a KOReader sync server.
- **Delete Book Cache:** Clear cached layout data for the current book and force
  re-indexing on next open.

Press **Back** at any time to close the menu and return to the current page.

### 5.1 Chapter Selection

Accessible by selecting **Chapters** from the Reader Menu.

1. Use **Left / Previous Page**, **Right / Next Page**, **Volume Up / Menu Up**,
   or **Volume Down / Menu Down** to highlight a chapter.
2. Press **Confirm / Select** to jump to that chapter.
3. Press **Back** to cancel and return to the current page.

### 5.2 Bookmarks

Bookmarks quickly save and restore your place in a book.

To create a bookmark, hold **Confirm / Select** for about half a second while
inside a book. A popup confirms the bookmark and disappears after a few seconds.

To open bookmarks, press **Confirm / Select** inside a book, then navigate to the
**Bookmarks** menu. Select a bookmark and press **Confirm / Select** to jump to
that location.

To delete a bookmark, hold **Confirm / Select** for about 0.7 seconds on the
bookmark entry, then press **Confirm / Select** again to confirm or **Back** to
cancel.

Bookmarks should be stored in `.crosspoint/bookmarks` in JSON format, or in an
XG24 product-equivalent path if the final storage layout changes.

Current project status: reader menu and bookmarks are not implemented yet.

---

## 6. BLE OTA Firmware Update

The current XG24 project already implements application-level BLE OTA update.
This should remain part of the product because it satisfies the CrossPoint
requirement for firmware updates without opening the device.

### BLE Services

| Service            | Purpose                                                                   |
| ------------------ | ------------------------------------------------------------------------- |
| Generic Access     | Device name and appearance.                                               |
| Device Information | Manufacturer, model, hardware, firmware, and system ID fields.            |
| Silicon Labs OTA   | Application firmware upload through OTA control and data characteristics. |

Silicon Labs OTA details:

| Item                    | Value                                  |
| ----------------------- | -------------------------------------- |
| Service UUID            | `1D14D6EE-FD63-4FA1-BFA4-8F47B42119F0` |
| Control characteristic  | `gattdb_ota_control`                   |
| Data characteristic     | `gattdb_ota_data`                      |
| Firmware start command  | `0x00`                                 |
| Firmware end command    | `0x03`                                 |
| Bootloader storage slot | `0`                                    |

### OTA Flow

1. A BLE client connects to `BLE OTA EInk`.
2. The client writes `0x00` to the OTA control characteristic.
3. The firmware resets the OTA write position, reads bootloader slot 0, and
   erases the slot if needed.
4. The client writes firmware bytes to the OTA data characteristic.
5. The client writes `0x03` to the OTA control characteristic.
6. The firmware verifies the image asynchronously.
7. If verification succeeds, the firmware closes the BLE connection.
8. On disconnect, the firmware marks slot 0 as the bootload image and reboots
   into Gecko Bootloader install.

### Testing With Simplicity Connect

1. Flash a compatible Gecko Bootloader with storage slot 0.
2. Build and flash this application as `.hex` or `.s37`.
3. Open Simplicity Connect on a phone or desktop.
4. Scan for `BLE OTA EInk`.
5. Connect and confirm the Generic Access, Device Information, and Silicon Labs
   OTA services are visible.
6. Start an OTA update using a compatible signed/packaged application image for
   the installed bootloader.

Do not flash a raw `.bin` file unless you know the flash layout. It can
overwrite bootloader regions.

---

## 7. Build And Flash

1. Open `BLE_OTA_EInk.slcp` in Simplicity Studio.
2. Let Simplicity Studio generate or update project files if prompted.
3. Confirm a compatible Gecko Bootloader with storage slot 0 is flashed to the
   target.
4. Build the project with the `GNU ARM v12.2.1 - Default` configuration.
5. Flash the generated `.hex` or `.s37` image to the XG24 dev kit.
6. Reset the board and scan for `BLE OTA EInk`.

---

## 8. Implementation Roadmap

This roadmap keeps every CrossPoint firmware feature in scope while sequencing
bring-up for the XG24 dev kit.

### 8.1 Hardware Bring-Up

- Configure the seven simple-button GPIOs in Simplicity Studio Pin Tool.
- Confirm whether reset uses the real hardware `RESETn` button or an app-level
  soft-reset GPIO.
- Verify the exact Waveshare 2.7 inch module variant and connector pin order.
- Verify selected GPIOs are available on the dev kit expansion header.
- Verify the e-paper module is 3.3 V compatible and does not require 5 V logic.
- Verify whether `PA0`, `PA1`, `PA2`, and `PA4` can be routed together cleanly
  for the selected SPI peripheral.

### 8.2 Display And Input

1. Implement e-paper reset, busy wait, command write, data write, and full
   refresh.
2. Add a smoke test: clear screen, draw text, refresh.
3. Add button event logging.
4. Add the reader input state machine for page prev/next, navigation, select,
   back, and power.
5. Add sleep/wake behavior for `button_power`.

### 8.3 Reader Product Features

1. Decide storage path for books: internal flash, external flash, SD card, BLE
   transfer, USB mass storage, or another product storage path.
2. Implement storage/filesystem abstraction.
3. Implement Home, Browse Files, Recent Books, File Transfer, and Settings.
4. Implement BMP image viewer and screenshot capture.
5. Implement EPUB parsing, layout, cache, and image handling.
6. Implement Reading Mode, Reader Menu, bookmarks, footnotes, chapter selection,
   go-to-percent, auto page turn, and orientation support.
7. Implement settings persistence.
8. Implement sleep screens, quick resume, and cover rendering.
9. Implement custom fonts and language support.
10. Implement OPDS, Calibre transfer, local settings management, and KOReader
    Sync if the final product includes the required local or companion transport.
11. Integrate BLE OTA into product settings/update UI.

---

## 9. Current Limitations And Roadmap

Current checked-in XG24 firmware limitations:

- E-paper display application behavior is not implemented in `app.c`.
- Button instances exist, but their GPIO pins are not configured yet.
- Reader UI, EPUB rendering, storage, screenshots, file transfer, OPDS, Calibre,
  custom fonts, and KOReader Sync are not implemented yet.
- A compatible Gecko Bootloader with storage slot 0 must be flashed before BLE
  OTA updates can work.

CrossPoint reader limitations to preserve/document once the reader is ported:

- **Cover Images:** Large embedded EPUB cover images can take several seconds to
  convert for sleep screen and home screen thumbnails. Consider optimizing EPUBs
  before transfer.
- **Unsupported Image Formats:** Most JPG and PNG images in EPUBs should render.
  GIFs and progressive JPEGs are not supported and should fall back to an
  `[Image]` placeholder.
- **Dictionary Lookup:** Inline word lookup is not yet implemented.

---

## 10. Troubleshooting Issues And Escaping Bootloop

### Device Does Not Advertise

- Confirm the application was flashed successfully.
- Confirm the board is powered and out of reset.
- Open the VCOM serial log and check for startup assertions.
- Confirm the Bluetooth stack boot event is reached in `app.c`.

### OTA Fails At Start

- Confirm a Gecko Bootloader is installed.
- Confirm bootloader storage slot 0 exists.
- Confirm slot 0 is large enough for the application image.
- Check logs for `Unable to get storage slot info`.

### OTA Upload Finishes But Does Not Install

- Confirm the uploaded image is compatible with the installed bootloader.
- Check logs for `application verification failed`.
- Make sure the BLE connection closes after successful verification; install is
  triggered from the connection-closed event.

### Buttons Do Not Work

- This is expected until GPIO pins are configured and button event handling is
  added to the application.
- Check each `config/sl_simple_button_button_*_config.h` file for unresolved
  `Simple Button GPIO pin not configured` warnings.

### E-Paper Display Does Not Update

- The e-paper display driver is not implemented yet.
- Verify power, ground, SPI pins, `DC`, `RST`, and `BUSY`.
- Run a minimal display smoke test before debugging the full reader UI.

### Reader Crash Reports

Target CrossPoint-compatible behavior:

After a crash, the firmware saves a crash report to storage. Include the crash
log with bug reports.

For deeper debugging, connect to VCOM/serial logs. A future XG24 debug monitor
should preserve the CrossPoint monitor features:

- Color-coded log output by category.
- Live memory usage reporting.
- Interactive command prompt.
- Screenshot capture from the connected device.
- Filtering and suppression of noisy log lines.

### Escaping A Bootloop

Target CrossPoint-compatible behavior:

If the device is stuck in a bootloop, press and release **Reset**, then hold the
configured **Back** button and **Power** button to boot to the Home Screen.

If the bootloop is caused by broken cache or configuration, delete the
`.crosspoint` directory from storage, or delete only `settings.json`,
`state.json`, or `epub_*` cache directories inside `.crosspoint/`.

Current project status: bootloop recovery shortcut and reader cache handling are
not implemented yet.
